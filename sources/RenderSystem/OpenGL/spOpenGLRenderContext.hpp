/*
 * OpenGL context base header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERCONTEXT_OPENGL_H__
#define __SP_RENDERCONTEXT_OPENGL_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/spDesktopRenderContext.hpp"

#if defined(SP_PLATFORM_MACOSX)
#   include <OpenGL/gl.h>
#else
#   include <GL/gl.h>
#endif

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#   include <GL/wglext.h>
#elif defined(SP_PLATFORM_LINUX)
#   include <X11/X.h>
#   include <X11/Xlib.h>
#   include <X11/Xutil.h>
#   include <X11/extensions/xf86vmode.h>
#   include <GL/glx.h>
#endif


namespace sp
{
namespace video
{


//! OpenGL render context. It's using WGL or GLX to create a context for Windows or Linux.
class SP_EXPORT OpenGLRenderContext : public DesktopRenderContext
{
    
    public:
        
        OpenGLRenderContext();
        ~OpenGLRenderContext();
        
        /* === Functions === */
        
        bool openGraphicsScreen(
            void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title,
            s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags
        );
        void closeGraphicsScreen();
        
        void flipBuffers();
        
        bool activate();
        bool deactivate();
        
        SharedRenderContext* createSharedContext();
        
        #ifdef SP_PLATFORM_WINDOWS
        void setFullscreen(bool Enable);
        bool setResolution(const dim::size2di &Resolution);
        #endif
        
        void setVsync(bool Enable);
        
    private:
        
        friend class OpenGLSharedRenderContext;
        
        /* === Functions === */
        
        bool createRenderContext();
        void releaseRenderContext();
        
        bool switchFullscreenMode(bool isFullscreen);
        
        #if defined(SP_PLATFORM_WINDOWS)
        
        void clearPixelFormatAA();
        bool selectPixelFormat();
        bool setupAntiAliasing();
        void deleteContextAndWindow();
        
        #endif
        
        void initRenderStates();
        
        /* === Members === */
        
        #if defined(SP_PLATFORM_WINDOWS)
        
        static const u32 PIXELFORMATAA_COUNT = 8;
        
        HGLRC RenderContext_;
        
        s32 PixelFormat_, MultiSamplePixelFormats_[PIXELFORMATAA_COUNT];
        u32 NumPixelFormatAA_;
        
        bool ExtContextCreation_;
        
        #elif defined(SP_PLATFORM_LINUX)
        
        GLXContext RenderContext_;


        #endif
        s32 GLVersionMajor_, GLVersionMinor_;
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
