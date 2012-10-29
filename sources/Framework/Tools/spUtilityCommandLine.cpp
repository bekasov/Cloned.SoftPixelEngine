/*
 * Command line file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spUtilityCommandLine.hpp"

//#ifdef SP_COMPILE_WITH_COMMANDLINE


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


const s32 CommandLineUI::TEXT_DISTANCE      = 4;
const s32 CommandLineUI::SCROLLBAR_DISTANCE = 5;
const s32 CommandLineUI::SCROLLBAR_WIDTH    = 7;

CommandLineUI::CommandLineUI() :
    BgColor_(0, 0, 0, 255),
    Rect_(
        0,
        0,
        gSharedObjects.ScreenWidth,
        gSharedObjects.ScreenHeight/2
    ),
    MaxLines_       (0),
    Scroll_         (0),
    ActiveFont_     (0),
    OrigFont_       (0),
    TextLineHeight_ (0)
{
    if (!__spVideoDriver)
        throw io::stringc("Render system has not been created yet");
    if (!__spInputControl)
        throw io::stringc("Input controller has not been created yet");
    
    /* Create default font object */
    OrigFont_ = __spVideoDriver->createFont(
        "courier new", 15, __spVideoDriver->getRendererType() == video::RENDERER_DIRECT3D9 ? video::FONT_BITMAP : 0
    );
    setFont(0);
    clear();
    
    __spInputControl->setWordInput(true);
}
CommandLineUI::~CommandLineUI()
{
    __spVideoDriver->deleteFont(OrigFont_);
}

void CommandLineUI::render(s32 Flags)
{
    __spVideoDriver->beginDrawing2D();
    {
        draw(Flags);
        updateInput(Flags);
    }
    __spVideoDriver->endDrawing2D();
}

void CommandLineUI::draw(s32 Flags)
{
    /* Draw background, text lines, cursor and optionally scrollbar */
    drawBackground();
    drawTextLines();
    drawCursor();
    
    if ((Flags & CMDFLAG_SCROLL) != 0)
        drawScrollbar();
}

void CommandLineUI::updateInput(s32 Flags)
{
    /* Update cursor blinking */
    Cursor_.update();
    
    /* Update scrolling input */
    if ((Flags & CMDFLAG_SCROLL) != 0)
        updateScrollInput();
    
    /* Update memento input */
    if ((Flags & CMDFLAG_MEMENTO) != 0)
    {
        if (__spInputControl->keyHit(io::KEY_UP))
            Memento_.up(__spInputControl->getEnteredWord());
        if (__spInputControl->keyHit(io::KEY_DOWN))
            Memento_.down(__spInputControl->getEnteredWord());
    }
    
    /* Get enterd characters */
    CommandLine_ = __spInputControl->getEnteredWord();
    
    if (__spInputControl->keyHit(io::KEY_RETURN))
    {
        /* Execute current command and push to command history */
        if (execute(CommandLine_))
        {
            if ((Flags & CMDFLAG_MEMENTO) != 0)
                Memento_.push(CommandLine_);
        }
        
        __spInputControl->getEnteredWord().clear();
    }
}

void CommandLineUI::updateScrollInput(s32 DefaultScrollSpeed)
{
    /* Update default scrolling input */
    if (__spInputControl->getMouseWheel())
        scroll(__spInputControl->getMouseWheel()*DefaultScrollSpeed);
    if (__spInputControl->keyHit(io::KEY_PAGEUP))
        scrollPage(1);
    if (__spInputControl->keyHit(io::KEY_PAGEDOWN))
        scrollPage(-1);
    if (__spInputControl->keyHit(io::KEY_HOME))
        scrollEnd();
    if (__spInputControl->keyHit(io::KEY_END))
        scrollStart();
}

