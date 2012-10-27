/*
 * Command line file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spUtilityCommandLine.hpp"

#ifdef SP_COMPILE_WITH_COMMANDLINE


#include "Base/spSharedObjects.hpp"
#include "Base/spInputOutputControl.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spRenderContext.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Platform/spSoftPixelDevice.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;
extern io::InputControl* __spInputControl;

namespace tool
{


const s32 CommandLineUI::TEXT_DISTANCE = 4;

CommandLineUI::CommandLineUI() :
    BgColor_(0, 0, 0, 255),
    Rect_(
        0,
        0,
        gSharedObjects.ScreenWidth,
        gSharedObjects.ScreenHeight/2
    ),
    MaxLines_(0)
{
    if (!__spVideoDriver)
        throw io::stringc("Render system has not been created yet");
    
    /* Create default font object */
    ActiveFont_ = OrigFont_ = __spVideoDriver->createFont(
        "courier new", 15, __spVideoDriver->getRendererType() == video::RENDERER_DIRECT3D9 ? video::FONT_BITMAP : 0
    );
    clear();
}
CommandLineUI::~CommandLineUI()
{
    __spVideoDriver->deleteFont(OrigFont_);
}

void CommandLineUI::draw()
{
    drawBackground();
    drawTextLines();
    drawCursor();
}

void CommandLineUI::updateInput()
{
    Cursor_.update();
    
    /* Get enterd characters */
    const io::stringc CharList(__spDevice->getUserCharList());
    
    if (__spInputControl->keyHit(io::KEY_RETURN))
    {
        /* Execute current command and push to command history */
        execute(CommandLine_);
        pushMemo();
        CommandLine_ = "";
    }
    else if (CharList.size())
    {
        /* Add/remove characters to current command */
        if (CharList[0] == '\b')
        {
            if (CommandLine_.size())
                CommandLine_ = CommandLine_.left(CommandLine_.size() - 1);
        }
        else
            CommandLine_ += CharList;
    }
}

void CommandLineUI::message(const io::stringc &Message, const video::color &Color)
{
    TextLines_.push_back(STextLine(Message, Color));
    if (MaxLines_ > 0 && TextLines_.size() > MaxLines_)
        TextLines_.erase(TextLines_.begin());
}

void CommandLineUI::warning(const io::stringc &Message)
{
    message("Warning: " + Message + "!", video::color(255, 255, 0));
}
void CommandLineUI::error(const io::stringc &Message)
{
    message("Error: " + Message + "!", video::color(255, 0, 0));
}

void CommandLineUI::setupCursorTimer(u64 IntervalDuration)
{
    Cursor_.IntervalTimer.start(IntervalDuration);
}

bool CommandLineUI::execute(const io::stringc &Command)
{
    /* Check for empty command */
    if (!Command.size())
    {
        blank();
        return false;
    }
    
    message(Command);
    
    if (!executeCommand(Command))
    {
        /* Print error message for unknown command */
        unknown(Command);
        return false;
    }
    
    return true;
}

void CommandLineUI::clear(bool isHelpInfo)
{
    TextLines_.clear();
    if (isHelpInfo)
        confirm("Enter \"help\" for information");
}


/*
 * ======= Protected: =======
 */

void CommandLineUI::drawBackground()
{
    __spVideoDriver->draw2DRectangle(Rect_, BgColor_);
}

void CommandLineUI::drawTextLines()
{
    const s32 FontHeight = ActiveFont_->getSize().Height;
    
    /* Draw output text lines */
    s32 PosVert = Rect_.Bottom - CommandLineUI::TEXT_DISTANCE*3 - FontHeight*2;
    
    for (u32 i = TextLines_.size(); i > 0 && PosVert > -FontHeight; --i)
    {
        drawTextLine(PosVert, TextLines_[i - 1]);
        PosVert -= (FontHeight + CommandLineUI::TEXT_DISTANCE);
    }
    
    /* Draw input text line */
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect_.Left, Rect_.Bottom - ActiveFont_->getSize().Height - CommandLineUI::TEXT_DISTANCE*2),
        dim::point2di(Rect_.Right, Rect_.Bottom - ActiveFont_->getSize().Height - CommandLineUI::TEXT_DISTANCE*2),
        255
    );
    
    drawTextLine(Rect_.Bottom - CommandLineUI::TEXT_DISTANCE - FontHeight, CommandLine_);
}

void CommandLineUI::drawCursor()
{
    if (Cursor_.isVisible)
    {
        const dim::point2di Pos(
            Rect_.Left + CommandLineUI::TEXT_DISTANCE + ActiveFont_->getStringWidth(CommandLine_),
            Rect_.Bottom - CommandLineUI::TEXT_DISTANCE - ActiveFont_->getSize().Height
        );
        __spVideoDriver->draw2DRectangle(
            dim::rect2di(Pos.X, Pos.Y, Pos.X + ActiveFont_->getSize().Height/2, Pos.Y + ActiveFont_->getSize().Height), 255
        );
    }
}

