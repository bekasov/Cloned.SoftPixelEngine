/*
 * Advanced renderer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/AdvancedRenderer/spDeferredRenderer.hpp"

#if defined(SP_COMPILE_WITH_ADVANCEDRENDERER)


#include "RenderSystem/AdvancedRenderer/spDeferredRendererShaderCallbacks.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Platform/spSoftPixelDevice.hpp"
#include "Base/spSharedObjects.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern SoftPixelDevice* GlbEngineDev;
extern video::RenderSystem* GlbRenderSys;
extern scene::SceneGraph* GlbSceneGraph;

namespace video
{


#define ISFLAG(n)       ((Config_.Flags & RENDERERFLAG_##n) != 0)
#define REMOVEFLAG(n)   math::removeFlag(Config_.Flags, RENDERERFLAG_##n)
#define ISRENDERER(n)   (RenderSys_ == RENDERER_##n)
#define ADDOP(n)        Shader::addOption(CompilerOp, n, UseGuard)

extern s32 GlbDfRnFlags;
extern s32 GlbDfRnLightGridRowSize;


const u32 AdvancedRenderer::VPL_COUNT = 100;

AdvancedRenderer::AdvancedRenderer(const EAdvancedRenderers Type) :
    RenderSys_          (GlbRenderSys->getRendererType()),
    ShadowShader_       (0                              ),
    ConstBufferLights_  (0                              ),
    ConstBufferLightsEx_(0                              ),
    Type_               (Type                           )
{
    #ifdef SP_COMPILE_WITH_CG
    if (!gSharedObjects.CgContext)
        GlbEngineDev->createCgShaderContext();
    #endif
}
AdvancedRenderer::~AdvancedRenderer()
{
    releaseResources();
}

bool AdvancedRenderer::generateResources(
    u32 Flags, s32 ShadowTexSize, u32 MaxPointLightCount, u32 MaxSpotLightCount, s32 MultiSampling)
{
    initResourceConfig(Flags, ShadowTexSize, MaxPointLightCount, MaxSpotLightCount, MultiSampling);
    
    /* Load all shaders */
    if (!loadAllShaders() || !setupFinalResources())
        return false;
    
    /* Print information */
    printInfo();
    
    return true;
}

void AdvancedRenderer::releaseResources()
{
    deleteAllShaders();
    ShadowMapper_.deleteShadowMaps();
}

void AdvancedRenderer::setGIReflectivity(f32 Reflectivity)
{
    /* Store new GI reflectivity setting */
    ShadingDesc_.GIReflectivity = Reflectivity;
}

void AdvancedRenderer::setAmbientColor(const dim::vector3df &ColorVec)
{
    /* Store new color setting */
    ShadingDesc_.AmbientColor = ColorVec;
}

void AdvancedRenderer::adjustResolution()
{
    /* Set the resolution to the current screen size */
    setResolution(dim::size2di(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight));
}

void AdvancedRenderer::setResolution(const dim::size2di &Resolution)
{
    if (Resolution_ == Resolution)
        return;
    
    LightGrid_.setResolution(Resolution);
    BloomEffect_.setResolution(Resolution);
    
    GlbDfRnLightGridRowSize = LightGrid_.getNumTiles().Width;
    
    Resolution_ = Resolution;
}


/*
 * ======= Protected: =======
 */

void AdvancedRenderer::initResourceConfig(
    u32 Flags, s32 ShadowTexSize, u32 MaxPointLightCount, u32 MaxSpotLightCount, s32 MultiSampling)
{
    /* Release old resources */
    releaseResources();
    
    /* Setup resource flags */
    Config_.setupFlags(Flags);
    
    Config_.ShadowTexSize       = ShadowTexSize;
    Config_.MaxNumPointLights   = math::Max(1u, MaxPointLightCount);
    Config_.MaxNumSpotLights    = math::Max(1u, MaxSpotLightCount);
    Config_.MultiSampling       = MultiSampling;
    
    Resolution_.Width   = gSharedObjects.ScreenWidth;
    Resolution_.Height  = gSharedObjects.ScreenHeight;
    
    LayerModel_.clear();
    
    /* Initialize light objects */
    Config_.MaxNumPointLights = math::Max(Config_.MaxNumPointLights, Config_.MaxNumSpotLights);
    
    Lights_.setStride(sizeof(SLightCB));
    Lights_.setCount(Config_.MaxNumPointLights);
    
    LightsEx_.setStride(sizeof(SLightExCB));
    LightsEx_.setCount(Config_.MaxNumSpotLights);
    
    PointLightsPositionAndRadius_.resize(Config_.MaxNumPointLights);
    
    /* Create new vertex formats */
    createVertexFormats();
    
    /* Load or unload debug VPL resources */
    if (ISFLAG(DEBUG_VIRTUALPOINTLIGHTS))
        DebugVPL_.load();
    else
        DebugVPL_.unload();
    
    /* Create the shadow maps */
    if (ISFLAG(SHADOW_MAPPING))
    {
        ShadowMapper_.createShadowMaps(
            Config_.ShadowTexSize, Config_.MaxNumPointLights, Config_.MaxNumSpotLights, true, ISFLAG(GLOBAL_ILLUMINATION)
        );
    }
}

