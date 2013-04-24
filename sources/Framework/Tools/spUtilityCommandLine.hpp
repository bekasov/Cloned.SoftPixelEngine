/*
 * Command line header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILITY_COMMANDLINE_H__
#define __SP_UTILITY_COMMANDLINE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_COMMANDLINE


#include "Base/spInputOutputControl.hpp"
#include "Base/spTimer.hpp"
#include "Base/spMaterialConfigTypes.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spBaseExceptions.hpp"
#include "RenderSystem/spRenderSystemFont.hpp"
#include "Framework/Tools/spUtilityCommandLineTasks.hpp"


namespace sp
{
namespace tool
{


//! Command line user interface options.
enum ECommandLineFlags
{
    CMDFLAG_SCROLL          = 0x01, //!< Enables scrolling. This can be used for drawing and input functions.
    CMDFLAG_MEMENTO         = 0x02, //!< Enables memento and stores all enterd commands. Get the previous memento by pressing page-up key.
    CMDFLAG_DISABLE_INPUT   = 0x04, //!< Disables command execution by user input.
    
    CMDFLAG_COMMON =
        CMDFLAG_SCROLL |
        CMDFLAG_MEMENTO,            //!< All common command line flags are enabled.
};


/**
The command line is used to have an extended debug control during you are develping your games.
Here is a code example on how to use this class:
\code
class MyOwnCMD : public tool::CommandLineUI
{
    public:
        MyOwnCMD() : tool::CommandLineUI()
        {
        }
        ~MyOwnCMD()
        {
        }
        // Overwrite the execution function
        bool execute(io::stringc Command)
        {
            // Check for your custom commands
            if (Command == "StartMyGame")
                return MyGame::Start();
            if (Command == "QuitMyGame")
                exit(0);
            // Use the default commands
            return tool::CommandLineUI::execute(Command);
        }
};

// ...

// Just add this after all your drawing operations: (must not be in a begin/endDrawing2D block)
MyCmdObj->render();
\endcode
\since Version 3.2
\todo Split default commands into separated namespace, e.g. "namespace CommandLineTasks".
*/
class SP_EXPORT CommandLineUI
{
    
    public:
        
        CommandLineUI() throw(io::RenderSystemException, io::stringc);
        virtual ~CommandLineUI();
        
        /* === Functions === */
        
        /**
        Renders the user interfaces, i.e. draws and updates it. This is equivalent to the following code:
        \code
        Cmd->draw(Flags);
        Cmd->updateInput(Flags);
        \endcode
        */
        virtual void render(s32 Flags = CMDFLAG_COMMON);
        
        /**
        Draws the command line interface.
        \param[in] Flags Specifies the flags for drawing.
        \see ECommandLineFlags
        */
        virtual void draw(s32 Flags = CMDFLAG_COMMON);
        
        /**
        Updates the command line input.
        \param[in] Flags Specifies the flags for input.
        \see ECommandLineFlags
        */
        virtual void updateInput(s32 Flags = CMDFLAG_COMMON);
        
        //! This will be called in "updateInput" if the CMDFLAG_SCROLL flag is set.
        virtual void updateScrollInput(s32 DefaultScrollSpeed = 3);
        
        //! Prints the given message in the specified color
        virtual void message(const io::stringc &Message, const video::color &Color = 255, u32 NewLineTab = 0);
        //! Prints a yellow message in the form: "Warning: " + Message + "!".
        virtual void warning(const io::stringc &Message);
        //! Prints a red message in the form: "Error: " + Message + "!".
        virtual void error(const io::stringc &Message);
        
        //! Prints an error message that the given command is unknown.
        virtual void unknown(const io::stringc &Command);
        //! Prints a message to confirm the current command with the given output.
        virtual void confirm(const io::stringc &Output);
        
        //! Prints the given image (or rather texture) as 'text'-line entry.
        virtual void image(video::Texture* Image);
        
