/*
 * Deferred renderer shader setup file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/AdvancedRenderer/spDeferredRenderer.hpp"

#if defined(SP_COMPILE_WITH_ADVANCEDRENDERER)


#include "RenderSystem/AdvancedRenderer/spDeferredRendererShaderCallbacks.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "Base/spMathRandomizer.hpp"
#include "Base/spSharedObjects.hpp"


//!!!
#if ( defined(SP_DEBUGMODE) || 1 ) && 1
#   define _DEB_LOAD_SHADERS_FROM_FILES_
#   ifdef _DEB_LOAD_SHADERS_FROM_FILES_
#       define _DEB_SHADER_PATH_ io::stringc("../../sources/RenderSystem/AdvancedRenderer/")
#   endif
#endif


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


#define ISFLAG(n)       ((Config_.Flags & RENDERERFLAG_##n) != 0)
#define ISRENDERER(n)   (RenderSys_ == RENDERER_##n)
#define ADDOP(n)        Shader::addOption(CompilerOp, n)

static const c8* ERR_MSG_CG_NOTSUPPORTED = "Engine was not compiled with Cg Toolkit";
static const c8* ERR_MSG_CG_NOTPROVIDED = "Not fully provided Cg shaders for deferred renderer";


bool DeferredRenderer::loadGBufferShader()
{
    const bool IsGL = ISRENDERER(OPENGL);
    
    s32 Flags = 0;
    
    /* Setup shader compilation options */
    std::list<io::stringc> CompilerOp;
    setupGeometryCompilerOptions(CompilerOp);
    
    /* Setup g-buffer shader source code */
    std::list<io::stringc> GBufferShdBufVert(CompilerOp), GBufferShdBufFrag(CompilerOp);
    
    switch (RenderSys_)
    {
        case RENDERER_OPENGL:
        {
            #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
            GBufferShdBufVert.push_back(
                #include "Resources/spGBufferShaderStr.glvert"
            );
            GBufferShdBufFrag.push_back(
                #include "Resources/spGBufferShaderStr.glfrag"
            );
            #else
            io::FileSystem fsys;
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spGBufferShader.glvert", GBufferShdBufVert);
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spGBufferShader.glfrag", GBufferShdBufFrag);
            #endif
            
            Flags = SHADERBUILD_GLSL;
        }
        break;
        
        case RENDERER_DIRECT3D11:
        {
            #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
            GBufferShdBufVert.push_back(
                #include "Resources/spGBufferShaderStr.hlsl"
            );
            #else
            io::FileSystem fsys;
            ShaderClass::loadShaderResourceFile(fsys, "../../sources/RenderSystem/AdvancedRenderer/spGBufferShader.hlsl", GBufferShdBufVert);
            #endif
            
            Flags = SHADERBUILD_HLSL5;
        }
        break;
        
        default:
        {
            #ifdef SP_COMPILE_WITH_CG
            
            Shader::addShaderCore(GBufferShdBufVert, true);
            
            GBufferShdBufVert.push_back(
                #include "Resources/spGBufferShaderStr.cg"
            );
            
            Flags = SHADERBUILD_CG;
            
            #else
            
            io::Log::error(ERR_MSG_CG_NOTSUPPORTED);
            return false;
            
            #endif
        }
    }
    
    /* Generate g-buffer shader */
    if (!buildShader(
            "g-buffer", GBufferShader_, getVertexFormat(), &GBufferShdBufVert,
            IsGL ? &GBufferShdBufFrag : &GBufferShdBufVert,
            "VertexMain", "PixelMain", Flags))
    {
        return false;
    }
    
    if (ISRENDERER(DIRECT3D11))
    {
        GBufferShader_->setObjectCallback(DfRnGBufferObjectShaderCallbackCB);
        GBufferShader_->setSurfaceCallback(DfRnGBufferSurfaceShaderCallbackCB);
    }
    else
    {
        GBufferShader_->setObjectCallback(DfRnGBufferObjectShaderCallback);
        GBufferShader_->setSurfaceCallback(DfRnGBufferSurfaceShaderCallback);
    }
    
    if (IsGL)
    {
        s32 SamplerIndex = 0;
        setupGeometrySampler(GBufferShader_->getPixelShader(), SamplerIndex);
    }
    
    return true;
}