void AdvancedRenderer::updateLightSources(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, Texture* DepthTexture, s32 &i, s32 &iEx)
{
    /* Update each light source */
    f32 Color[4];
    u32 ShadowCubeMapIndex = 0, ShadowMapIndex = 0;
    
    const s32 LightCount    = static_cast<s32>(Lights_.getCount());
    const s32 LightExCount  = static_cast<s32>(LightsEx_.getCount());
    
    SLightCB* Lit = 0;
    SLightExCB* LitEx = 0;
    
    std::vector<scene::Light*>::const_iterator it = Graph->getLightList().begin(), itEnd = Graph->getLightList().end();
    
    const bool UseShadow = ISFLAG(SHADOW_MAPPING);
    
    if (UseShadow)
        GlbRenderSys->setGlobalShaderClass(ShadowShader_);
    
    for (; it != itEnd && i < LightCount; ++it)
    {
        /* Get current light source object */
        scene::Light* LightObj = *it;
        
        if ( !LightObj->getVisible() || ( LightObj->getLightModel() != scene::LIGHT_POINT && iEx >= LightExCount ) )
            continue;
        
        Lit = Lights_.getRef<SLightCB>(i);
        
        LightObj->getDiffuseColor().getFloatArray(Color);
        
        if (UseShadow && LightObj->getShadow())
        {
            /* Render shadow map */
            switch (LightObj->getLightModel())
            {
                case scene::LIGHT_POINT:
                    Lit->ShadowIndex = ShadowCubeMapIndex;
                    ShadowMapper_.renderShadowMap(Graph, ActiveCamera, LightObj, ShadowCubeMapIndex++);
                    break;
                case scene::LIGHT_SPOT:
                    Lit->ShadowIndex = ShadowMapIndex;
                    ShadowMapper_.renderShadowMap(Graph, ActiveCamera, LightObj, ShadowMapIndex++);
                    break;
                default:
                    break;
            }
        }
        else
            Lit->ShadowIndex = -1;
        
        /* Copy basic data */
        Lit->Position           = LightObj->getPosition(true);
        Lit->InvRadius          = 1.0f / (LightObj->getVolumetric() ? LightObj->getVolumetricRadius() : 1000.0f);
        Lit->Color              = dim::vector3df(Color[0], Color[1], Color[2]);
        Lit->Type               = static_cast<u8>(LightObj->getLightModel());
        Lit->UsedForLightmaps   = (LightObj->getShadow() ? 0 : 1);//!!!
        Lit->ExID               = iEx;
        
        if (Lit->Type != scene::LIGHT_POINT)
        {
            /* Get extended light object */
            LitEx = LightsEx_.getRef<SLightExCB>(iEx);
            
            /* Copy extended data */
            const scene::Transformation Transform(LightObj->getTransformation(true));
            
            if (Lit->Type == scene::LIGHT_SPOT)
            {
                dim::matrix4f ViewMatrix(Transform.getInverseMatrix());
                
                LitEx->ViewProjection.setPerspectiveLH(LightObj->getSpotConeOuter()*2, 1.0f, 0.01f, 1000.0f);
                
                if (ISFLAG(GLOBAL_ILLUMINATION))
                {
                    /* Setup inverse view-projection and finalize standard view-projection matrix */
                    LitEx->InvViewProjection = LitEx->ViewProjection;
                    LitEx->ViewProjection *= ViewMatrix;
                    
                    ViewMatrix.setPosition(0.0f);
                    
                    /* Finalize inverse view-projection matrix */
                    LitEx->InvViewProjection *= ViewMatrix;
                    LitEx->InvViewProjection.setInverse();
                }
                else
                {
                    /* Finalize standard view-projection matrix */
                    LitEx->ViewProjection *= ViewMatrix;
                }
            }
            
            LitEx->Direction = Transform.getDirection();
            LitEx->Direction.normalize();
            
            LitEx->SpotTheta            = LightObj->getSpotConeInner() * math::DEG;
            LitEx->SpotPhiMinusTheta    = LightObj->getSpotConeOuter() * math::DEG - LitEx->SpotTheta;
            
            ++iEx;
        }

        /* Setup data for raw light model (used for light-grid compute shader) */
        if (ISFLAG(TILED_SHADING))
        {
            PointLightsPositionAndRadius_[i] = dim::vector4df(
                Lit->Position,
                #if 1
                LightObj->getVolumetricRadius()*2.0f
                #else
                5.0f//1.0f//!!!
                #endif
            );
        }

        ++i;
    }
    
    if (UseShadow)
        GlbRenderSys->setGlobalShaderClass(0);
    
    Shader* FragShd = 0;
    
    /* Update debug-vpl shader constants */
    if (ISFLAG(DEBUG_VIRTUALPOINTLIGHTS) && Lit && LitEx && Lit->ShadowIndex != -1)
    {
        FragShd = (DebugVPL_.ShdClass ? DebugVPL_.ShdClass->getVertexShader() : 0);
        
        if (FragShd)
        {
            FragShd->setConstant("LightShadowIndex",        Lit->ShadowIndex        );
            FragShd->setConstant("LightPosition",           Lit->Position           );
            FragShd->setConstant("LightColor",              Lit->Color              );
            FragShd->setConstant("LightInvViewProjection",  LitEx->InvViewProjection);
        }
    }
    
    /* Build light grid if tiled shading is used */
    if (ISFLAG(TILED_SHADING))
    {
        LightGrid_.updateLights(PointLightsPositionAndRadius_, i);
        LightGrid_.build(Graph, ActiveCamera, DepthTexture);
    }
}