void CommandLineUI::message(const io::stringc &Message, const video::color &Color)
{
    /* Add new message */
    TextLines_.push_back(STextLine(Message, Color));
    
    /* Pop old messages */
    if (MaxLines_ > 0 && TextLines_.size() > MaxLines_)
        TextLines_.erase(TextLines_.begin());
    
    /* Update scrolling */
    if (Scroll_ > 0)
        scroll(1);
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

bool CommandLineUI::isScrollingEnabled() const
{
    /* Get text height and visible height to determine whether scrolling is enabled or not */
    s32 TextHeight = 0, VisibleHeight = 0;
    getScrollingRange(TextHeight, VisibleHeight);
    return TextHeight >= VisibleHeight;
}

bool CommandLineUI::scroll(s32 Direction)
{
    if (Scroll_ <= 0 && Direction < 0)
        return false;
    
    /* Get text height and visible height to determine whether scrolling is enabled or not */
    s32 TextHeight = 0, VisibleHeight = 0;
    getScrollingRange(TextHeight, VisibleHeight);
    
    if (TextHeight < VisibleHeight)
        return false;
    
    /* Get scrolling range */
    const s32 PrevScroll = Scroll_;
    
    Scroll_ += Direction;
    clampScrolling(TextHeight, VisibleHeight);
    
    return PrevScroll != Scroll_;
}

bool CommandLineUI::scrollPage(s32 Direction)
{
    return scroll(Direction * getScrollPage());
}

void CommandLineUI::scrollStart()
{
    Scroll_ = 0;
}
void CommandLineUI::scrollEnd()
{
    Scroll_ = getMaxScrollPosition();
}

void CommandLineUI::setScrollPosition(s32 Pos)
{
    Scroll_ = Pos;
    clampScrolling();
}

s32 CommandLineUI::getMaxScrollPosition() const
{
    s32 TextHeight = 0, VisibleHeight = 0;
    getScrollingRange(TextHeight, VisibleHeight);
    return getMaxScrollPosition(TextHeight, VisibleHeight);
}

s32 CommandLineUI::getScrollPage() const
{
    s32 TextHeight = 0, VisibleHeight = 0;
    getScrollingRange(TextHeight, VisibleHeight);
    return VisibleHeight / TextLineHeight_;
}

void CommandLineUI::setFont(video::Font* FontObj)
{
    /* Setup new font and update text line height */
    ActiveFont_     = (FontObj ? FontObj : OrigFont_);
    TextLineHeight_ = ActiveFont_->getSize().Height + CommandLineUI::TEXT_DISTANCE;
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
    
    for (s32 i = static_cast<s32>(TextLines_.size()) - Scroll_; i > 0 && PosVert > -FontHeight; --i)
    {
        drawTextLine(PosVert, TextLines_[i - 1]);
        PosVert -= TextLineHeight_;
    }
    
    /* Draw input separation line */
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect_.Left, Rect_.Bottom - TextLineHeight_ - CommandLineUI::TEXT_DISTANCE),
        dim::point2di(Rect_.Right, Rect_.Bottom - TextLineHeight_ - CommandLineUI::TEXT_DISTANCE),
        FgColor_
    );
    
    drawTextLine(Rect_.Bottom - CommandLineUI::TEXT_DISTANCE - FontHeight, CommandLine_);
}

void CommandLineUI::drawCursor()
{
    if (Cursor_.isVisible)
    {
        const dim::point2di Pos(
            Rect_.Left + CommandLineUI::TEXT_DISTANCE + ActiveFont_->getStringWidth(CommandLine_),
            Rect_.Bottom - TextLineHeight_
        );
        __spVideoDriver->draw2DRectangle(
            dim::rect2di(Pos.X, Pos.Y, Pos.X + ActiveFont_->getSize().Height/2, Pos.Y + ActiveFont_->getSize().Height), FgColor_
        );
    }
}

