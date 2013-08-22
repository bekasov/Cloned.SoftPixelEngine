/*
 * OpenGL extensions file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"
#include "Base/spInputOutputLog.hpp"


#if defined(SP_COMPILE_WITH_OPENGL)


namespace sp
{


/*
 * Global OpenGL extension procedure objects
 */

bool GlbGLCoreProfile = false;

#if defined(SP_PLATFORM_WINDOWS)
PFNWGLCHOOSEPIXELFORMATARBPROC              wglChoosePixelFormatARB             = 0;
PFNWGLSWAPINTERVALFARPROC                   wglSwapIntervalEXT                  = 0;
#   ifdef WGL_ARB_create_context
PFNWGLCREATECONTEXTATTRIBSARBPROC           wglCreateContextAttribsARB          = 0;
#   endif
#   ifdef WGL_ARB_extensions_string
PFNWGLGETEXTENSIONSSTRINGARBPROC            wglGetExtensionsStringARB           = 0;
#   endif
#elif defined(SP_PLATFORM_LINUX)
PFNGLXSWAPINTERVALSGIPROC                   glXSwapIntervalSGI                  = 0;
#endif

#if defined(GL_VERSION_3_0) && !defined(GL_GLEXT_PROTOTYPES)
PFNGLGETSTRINGIPROC                         glGetStringi                        = 0;
#endif

/* Multi-texturing procedures */
PFNGLMULTITEXCOORD2FARBPROC                 glMultiTexCoord2fARB                = 0;
PFNGLACTIVETEXTUREARBPROC                   glActiveTextureARB                  = 0;
//PFNGLACTIVESTENCILFACEEXTPROC               glActiveStencilFaceEXT              = 0;
PFNGLTEXIMAGE3DEXTPROC                      glTexImage3DEXT                     = 0;
PFNGLTEXSUBIMAGE3DEXTPROC                   glTexSubImage3DEXT                  = 0;
PFNGLCLIENTACTIVETEXTUREARBPROC             glClientActiveTextureARB            = 0;

PFNGLFOGCOORDPOINTERPROC                    glFogCoordPointer                   = 0;

/* Vertex buffer object (VBO) extension procedures */
PFNGLGENBUFFERSARBPROC                      glGenBuffersARB                     = 0;
PFNGLBINDBUFFERARBPROC                      glBindBufferARB                     = 0;
PFNGLBUFFERDATAARBPROC                      glBufferDataARB                     = 0;
PFNGLBUFFERSUBDATAARBPROC                   glBufferSubDataARB                  = 0;
PFNGLDELETEBUFFERSARBPROC                   glDeleteBuffersARB                  = 0;

/* Objects for "GL_ARB_draw_instanced" extensions */
PFNGLDRAWELEMENTSINSTANCEDARBPROC           glDrawElementsInstancedARB          = 0;
PFNGLDRAWARRAYSINSTANCEDARBPROC             glDrawArraysInstancedARB            = 0;

/* Frame buffer objects (FBO) extension procedures */
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

/* OpenGL shading language (GLSL) extension procedures */
PFNGLBINDPROGRAMARBPROC                     glBindProgramARB                    = 0;
PFNGLPROGRAMSTRINGARBPROC                   glProgramStringARB                  = 0;
PFNGLGENPROGRAMSARBPROC                     glGenProgramsARB                    = 0;
PFNGLDELETEPROGRAMSARBPROC                  glDeleteProgramsARB                 = 0;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC        glProgramLocalParameter4fvARB       = 0;
PFNGLDRAWBUFFERSARBPROC                     glDrawBuffersARB                    = 0;

PFNGLDELETEPROGRAMPROC                      glDeleteProgram                     = 0;
PFNGLDELETESHADERPROC                       glDeleteShader                      = 0;
//PFNGLCREATEPROGRAMPROC                      glCreateProgram                     = 0;
//PFNGLCREATESHADERPROC                       glCreateShader                      = 0;
PFNGLCREATEPROGRAMOBJECTARBPROC             glCreateProgramObjectARB            = 0;
PFNGLCREATESHADEROBJECTARBPROC              glCreateShaderObjectARB             = 0;
PFNGLSHADERSOURCEARBPROC                    glShaderSourceARB                   = 0;
PFNGLCOMPILESHADERARBPROC                   glCompileShaderARB                  = 0;
PFNGLATTACHOBJECTARBPROC                    glAttachObjectARB                   = 0;
PFNGLDELETEOBJECTARBPROC                    glDeleteObjectARB                   = 0;
PFNGLLINKPROGRAMARBPROC                     glLinkProgramARB                    = 0;
//PFNGLUSEPROGRAMPROC                         glUseProgram                        = 0;
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