void AdvancedRenderer::renderDebugVPLs(scene::Camera* ActiveCamera)
{
    /* Setup render view and mode */
    ActiveCamera->setupRenderView();
    GlbRenderSys->setRenderMode(video::RENDERMODE_SCENE);
    GlbRenderSys->setWorldMatrix(dim::matrix4f::IDENTITY);
    
    /* Setup render states */
    GlbRenderSys->setupMaterialStates(&DebugVPL_.Material);
    
    /* Bind textures */
    ShadowMapper_.bind(0);
    
    /* Setup shader class and draw model */
    GlbRenderSys->setupShaderClass(0, DebugVPL_.ShdClass);
    GlbRenderSys->drawMeshBuffer(&DebugVPL_.Model);
    
    /* Unbind textures */
    ShadowMapper_.unbind(0);
}

bool AdvancedRenderer::buildShader(
    const io::stringc &Name,
    ShaderClass* &ShdClass,
    const VertexFormat* VertFmt,
    
    const std::list<io::stringc>* ShdBufferVertex,
    const std::list<io::stringc>* ShdBufferPixel,
    
    const io::stringc &VertexMain,
    const io::stringc &PixelMain,
    
    s32 Flags)
{
    if (!ShaderClass::build(Name, ShdClass, VertFmt, ShdBufferVertex, ShdBufferPixel, VertexMain, PixelMain, Flags))
    {
        deleteAllShaders();
        return false;
    }
    return true;
}

void AdvancedRenderer::deleteShader(ShaderClass* &ShdClass)
{
    GlbRenderSys->deleteShaderClass(ShdClass);
    ShdClass = 0;
}

bool AdvancedRenderer::loadAllShaders()
{
    return
        loadShadowShader    () &&
        loadDebugVPLShader  ();
}

void AdvancedRenderer::deleteAllShaders()
{
    deleteShader(ShadowShader_      );
    deleteShader(DebugVPL_.ShdClass );
}

void AdvancedRenderer::createVertexFormats()
{
    /* Create object vertex format */
    VertexFormat_.clear();
    
    VertexFormat_.addCoord();
    VertexFormat_.addNormal();
    VertexFormat_.addTexCoord();
    
    if (ISFLAG(NORMAL_MAPPING))
    {
        /* Add texture-coordinates for normal-mapping (tangent and binormal is texture-coordinates) */
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 3);
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 3);
    }
    
    if (ISFLAG(HAS_LIGHT_MAP))
    {
        /* Add texture-coordinates for lightmaps */
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 2);
    }
}

bool AdvancedRenderer::setupFinalResources()
{
    /* Initialize extended shader constants */
    if (ISFLAG(GLOBAL_ILLUMINATION))
        setGIReflectivity(ShadingDesc_.GIReflectivity);
    
    /* Generate bloom filter shader */
    if (ISFLAG(BLOOM))
    {
        if (!BloomEffect_.createResources(Resolution_))
            REMOVEFLAG(BLOOM);
    }
    
    /* Create light grid */
    if (ISFLAG(TILED_SHADING))
    {
        if (!LightGrid_.createGrid(Resolution_, Config_.MaxNumPointLights))
            return false;
        
        GlbDfRnLightGridRowSize = LightGrid_.getNumTiles().Width;
    }
    
    return true;
}

