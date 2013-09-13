/*
 * Desktop render context header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERCONTEXT_DESKTOP_H__
#define __SP_RENDERCONTEXT_DESKTOP_H__


#include "Base/spStandard.hpp"

#if defined(SP_PLATFORM_WINDOWS) || defined(SP_PLATFORM_LINUX)


#include "RenderSystem/spRenderContext.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#elif defined(SP_PLATFORM_LINUX)
#   include <X11/X.h>
#   include <X11/Xlib.h>
#   include <X11/Xutil.h>
#   include <X11/extensions/xf86vmode.h>
#   include <GL/glx.h>
#endif


namespace sp
{

#if defined(SP_PLATFORM_WINDOWS)
class SoftPixelDeviceWin32;
SP_EXPORT LRESULT CALLBACK SpWin32Callback(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
#elif defined(SP_PLATFORM_LINUX)
class SoftPixelDeviceLinux;
#endif

namespace video
{


//! Render context class for desktop operating systems such as Windows and Linux.
class SP_EXPORT DesktopRenderContext : public RenderContext
{
    
    public:
        
        DesktopRenderContext();
        virtual ~DesktopRenderContext();
        
        /* === Functions === */
        
        virtual bool openGraphicsScreen(
            void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title,
            s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags
        );
        virtual void closeGraphicsScreen();
        
        virtual void flipBuffers();
        
        virtual bool activate();
        virtual bool deactivate();
        
        virtual void setWindowTitle(const io::stringc &Title);
        virtual io::stringc getWindowTitle() const;
        
        virtual void setWindowPosition(const dim::point2di &Position);
        virtual dim::point2di getWindowPosition() const;
        
        virtual dim::size2di getWindowSize() const;
        virtual dim::size2di getWindowBorder() const;
        
        virtual bool isWindowActive() const;
        
        virtual void* getWindowObject();
        
    protected:
        
        #ifdef SP_COMPILE_WITH_OPENGL
        friend class OpenGLSharedRenderContext;
        #endif
        
        #if defined(SP_PLATFORM_WINDOWS)
        
        friend class sp::SoftPixelDeviceWin32;
        friend SP_EXPORT LRESULT CALLBACK sp::SpWin32Callback(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
        
        static const c8* WINDOW_CLASSNAME;
        
        #elif defined(SP_PLATFORM_LINUX)
        
        friend class sp::SoftPixelDeviceLinux;
        
        #endif
        
        /* === Functions === */
        
        #if defined(SP_PLATFORM_WINDOWS)
        
        void registerWindowClass();
        void unregisterWindowClass();
        
        DWORD getWindowStyle() const;
        void getWindowDimension(dim::point2di &Position, dim::size2di &Size) const;
        
        void updateWindowStyleAndDimension();
        void updateScreenOffset(bool isFullscreen);
        
        bool createWindow(const io::stringc &Title);
        void deleteWindow();
        void showWindow();
        
        #elif defined(SP_PLATFORM_LINUX)
        
        bool openDisplay();
        bool chooseVisual();
        bool createWindow(const io::stringc &Title);
        
        #endif
        
        /* === Members === */
        
        #if defined(SP_PLATFORM_WINDOWS)
        
        HWND Window_;
        HDC DeviceContext_;
        
        #elif defined(SP_PLATFORM_LINUX)
        
        Display* Display_;
        Window Window_, DefaultWindow_;
        Colormap ColorMap_;
        s32 Screen_;
        
        XVisualInfo* Visual_;
        XSetWindowAttributes WinAttribs_;
        XF86VidModeModeInfo DesktopVideoMode_;
        
        #endif
        
        /* === Static members === */
        
        static bool EnableWindowResize_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
