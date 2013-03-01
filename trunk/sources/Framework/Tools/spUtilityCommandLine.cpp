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
#include "SceneGraph/spSceneGraph.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;
extern io::InputControl* __spInputControl;

namespace tool
{


const s32 CommandLineUI::TEXT_DISTANCE      = 4;
const s32 CommandLineUI::SCROLLBAR_DISTANCE = 5;
const s32 CommandLineUI::SCROLLBAR_WIDTH    = 7;

CommandLineUI::CommandLineUI() throw(io::RenderSystemException, io::stringc) :
    BgColor_(0, 0, 0, 255),
    Rect_(
        0,
        0,
        gSharedObjects.ScreenWidth,
        gSharedObjects.ScreenHeight/2
    ),
    MaxLines_       (0      ),
    Scroll_         (0      ),
    ActiveFont_     (0      ),
    OrigFont_       (0      ),
    TextLineHeight_ (0      ),
    TransBgOffset_  (0.0f   ),
    MaxHelpCommand_ (0      )
{
    if (!__spVideoDriver)
        throw io::RenderSystemException("tool::CommandLineUI");
    if (!__spInputControl)
        throw io::stringc("Input controller has not been created yet");
    
    /* Create default font object */
    OrigFont_ = __spVideoDriver->createFont(
        "courier new", 15, __spVideoDriver->getRendererType() == video::RENDERER_DIRECT3D9 ? video::FONT_BITMAP : 0
    );
    setFont(0);
    clear();
    
    __spInputControl->setWordInput(true);
    
    /* Register all default commands */
    registerDefaultCommands();
}
CommandLineUI::~CommandLineUI()
{
    __spVideoDriver->deleteFont(OrigFont_);
}

void CommandLineUI::render(s32 Flags)
{
    draw(Flags);
    updateInput(Flags);
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
    
    /* Update transparent image background offset */
    TransBgOffset_ += 0.01f * io::Timer::getGlobalSpeed();
    if (TransBgOffset_ > 1.0f)
        TransBgOffset_ -= 1.0f;
    
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
    
    /* Update auto-completion */
    if (__spInputControl->keyHit(io::KEY_TAB) && CommandLine_.size())
        findAutoCompletion(__spInputControl->getEnteredWord());
    
    /* Get enterd characters */
    CommandLine_ = __spInputControl->getEnteredWord();
    
    if (!(Flags & CMDFLAG_DISABLE_INPUT) && __spInputControl->keyHit(io::KEY_RETURN))
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

void CommandLineUI::message(const io::stringc &Message, const video::color &Color, u32 NewLineTab)
{
    /* Check if line is too long */
    const s32 MaxWidth = Rect_.getWidth() - CommandLineUI::TEXT_DISTANCE*2 - CommandLineUI::SCROLLBAR_WIDTH - CommandLineUI::SCROLLBAR_DISTANCE;
    
    if (ActiveFont_->getStringWidth(Message) > MaxWidth)
    {
        /* Use a copy of given message for later reduction */
        io::stringc Msg(Message);
        
        /* Get reference to font glyph metrics */
        const std::vector<video::SFontGlyph>& GlyphList = ActiveFont_->getGlyphList();
        
        do
        {
            /* Determine width of current message part */
            s32 CurWidth = ActiveFont_->getStringWidth(Msg);
            
            if (CurWidth <= MaxWidth)
            {
                if (Msg.size())
                    addNewLine(Msg, Color);
                break;
            }
            else if (CurWidth > MaxWidth*2)
            {
                /* Find next suitable message part from right-to-left */
                for (u32 i = Msg.size(); i > 0; --i)
                {
                    CurWidth -= GlyphList[Msg[i - 1]].getWidth();
                    
                    if (CurWidth <= MaxWidth || i <= 2)
                    {
                        /* Add new suitable message part */
                        addNewLine(Msg.left(i - 1), Color);
                        Msg = Msg.right(Msg.size() - i + 1);
                        break;
                    }
                }
            }
            else
            {
                CurWidth = 0;
                
                /* Find next suitable message part from left-to-right */
                for (u32 i = 0; i < Msg.size(); ++i)
                {
                    CurWidth += GlyphList[Msg[i]].getWidth();
                    
                    if (CurWidth > MaxWidth && i >= 2)
                    {
                        /* Add new suitable message part */
                        addNewLine(Msg.left(i), Color);
                        Msg = Msg.right(Msg.size() - i);
                        break;
                    }
                }
            }
            
            /* Add new line tab characters */
            Msg = io::stringc::space(NewLineTab) + Msg;
        }
        while (Msg.size());
    }
    else
        addNewLine(Message, Color);
}

void CommandLineUI::warning(const io::stringc &Message)
{
    message("Warning: " + Message + "!", video::color(255, 255, 0));
}
void CommandLineUI::error(const io::stringc &Message)
{
    message("Error: " + Message + "!", video::color(255, 0, 0));
}

void CommandLineUI::unknown(const io::stringc &Command)
{
    error("Unknown command: \"" + Command + "\"");
}

void CommandLineUI::confirm(const io::stringc &Output)
{
    if (Output.size())
    {
        /* Determine new line tab size */
        u32 NewLineTab = 3;
        
        for (u32 i = 0; i < Output.size(); ++i)
        {
            if (Output[i] != ' ')
            {
                NewLineTab += i;
                break;
            }
        }
        
        /* Print confirmation message */
        message(" > " + Output, video::color(0, 255, 0), NewLineTab);
    }
}

void CommandLineUI::image(video::Texture* Image)
{
    if (Image)
        addNewLine(Image);
}

bool CommandLineUI::executeCommand(const io::stringc &Command)
{
    /* Check for default commands */
    if (Command == "help")
    {
        foreach (const SCommand &Cmd, RegisteredCommands_)
            addHelpLine(Cmd.Name, Cmd.Docu);
        printHelpLines();
    }
    else if (Command == "clear")
        clear();
    else if (Command == "solid")
        CommandLineTasks::cmdWireframe(*this, __spSceneManager, video::WIREFRAME_SOLID);
    else if (Command == "lines")
        CommandLineTasks::cmdWireframe(*this, __spSceneManager, video::WIREFRAME_LINES);
    else if (Command == "points")
        CommandLineTasks::cmdWireframe(*this, __spSceneManager, video::WIREFRAME_POINTS);
    else if (Command == "fullscreen")
        CommandLineTasks::cmdFullscreen(*this);
    else if (Command == "view")
        CommandLineTasks::cmdView(*this, __spSceneManager ? __spSceneManager->getActiveCamera() : 0);
    else if (Command == "vsync")
        CommandLineTasks::cmdVsync(*this);
    else if (Command == "scene")
        CommandLineTasks::cmdScene(*this);
    else if (Command == "hardware")
        CommandLineTasks::cmdHardware(*this);
    else if (Command == "network")
        CommandLineTasks::cmdNetwork(*this);
    else if (Command.leftEqual("resolution", 10))
        CommandLineTasks::cmdResolution(*this, Command);
    else if (Command == "drawcalls")
        CommandLineTasks::cmdDrawCalls(*this);
    else if (Command.leftEqual("images", 6))
        CommandLineTasks::cmdShowImages(*this, Command);
    else
        return false;
    
    return true;
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
    Scroll_ = 0;
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

bool CommandLineUI::getCmdParam(const io::stringc &Command, io::stringc &Param)
{
    s32 Pos1, Pos2;
    
    if ( ( Pos1 = Command.find("\"") ) == -1 )
    {
        error("Missing quotation marks for command parameter");
        return false;
    }
    
    if ( ( Pos2 = Command.find("\"", Pos1 + 1) ) == -1 )
    {
        error("Missing closing quotation mark for command parameter");
        return false;
    }
    
    if (Pos1 + 1 == Pos2)
    {
        error("Command parameter must not be empty");
        return false;
    }
    
    Param = Command.section(Pos1 + 1, Pos2);
    
    return true;
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
    s32 PosVert = Rect_.Bottom - CommandLineUI::TEXT_DISTANCE*3 - FontHeight;
    
    for (s32 i = static_cast<s32>(TextLines_.size()) - Scroll_; i > 0 && PosVert > -FontHeight; --i)
        drawTextLine(PosVert, TextLines_[i - 1]);
    
    /* Draw input separation line */
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect_.Left, Rect_.Bottom - FontHeight - CommandLineUI::TEXT_DISTANCE*2),
        dim::point2di(Rect_.Right, Rect_.Bottom - FontHeight - CommandLineUI::TEXT_DISTANCE*2),
        FgColor_
    );
    
    PosVert = Rect_.Bottom - CommandLineUI::TEXT_DISTANCE;
    drawTextLine(PosVert, STextLine(ActiveFont_, CommandLine_));
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

void CommandLineUI::drawTextLine(s32 &PosVert, STextLine &Line)
{
    Line.draw(
        Rect_.getLTPoint(),
        dim::size2di(
            Rect_.getWidth() - CommandLineUI::TEXT_DISTANCE*2,
            Rect_.getHeight() - TextLineHeight_*5
        ),
        PosVert,
        TransBgOffset_
    );
}

void CommandLineUI::addHelpLine(const io::stringc &Command, const io::stringc &Description)
{
    SHelpLine Help;
    {
        Help.Command        = Command;
        Help.Description    = Description;
        
        if (MaxHelpCommand_ < Command.size())
            MaxHelpCommand_ = Command.size();
    }
    TempHelpLines_.push_back(Help);
}

void CommandLineUI::printHelpLines(c8 SeparationChar, u32 MinSeparationChars)
{
    /* Print all help lines */
    foreach (const SHelpLine &Help, TempHelpLines_)
    {
        confirm(
            Help.Command + " " +
            io::stringc::space(MaxHelpCommand_ - Help.Command.size() + MinSeparationChars, SeparationChar) +
            " " + Help.Description
        );
    }
    
    /* Reset temporary list */
    TempHelpLines_.clear();
    MaxHelpCommand_ = 0;
}

bool CommandLineUI::findAutoCompletion(io::stringc &Command)
{
    u32 PrevLen = Command.size();
    
    /* Store duplicate similar commands */
    std::list<io::stringc> DupSimCommands;
    
    /* Search for nearest similiarity */
    for (std::vector<SCommand>::const_iterator it = RegisteredCommands_.begin(), itNext; it != RegisteredCommands_.end(); ++it)
    {
        if (it->Name.size() >= Command.size() && it->Name.leftEqual(Command, Command.size()))
        {
            /* Store first similar command */
            Command = it->Name;
            
            DupSimCommands.push_back(it->Name);
            
            /* Check if there are other commands that fit this similiarity */
            for (itNext = it + 1; itNext != RegisteredCommands_.end(); ++itNext)
            {
                /* Get count of first equal characters */
                u32 EqCharCount = itNext->Name.getLeftEquality(Command);
                
                if (EqCharCount >= PrevLen)
                {
                    /* Reduce auto-completion */
                    PrevLen = EqCharCount;
                    Command = Command.left(EqCharCount);
                    DupSimCommands.push_back(itNext->Name);
                }
                else if (EqCharCount == 0)
                    break;
            }
            
            /* Print command names if there are several similar commands */
            if (DupSimCommands.size() > 1)
            {
                message("Found several similar commands:");
                foreach (const io::stringc &Name, DupSimCommands)
                    message("[ " + Name + " ]");
            }   
            
            return true;
        }
    }
    return false;
}

void CommandLineUI::registerCommand(const io::stringc &Name, const io::stringc &Docu)
{
    /* Register new command with documentation */
    SCommand Cmd;
    {
        Cmd.Name = Name;
        Cmd.Docu = Docu;
    }
    RegisteredCommands_.push_back(Cmd);
    
    /* Sort command list */
    std::sort(RegisteredCommands_.begin(), RegisteredCommands_.end());
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

void CommandLineUI::registerDefaultCommands()
{
    registerCommand("clear",            "Clears the console content."                                       );
    registerCommand("drawcalls",        "Prints information about the draw calls."                          );
    registerCommand("fullscreen",       "Toggles the fullscreen mode."                                      );
    registerCommand("hardware",         "Prints information about the hardware."                            );
    registerCommand("help",             "Prints this help document."                                        );
    registerCommand("images",           "Shows all images (or rather textures) with optional search filter.");
    registerCommand("lines",            "Switches the active scene-graph wireframe-mode to lines."          );
    registerCommand("network",          "Prints information about the network session."                     );
    registerCommand("points",           "Switches the active scene-graph wireframe-mode to points."         );
    registerCommand("resolution size$", "Change the screen resolution (e.g. 'resolution \"800x600\"')."     );
    registerCommand("scene",            "Prints information about the scene manager."                       );
    registerCommand("solid",            "Switches the active scene-graph wireframe-mode to solid."          );
    registerCommand("view",             "Prints the global position and rotation of the active camera."     );
    registerCommand("vsync",            "Toggles vertical synchronisation."                                 );
}

void CommandLineUI::addNewLine(const STextLine &Line)
{
    /* Add new message */
    TextLines_.push_back(Line);
    
    /* Pop old messages */
    if (MaxLines_ > 0 && TextLines_.size() > MaxLines_)
        TextLines_.erase(TextLines_.begin());
    
    /* Update scrolling */
    if (Scroll_ > 0)
        scroll(1);
}

void CommandLineUI::addNewLine(const io::stringc &Message, const video::color &Color)
{
    addNewLine(STextLine(ActiveFont_, Message, Color));
}


/*
 * STextLine structure
 */

CommandLineUI::STextLine::STextLine() :
    TextFont(0),
    Image   (0)
{
}
CommandLineUI::STextLine::STextLine(
    video::Font* LineFont, const io::stringc &LineText, const video::color &LineColor) :
    TextFont(LineFont   ),
    Image   (0          ),
    Text    (LineText   ),
    Color   (LineColor  )
{
}
CommandLineUI::STextLine::STextLine(video::Texture* LineImage) :
    TextFont(0          ),
    Image   (LineImage  )
{
}
CommandLineUI::STextLine::~STextLine()
{
}

s32 CommandLineUI::STextLine::getHeight() const
{
    if (Image)
        return Image->getSize().Height + CommandLineUI::TEXT_DISTANCE;
    if (TextFont)
        return TextFont->getHeight() + CommandLineUI::TEXT_DISTANCE;
    return 0;
}

void CommandLineUI::STextLine::draw(
    const dim::point2di &Origin, const dim::size2di &MaxLineSize, s32 &PosVert, f32 TransBgOffset)
{
    if (TextFont)
    {
        __spVideoDriver->draw2DText(
            TextFont,
            dim::point2di(
                Origin.X + CommandLineUI::TEXT_DISTANCE,
                Origin.Y + PosVert - TextFont->getHeight()
            ),
            Text, Color
        );
        
        PosVert -= TextFont->getHeight() + CommandLineUI::TEXT_DISTANCE;
    }
    else if (Image)
    {
        const dim::size2di ImgSize(Image->getSize().getClampedSize(MaxLineSize));
        
        const dim::rect2di ImgRect(
            Origin.X + CommandLineUI::TEXT_DISTANCE,
            Origin.Y + PosVert - ImgSize.Height,
            ImgSize.Width,
            ImgSize.Height
        );
        
        if (Image->getImageBuffer()->hasAlphaChannel())
        {
            static const f32 Scaling = 1.0f / 32.0f;
            
            __spVideoDriver->draw2DImage(
                __spVideoDriver->getDefaultTexture(video::DEFAULT_TEXTURE_TILES),
                ImgRect,
                dim::rect2df(
                    TransBgOffset,
                    TransBgOffset,
                    TransBgOffset + Scaling * ImgSize.Width,
                    TransBgOffset + Scaling * ImgSize.Height
                )
            );
        }
        
        __spVideoDriver->draw2DImage(Image, ImgRect);
        
        PosVert -= (ImgSize.Height + CommandLineUI::TEXT_DISTANCE);
    }
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


/*
 * SCommand structure
 */

CommandLineUI::SCommand::SCommand()
{
}
CommandLineUI::SCommand::~SCommand()
{
}

bool CommandLineUI::SCommand::operator < (const SCommand &Other) const
{
    return Name.str() < Other.Name.str();
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