void CommandLineUI::drawScrollbar()
{
    /* Get text- and visible height */
    s32 TextHeight = 0, VisibleHeight = 0;
    getScrollingRange(TextHeight, VisibleHeight);
    
    const s32 MaxScroll = getMaxScrollPosition(TextHeight, VisibleHeight);
    
    if (TextHeight < VisibleHeight || MaxScroll <= 0)
        return;
    
    /* Compute scroll bar height */
    const s32 BarMinPos = Rect_.Top + CommandLineUI::SCROLLBAR_DISTANCE;
    const s32 BarMaxPos = Rect_.Bottom - CommandLineUI::SCROLLBAR_DISTANCE - TextLineHeight_ - CommandLineUI::TEXT_DISTANCE;
    
    f32 BarSizePercent = static_cast<f32>(VisibleHeight) / TextHeight;
    math::Clamp(BarSizePercent, 0.1f, 0.9f);
    
    /* Compuite scroll bar position */
    f32 BarPosPercent = static_cast<f32>(Scroll_) / MaxScroll;
    BarPosPercent -= BarSizePercent * BarPosPercent;
    
    const s32 BarTop    = BarMaxPos - static_cast<s32>((BarPosPercent + BarSizePercent) * (BarMaxPos - BarMinPos));
    const s32 BarBottom = BarMaxPos - static_cast<s32>(BarPosPercent * (BarMaxPos - BarMinPos));
    
    /* Draw vertical scroll bar */
    const dim::rect2di BarRect(
        Rect_.Right - CommandLineUI::SCROLLBAR_WIDTH - CommandLineUI::SCROLLBAR_DISTANCE,
        BarTop,
        Rect_.Right - CommandLineUI::SCROLLBAR_DISTANCE,
        BarBottom
    );
    
    __spVideoDriver->draw2DRectangle(BarRect, FgColor_);
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


/*
 * ======= Private: =======
 */

void CommandLineUI::getScrollingRange(s32 &TextHeight, s32 &VisibleHeight) const
{
    /* Get text- and visible height */
    TextHeight      = TextLineHeight_ * TextLines_.size();
    VisibleHeight   = Rect_.getHeight() - TextLineHeight_ - CommandLineUI::TEXT_DISTANCE;
}

s32 CommandLineUI::getMaxScrollPosition(s32 TextHeight, s32 VisibleHeight) const
{
    return TextHeight < VisibleHeight ? 0 : (TextHeight - VisibleHeight) / TextLineHeight_ + 1;
}

void CommandLineUI::clampScrolling(s32 TextHeight, s32 VisibleHeight)
{
    math::Clamp(Scroll_, 0, getMaxScrollPosition(TextHeight, VisibleHeight));
}

void CommandLineUI::clampScrolling()
{
    s32 TextHeight = 0, VisibleHeight = 0;
    getScrollingRange(TextHeight, VisibleHeight);
    clampScrolling(TextHeight, VisibleHeight);
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


/*
 * SMemento structure
 */

CommandLineUI::SMemento::SMemento() :
    Limit   (0              ),
    Current (Commands.end() )
{
}
CommandLineUI::SMemento::~SMemento()
{
}

void CommandLineUI::SMemento::push(const io::stringc &Command)
{
    if (Command.size())
    {
        /* Add new command entry and clamp entries to limit */
        Commands.push_back(Command);
        if (Limit > 0 && Commands.size() > Limit)
            Commands.pop_back();
        
        /* Update view pointer */
        Current = Commands.end();
    }
}

void CommandLineUI::SMemento::up(io::stringc &Str)
{
    if (!Commands.empty())
    {
        if (Current != Commands.begin())
            --Current;
        Str = *Current;
    }
}
void CommandLineUI::SMemento::down(io::stringc &Str)
{
    if (!Commands.empty())
    {
        if (Current != Commands.end())
            ++Current;
        Str = (Current == Commands.end() ? "" : *Current);
    }
}


} // /namespace tool

} // /namespace sp


//#endif



// ================================================================================
