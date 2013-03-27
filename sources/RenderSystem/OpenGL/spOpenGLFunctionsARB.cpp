/*
 * OpenGL extensions file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"


#if defined(SP_COMPILE_WITH_OPENGL)


namespace sp
{


#if defined(SP_PLATFORM_WINDOWS)
PFNWGLCHOOSEPIXELFORMATARBPROC              wglChoosePixelFormatARB             = 0;
PFNWGLSWAPINTERVALFARPROC                   wglSwapIntervalEXT                  = 0;
#elif defined(SP_PLATFORM_LINUX)
PFNGLXSWAPINTERVALSGIPROC                   glXSwapIntervalSGI                  = 0;
#endif

PFNGLMULTITEXCOORD2FARBPROC                 glMultiTexCoord2fARB                = 0;
PFNGLACTIVETEXTUREARBPROC                   glActiveTextureARB                  = 0;
//PFNGLACTIVESTENCILFACEEXTPROC               glActiveStencilFaceEXT              = 0;
PFNGLTEXIMAGE3DEXTPROC                      glTexImage3DEXT                     = 0;
PFNGLTEXSUBIMAGE3DEXTPROC                   glTexSubImage3DEXT                  = 0;

/* Vertex buffer objects extension pointers */

PFNGLCLIENTACTIVETEXTUREARBPROC             glClientActiveTextureARB            = 0;
PFNGLFOGCOORDPOINTERPROC                    glFogCoordPointer                   = 0;

PFNGLGENBUFFERSARBPROC                      glGenBuffersARB                     = 0;
PFNGLBINDBUFFERARBPROC                      glBindBufferARB                     = 0;
PFNGLBUFFERDATAARBPROC                      glBufferDataARB                     = 0;
PFNGLBUFFERSUBDATAARBPROC                   glBufferSubDataARB                  = 0;
PFNGLDELETEBUFFERSARBPROC                   glDeleteBuffersARB                  = 0;

/* Objects for "GL_ARB_draw_instanced" extensions */

PFNGLDRAWELEMENTSINSTANCEDARBPROC           glDrawElementsInstancedARB          = 0;
PFNGLDRAWARRAYSINSTANCEDARBPROC             glDrawArraysInstancedARB            = 0;

/* Frame buffer objects extension pointers */

PFNGLGENFRAMEBUFFERSEXTPROC                 glGenFramebuffersEXT                = 0;
PFNGLGENRENDERBUFFERSEXTPROC                glGenRenderbuffersEXT               = 0;
PFNGLDELETEFRAMEBUFFERSEXTPROC              glDeleteFramebuffersEXT             = 0;
PFNGLDELETERENDERBUFFERSEXTPROC             glDeleteRenderbuffersEXT            = 0;
PFNGLBINDFRAMEBUFFEREXTPROC                 glBindFramebufferEXT                = 0;
PFNGLBINDRENDERBUFFEREXTPROC                glBindRenderbufferEXT               = 0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC         glFramebufferRenderbufferEXT        = 0;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC            glFramebufferTexture1DEXT           = 0;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC            glFramebufferTexture2DEXT           = 0;
PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC         glFramebufferTextureLayerEXT        = 0;
PFNGLRENDERBUFFERSTORAGEEXTPROC             glRenderbufferStorageEXT            = 0;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC  glRenderbufferStorageMultisampleEXT = 0;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC          glCheckFramebufferStatusEXT         = 0;
PFNGLBLITFRAMEBUFFEREXTPROC                 glBlitFramebufferEXT                = 0;
PFNGLGENERATEMIPMAPEXTPROC                  glGenerateMipmapEXT                 = 0;

/* OpenGL shading language (GLSL) extension pointers */

PFNGLBINDPROGRAMARBPROC                     glBindProgramARB                    = 0;
PFNGLPROGRAMSTRINGARBPROC                   glProgramStringARB                  = 0;
PFNGLGENPROGRAMSARBPROC                     glGenProgramsARB                    = 0;
PFNGLDELETEPROGRAMSARBPROC                  glDeleteProgramsARB                 = 0;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC        glProgramLocalParameter4fvARB       = 0;
PFNGLDRAWBUFFERSARBPROC                     glDrawBuffersARB                    = 0;