/* Tessellation extension procedures */
#ifdef GL_ARB_tessellation_shader
PFNGLPATCHPARAMETERIPROC                    glPatchParameteriARB                = 0;
PFNGLPATCHPARAMETERFVPROC                   glPatchParameterfvARB               = 0;
#endif

/* Compute shader extension procedures */
#ifdef GL_ARB_compute_shader
PFNGLDISPATCHCOMPUTEPROC                    glDispatchCompute                   = 0;
PFNGLBINDIMAGETEXTUREPROC                   glBindImageTexture                  = 0;
#endif

/* Shader storage buffer objects (SSBO) extension procedures */
#if defined(GL_ARB_shader_storage_buffer_object) && defined(GL_ARB_shader_atomic_counters)
PFNGLSHADERSTORAGEBLOCKBINDINGPROC          glShaderStorageBlockBinding         = 0;
PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC     glGetActiveAtomicCounterBufferiv    = 0;
#endif

#ifndef GL_GLEXT_PROTOTYPES

PFNGLBINDBUFFERBASEPROC                     glBindBufferBase                    = 0;
PFNGLTEXBUFFERPROC                          glTexBuffer                         = 0;

/* Uniform buffer objects (UBO) extension procedures */
PFNGLGETUNIFORMBLOCKINDEXPROC               glGetUniformBlockIndex              = 0;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC            glGetActiveUniformBlockiv           = 0;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC          glGetActiveUniformBlockName         = 0;
PFNGLUNIFORMBLOCKBINDINGPROC                glUniformBlockBinding               = 0;

#endif


/*
 * OpenGL Extension Loader
 */

