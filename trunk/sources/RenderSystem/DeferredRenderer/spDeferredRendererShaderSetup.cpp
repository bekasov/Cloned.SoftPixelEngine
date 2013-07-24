/*
 * Deferred renderer shader setup file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spDeferredRenderer.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/DeferredRenderer/spDeferredRendererShaderCallbacks.hpp"
#include "Base/spMathRandomizer.hpp"
#include "Base/spSharedObjects.hpp"


//!!!
#ifdef SP_DEBUGMODE
#   define _DEB_LOAD_SHADERS_FROM_FILES_
#   ifdef _DEB_LOAD_SHADERS_FROM_FILES_
#       define _DEB_SHADER_PATH_ io::stringc("../../sources/RenderSystem/DeferredRenderer/")
#   endif
#endif


namespace sp
{
namespace video
{


#define ISFLAG(n)       ((Flags_ & DEFERREDFLAG_##n) != 0)
#define ISRENDERER(n)   (RenderSys_ == RENDERER_##n)
#define ADDOP(n)        Shader::addOption(CompilerOp, n)

static const c8* ERR_MSG_CG_NOTSUPPORTED = "Engine was not compiled with Cg Toolkit";
static const c8* ERR_MSG_CG_NOTPROVIDED = "Not fully provided Cg shaders for deferred renderer";
static const c8* ERR_MSG_MISSING_SHADER = "Missing deferred renderer shaders for this render system";


bool DeferredRenderer::loadGBufferShader()
{
    const bool IsGL = ISRENDERER(OPENGL);
    
    s32 Flags = 0;
    
    /* Setup shader compilation options */
    std::list<io::stringc> CompilerOp;
    setupGBufferCompilerOptions(CompilerOp);
    
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
            ShaderClass::loadShaderResourceFile(fsys, "../../sources/RenderSystem/DeferredRenderer/spGBufferShader.hlsl", GBufferShdBufVert);
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
            "g-buffer", GBufferShader_, &VertexFormat_, &GBufferShdBufVert,
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
        setupGBufferSampler(GBufferShader_->getPixelShader());
    
    return true;
}