void CommandLineUI::drawTextLine(s32 PosVert, const STextLine &Line)
{
    __spVideoDriver->draw2DText(
        ActiveFont_, dim::point2di(Rect_.Left + CommandLineUI::TEXT_DISTANCE, Rect_.Top + PosVert), Line.Text, Line.Color
    );
}

void CommandLineUI::unknown(const io::stringc &Command)
{
    error("Unknown command: \"" + Command + "\"");
}

void CommandLineUI::confirm(const io::stringc &Output)
{
    message(" > " + Output, video::color(0, 255, 0));
}

bool CommandLineUI::executeCommand(const io::stringc &Command)
{
    /* Check for default commands */
    if (Command == "help")
        return cmdHelp();
    if (Command == "clear")
    {
        clear();
        return true;
    }
    if (Command == "solid")
        return cmdWireframe(video::WIREFRAME_SOLID);
    if (Command == "lines")
        return cmdWireframe(video::WIREFRAME_LINES);
    if (Command == "points")
        return cmdWireframe(video::WIREFRAME_POINTS);
    if (Command == "fullscreen")
        return cmdFullscreen();
    if (Command == "cam pos")
        return cmdPrintCameraPosition();
    if (Command == "cam rot")
        return cmdPrintCameraRotation();
    
    return false;
}


/*
 * ======= Private: =======
 */

bool CommandLineUI::cmdHelp()
{
    confirm("help ......... Prints this help document.");
    confirm("clear ........ Clears the console content.");
    confirm("solid ........ Switches the active scene-graph wireframe-mode to solid.");
    confirm("lines ........ Switches the active scene-graph wireframe-mode to lines.");
    confirm("points ....... Switches the active scene-graph wireframe-mode to points.");
    confirm("fullscreen ... Toggles the fullscreen mode.");
    confirm("cam pos ...... Prints the global active camera position.");
    confirm("cam rot ...... Prints the global active camera rotation.");
    return true;
}

bool CommandLineUI::cmdWireframe(const video::EWireframeTypes Type)
{
    if (__spSceneManager)
    {
        __spSceneManager->setWireframe(Type);
        confirm("switched wireframe mode");
        return true;
    }
    return false;
}

bool CommandLineUI::cmdFullscreen()
{
    video::RenderContext* ActiveContext = video::RenderContext::getActiveRenderContext();
    
    if (ActiveContext)
    {
        ActiveContext->setFullscreen(!ActiveContext->getFullscreen());
        confirm("switched fullscreen mode");
        return true;
    }
    
    return false;
}

bool CommandLineUI::cmdPrintCameraPosition()
{
    if (__spSceneManager && __spSceneManager->getActiveCamera())
    {
        const dim::vector3df Pos(__spSceneManager->getActiveCamera()->getPosition(true));
        
        confirm(
            "Camera Position = ( " +
            io::stringc::numberFloat(Pos.X, 1, true) + " , " +
            io::stringc::numberFloat(Pos.Y, 1, true) + " , " +
            io::stringc::numberFloat(Pos.Z, 1, true) + " )"
        );
        
        return true;
    }
    return false;
}

bool CommandLineUI::cmdPrintCameraRotation()
{
    if (__spSceneManager && __spSceneManager->getActiveCamera())
    {
        const dim::vector3df Rot(__spSceneManager->getActiveCamera()->getRotation(true));
        
        confirm(
            "Camera Rotation = ( " +
            io::stringc::numberFloat(Rot.X, 1, true) + " , " +
            io::stringc::numberFloat(Rot.Y, 1, true) + " , " +
            io::stringc::numberFloat(Rot.Z, 1, true) + " )"
        );
        
        return true;
    }
    return false;
}

void CommandLineUI::pushMemo()
{
    //todo
}

void CommandLineUI::popMemo()
{
    //todo
}


/*
 * STextLine structure
 */

CommandLineUI::STextLine::STextLine()
{
}
CommandLineUI::STextLine::STextLine(
    const io::stringc &LineText, const video::color &LineColor) :
    Text    (LineText   ),
    Color   (LineColor  )
{
}
CommandLineUI::STextLine::~STextLine()
{
}


/*
 * SCursor structure
 */

CommandLineUI::SCursor::SCursor() :
    IntervalTimer   (500ul  ),
    isVisible       (true   )
{
}
CommandLineUI::SCursor::~SCursor()
{
}

void CommandLineUI::SCursor::update()
{
    /* Toogle cursor visibility */
    if (IntervalTimer.finish())
    {
        IntervalTimer.reset();
        isVisible = !isVisible;
    }
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