PFNGLDELETEPROGRAMPROC                      glDeleteProgram                     = 0;
PFNGLDELETESHADERPROC                       glDeleteShader                      = 0;
PFNGLCREATEPROGRAMOBJECTARBPROC             glCreateProgramObjectARB            = 0;
PFNGLCREATESHADEROBJECTARBPROC              glCreateShaderObjectARB             = 0;
PFNGLSHADERSOURCEARBPROC                    glShaderSourceARB                   = 0;
PFNGLCOMPILESHADERARBPROC                   glCompileShaderARB                  = 0;
PFNGLATTACHOBJECTARBPROC                    glAttachObjectARB                   = 0;
PFNGLDELETEOBJECTARBPROC                    glDeleteObjectARB                   = 0;
PFNGLLINKPROGRAMARBPROC                     glLinkProgramARB                    = 0;
PFNGLUSEPROGRAMOBJECTARBPROC                glUseProgramObjectARB               = 0;
PFNGLGETOBJECTPARAMETERIVARBPROC            glGetObjectParameterivARB           = 0;
PFNGLGETINFOLOGARBPROC                      glGetInfoLogARB                     = 0;
PFNGLDETACHOBJECTARBPROC                    glDetachObjectARB                   = 0;
PFNGLGETACTIVEUNIFORMARBPROC                glGetActiveUniformARB               = 0;
PFNGLGETUNIFORMLOCATIONARBPROC              glGetUniformLocationARB             = 0;
PFNGLUNIFORM1FARBPROC                       glUniform1fARB                      = 0;
PFNGLUNIFORM3FARBPROC                       glUniform3fARB                      = 0;
PFNGLUNIFORM4FARBPROC                       glUniform4fARB                      = 0;
PFNGLUNIFORM1IARBPROC                       glUniform1iARB                      = 0;
PFNGLUNIFORM1IVARBPROC                      glUniform1ivARB                     = 0;
PFNGLUNIFORM1FVARBPROC                      glUniform1fvARB                     = 0;
PFNGLUNIFORM2FVARBPROC                      glUniform2fvARB                     = 0;
PFNGLUNIFORM3FVARBPROC                      glUniform3fvARB                     = 0;
PFNGLUNIFORM4FVARBPROC                      glUniform4fvARB                     = 0;
PFNGLUNIFORMMATRIX2FVARBPROC                glUniformMatrix2fvARB               = 0;
PFNGLUNIFORMMATRIX3FVARBPROC                glUniformMatrix3fvARB               = 0;
PFNGLUNIFORMMATRIX4FVARBPROC                glUniformMatrix4fvARB               = 0;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC         glEnableVertexAttribArrayARB        = 0;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC        glDisableVertexAttribArrayARB       = 0;
PFNGLVERTEXATTRIBPOINTERARBPROC             glVertexAttribPointerARB            = 0;
PFNGLBINDATTRIBLOCATIONARBPROC              glBindAttribLocationARB             = 0;
PFNGLBINDFRAGDATALOCATIONEXTPROC            glBindFragDataLocationEXT           = 0;

PFNGLPROGRAMPARAMETERIEXTPROC               glProgramParameteriEXT              = 0;

#ifdef GL_ARB_tessellation_shader
PFNGLPATCHPARAMETERIPROC                    glPatchParameteriARB                = 0;
PFNGLPATCHPARAMETERFVPROC                   glPatchParameterfvARB               = 0;
#endif

#ifndef GL_GLEXT_PROTOTYPES

PFNGLBINDBUFFERBASEPROC                     glBindBufferBase                    = 0;

PFNGLGETUNIFORMBLOCKINDEXPROC               glGetUniformBlockIndex              = 0;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC            glGetActiveUniformBlockiv           = 0;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC          glGetActiveUniformBlockName         = 0;
PFNGLUNIFORMBLOCKBINDINGPROC                glUniformBlockBinding               = 0;

#endif


} // /namespace sp


#endif



// ================================================================================