        //! Executes the given command
        virtual bool executeCommand(const io::stringc &Command);
        
        void setupCursorTimer(u64 IntervalDuration);
        
        /**
        Excecutes the given command.
        \param[in] Command Specifies the command which is to be executed. The default commands are case sensitive
        and are all in lower case. Enter "help" for a detailed list of all default commands.
        \return True if the given command was executed successful. Otherwise false and an error message
        should be printed in this command line.
        */
        bool execute(const io::stringc &Command);
        
        //! Clears the console content.
        virtual void clear(bool isHelpInfo = true);
        
        //! Returns true if vertical scrolling is enabled.
        bool isScrollingEnabled() const;
        /**
        Scrolls vertical in the given direction.
        Positive values mean up scrolling and negative values mean down scrolling.
        \return True if the scolling could be performed.
        */
        virtual bool scroll(s32 Direction);
        /**
        Scrolls a whole page.
        \see scroll
        */
        virtual bool scrollPage(s32 Direction);
        //! Scrolls to the start.
        virtual void scrollStart();
        //! Scrolls to the end.
        virtual void scrollEnd();
        
        //! Sets the new scroll position. Start position is 0.
        void setScrollPosition(s32 Pos);
        //! Returns the maximal scroll position.
        s32 getMaxScrollPosition() const;
        //! Returns the scrolling size for one page.
        s32 getScrollPage() const;
        
        /**
        Sets the new font for text drawing. By default "courier new".
        \param[in] FontObj Pointer to the new font object. If this is a null pointer the original font will be used.
        */
        void setFont(video::Font* FontObj);
        
        /**
        Returns the parameter part of the given command.
        \param[in] Command Specifies the whole command string (e.g. 'MyCommand "MyParameter"').
        \param[out] Param Specifies the output parameter string.
        \return True if a string parameter could be extracted.
        */
        bool getCmdParam(const io::stringc &Command, io::stringc &Param);
        
        /* === Inline functions === */
        
        //! Returns a pointer to the active font object.
        inline video::Font* getFont() const
        {
            return ActiveFont_;
        }
        //! Returns a pointer to the original font object.
        inline video::Font* getOrigFont() const
        {
            return OrigFont_;
        }
        
        //! Returns the current scroll position. Start position is 0.
        inline s32 getScrollPosition() const
        {
            return Scroll_;
        }
        
        //! Sets the user interface background color. By default black or rather video::color(0, 0, 0, 255).
        inline void setBackgroundColor(const video::color &Color)
        {
            BgColor_ = Color;
        }
        //! Returns the background color.
        inline const video::color& getBackgroundColor() const
        {
            return BgColor_;
        }
        
        //! Sets the user interface background color. By default white or rather video::color(255).
        inline void setForegroundColor(const video::color &Color)
        {
            FgColor_ = Color;
        }
        //! Returns the foreground color (for the cursor, separation line and scroll bar).
        inline const video::color& getForegroundColor() const
        {
            return FgColor_;
        }
        
        //! Sets the view rectangle. By default dim::rect2di(0, 0, ScreenWidth, ScreenHeight/2).
        inline void setRect(const dim::rect2di &Rect)
        {
            Rect_ = Rect;
        }
        //! Returbs the view rectangle.
        inline dim::rect2di getRect() const
        {
            return Rect_;
        }
        
        //! Sets the maximal lines in the user interface. To make it unlimited set it to 0. By default 0.
        inline void setLineLimit(u32 Limit)
        {
            MaxLines_ = Limit;
        }
        //! Returns the count of maximal lines. 0 means unlimited. By default 0.
        inline u32 getLineLimit() const
        {
            return MaxLines_;
        }
        
        //! Sets the maximal count of memento entries. To make it unlimited set it to 0. By default 0.
        inline void setMementoLimit(u32 Limit)
        {
            Memento_.Limit = Limit;
        }
        //! Returns the count of maximal memento entries. 0 means unlimited. By default 0.
        inline u32 getMementoLimit() const
        {
            return Memento_.Limit;
        }
        
