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
#include "RenderSystem/spRenderSystemFont.hpp"


namespace sp
{
namespace tool
{


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
\endcode
\since Version 3.2
*/
class SP_EXPORT CommandLineUI
{
    
    public:
        
        CommandLineUI();
        virtual ~CommandLineUI();
        
        /* === Functions === */
        
        virtual void draw();
        virtual void updateInput();
        
        virtual void message(const io::stringc &Message, const video::color &Color = 255);
        void warning(const io::stringc &Message);
        void error(const io::stringc &Message);
        
        void setupCursorTimer(u64 IntervalDuration);
        
        /**
        Excecutes the given command.
        \param[in] Command Specifies the command which is to be executed. The default commands are case sensitive,
        i.e. "fullscreen" means the same as "FULLscreen". This function should be overwritten by your own class.
        Here is a list of all default commands:
        \li <tt>"help"</tt> Prints default all commands.
        \li <tt>"clear"</tt> Clears the console content.
        \li <tt>"solid"</tt> Changes the wireframe mode of the active scene graph to \c solid.
        \li <tt>"lines"</tt> Changes the wireframe mode of the active scene graph to \c lines.
        \li <tt>"points"</tt> Changes the wireframe mode of the active scene graph to \c points.
        \li <tt>"fullscreen"</tt> Toggles the fullscreen mode.
        \li <tt>"cam pos"</tt> Prints the global position of the active camera.
        \li <tt>"cam rot"</tt> Prints the global rotation of the active camera.
        \return True if the given command was executed successful. Otherwise false and an error message
        should be printed in this command line.
        */
        bool execute(const io::stringc &Command);
        
        //! Clears the console content.
        virtual void clear(bool isHelpInfo = true);
        
        /* === Inline functions === */
        
        //! Sets the user interface text font. By default this is a "courier new" font.
        inline void setFont(video::Font* FontObj)
        {
            ActiveFont_ = (FontObj ? FontObj : ActiveFont_);
        }
        inline video::Font* getFont() const
        {
            return ActiveFont_;
        }
        
        //! Sets the user interface background color. By default black or rather video::color(0, 0, 0, 255).
        inline void setBackgroundColor(const video::color &Color)
        {
            BgColor_ = Color;
        }
        inline video::color getBackgroundColor() const
        {
            return BgColor_;
        }
        
        //! Sets the user interface rectangle. By default dim::rect2di(0, 0, ScreenWidth, ScreenHeight/2).
        inline void setRect(const dim::rect2di &Rect)
        {
            Rect_ = Rect;
        }
        inline dim::rect2di getRect() const
        {
            return Rect_;
        }
        
        //! Sets the maximal lines in the user interface. To make it unlimited set it to 0. By default 0.
        inline void setLineLimit(u32 Limit)
        {
            MaxLines_ = Limit;
        }
        inline u32 getLineLimit() const
        {
            return MaxLines_;
        }
        
        //! Prints a new blank line. This is equivalent to: message("");
        inline void blank()
        {
            message("");
        }
        
    protected:
        
        /* === Macros === */
        
        static const s32 TEXT_DISTANCE;
        
        /* === Structures === */
        
        struct SP_EXPORT STextLine
        {
            STextLine();
            STextLine(const io::stringc &LineText, const video::color &LineColor = 255);
            ~STextLine();
            
            /* Members */
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
        
        /* === Functions === */
        
        virtual void drawBackground();
        virtual void drawTextLines();
        virtual void drawCursor();
        
        virtual void drawTextLine(s32 PosVert, const STextLine &Line);
        
        //! Prints an error message that the given command is unknown.
        virtual void unknown(const io::stringc &Command);
        //! Prints a message to confirm the current command with the given output.
        virtual void confirm(const io::stringc &Output);
        
        virtual bool executeCommand(const io::stringc &Command);
        
        /* === Members === */
        
        video::color BgColor_;
        dim::rect2di Rect_;
        SCursor Cursor_;
        
        u32 MaxLines_;
        
        std::vector<STextLine> TextLines_;
        
        io::stringc CommandLine_;
        
    private:
        
        /* === Functions === */
        
        bool cmdHelp();
        bool cmdClearConsole();
        bool cmdWireframe(const video::EWireframeTypes Type);
        bool cmdFullscreen();
        bool cmdPrintCameraPosition();
        bool cmdPrintCameraRotation();
        
        void pushMemo();
        void popMemo();
        
        /* === Members === */
        
        video::Font* ActiveFont_;
        video::Font* OrigFont_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
