/*
 * OpenGL|ES EGL render context header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERCONTEXT_OPENGLES_H__
#define __SP_RENDERCONTEXT_OPENGLES_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGLES1) || defined(SP_COMPILE_WITH_OPENGLES2)


#include "Base/spDimensionSize2D.hpp"
#include "Base/spInputOutputString.hpp"
#include "Platform/spSoftPixelDeviceFlags.hpp"
#include "RenderSystem/spRenderContext.hpp"
#include "RenderSystem/OpenGL/spOpenGLCoreHeader.hpp"


namespace sp
{
namespace video
{


//! EGL context base for the OpenGL|ES renderers.
class SP_EXPORT OpenGLESRenderContext : public RenderContext
{
    
    public:
        
        OpenGLESRenderContext(bool isGLES2);
        ~OpenGLESRenderContext();
        
        /* Functions */
        
        bool openGraphicsScreen(
            void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title,
            s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags
        );
        void closeGraphicsScreen();
        
        void flipBuffers();
        
        bool activate();
        bool deactivate();
        
        void setWindowTitle(const io::stringc &Title);
        io::stringc getWindowTitle() const;
        
        void setWindowPosition(const dim::point2di &Position);
        dim::point2di getWindowPosition() const;
        
        dim::size2di getWindowSize() const;
        dim::size2di getWindowBorder() const;
        
        bool isWindowActive() const;
        
        void* getWindowObject();
        
    private:
        
        /* Functions */
        
        bool createRenderContext(void* Window, bool isGLES2);
        void releaseRenderContext(void* Window);
        
        /* Members */
        
        bool isGLES2_;
        
        #ifdef SP_USE_GLES_EGL
        EGLDisplay Display_;
        EGLSurface Surface_;
        EGLContext Context_;
        #endif
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
