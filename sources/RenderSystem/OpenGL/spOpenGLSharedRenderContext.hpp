/*
 * OpenGL shared context base header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHAREDRENDERCONTEXT_OPENGL_H__
#define __SP_SHAREDRENDERCONTEXT_OPENGL_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/spSharedRenderContext.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#   include <GL/gl.h>
#   include <GL/wglext.h>
#elif defined(SP_PLATFORM_LINUX)
#   include <GL/gl.h>
#   include <GL/glx.h>
#elif defined(SP_PLATFORM_MACOSX)
#   include <OpenGL/gl.h>
#endif


namespace sp
{
namespace video
{


class OpenGLRenderContext;

class SP_EXPORT OpenGLSharedRenderContext : public SharedRenderContext
{
    
    public:
        
        OpenGLSharedRenderContext(OpenGLRenderContext* RootRenderContext);
        ~OpenGLSharedRenderContext();
        
        /* Functions */
        
        bool activate();
        bool deactivate();
        
    private:
        
        /* Members */
        
        #if defined(SP_PLATFORM_WINDOWS)
        
        HGLRC RenderContext_;
        HDC DeviceContext_;
        
        #elif defined(SP_PLATFORM_LINUX)
        
        GLXContext RenderContext_;
        Display* Display_;
        Window Window_;
        XVisualInfo* Visual_;
        
        #endif
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
