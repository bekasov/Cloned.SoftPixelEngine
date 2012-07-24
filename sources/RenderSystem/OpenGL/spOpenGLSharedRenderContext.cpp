/*
 * OpenGL shared context base file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLSharedRenderContext.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/OpenGL/spOpenGLRenderContext.hpp"
#include "Base/spInputOutputLog.hpp"


namespace sp
{
namespace video
{


/*
 * External macros
 */

extern const c8* GLCONTEXT_ERROR_CREATE;
extern const c8* GLCONTEXT_ERROR_ACTIVATE;
extern const c8* GLCONTEXT_ERROR_DEACTIVATE;
extern const c8* GLCONTEXT_ERROR_DESTROY;

/*
 * OpenGLRenderContext class
 */

#if defined(SP_PLATFORM_WINDOWS)

OpenGLSharedRenderContext::OpenGLSharedRenderContext(OpenGLRenderContext* RootRenderContext) :
    RenderContext_(0),
    DeviceContext_(0)
{
    if (RootRenderContext)
    {
        DeviceContext_ = RootRenderContext->DeviceContext_;
        
        /* Create OpenGL render context */
        RenderContext_ = wglCreateContext(DeviceContext_);
        
        if (RenderContext_)
        {
            /* Share render contexts */
            if (!wglShareLists(RootRenderContext->RenderContext_, RenderContext_))
                io::Log::error("Could not share OpenGL render contexts");
        }
        else
            io::Log::error(GLCONTEXT_ERROR_CREATE);
    }
}
OpenGLSharedRenderContext::~OpenGLSharedRenderContext()
{
    /* Delete OpenGL render context */
    if (!RenderContext_ || !wglDeleteContext(RenderContext_))
        io::Log::error(GLCONTEXT_ERROR_DESTROY);
}

bool OpenGLSharedRenderContext::activate()
{
    return wglMakeCurrent(DeviceContext_, RenderContext_) == TRUE;
}
bool OpenGLSharedRenderContext::deactivate()
{
    return wglMakeCurrent(0, 0) == TRUE;
}

#elif defined(SP_PLATFORM_LINUX)

OpenGLSharedRenderContext::OpenGLSharedRenderContext(OpenGLRenderContext* RootRenderContext) :
    RenderContext_  (0),
    Display_        (0),
    Window_         (0)
{
    if (RootRenderContext)
    {
        Display_    = RootRenderContext->Display_;
        Window_     = RootRenderContext->Window_;
        
        /* Create OpenGL render context */
        RenderContext_ = glXCreateContext(Display_, Visual_, 0, GL_TRUE);
        
        if (!RenderContext_)
            io::Log::error(GLCONTEXT_ERROR_CREATE);
    }
}
OpenGLSharedRenderContext::~OpenGLSharedRenderContext()
{
    /* Delete OpenGL render context */
    if (Display_ && RenderContext_)
        glXDestroyContext(Display_, RenderContext_);
}

bool OpenGLRenderContext::activate()
{
    return glXMakeCurrent(Display_, Window_, RenderContext_);
}
bool OpenGLRenderContext::deactivate()
{
    return glXMakeCurrent(Display_, None, 0);
}

#endif


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
