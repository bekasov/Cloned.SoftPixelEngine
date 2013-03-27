/*
 * OpenGL extensions header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENGLFUNCTIONSARB_H__
#define __SP_OPENGLFUNCTIONSARB_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#elif defined(SP_PLATFORM_LINUX)
#   include <sys/utsname.h>
#endif

#if defined(SP_PLATFORM_MACOSX)
#   include <OpenGL/gl.h>
#   include <OpenGL/glext.h>
#else
#   include <GL/gl.h>
#   include <GL/glext.h>
#endif

#if defined(SP_PLATFORM_WINDOWS)
#   include <GL/wglext.h>
#elif defined(SP_PLATFORM_LINUX)
#   include <GL/glx.h>
#endif


namespace sp
{


/* ARB balance macros between OpenGL and OpenGL|ES */

#define glGetProgramiv      glGetObjectParameterivARB
#define glGetShaderiv       glGetObjectParameterivARB
#define glGetProgramInfoLog glGetInfoLogARB
#define glGetShaderInfoLog  glGetInfoLogARB

/* Extended type definitions */

#if defined(SP_PLATFORM_WINDOWS)
typedef BOOL (APIENTRY * PFNWGLSWAPINTERVALFARPROC)(int);
#endif

typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC)    (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint* buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC)    (GLsizei n, GLuint* buffers);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC)    (GLenum target, int size, const GLvoid* data, GLenum usage);

/* Function pointer (OpenGL extensions & ARB functions) */

// OpenGL extendet functions -> (glext.h)
// Multitexturing: GL_TEXTURE0_ARB - GL_TEXTURE31_ARB

#if defined(SP_PLATFORM_WINDOWS)

extern PFNWGLCHOOSEPIXELFORMATARBPROC       wglChoosePixelFormatARB;
extern PFNWGLSWAPINTERVALFARPROC            wglSwapIntervalEXT;

#elif defined(SP_PLATFORM_LINUX)

extern PFNGLXSWAPINTERVALSGIPROC            glXSwapIntervalSGI;

#endif

extern PFNGLMULTITEXCOORD2FARBPROC          glMultiTexCoord2fARB;
extern PFNGLACTIVETEXTUREARBPROC            glActiveTextureARB;
//extern PFNGLACTIVESTENCILFACEEXTPROC        glActiveStencilFaceEXT;
extern PFNGLTEXIMAGE3DEXTPROC               glTexImage3DEXT;
extern PFNGLTEXSUBIMAGE3DEXTPROC            glTexSubImage3DEXT;

/* Vertex buffer objects extension pointers */

extern PFNGLCLIENTACTIVETEXTUREARBPROC      glClientActiveTextureARB;
extern PFNGLFOGCOORDPOINTERPROC             glFogCoordPointer;

extern PFNGLGENBUFFERSARBPROC               glGenBuffersARB;
extern PFNGLBINDBUFFERARBPROC               glBindBufferARB;
extern PFNGLBUFFERDATAARBPROC               glBufferDataARB;
extern PFNGLBUFFERSUBDATAARBPROC            glBufferSubDataARB;
extern PFNGLDELETEBUFFERSARBPROC            glDeleteBuffersARB;

/* Objects for "GL_ARB_draw_instanced" extensions */

extern PFNGLDRAWELEMENTSINSTANCEDARBPROC    glDrawElementsInstancedARB;
extern PFNGLDRAWARRAYSINSTANCEDARBPROC      glDrawArraysInstancedARB;

/* Frame buffer objects extension pointers */

extern PFNGLGENFRAMEBUFFERSEXTPROC                  glGenFramebuffersEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC                 glGenRenderbuffersEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC               glDeleteFramebuffersEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC              glDeleteRenderbuffersEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC                  glBindFramebufferEXT;
extern PFNGLBINDRENDERBUFFEREXTPROC                 glBindRenderbufferEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC          glFramebufferRenderbufferEXT;
extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC             glFramebufferTexture1DEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC             glFramebufferTexture2DEXT;
extern PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC          glFramebufferTextureLayerEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC              glRenderbufferStorageEXT;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC   glRenderbufferStorageMultisampleEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC           glCheckFramebufferStatusEXT;
extern PFNGLBLITFRAMEBUFFEREXTPROC                  glBlitFramebufferEXT;
extern PFNGLGENERATEMIPMAPEXTPROC                   glGenerateMipmapEXT;