        //! Prints a new blank line. This is equivalent to: message("");
        inline void blank()
        {
            message("");
        }
        
    protected:
        
        /* === Macros === */
        
        static const s32 TEXT_DISTANCE;
        static const s32 SCROLLBAR_WIDTH;
        static const s32 SCROLLBAR_DISTANCE;
        
        /* === Structures === */
        
        struct SP_EXPORT STextLine
        {
            STextLine();
            STextLine(video::Font* LineFont, const io::stringc &LineText, const video::color &LineColor = 255);
            STextLine(video::Texture* LineImage);
            ~STextLine();
            
            /* Functions */
            s32 getHeight() const;
            
            void draw(
                const dim::point2di &Origin, const dim::size2di &MaxLineSize,
                s32 &PosVert, f32 TransBgOffset
            ) const;
            
            /* Members */
            video::Font* TextFont;
            video::Texture* Image;
            io::stringc Text;
            video::color Color;
        };
        
        struct SP_EXPORT SCursor
        {
            SCursor();
            ~SCursor();
            
            /* Functions */
            void update();
            
            /* Members */
            io::Timer IntervalTimer;
            bool isVisible;
        };
        
        struct SP_EXPORT SMemento
        {
            SMemento();
            ~SMemento();
            
            /* Functions */
            void push(const io::stringc &Command);
            
            void up(io::stringc &Str);
            void down(io::stringc &Str);
            
            /* Members */
            u32 Limit;
            std::list<io::stringc> Commands;
            std::list<io::stringc>::iterator Current;
        };
        
        struct SP_EXPORT SCommand
        {
            SCommand();
            ~SCommand();
            
            /* Operators */
            bool operator < (const SCommand &Other) const;
            
            /* Members */
            io::stringc Name, Docu;
        };
        
        /* === Functions === */
        
        virtual void drawBackground();
        virtual void drawTextLines();
        virtual void drawCursor();
        virtual void drawScrollbar();
        
        virtual void drawTextLine(s32 &PosVert, const STextLine &Line);
        
        virtual void addHelpLine(const io::stringc &Command, const io::stringc &Description);
        virtual void printHelpLines(c8 SeparationChar = '.', u32 MinSeparationChars = 3);
        
        /**
        Tries to find an automatic completion for the given command string.
        \param[in,out] Command Specifies the command for the auto-completion to search for.
        \return True if an auto-completion could be found.
        */
        virtual bool findAutoCompletion(io::stringc &Command);
        
        void registerCommand(const io::stringc &Name, const io::stringc &Docu);
        
        void addNewLine(const STextLine &Line);
        void addNewLine(const io::stringc &Message, const video::color &Color);
        
        /* === Members === */
        
        video::color BgColor_, FgColor_;
        dim::rect2di Rect_;
        
        SCursor Cursor_;
        SMemento Memento_;
        
        u32 MaxLines_;
        s32 Scroll_;
        
        std::vector<STextLine> TextLines_;
        std::vector<SCommand> RegisteredCommands_;
        
        io::stringc CommandLine_;
        
    private:
        
        /* === Structures === */
        
        struct SHelpLine
        {
            io::stringc Command;
            io::stringc Description;
        };
        
        /* === Functions === */
        
        s32 getMaxScrollPosition(s32 TextHeight, s32 VisibleHeight) const;
        
        void clampScrolling(s32 TextHeight, s32 VisibleHeight);
        void clampScrolling();
        
        void getScrollingRange(s32 &TextHeight, s32 &VisibleHeight) const;
        
        void registerDefaultCommands();
        
        /* === Members === */
        
        video::Font* ActiveFont_;
        video::Font* OrigFont_;
        
        s32 TextLineHeight_;
        f32 TransBgOffset_;
        
        std::list<SHelpLine> TempHelpLines_;
        u32 MaxHelpCommand_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
