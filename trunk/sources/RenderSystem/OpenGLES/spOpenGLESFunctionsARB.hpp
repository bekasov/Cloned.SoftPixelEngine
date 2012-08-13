/*
 * OpenGL|ES macros header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENGLESMACROS_H__
#define __SP_OPENGLESMACROS_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGLES1) || defined(SP_COMPILE_WITH_OPENGLES2)


/* === Define ARB macros === */

#ifndef GL_MAX_TEXTURE_UNITS_ARB
#   define GL_MAX_TEXTURE_UNITS_ARB     GL_MAX_TEXTURE_UNITS
#endif

#ifndef GL_ARRAY_BUFFER_ARB
#   define GL_ARRAY_BUFFER_ARB          GL_ARRAY_BUFFER
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER_ARB
#   define GL_ELEMENT_ARRAY_BUFFER_ARB  GL_ELEMENT_ARRAY_BUFFER
#endif
#ifndef GL_STATIC_DRAW_ARB
#   define GL_STATIC_DRAW_ARB           GL_STATIC_DRAW
#endif
#ifndef GL_DYNAMIC_DRAW_ARB
#   define GL_DYNAMIC_DRAW_ARB          GL_DYNAMIC_DRAW
#endif

#ifndef GL_VERTEX_SHADER_ARB
#   define GL_VERTEX_SHADER_ARB         GL_VERTEX_SHADER
#endif
#ifndef GL_FRAGMENT_SHADER_ARB
#   define GL_FRAGMENT_SHADER_ARB       GL_FRAGMENT_SHADER
#endif

#ifndef GL_RENDERBUFFER_EXT
#   define GL_RENDERBUFFER_EXT                              GL_RENDERBUFFER
#endif
#ifndef GL_DEPTH_ATTACHMENT_EXT
#   define GL_DEPTH_ATTACHMENT_EXT                          GL_DEPTH_ATTACHMENT
#endif
#ifndef GL_COLOR_ATTACHMENT0_EXT
#   define GL_COLOR_ATTACHMENT0_EXT                         GL_COLOR_ATTACHMENT0
#endif
#ifndef GL_FRAMEBUFFER_EXT
#   define GL_FRAMEBUFFER_EXT                               GL_FRAMEBUFFER
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT
#   define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT
#   define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT         GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
#   define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT         GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
#endif
#ifndef GL_FRAMEBUFFER_UNSUPPORTED_EXT
#   define GL_FRAMEBUFFER_UNSUPPORTED_EXT                   GL_FRAMEBUFFER_UNSUPPORTED
#endif
#ifndef GL_FRAMEBUFFER_COMPLETE_EXT
#   define GL_FRAMEBUFFER_COMPLETE_EXT                      GL_FRAMEBUFFER_COMPLETE
#endif

#ifndef GL_FLOAT_VEC2_ARB
#   define GL_FLOAT_VEC2_ARB    GL_FLOAT_VEC2
#endif
#ifndef GL_FLOAT_VEC3_ARB
#   define GL_FLOAT_VEC3_ARB    GL_FLOAT_VEC3
#endif
#ifndef GL_FLOAT_VEC4_ARB
#   define GL_FLOAT_VEC4_ARB    GL_FLOAT_VEC4
#endif
#ifndef GL_FLOAT_MAT2_ARB
#   define GL_FLOAT_MAT2_ARB    GL_FLOAT_MAT2
#endif
#ifndef GL_FLOAT_MAT3_ARB
#   define GL_FLOAT_MAT3_ARB    GL_FLOAT_MAT3
#endif
#ifndef GL_FLOAT_MAT4_ARB
#   define GL_FLOAT_MAT4_ARB    GL_FLOAT_MAT4
#endif

#ifndef GL_OBJECT_LINK_STATUS_ARB
#   define GL_OBJECT_LINK_STATUS_ARB                GL_LINK_STATUS
#endif
#ifndef GL_OBJECT_COMPILE_STATUS_ARB
#   define GL_OBJECT_COMPILE_STATUS_ARB             GL_COMPILE_STATUS
#endif
#ifndef GL_OBJECT_INFO_LOG_LENGTH_ARB
#   define GL_OBJECT_INFO_LOG_LENGTH_ARB            GL_INFO_LOG_LENGTH
#endif
#ifndef GL_OBJECT_ACTIVE_UNIFORMS_ARB
#   define GL_OBJECT_ACTIVE_UNIFORMS_ARB            GL_ACTIVE_UNIFORMS
#endif
#ifndef GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB
#   define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB  GL_ACTIVE_UNIFORM_MAX_LENGTH
#endif


/* === Define OpenGL types */

typedef GLuint GLhandleARB;


/* === Define OpenGL functions === */

#define glFogi                          glFogx
#define glLighti                        glLightx
#define glDepthRange                    glDepthRangef

#define glClientActiveTextureARB        glClientActiveTexture
#define glActiveTextureARB              glActiveTexture
#define glGenerateMipmapEXT             glGenerateMipmap

#define glFramebufferRenderbufferEXT    glFramebufferRenderbuffer
#define glDeleteRenderbuffersEXT        glDeleteRenderbuffers
#define glDeleteFramebuffersEXT         glDeleteFramebuffers
#define glGenFramebuffersEXT            glGenFramebuffers
#define glGenRenderbuffersEXT           glGenRenderbuffers
#define glBindFramebufferEXT            glBindFramebuffer
#define glBindRenderbufferEXT           glBindRenderbuffer
#define glFramebufferTexture2DEXT       glFramebufferTexture2D
#define glRenderbufferStorageEXT        glRenderbufferStorage
#define glCheckFramebufferStatusEXT     glCheckFramebufferStatus

#define glBindBufferARB                 glBindBuffer
#define glGenBuffersARB                 glGenBuffers
#define glDeleteBuffersARB              glDeleteBuffers
#define glBufferDataARB                 glBufferData
#define glBufferSubDataARB              glBufferSubData
#define glClipPlane                     glClipPlanef

#define glCreateProgramObjectARB        glCreateProgram
#define glUseProgramObjectARB           glUseProgram
#define glLinkProgramARB                glLinkProgram
#define glAttachObjectARB               glAttachShader
#define glCreateShaderObjectARB         glCreateShader
#define glShaderSourceARB               glShaderSource
#define glCompileShaderARB              glCompileShader
#define glGetActiveUniformARB           glGetActiveUniform
#define glBindAttribLocationARB         glBindAttribLocation
#define glGetUniformLocationARB         glGetUniformLocation
#define glUniform1iARB                  glUniform1i
#define glUniform1ivARB                 glUniform1iv
#define glUniform1fARB                  glUniform1f
#define glUniform2fARB                  glUniform2f
#define glUniform3fARB                  glUniform3f
#define glUniform4fARB                  glUniform4f
#define glUniform1fvARB                 glUniform1fv
#define glUniform2fvARB                 glUniform2fv
#define glUniform3fvARB                 glUniform3fv
#define glUniform4fvARB                 glUniform4fv
#define glUniformMatrix2fvARB           glUniformMatrix2fv
#define glUniformMatrix3fvARB           glUniformMatrix3fv
#define glUniformMatrix4fvARB           glUniformMatrix4fv

#define glEnableVertexAttribArrayARB    glEnableVertexAttribArray
#define glDisableVertexAttribArrayARB   glDisableVertexAttribArray
#define glVertexAttribPointerARB        glVertexAttribPointer


#endif

#endif



// ================================================================================
