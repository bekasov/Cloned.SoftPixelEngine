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

#include <map>
#include <string>


namespace sp
{


extern bool GlbGLCoreProfile;

/* ARB balance macros between OpenGL and OpenGL|ES */
#define glGetProgramiv      glGetObjectParameterivARB
#define glGetShaderiv       glGetObjectParameterivARB
#define glGetProgramInfoLog glGetInfoLogARB
#define glGetShaderInfoLog  glGetInfoLogARB

/* Extended type definitions */
#if defined(SP_PLATFORM_WINDOWS)
typedef BOOL (APIENTRY * PFNWGLSWAPINTERVALFARPROC)(int);
#endif

/* Function pointer (OpenGL extensions & ARB functions) */
#if defined(SP_PLATFORM_WINDOWS)
extern PFNWGLCHOOSEPIXELFORMATARBPROC               wglChoosePixelFormatARB;
extern PFNWGLSWAPINTERVALFARPROC                    wglSwapIntervalEXT;
#   ifdef WGL_ARB_create_context
extern PFNWGLCREATECONTEXTATTRIBSARBPROC            wglCreateContextAttribsARB;
#   endif
#   ifdef WGL_ARB_extensions_string
extern PFNWGLGETEXTENSIONSSTRINGARBPROC             wglGetExtensionsStringARB;
#   endif
#elif defined(SP_PLATFORM_LINUX)
extern PFNGLXSWAPINTERVALSGIPROC                    glXSwapIntervalSGI;
#endif

#if defined(GL_VERSION_3_0) && !defined(GL_GLEXT_PROTOTYPES)
extern PFNGLGETSTRINGIPROC                          glGetStringi;
#endif

/* Multi-texturing procedures */
extern PFNGLMULTITEXCOORD2FARBPROC                  glMultiTexCoord2fARB;
extern PFNGLACTIVETEXTUREARBPROC                    glActiveTextureARB;
//extern PFNGLACTIVESTENCILFACEEXTPROC                glActiveStencilFaceEXT;
extern PFNGLTEXIMAGE3DEXTPROC                       glTexImage3DEXT;
extern PFNGLTEXSUBIMAGE3DEXTPROC                    glTexSubImage3DEXT;

/* Vertex buffer object (VBO) extension procedures */
extern PFNGLCLIENTACTIVETEXTUREARBPROC              glClientActiveTextureARB;
extern PFNGLFOGCOORDPOINTERPROC                     glFogCoordPointer;

extern PFNGLGENBUFFERSARBPROC                       glGenBuffersARB;
extern PFNGLBINDBUFFERARBPROC                       glBindBufferARB;
extern PFNGLBUFFERDATAARBPROC                       glBufferDataARB;
extern PFNGLBUFFERSUBDATAARBPROC                    glBufferSubDataARB;
extern PFNGLDELETEBUFFERSARBPROC                    glDeleteBuffersARB;

/* Objects for "GL_ARB_draw_instanced" extensions */
extern PFNGLDRAWELEMENTSINSTANCEDARBPROC            glDrawElementsInstancedARB;
extern PFNGLDRAWARRAYSINSTANCEDARBPROC              glDrawArraysInstancedARB;

/* Frame buffer objects (FBO) extension procedures */
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

/* OpenGL shading language (GLSL) extension procedures */
extern PFNGLBINDPROGRAMARBPROC                      glBindProgramARB;
extern PFNGLPROGRAMSTRINGARBPROC                    glProgramStringARB;
extern PFNGLGENPROGRAMSARBPROC                      glGenProgramsARB;
extern PFNGLDELETEPROGRAMSARBPROC                   glDeleteProgramsARB;
extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC         glProgramLocalParameter4fvARB;
extern PFNGLDRAWBUFFERSARBPROC                      glDrawBuffersARB;

extern PFNGLDELETEPROGRAMPROC                       glDeleteProgram;
extern PFNGLDELETESHADERPROC                        glDeleteShader;
extern PFNGLCREATEPROGRAMOBJECTARBPROC              glCreateProgramObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC               glCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC                     glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC                    glCompileShaderARB;
extern PFNGLATTACHOBJECTARBPROC                     glAttachObjectARB;
extern PFNGLDELETEOBJECTARBPROC                     glDeleteObjectARB;
extern PFNGLLINKPROGRAMARBPROC                      glLinkProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC                 glUseProgramObjectARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC             glGetObjectParameterivARB;
extern PFNGLGETINFOLOGARBPROC                       glGetInfoLogARB;
extern PFNGLDETACHOBJECTARBPROC                     glDetachObjectARB;
extern PFNGLGETACTIVEUNIFORMARBPROC                 glGetActiveUniformARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC               glGetUniformLocationARB;
extern PFNGLUNIFORM1FARBPROC                        glUniform1fARB;
extern PFNGLUNIFORM3FARBPROC                        glUniform3fARB;
extern PFNGLUNIFORM4FARBPROC                        glUniform4fARB;
extern PFNGLUNIFORM1IARBPROC                        glUniform1iARB;
extern PFNGLUNIFORM1IVARBPROC                       glUniform1ivARB;
extern PFNGLUNIFORM1FVARBPROC                       glUniform1fvARB;
extern PFNGLUNIFORM2FVARBPROC                       glUniform2fvARB;
extern PFNGLUNIFORM3FVARBPROC                       glUniform3fvARB;
extern PFNGLUNIFORM4FVARBPROC                       glUniform4fvARB;
extern PFNGLUNIFORMMATRIX2FVARBPROC                 glUniformMatrix2fvARB;
extern PFNGLUNIFORMMATRIX3FVARBPROC                 glUniformMatrix3fvARB;
extern PFNGLUNIFORMMATRIX4FVARBPROC                 glUniformMatrix4fvARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC          glEnableVertexAttribArrayARB;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC         glDisableVertexAttribArrayARB;
extern PFNGLVERTEXATTRIBPOINTERARBPROC              glVertexAttribPointerARB;
extern PFNGLBINDATTRIBLOCATIONARBPROC               glBindAttribLocationARB;
extern PFNGLBINDFRAGDATALOCATIONEXTPROC             glBindFragDataLocationEXT;

extern PFNGLPROGRAMPARAMETERIEXTPROC                glProgramParameteriEXT;

/* Tessellation extension procedures */
#ifdef GL_ARB_tessellation_shader
extern PFNGLPATCHPARAMETERIPROC                     glPatchParameteriARB;
extern PFNGLPATCHPARAMETERFVPROC                    glPatchParameterfvARB;
#endif

/* Compute shader extension procedures */
#ifdef GL_ARB_compute_shader
extern PFNGLDISPATCHCOMPUTEPROC                     glDispatchCompute;
extern PFNGLBINDIMAGETEXTUREPROC                    glBindImageTexture;
#endif

/* Shader storage buffer objects (SSBO) extension procedures */
#if defined(GL_ARB_shader_storage_buffer_object) && defined(GL_ARB_shader_atomic_counters)
extern PFNGLSHADERSTORAGEBLOCKBINDINGPROC           glShaderStorageBlockBinding;
extern PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC      glGetActiveAtomicCounterBufferiv;
#endif

#ifndef GL_GLEXT_PROTOTYPES

extern PFNGLBINDBUFFERBASEPROC                      glBindBufferBase;
extern PFNGLTEXBUFFERPROC                           glTexBuffer;

/* Uniform buffer objects (UBO) extension procedures */
extern PFNGLGETUNIFORMBLOCKINDEXPROC                glGetUniformBlockIndex;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC             glGetActiveUniformBlockiv;
extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC           glGetActiveUniformBlockName;
extern PFNGLUNIFORMBLOCKBINDINGPROC                 glUniformBlockBinding;

/* Query objects */
extern PFNGLGENQUERIESPROC                          glGenQueries;
extern PFNGLDELETEQUERIESPROC                       glDeleteQueries;
extern PFNGLBEGINQUERYPROC                          glBeginQuery;
extern PFNGLENDQUERYPROC                            glEndQuery;
extern PFNGLGETQUERYOBJECTIVPROC                    glGetQueryObjectiv;
extern PFNGLGETQUERYOBJECTUIVPROC                   glGetQueryObjectuiv;

#endif

namespace GLExtensionLoader
{

void filterExtensionStrings(std::map<std::string, bool> &ExtMap);

bool loadSwapIntervalProcs();
bool loadPixelFormatProcs();
bool loadCreateContextProcs();
bool loadMultiTextureProcs();
bool loadVBOProcs();
bool loadFBOProcs();
bool loadFBOMultiSampledProcs();
bool loadSSBOProcs();
bool loadDrawInstancedProcs();
bool loadShaderProcs();
bool loadShaderConstBufferProcs();
bool loadGeometryShaderProcs();
bool loadTessellationShaderProcs();
bool loadComputeShaderProcs();
bool loadFogCoordProcs();
bool loadTex3DProcs();
bool loadQueryObjectProcs();

} // /namesapce GLExtensionLoader


} // /namespace sp


#endif

#endif



// ================================================================================