namespace GLExtensionLoader
{

template <typename T> inline bool loadGLProc(T &GLProc, const c8* ProcName)
{
    /* Load OpenGL procedure address */
    #if defined(SP_PLATFORM_WINDOWS)
    GLProc = (T)wglGetProcAddress(ProcName);
    #elif defined(SP_PLATFORM_LINUX)
    GLProc = (T)glXGetProcAddress(reinterpret_cast<const GLubyte*>(ProcName))
    #else
    io::Log::error("OS not supported for loading OpenGL extensions");
    return false;
    #endif
    
    /* Check for errors */
    if (!GLProc)
    {
        io::Log::error("Could not load OpenGL function \"" + io::stringc(ProcName) + "\"");
        return false;
    }
    
    return true;
}

static void filterExtensionsFromString(std::map<std::string, bool> &ExtMap, const std::string &ExtString)
{
    size_t First = 0, Last = 0;
    
    /* Find next extension name in string */
    while ( ( Last = ExtString.find(' ', First) ) != std::string::npos )
    {
        /* Store current extension name in hash-map */
        const std::string Name = ExtString.substr(First, Last - First);
        ExtMap[Name] = true;
        First = Last + 1;
    }
}

void filterExtensionStrings(std::map<std::string, bool> &ExtMap)
{
    const c8* ExtString = 0;
    
    /* Filter standard GL extensions */
    if (GlbGLCoreProfile)
    {
        #if defined(GL_VERSION_3_0) && !defined(GL_GLEXT_PROTOTYPES)
        
        if (glGetStringi || loadGLProc(glGetStringi, "glGetStringi"))
        {
            /* Get number of extensions */
            s32 NumExtensions = 0;
            glGetIntegerv(GL_NUM_EXTENSIONS, &NumExtensions);
            
            for (s32 i = 0; i < NumExtensions; ++i)
            {
                /* Get current extension string */
                ExtString = reinterpret_cast<const c8*>(glGetStringi(GL_EXTENSIONS, i));
                if (ExtString)
                    ExtMap[std::string(ExtString)] = true;
            }
        }
        
        #endif
    }
    else
    {
        /* Get complete extension string */
        ExtString = reinterpret_cast<const c8*>(glGetString(GL_EXTENSIONS));
        if (ExtString)
            filterExtensionsFromString(ExtMap, ExtString);
    }
    
    #if defined(SP_PLATFORM_WINDOWS) && defined(WGL_ARB_extensions_string)
    
    /* Filter Win32 related extensions */
    if (wglGetExtensionsStringARB || loadGLProc(wglGetExtensionsStringARB, "wglGetExtensionsStringARB"))
    {
        ExtString = wglGetExtensionsStringARB(wglGetCurrentDC());
        if (ExtString)
            filterExtensionsFromString(ExtMap, ExtString);
    }
    
    #endif
}

bool loadSwapIntervalProcs()
{
    return 
        #if defined(SP_PLATFORM_WINDOWS)
        loadGLProc(wglSwapIntervalEXT, "wglSwapIntervalEXT");
        #elif defined(SP_PLATFORM_LINUX)
        loadGLProc(glXSwapIntervalSGI, "glXSwapIntervalSGI");
        #endif
}

bool loadPixelFormatProcs()
{
    #if defined(SP_PLATFORM_WINDOWS)
    return GLExtensionLoader::loadGLProc(wglChoosePixelFormatARB, "wglChoosePixelFormatARB");
    #else
    return false;
    #endif
}

bool loadCreateContextProcs()
{
    #if defined(SP_PLATFORM_WINDOWS) && defined(WGL_ARB_create_context)
    return GLExtensionLoader::loadGLProc(wglCreateContextAttribsARB, "wglCreateContextAttribsARB");
    #else
    return false;
    #endif
}

bool loadMultiTextureProcs()
{
    return
        loadGLProc(glActiveTextureARB,          "glActiveTextureARB"        ) &&
        loadGLProc(glMultiTexCoord2fARB,        "glMultiTexCoord2fARB"      ) &&
        loadGLProc(glClientActiveTextureARB,    "glClientActiveTextureARB"  );
}

bool loadVBOProcs()
{
    return
        loadGLProc(glGenBuffersARB,     "glGenBuffersARB"   ) &&
        loadGLProc(glBindBufferARB,     "glBindBufferARB"   ) &&
        loadGLProc(glBufferDataARB,     "glBufferDataARB"   ) &&
        loadGLProc(glBufferSubDataARB,  "glBufferSubDataARB") &&
        loadGLProc(glDeleteBuffersARB,  "glDeleteBuffersARB");
}

bool loadFBOProcs()
{
    return
        loadGLProc(glGenFramebuffersEXT,            "glGenFramebuffersEXT"          ) &&
        loadGLProc(glGenRenderbuffersEXT,           "glGenRenderbuffersEXT"         ) &&
        loadGLProc(glDeleteFramebuffersEXT,         "glDeleteFramebuffersEXT"       ) &&
        loadGLProc(glDeleteRenderbuffersEXT,        "glDeleteRenderbuffersEXT"      ) &&
        loadGLProc(glBindFramebufferEXT,            "glBindFramebufferEXT"          ) &&
        loadGLProc(glBindRenderbufferEXT,           "glBindRenderbufferEXT"         ) &&
        loadGLProc(glFramebufferRenderbufferEXT,    "glFramebufferRenderbufferEXT"  ) &&
        loadGLProc(glFramebufferTexture1DEXT,       "glFramebufferTexture1DEXT"     ) &&
        loadGLProc(glFramebufferTexture2DEXT,       "glFramebufferTexture2DEXT"     ) &&
        loadGLProc(glFramebufferTextureLayerEXT,    "glFramebufferTextureLayerEXT"  ) &&
        loadGLProc(glRenderbufferStorageEXT,        "glRenderbufferStorageEXT"      ) &&
        loadGLProc(glCheckFramebufferStatusEXT,     "glCheckFramebufferStatusEXT"   ) &&
        loadGLProc(glGenerateMipmapEXT,             "glGenerateMipmapEXT"           );
}

bool loadFBOMultiSampledProcs()
{
    return
        loadGLProc(glRenderbufferStorageMultisampleEXT, "glRenderbufferStorageMultisampleEXT"   ) &&
        loadGLProc(glBlitFramebufferEXT,                "glBlitFramebufferEXT"                  );
}

bool loadSSBOProcs()
{
    #if defined(GL_ARB_shader_storage_buffer_object) && defined(GL_ARB_shader_atomic_counters)
    return
        loadGLProc(glShaderStorageBlockBinding,         "glShaderStorageBlockBinding"       ) &&
        loadGLProc(glGetActiveAtomicCounterBufferiv,    "glGetActiveAtomicCounterBufferiv"  );
    #else
    return false;
    #endif
}

bool loadDrawInstancedProcs()
{
    return
        loadGLProc(glDrawElementsInstancedARB,  "glDrawElementsInstancedARB") &&
        loadGLProc(glDrawArraysInstancedARB,    "glDrawArraysInstancedARB"  );
}

bool loadShaderProcs()
{
    return
        loadGLProc(glBindProgramARB,                "glBindProgramARB"              ) &&
        loadGLProc(glProgramStringARB,              "glProgramStringARB"            ) &&
        loadGLProc(glGenProgramsARB,                "glGenProgramsARB"              ) &&
        loadGLProc(glDeleteProgramsARB,             "glDeleteProgramsARB"           ) &&
        loadGLProc(glProgramLocalParameter4fvARB,   "glProgramLocalParameter4fvARB" ) &&
        loadGLProc(glDrawBuffersARB,                "glDrawBuffersARB"              ) &&
        loadGLProc(glDeleteProgram,                 "glDeleteProgram"               ) &&
        loadGLProc(glDeleteShader,                  "glDeleteShader"                ) &&
        loadGLProc(glCreateProgramObjectARB,        "glCreateProgramObjectARB"      ) &&
        loadGLProc(glCreateShaderObjectARB,         "glCreateShaderObjectARB"       ) &&
        loadGLProc(glShaderSourceARB,               "glShaderSourceARB"             ) &&
        loadGLProc(glCompileShaderARB,              "glCompileShaderARB"            ) &&
        loadGLProc(glAttachObjectARB,               "glAttachObjectARB"             ) &&
        loadGLProc(glDeleteObjectARB,               "glDeleteObjectARB"             ) &&
        loadGLProc(glLinkProgramARB,                "glLinkProgramARB"              ) &&
        loadGLProc(glUseProgramObjectARB,           "glUseProgramObjectARB"         ) &&
        loadGLProc(glGetObjectParameterivARB,       "glGetObjectParameterivARB"     ) &&
        loadGLProc(glGetInfoLogARB,                 "glGetInfoLogARB"               ) &&
        loadGLProc(glDetachObjectARB,               "glDetachObjectARB"             ) &&
        loadGLProc(glGetActiveUniformARB,           "glGetActiveUniformARB"         ) &&
        loadGLProc(glGetUniformLocationARB,         "glGetUniformLocationARB"       ) &&
        loadGLProc(glUniform1fARB,                  "glUniform1fARB"                ) &&
        loadGLProc(glUniform3fARB,                  "glUniform3fARB"                ) &&
        loadGLProc(glUniform4fARB,                  "glUniform4fARB"                ) &&
        loadGLProc(glUniform1iARB,                  "glUniform1iARB"                ) &&
        loadGLProc(glUniform1ivARB,                 "glUniform1ivARB"               ) &&
        loadGLProc(glUniform1fvARB,                 "glUniform1fvARB"               ) &&
        loadGLProc(glUniform2fvARB,                 "glUniform2fvARB"               ) &&
        loadGLProc(glUniform3fvARB,                 "glUniform3fvARB"               ) &&
        loadGLProc(glUniform4fvARB,                 "glUniform4fvARB"               ) &&
        loadGLProc(glUniformMatrix2fvARB,           "glUniformMatrix2fvARB"         ) &&
        loadGLProc(glUniformMatrix3fvARB,           "glUniformMatrix3fvARB"         ) &&
        loadGLProc(glUniformMatrix4fvARB,           "glUniformMatrix4fvARB"         ) &&
        loadGLProc(glEnableVertexAttribArrayARB,    "glEnableVertexAttribArrayARB"  ) &&
        loadGLProc(glDisableVertexAttribArrayARB,   "glDisableVertexAttribArrayARB" ) &&
        loadGLProc(glVertexAttribPointerARB,        "glVertexAttribPointerARB"      ) &&
        loadGLProc(glBindAttribLocationARB,         "glBindAttribLocationARB"       ) &&
        loadGLProc(glBindFragDataLocationEXT,       "glBindFragDataLocationEXT"     );
}

bool loadShaderConstBufferProcs()
{
    #ifdef GL_GLEXT_PROTOTYPES
    return true;
    #else
    return
        loadGLProc(glBindBufferBase,            "glBindBufferBase"              ) &&
        loadGLProc(glTexBuffer,                 "glTexBuffer"                   ) &&
        loadGLProc(glGetUniformBlockIndex,      "glGetUniformBlockIndex"        ) &&
        loadGLProc(glGetActiveUniformBlockiv,   "glGetActiveUniformBlockiv"     ) &&
        loadGLProc(glGetActiveUniformBlockName, "glGetActiveUniformBlockName"   ) &&
        loadGLProc(glUniformBlockBinding,       "glUniformBlockBinding"         );
    #endif
}

bool loadGeometryShaderProcs()
{
    return loadGLProc(glProgramParameteriEXT, "glProgramParameteriEXT");
}

bool loadTessellationShaderProcs()
{
    #ifdef GL_ARB_tessellation_shader
    return
        loadGLProc(glPatchParameteriARB,    "glPatchParameteri" ) &&
        loadGLProc(glPatchParameterfvARB,   "glPatchParameterfv");
    #else
    return false;
    #endif
}

bool loadComputeShaderProcs()
{
    #ifdef GL_ARB_compute_shader
    return
        loadGLProc(glDispatchCompute,   "glDispatchCompute" ) &&
        loadGLProc(glBindImageTexture,  "glBindImageTexture");
    #else
    return false;
    #endif
}

bool loadFogCoordProcs()
{
    return loadGLProc(glFogCoordPointer, "glFogCoordPointer");
}

bool loadTex3DProcs()
{
    return
        loadGLProc(glTexImage3DEXT,     "glTexImage3DEXT"   ) &&
        loadGLProc(glTexSubImage3DEXT,  "glTexSubImage3DEXT");
}

} // /namespace GLExtensionLoader


} // /namespace sp


#endif



// ================================================================================