bool DeferredRenderer::loadDeferredShader()
{
    const bool IsGL = ISRENDERER(OPENGL);
    
    s32 Flags = 0;
    
    /* Setup shader compilation options */
    std::list<io::stringc> CompilerOp;
    setupShadingCompilerOptions(CompilerOp);
    
    /* Setup deferred shader source code */
    std::list<io::stringc> DeferredShdBufVert(CompilerOp), DeferredShdBufFrag(CompilerOp);
    
    switch (RenderSys_)
    {
        case RENDERER_OPENGL:
        {
            #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
            DeferredShdBufVert.push_back(
                #include "Resources/spDeferredShaderStr.glvert"
            );
            DeferredShdBufFrag.push_back(
                #include "Resources/spDeferredShaderStr.glfrag"
            );
            #else
            io::FileSystem fsys;
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spDeferredShader.glvert", DeferredShdBufVert);
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spDeferredShader.glfrag", DeferredShdBufFrag);
            #endif
            
            Flags = SHADERBUILD_GLSL;
        }
        break;
        
        case RENDERER_DIRECT3D11:
        {
            #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
            DeferredShdBufVert.push_back(
                #include "Resources/spDeferredShaderStr.hlsl"
            );
            #else
            io::FileSystem fsys;
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spDeferredShader.hlsl", DeferredShdBufVert);
            #endif
            
            Flags = SHADERBUILD_HLSL5;
        }
        break;
        
        default:
        {
            #ifdef SP_COMPILE_WITH_CG
            
            Shader::addShaderCore(DeferredShdBufVert, true);
            
            DeferredShdBufVert.push_back(
                #include "Resources/spDeferredShaderStr.cg"
            );
            
            Flags = SHADERBUILD_CG;
            
            #else
            
            io::Log::error(ERR_MSG_CG_NOTSUPPORTED);
            return false;
            
            #endif
        }
    }
    
    /* Generate deferred shader */
    if (!buildShader(
            "deferred", DeferredShader_, &ImageVertexFormat_, &DeferredShdBufVert,
            IsGL ? &DeferredShdBufFrag : &DeferredShdBufVert,
            "VertexMain", "PixelMain", Flags))
    {
        return false;
    }
    
    if (ISRENDERER(DIRECT3D11))
        DeferredShader_->setObjectCallback(DfRnDeferredShaderCallbackCB);
    else
        DeferredShader_->setObjectCallback(DfRnDeferredShaderCallback);
    
    /* Setup uniforms/ constant buffers */
    if (IsGL)
        setupDeferredSampler(DeferredShader_->getPixelShader());
    
    setupLightShaderConstants(DeferredShader_->getPixelShader());
    
    if (ISFLAG(GLOBAL_ILLUMINATION))
    {
        setAmbientColor(ShadingDesc_.AmbientColor);
        setupVPLOffsets(DeferredShader_->getPixelShader(), "BufferVPL", AdvancedRenderer::VPL_COUNT);
    }

    return true;
}

bool DeferredRenderer::loadLowResVPLShader()
{
    if (!ISFLAG(USE_VPL_OPTIMIZATION))
        return true;
    
    const bool IsGL = ISRENDERER(OPENGL);
    
    s32 Flags = 0;
    
    /* Setup shader compilation options */
    std::list<io::stringc> CompilerOp;
    setupShadingCompilerOptions(CompilerOp);
    
    /* Setup deferred shader source code */
    std::list<io::stringc> LowResVPLShdBufVert(CompilerOp), LowResVPLShdBufFrag(CompilerOp);
    
    switch (RenderSys_)
    {
        case RENDERER_OPENGL:
        {
            #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
            LowResVPLShdBufVert.push_back(
                #include "Resources/spDeferredShaderStr.glvert"
            );
            LowResVPLShdBufFrag.push_back(
                #include "Resources/spDeferredShaderLowResVPLStr.glfrag"
            );
            #else
            io::FileSystem fsys;
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spDeferredShader.glvert", LowResVPLShdBufVert);
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spDeferredShaderLowResVPL.glfrag", LowResVPLShdBufFrag);
            #endif
            
            Flags = SHADERBUILD_GLSL;
        }
        break;
        
        default:
        {
            io::Log::error("Missing low resolution VPL shader for this render system");
            return false;
        }
    }
    
    /* Generate low-resolution VPL deferred shader */
    if (!buildShader(
            "low-resolution VPL", LowResVPLShader_, &ImageVertexFormat_, &LowResVPLShdBufVert,
            IsGL ? &LowResVPLShdBufFrag : &LowResVPLShdBufVert,
            "VertexMain", "PixelMain", Flags))
    {
        return false;
    }
    
    if (ISRENDERER(DIRECT3D11))
        LowResVPLShader_->setObjectCallback(DfRnDeferredShaderCallbackCB);
    else
        LowResVPLShader_->setObjectCallback(DfRnDeferredShaderCallback);
    
    /* Setup uniforms/ constant buffers */
    if (IsGL)
        setupDeferredSampler(LowResVPLShader_->getPixelShader(), true);
    
    setupVPLOffsets(LowResVPLShader_->getPixelShader(), "BufferVPL", DeferredRenderer::VPL_COUNT);
    
    return true;
}

void DeferredRenderer::setupDeferredSampler(Shader* ShaderObj, bool IsLowResVPL)
{
    if (!ShaderObj)
        return;
    
    s32 SamplerIndex = 0;
    
    if (!IsLowResVPL)
        ShaderObj->setConstant("DiffuseAndSpecularMap", SamplerIndex++);
    
    ShaderObj->setConstant("NormalAndDepthMap", SamplerIndex++);
    
    if (!IsLowResVPL)
    {
        if (ISFLAG(HAS_LIGHT_MAP))
            ShaderObj->setConstant("IlluminationMap", SamplerIndex++);
        if (ISFLAG(USE_VPL_OPTIMIZATION))
            ShaderObj->setConstant("VPLColorMap", SamplerIndex++);
    }
    
    if (ISFLAG(SHADOW_MAPPING))
    {
        ShaderObj->setConstant("DirLightShadowMaps", SamplerIndex++);
        ShaderObj->setConstant("PointLightShadowMaps", SamplerIndex++);
        
        if (ISFLAG(GLOBAL_ILLUMINATION))
        {
            ShaderObj->setConstant("DirLightDiffuseMaps", SamplerIndex++);
            ShaderObj->setConstant("PointLightDiffuseMaps", SamplerIndex++);
            //ShaderObj->setConstant("DirLightNormalMaps", SamplerIndex++);
            //ShaderObj->setConstant("PointLightNormalMaps", SamplerIndex++);
        }
    }
    
    if (ISFLAG(TILED_SHADING))
        ShaderObj->setConstant("TileLightIndexList", SamplerIndex++);
}


#undef ISFLAG
#undef ISRENDERER
#undef ADDOP


} // /namespace video

} // /namespace sp


#endif



// ================================================================================