bool DeferredRenderer::loadDeferredShader()
{
    const bool IsGL = ISRENDERER(OPENGL);
    
    s32 Flags = 0;
    
    /* Setup shader compilation options */
    std::list<io::stringc> CompilerOp;
    setupDeferredCompilerOptions(CompilerOp);
    
    if (ISFLAG(TILED_SHADING))
        setupTiledShadingOptions(CompilerOp);
    
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
    
    setupLightShaderConstants();
    //setupJitteredOffsets();
    
    if (ISFLAG(GLOBAL_ILLUMINATION))
    {
        setAmbientColor(ShadingDesc_.AmbientColor);
        setupVPLOffsets(DeferredShader_->getPixelShader(), "VPLOffsetBlock", DeferredRenderer::VPL_COUNT);
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
    setupDeferredCompilerOptions(CompilerOp);
    
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
            io::Log::error(ERR_MSG_MISSING_SHADER);
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
    
    setupVPLOffsets(LowResVPLShader_->getPixelShader(), "VPLOffsetBlock", DeferredRenderer::VPL_COUNT);
    
    return true;
}

bool DeferredRenderer::loadShadowShader()
{
    /* Generate shadow shader */
    if (!ISFLAG(SHADOW_MAPPING))
        return true;
    
    s32 Flags = 0;
    
    /* Setup shader compilation options */
    std::list<io::stringc> ShadowShdBuf;
    setupShadowCompilerOptions(ShadowShdBuf);
    
    /* Setup shadow shader source code */
    switch (RenderSys_)
    {
        case RENDERER_OPENGL:
        {
            #ifdef SP_COMPILE_WITH_CG
            
            #   ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
            ShadowShdBuf.push_back(
                #include "Resources/spShadowShaderStr.cg"
            );
            #   else
            io::FileSystem fsys;
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spShadowShader.cg", ShadowShdBuf, true);
            #   endif
            
            Flags = SHADERBUILD_CG;
            
            #else
            
            io::Log::error(ERR_MSG_CG_NOTSUPPORTED);
            return false;
            
            #endif
        }
        break;
        
        case RENDERER_DIRECT3D11:
        {
            #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
            ShadowShdBuf.push_back(
                #include "Resources/spShadowShaderStr.hlsl"
            );
            #else
            io::FileSystem fsys;
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spShadowShader.hlsl", ShadowShdBuf);
            #endif
            
            Flags = SHADERBUILD_HLSL5;
        }
        break;
        
        default:
        {
            io::Log::error(ERR_MSG_MISSING_SHADER);
            return false;
        }
    }
    
    /* Generate shadow shader */
    if (!buildShader(
            "shadow", ShadowShader_, &VertexFormat_, &ShadowShdBuf, &ShadowShdBuf,
            "VertexMain", "PixelMain", Flags))
    {
        return false;
    }
    
    if (ISRENDERER(DIRECT3D11))
        ShadowShader_->setObjectCallback(DfRnShadowShaderCallbackCB);
    else
        ShadowShader_->setObjectCallback(DfRnShadowShaderCallback);
    
    return true;
}

bool DeferredRenderer::loadDebugVPLShader()
{
    const bool IsGL = ISRENDERER(OPENGL);
    
    /* Generate debug VPL shader */
    if (!ISFLAG(DEBUG_VIRTUALPOINTLIGHTS) || !IsGL)
        return true;
    
    /* Setup g-buffer shader source code */
    std::list<io::stringc> DebugVPLShdBufVert, DebugVPLShdBufFrag;
    
    #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
    DebugVPLShdBufVert.push_back(
        #include "Resources/spDebugVPLStr.glvert"
    );
    DebugVPLShdBufFrag.push_back(
        #include "Resources/spDebugVPLStr.glfrag"
    );
    #else
    io::FileSystem fsys;
    ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spDebugVPL.glvert", DebugVPLShdBufVert);
    ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spDebugVPL.glfrag", DebugVPLShdBufFrag);
    #endif
    
    /* Generate g-buffer shader */
    if (!buildShader(
            "debug VPL", DebugVPL_.ShdClass, &VertexFormat_,
            &DebugVPLShdBufVert, &DebugVPLShdBufFrag,
            "VertexMain", "PixelMain", SHADERBUILD_GLSL))
    {
        return false;
    }
    
    DebugVPL_.ShdClass->setObjectCallback(DfRnDebugVPLShaderCallback);
    
    /* Setup uniforms/ constant buffers */
    setupDebugVPLSampler(DebugVPL_.ShdClass->getVertexShader());
    
    setupVPLOffsets(DebugVPL_.ShdClass->getVertexShader(), "VPLOffsetBlock", DeferredRenderer::VPL_COUNT);
    
    return true;
}

void DeferredRenderer::setupGBufferCompilerOptions(std::list<io::stringc> &CompilerOp)
{
    if (ISFLAG(USE_TEXTURE_MATRIX))
        ADDOP("USE_TEXTURE_MATRIX");
    if (ISFLAG(HAS_SPECULAR_MAP))
        ADDOP("HAS_SPECULAR_MAP");
    
    if (ISFLAG(HAS_LIGHT_MAP))
        ADDOP("HAS_LIGHT_MAP");
    
    if (ISFLAG(NORMAL_MAPPING))
    {
        ADDOP("NORMAL_MAPPING");
        
        if (ISFLAG(PARALLAX_MAPPING))
        {
            ADDOP("PARALLAX_MAPPING");
            if (ISFLAG(NORMALMAP_XYZ_H))
                ADDOP("NORMALMAP_XYZ_H");
        }
    }
    
    if (ISFLAG(DEBUG_GBUFFER))
    {
        ADDOP("DEBUG_GBUFFER");
        
        if (ISFLAG(DEBUG_GBUFFER_TEXCOORDS))
            ADDOP("DEBUG_GBUFFER_TEXCOORDS");
    }
    
    if (ISFLAG(SHADOW_MAPPING))
        ADDOP("SHADOW_MAPPING");
}

void DeferredRenderer::setupDeferredCompilerOptions(std::list<io::stringc> &CompilerOp)
{
    if (ISFLAG(HAS_LIGHT_MAP))
        ADDOP("HAS_LIGHT_MAP");
    
    if (ISFLAG(ALLOW_OVERBLENDING))
        ADDOP("ALLOW_OVERBLENDING");
    
    if (ISFLAG(DEBUG_GBUFFER))
    {
        ADDOP("DEBUG_GBUFFER");
        
        if (ISFLAG(DEBUG_GBUFFER_WORLDPOS))
            ADDOP("DEBUG_GBUFFER_WORLDPOS");
    }
    
    if (ISFLAG(BLOOM))
        ADDOP("BLOOM_FILTER");
    
    if (ISFLAG(SHADOW_MAPPING))
    {
        ADDOP("SHADOW_MAPPING");
        
        if (ISFLAG(GLOBAL_ILLUMINATION))
        {
            ADDOP("GLOBAL_ILLUMINATION");
            
            if (ISFLAG(USE_VPL_OPTIMIZATION))
                ADDOP("USE_LOWRES_VPL_SHADING");
        }
    }
    
    if (ISFLAG(TILED_SHADING))
        ADDOP("TILED_SHADING");
    
    ADDOP("MAX_LIGHTS " + io::stringc(MaxPointLightCount_));
    ADDOP("MAX_EX_LIGHTS " + io::stringc(MaxSpotLightCount_));
    
    #ifdef _DEB_USE_LIGHT_CONSTANT_BUFFER_
    ADDOP("LIGHT_CONSTANT_BUFFERS");
    #endif
}

void DeferredRenderer::setupShadowCompilerOptions(std::list<io::stringc> &CompilerOp)
{
    ADDOP("USE_VSM");
    ADDOP("USE_TEXTURE");
    
    if (ISFLAG(GLOBAL_ILLUMINATION))
        ADDOP("USE_RSM");
    
    //if (ISFLAG(USE_TEXTURE_MATRIX))
    //    ADDOP("USE_TEXTURE_MATRIX");
}

void DeferredRenderer::setupTiledShadingOptions(std::list<io::stringc> &CompilerOp)
{
    /* Derivate light grid size from tile count */
    const dim::size2di Resolution(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight);
    
    LightGridDesc_.TileCount = Resolution / 32;

    //!TODO! -> use "LightGrid::getGridSize()"
    LightGridDesc_.GridSize = dim::size2di(
        (Resolution.Width + LightGridDesc_.TileCount.Width - 1) / LightGridDesc_.TileCount.Width,
        (Resolution.Height + LightGridDesc_.TileCount.Height - 1) / LightGridDesc_.TileCount.Height
    );

    /* Setup shader constants */
    ADDOP("TILED_LIGHT_GRID_NUM_X " + io::stringc(LightGridDesc_.TileCount.Width));
    ADDOP("TILED_LIGHT_GRID_NUM_Y " + io::stringc(LightGridDesc_.TileCount.Height));

    ADDOP("TILED_LIGHT_GRID_WIDTH " + io::stringc(LightGridDesc_.GridSize.Width));
    ADDOP("TILED_LIGHT_GRID_HEIGHT " + io::stringc(LightGridDesc_.GridSize.Height));
}

void DeferredRenderer::setupGBufferSampler(Shader* ShaderObj)
{
    if (!ShaderObj)
        return;
    
    s32 SamplerIndex = 0;
    
    LayerModel_.DiffuseMap = static_cast<u8>(SamplerIndex);
    ShaderObj->setConstant("DiffuseMap", SamplerIndex++);
    
    if (ISFLAG(HAS_SPECULAR_MAP))
    {
        LayerModel_.SpecularMap = static_cast<u8>(SamplerIndex);
        ShaderObj->setConstant("SpecularMap", SamplerIndex++);
    }
    
    if (ISFLAG(NORMAL_MAPPING))
    {
        LayerModel_.NormalMap = static_cast<u8>(SamplerIndex);
        ShaderObj->setConstant("NormalMap", SamplerIndex++);
        
        if (ISFLAG(PARALLAX_MAPPING))
        {
            if (!ISFLAG(NORMALMAP_XYZ_H))
            {
                LayerModel_.HeightMap = static_cast<u8>(SamplerIndex);
                ShaderObj->setConstant("HeightMap", SamplerIndex++);
            }
            else
                LayerModel_.HeightMap = LayerModel_.NormalMap;
        }
    }
    
    if (ISFLAG(HAS_LIGHT_MAP))
    {
        LayerModel_.LightMap = static_cast<u8>(SamplerIndex);
        ShaderObj->setConstant("LightMap", SamplerIndex++);
    }
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

void DeferredRenderer::setupDebugVPLSampler(Shader* ShaderObj)
{
    if (ShaderObj)
    {
        s32 SamplerIndex = 0;
        
        ShaderObj->setConstant("DirLightShadowMaps",    SamplerIndex++);
        ShaderObj->setConstant("PointLightShadowMaps",  SamplerIndex++);
        ShaderObj->setConstant("DirLightDiffuseMaps",   SamplerIndex++);
        ShaderObj->setConstant("PointLightDiffuseMaps", SamplerIndex++);
        //ShaderObj->setConstant("DirLightNormalMaps",    SamplerIndex++);
        //ShaderObj->setConstant("PointLightNormalMaps",  SamplerIndex++);
    }
}

void DeferredRenderer::setupLightShaderConstants()
{
    Shader* FragShd = DeferredShader_->getPixelShader();
    
    LightDesc_.LightCountConstant = FragShd->getConstant("LightCount");
    
    #ifdef _DEB_USE_LIGHT_CONSTANT_BUFFER_
    
    ConstBufferLights_      = FragShd->getConstantBuffer("BufferLight");
    ConstBufferLightsEx_    = FragShd->getConstantBuffer("BufferLightEx");
    
    #else
    
    for (u32 i = 0, c = Lights_.size(); i < c; ++i)
    {
        SLight& Lit = Lights_[i];
        
        const io::stringc n = "Lights[" + io::stringc(i) + "].";
        
        Lit.Constants[0] = FragShd->getConstant(n + "PositionAndInvRadius"  );
        Lit.Constants[1] = FragShd->getConstant(n + "Color"                 );
        Lit.Constants[2] = FragShd->getConstant(n + "Type"                  );
        Lit.Constants[3] = FragShd->getConstant(n + "ShadowIndex"           );
        Lit.Constants[4] = FragShd->getConstant(n + "UsedForLightmaps"      );
    }
    
    for (u32 i = 0, c = LightsEx_.size(); i < c; ++i)
    {
        SLightEx& Lit = LightsEx_[i];
        
        const io::stringc n = "LightsEx[" + io::stringc(i) + "].";
        
        Lit.Constants[0] = FragShd->getConstant(n + "ViewProjection"    );
        Lit.Constants[1] = FragShd->getConstant(n + "Direction"         );
        Lit.Constants[2] = FragShd->getConstant(n + "SpotTheta"         );
        Lit.Constants[3] = FragShd->getConstant(n + "SpotPhiMinusTheta" );
        
        if (ISFLAG(GLOBAL_ILLUMINATION))
            Lit.Constants[4] = FragShd->getConstant(n + "InvViewProjection");
    }
    
    #endif
}

#if 0//!!!

void DeferredRenderer::setupJitteredOffsets()
{
    Shader* FragShd = DeferredShader_->getPixelShader();
    
    static const s32 NUM_JITTERD_OFFSETS    = 20;
    static const f32 MAX_JITTER_FACTOR      = 0.035f;

    dim::point2df JitteredOffsets[NUM_JITTERD_OFFSETS];

    for (s32 i = 0; i < NUM_JITTERD_OFFSETS; ++i)
    {
        JitteredOffsets[i].X = math::Randomizer::randFloat(-MAX_JITTER_FACTOR, MAX_JITTER_FACTOR);
        JitteredOffsets[i].Y = math::Randomizer::randFloat(-MAX_JITTER_FACTOR, MAX_JITTER_FACTOR);
    }

    FragShd->setConstant("JitteredOffsets", &JitteredOffsets[0].X, NUM_JITTERD_OFFSETS);
}

#endif

void DeferredRenderer::setupVPLOffsets(
    Shader* ShaderObj, const io::stringc &BufferName, u32 OffsetCount,
    s32 Rings, s32 Rotations, f32 Bias, f32 JitterBias)
{
    if (!ShaderObj)
        return;
    
    /* Generate VPL offsets */
    std::vector<f32> Offsets(OffsetCount*4);
    
    const f32 MaxRotation = static_cast<f32>(Rotations) / OffsetCount;
    const f32 TexSize = static_cast<f32>(ShadowTexSize_);
    
    for (u32 i = 0, j = 0; i < OffsetCount; ++i)
    {
        /*
        Offset generation derived from:
        http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter17.html (Figure 17-2)
        */
        dim::point2df Vec(
            (static_cast<f32>(i % Rings) + Bias) / static_cast<f32>(Rings + 1),
            static_cast<f32>(i / Rings) * MaxRotation
        );
        
        /* Adjust with jittering */
        Vec.X += math::Randomizer::randFloat(-JitterBias, JitterBias);
        Vec.Y += math::Randomizer::randFloat(-JitterBias, JitterBias);
        
        /* Transform to final offsets */
        f32 u = (math::pow2(Vec.X) * math::Cos(Vec.Y*360.0f)) * 0.5f + 0.5f;
        f32 v = (math::pow2(Vec.X) * math::Sin(Vec.Y*360.0f)) * 0.5f + 0.5f;
        
        /* Avoid linear texture filtering by clamping the offsets to integer numbers */
        u = (floor(u * TexSize) + 0.5f) / TexSize;
        v = (floor(v * TexSize) + 0.5f) / TexSize;
        
        /* Store in final offset buffer */
        Offsets[j++] = u;
        Offsets[j++] = v;
        Offsets[j++] = 0.0f;
        Offsets[j++] = 0.0f;
    }
    
    ShaderObj->setConstantBuffer(BufferName, &Offsets[0]);
}


#undef ISFLAG
#undef ISRENDERER
#undef ADDOP


} // /namespace video

} // /namespace sp


#endif



// ================================================================================