/*
 * OpenGL core header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENGL_COREHEADER_H__
#define __SP_OPENGL_COREHEADER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_OPENGL
#   if defined(SP_PLATFORM_MACOSX)
#       include <OpenGL/gl.h>
#       include <OpenGL/glext.h>
#   else
#       include <GL/gl.h>
#       include <GL/glext.h>
#   endif
#endif

#ifdef SP_COMPILE_WITH_OPENGLES1
#   ifdef SP_PLATFORM_IOS
#       include <OpenGLES/ES1/gl.h>
#       include <OpenGLES/ES1/glext.h>
#   else
#       include <EGL/egl.h>
#       include <GLES/gl.h>
#       include <GLES/glext.h>
#   endif
#endif

#ifdef SP_COMPILE_WITH_OPENGLES2
#   ifdef SP_PLATFORM_IOS
#       include <OpenGLES/ES2/gl.h>
#       include <OpenGLES/ES2/glext.h>
#   else
#       include <EGL/egl.h>
#       include <GLES2/gl2.h>
#       include <GLES2/gl2ext.h>
#   endif
#endif


/* === Macros === */

#define ChangeRenderStateForEachContext(c)                                      \
    if (ContextList_.size() > 1)                                                \
    {                                                                           \
        RenderContext* PrevContext = RenderContext::getActiveRenderContext();   \
        foreach (RenderContext* Context, ContextList_)                          \
        {                                                                       \
            Context->activate();                                                \
            c                                                                   \
        }                                                                       \
        if (PrevContext)                                                        \
            PrevContext->activate();                                            \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        c                                                                       \
    }


#endif



// ================================================================================