void AdvancedRenderer::setupGeometryCompilerOptions(std::list<io::stringc> &CompilerOp, bool UseGuard)
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

void AdvancedRenderer::setupShadingCompilerOptions(std::list<io::stringc> &CompilerOp, bool UseGuard)
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
    
    UseGuard = false;
    
    ADDOP("MAX_LIGHTS " + io::stringc(Config_.MaxNumPointLights));
    ADDOP("MAX_EX_LIGHTS " + io::stringc(Config_.MaxNumSpotLights));
}

void AdvancedRenderer::setupGeometrySampler(Shader* ShaderObj, s32 &SamplerIndex)
{
    if (!ShaderObj)
        return;
    
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

void AdvancedRenderer::setupLightShaderConstants(Shader* FragShd)
{
    if (FragShd)
    {
        LightDesc_.LightCountConstant = FragShd->getConstant("LightCount");
        
        ConstBufferLights_      = FragShd->getConstantBuffer("BufferLight");
        ConstBufferLightsEx_    = FragShd->getConstantBuffer("BufferLightEx");
    }
}

void AdvancedRenderer::setupVPLOffsets(
    Shader* ShaderObj, const io::stringc &BufferName, u32 OffsetCount,
    s32 Rings, s32 Rotations, f32 Bias, f32 JitterBias)
{
    if (!ShaderObj)
        return;
    
    /* Generate VPL offsets */
    std::vector<f32> Offsets(OffsetCount*4);
    
    const f32 MaxRotation = static_cast<f32>(Rotations) / OffsetCount;
    const f32 TexSize = static_cast<f32>(Config_.ShadowTexSize);
    
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

void AdvancedRenderer::printInfo()
{
    /* Print basic information */
    io::Log::message("=== Setup " + getDescription() + ": === ");
    io::Log::message(
        "Max. Point Lights (" + io::stringc(Config_.MaxNumPointLights) +
        "), Max. Spot Lights (" + io::stringc(Config_.MaxNumSpotLights) + "):"
    );
    
    /* Print flags info */
    io::stringc FlagsStr;
    
    pushBackInfo(FlagsStr, RENDERERFLAG_DEBUG_GBUFFER,          "Debug"                 );
    pushBackInfo(FlagsStr, RENDERERFLAG_HAS_LIGHT_MAP,          "Lightmaps"             );
    pushBackInfo(FlagsStr, RENDERERFLAG_NORMAL_MAPPING,         "Bump Mapping"          );
    pushBackInfo(FlagsStr, RENDERERFLAG_PARALLAX_MAPPING,       "Relief Mapping"        );
    pushBackInfo(FlagsStr, RENDERERFLAG_SHADOW_MAPPING,         "Shadow Mapping"        );
    pushBackInfo(FlagsStr, RENDERERFLAG_GLOBAL_ILLUMINATION,    "Global Illumination"   );
    pushBackInfo(FlagsStr, RENDERERFLAG_TILED_SHADING,          "Tiled Shading"         );
    pushBackInfo(FlagsStr, RENDERERFLAG_BLOOM,                  "Bloom"                 );
    
    if (FlagsStr.empty())
        io::Log::message("Flags { None }");
    else
        io::Log::message("Flags { " + FlagsStr + " }");
    
    /* Print tiled shading info */
    if (ISFLAG(TILED_SHADING))
    {
        const dim::size2di& NumTiles(LightGrid_.getNumTiles());
        io::Log::message(
            "Tiled Shading Raster (" + io::stringc(NumTiles.Width) + " x " + io::stringc(NumTiles.Height) + ")"
        );
    }
    
    io::Log::message("");
}

void AdvancedRenderer::pushBackInfo(io::stringc &FlagsStr, u32 Flag, const io::stringc &Desc)
{
    if ((Config_.Flags & Flag) != 0)
    {
        if (!FlagsStr.empty())
            FlagsStr += ", ";
        FlagsStr += Desc;
    }
}

bool AdvancedRenderer::getActiveCamera(scene::SceneGraph* Graph, scene::Camera* &ActiveCamera) const
{
    if (!ActiveCamera)
    {
        ActiveCamera = Graph->getActiveCamera();
        
        if (!ActiveCamera)
        {
            if (Graph->getCameraList().empty())
            {
                #ifdef SP_DEBUGMODE
                io::Log::debug("AdvancedRenderer::getActiveCamera", "No active camera", io::LOG_UNIQUE);
                #endif
                return false;
            }
            else
                ActiveCamera = *Graph->getCameraList().begin();
        }
    }
    return true;
}


/*
 * ======= Private: =======
 */

bool AdvancedRenderer::loadShadowShader()
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
            io::Log::error("Missing shadow shader for this render system");
            return false;
        }
    }
    
    /* Generate shadow shader */
    if (!buildShader(
            "shadow", ShadowShader_, getVertexFormat(), &ShadowShdBuf, &ShadowShdBuf,
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

bool AdvancedRenderer::loadDebugVPLShader()
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
            "debug VPL", DebugVPL_.ShdClass, getVertexFormat(),
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

void AdvancedRenderer::setupShadowCompilerOptions(std::list<io::stringc> &CompilerOp, bool UseGuard)
{
    ADDOP("USE_VSM");
    ADDOP("USE_TEXTURE");
    
    if (ISFLAG(GLOBAL_ILLUMINATION))
        ADDOP("USE_RSM");
    
    //if (ISFLAG(USE_TEXTURE_MATRIX))
    //    ADDOP("USE_TEXTURE_MATRIX");
}

void AdvancedRenderer::setupDebugVPLSampler(Shader* ShaderObj)
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


/*
 * SShadingDescCB structure
 */

AdvancedRenderer::SShadingDescCB::SShadingDescCB() :
    AmbientColor    (0.07f  ),
    GIReflectivity  (0.1f   ),
    LightCount      (0      )
{
}
AdvancedRenderer::SShadingDescCB::~SShadingDescCB()
{
}


/*
 * SDebugVPL structure
 */

AdvancedRenderer::SDebugVPL::SDebugVPL() :
    ShdClass    (0      ),
    VtxFormat   (0      ),
    Enabled     (true   )
{
}
AdvancedRenderer::SDebugVPL::~SDebugVPL()
{
}

void AdvancedRenderer::SDebugVPL::load()
{
    if (!VtxFormat)
    {
        /* Setup vertex format */
        VtxFormat = GlbRenderSys->createVertexFormat<VertexFormatUniversal>();
        VtxFormat->addUniversal(video::DATATYPE_FLOAT, 3, "Position", false, VERTEXFORMAT_COORD);
        
        /* Create cube model */
        Model.createMeshBuffer();
        Model.setVertexFormat(VtxFormat);
        scene::MeshGenerator::createIcoSphere(Model, 0.1f, 2);
        Model.setHardwareInstancing(math::pow2(10));
        
        /* Configure material states */
        Material.setLighting(false);
        Material.setFog(false);
    }
}
void AdvancedRenderer::SDebugVPL::unload()
{
    if (VtxFormat)
    {
        Model.deleteMeshBuffer();
        GlbRenderSys->deleteVertexFormat(VtxFormat);
        VtxFormat = 0;
    }
}


/*
 * SRendererConfig structure
 */

#undef ISFLAG
#undef REMOVEFLAG

#define ISFLAG(n)       ((Flags & RENDERERFLAG_##n) != 0)
#define REMOVEFLAG(n)   math::removeFlag(Flags, RENDERERFLAG_##n)

AdvancedRenderer::SRendererConfig::SRendererConfig() :
    Flags               (0  ),
    ShadowTexSize       (128),
    MaxNumPointLights   (1  ),
    MaxNumSpotLights    (1  ),
    MultiSampling       (0  )
{
}
AdvancedRenderer::SRendererConfig::~SRendererConfig()
{
}

void AdvancedRenderer::SRendererConfig::setupFlags(s32 NewFlags)
{
    Flags = NewFlags;
    
    /* Remove flags with missing meta flag */
    if (!ISFLAG(NORMAL_MAPPING))
        REMOVEFLAG(PARALLAX_MAPPING);
    if (!ISFLAG(PARALLAX_MAPPING))
        REMOVEFLAG(NORMALMAP_XYZ_H);
    if (!ISFLAG(SHADOW_MAPPING))
        REMOVEFLAG(GLOBAL_ILLUMINATION);
    if (!ISFLAG(GLOBAL_ILLUMINATION))
    {
        REMOVEFLAG(DEBUG_VIRTUALPOINTLIGHTS);
        REMOVEFLAG(USE_VPL_OPTIMIZATION);
    }
}

#undef ISFLAG
#undef REMOVEFLAG
#undef ISRENDERER
#undef ADDOP


} // /namespace video

} // /namespace sp


#endif



// ================================================================================