/* OpenGL shading language (GLSL) extension pointers */

extern PFNGLBINDPROGRAMARBPROC              glBindProgramARB;
extern PFNGLPROGRAMSTRINGARBPROC            glProgramStringARB;
extern PFNGLGENPROGRAMSARBPROC              glGenProgramsARB;
extern PFNGLDELETEPROGRAMSARBPROC           glDeleteProgramsARB;
extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB;
extern PFNGLDRAWBUFFERSARBPROC              glDrawBuffersARB;

extern PFNGLDELETEPROGRAMPROC               glDeleteProgram;
extern PFNGLDELETESHADERPROC                glDeleteShader;
extern PFNGLCREATEPROGRAMOBJECTARBPROC      glCreateProgramObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC       glCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC             glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC            glCompileShaderARB;
extern PFNGLATTACHOBJECTARBPROC             glAttachObjectARB;
extern PFNGLDELETEOBJECTARBPROC             glDeleteObjectARB;
extern PFNGLLINKPROGRAMARBPROC              glLinkProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC         glUseProgramObjectARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC     glGetObjectParameterivARB;
extern PFNGLGETINFOLOGARBPROC               glGetInfoLogARB;
extern PFNGLDETACHOBJECTARBPROC             glDetachObjectARB;
extern PFNGLGETACTIVEUNIFORMARBPROC         glGetActiveUniformARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC       glGetUniformLocationARB;
extern PFNGLUNIFORM1FARBPROC                glUniform1fARB;
extern PFNGLUNIFORM3FARBPROC                glUniform3fARB;
extern PFNGLUNIFORM4FARBPROC                glUniform4fARB;
extern PFNGLUNIFORM1IARBPROC                glUniform1iARB;
extern PFNGLUNIFORM1IVARBPROC               glUniform1ivARB;
extern PFNGLUNIFORM1FVARBPROC               glUniform1fvARB;
extern PFNGLUNIFORM2FVARBPROC               glUniform2fvARB;
extern PFNGLUNIFORM3FVARBPROC               glUniform3fvARB;
extern PFNGLUNIFORM4FVARBPROC               glUniform4fvARB;
extern PFNGLUNIFORMMATRIX2FVARBPROC         glUniformMatrix2fvARB;
extern PFNGLUNIFORMMATRIX3FVARBPROC         glUniformMatrix3fvARB;
extern PFNGLUNIFORMMATRIX4FVARBPROC         glUniformMatrix4fvARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glEnableVertexAttribArrayARB;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
extern PFNGLVERTEXATTRIBPOINTERARBPROC      glVertexAttribPointerARB;
extern PFNGLBINDATTRIBLOCATIONARBPROC       glBindAttribLocationARB;
extern PFNGLBINDFRAGDATALOCATIONEXTPROC     glBindFragDataLocationEXT;

extern PFNGLPROGRAMPARAMETERIEXTPROC        glProgramParameteriEXT;

#ifdef GL_ARB_tessellation_shader
extern PFNGLPATCHPARAMETERIPROC             glPatchParameteriARB;
extern PFNGLPATCHPARAMETERFVPROC            glPatchParameterfvARB;
#endif

#ifndef GL_GLEXT_PROTOTYPES

extern PFNGLGETUNIFORMBLOCKINDEXPROC        glGetUniformBlockIndex;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC     glGetActiveUniformBlockiv;
extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC   glGetActiveUniformBlockName;
extern PFNGLUNIFORMBLOCKBINDINGPROC         glUniformBlockBinding;

#endif


} // /namespace sp


#endif

#endif



// ================================================================================
