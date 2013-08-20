/*
 * Deferred renderer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spDeferredRenderer.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Platform/spSoftPixelDevice.hpp"
#include "Base/spSharedObjects.hpp"

#include <boost/foreach.hpp>


//#define _DEB_PERFORMANCE_ //!!!
#ifdef _DEB_PERFORMANCE_
#   include "Base/spTimer.hpp"
#endif


namespace sp
{

extern SoftPixelDevice* GlbEngineDev;
extern video::RenderSystem* GlbRenderSys;
extern scene::SceneGraph* GlbSceneGraph;

namespace video
{


#define ISFLAG(n)       ((Flags_ & DEFERREDFLAG_##n) != 0)
#define REMOVEFLAG(n)   math::removeFlag(Flags_, DEFERREDFLAG_##n);

extern s32 gDRFlags;


const u32 DeferredRenderer::VPL_COUNT = 100;

DeferredRenderer::DeferredRenderer() :
    RenderSys_          (GlbRenderSys->getRendererType()),
    GBufferShader_      (0                              ),
    DeferredShader_     (0                              ),
    LowResVPLShader_    (0                              ),
    ShadowShader_       (0                              ),
    ConstBufferLights_  (0                              ),
    ConstBufferLightsEx_(0                              ),
    Flags_              (0                              )
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("DeferredRenderer", "The deferred renderer is still in progress");
    #endif
    
    #ifdef SP_COMPILE_WITH_CG
    if (!gSharedObjects.CgContext)
        GlbEngineDev->createCgShaderContext();
    #endif
}
DeferredRenderer::~DeferredRenderer()
{
    releaseResources();
}

bool DeferredRenderer::generateResources(
    s32 Flags, s32 ShadowTexSize, u32 MaxPointLightCount, u32 MaxSpotLightCount, s32 MultiSampling)
{
    /*#ifndef SP_COMPILE_WITH_CG
    if (Flags & DEFERREDFLAG_SHADOW_MAPPING)
    {
        REMOVEFLAG(SHADOW_MAPPING);
        io::Log::warning("Cannot use shadow mapping in deferred renderer without 'Cg Toolkit'");
    }
    #endif*/
    
    /* Setup resource flags */
    setupFlags(Flags);
    
    ShadowTexSize_ = ShadowTexSize;
    MaxPointLightCount_ = math::Max(1u, MaxPointLightCount);
    MaxSpotLightCount_ = math::Max(1u, MaxSpotLightCount);
    
    LayerModel_.clear();
    
    const dim::size2di Resolution(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight);
    
    /* Initialize light objects */
    MaxPointLightCount_ = math::Max(MaxPointLightCount_, MaxSpotLightCount_);
    
    #ifdef _DEB_USE_LIGHT_CONSTANT_BUFFER_
    Lights_.setStride(sizeof(SLightCB));
    Lights_.setCount(MaxPointLightCount_);
    
    LightsEx_.setStride(sizeof(SLightExCB));
    LightsEx_.setCount(MaxSpotLightCount_);
    #else
    Lights_.resize(MaxPointLightCount_);
    LightsEx_.resize(MaxSpotLightCount_);
    #endif

    PointLightsPositionAndRadius_.resize(MaxPointLightCount_);
    
    if (ISFLAG(DEBUG_VIRTUALPOINTLIGHTS))
        DebugVPL_.load();
    else
        DebugVPL_.unload();
    
    /* Release old resources */
    releaseResources();
    
    /* Create new vertex formats */
    createVertexFormats();
    
    /* Create the shadow maps */
    if (ISFLAG(SHADOW_MAPPING))
    {
        ShadowMapper_.createShadowMaps(
            ShadowTexSize_, MaxPointLightCount_, MaxSpotLightCount_, true, ISFLAG(GLOBAL_ILLUMINATION)
        );
    }
    
    /* Load all shaders */
    if ( !loadGBufferShader     () ||
         !loadDeferredShader    () ||
         !loadLowResVPLShader   () ||
         !loadShadowShader      () ||
         !loadDebugVPLShader    () )
    {
        return false;
    }
    
    /* Initialize extended shader constants */
    if (ISFLAG(GLOBAL_ILLUMINATION))
        setGIReflectivity(ShadingDesc_.GIReflectivity);
    
    /* Generate bloom filter shader */
    if (ISFLAG(BLOOM))
    {
        if (!BloomEffect_.createResources(Resolution))
            REMOVEFLAG(BLOOM);
    }
    
    /* Build g-buffer */
    if ( !GBuffer_.createGBuffer(
            Resolution, MultiSampling, ISFLAG(HAS_LIGHT_MAP),
            ISFLAG(GLOBAL_ILLUMINATION) && ISFLAG(USE_VPL_OPTIMIZATION)) )
    {
        return false;
    }

    /* Create light grid */
    if (ISFLAG(TILED_SHADING))
    {
        if (LightGrid_.createGrid(Resolution, LightGridDesc_.TileCount, MaxPointLightCount_))
        {
            DeferredShader_->addShaderResource(LightGrid_.getLGShaderResource());
            DeferredShader_->addShaderResource(LightGrid_.getTLIShaderResource());
        }
        else
            return false;
    }

    /* Print information */
    printInfo();
    
    return true;
}

void DeferredRenderer::releaseResources()
{
    deleteShaders();
    GBuffer_.deleteGBuffer();
    ShadowMapper_.deleteShadowMaps();
}

void DeferredRenderer::renderScene(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, Texture* RenderTarget, bool UseDefaultGBufferShader)
{
    gDRFlags = Flags_;
    
    if ( Graph && GBufferShader_ && DeferredShader_ && ( !RenderTarget || RenderTarget->getRenderTarget() ) )
    {
        updateLightSources(Graph, ActiveCamera);
        
        renderSceneIntoGBuffer(Graph, ActiveCamera, UseDefaultGBufferShader);
        
        if (ISFLAG(GLOBAL_ILLUMINATION) && ISFLAG(USE_VPL_OPTIMIZATION))
            renderLowResVPLShading();
        
        renderDeferredShading(RenderTarget);
        
        if (ISFLAG(BLOOM))
            BloomEffect_.drawEffect(RenderTarget);

        if (ISFLAG(DEBUG_VIRTUALPOINTLIGHTS) && DebugVPL_.Enabled)
            renderDebugVPLs(ActiveCamera);
    }
    #ifdef SP_DEBUGMODE
    else if ( !Graph || ( RenderTarget && !RenderTarget->getRenderTarget() ) )
        io::Log::debug("DeferredRenderer::renderScene");
    #endif
}

void DeferredRenderer::setGIReflectivity(f32 Reflectivity)
{
    /* Store new GI reflectivity setting */
    ShadingDesc_.GIReflectivity = Reflectivity;
    
    /* Update deferred shader constant */
    if (DeferredShader_)
    {
        Shader* FragShd = DeferredShader_->getPixelShader();
        
        if (RenderSys_ == RENDERER_DIRECT3D11)
            FragShd->setConstantBuffer("BufferShading", &ShadingDesc_);
        else
            FragShd->setConstant("GIReflectivity", ShadingDesc_.GIReflectivity);
    }
    
    /* Update low-resolution VPL shader constant */
    if (LowResVPLShader_)
    {
        Shader* FragShd = LowResVPLShader_->getPixelShader();
        
        #if 0//!!!
        if (RenderSys_ == RENDERER_DIRECT3D11)
            //todo
        else
        #endif
            FragShd->setConstant("GIReflectivity", ShadingDesc_.GIReflectivity);
    }
}

void DeferredRenderer::setAmbientColor(const dim::vector3df &ColorVec)
{
    /* Store new color setting */
    ShadingDesc_.AmbientColor = ColorVec;
    
    /* Update shader constant */
    if (!DeferredShader_)
        return;

    Shader* FragShd = DeferredShader_->getPixelShader();
    
    if (RenderSys_ == RENDERER_DIRECT3D11)
        FragShd->setConstantBuffer("BufferShading", &ShadingDesc_);
    else
        FragShd->setConstant("AmbientColor", ShadingDesc_.AmbientColor);
}


/*
 * ======= Protected: =======
 */

void DeferredRenderer::setupFlags(s32 Flags)
{
    Flags_ = Flags;
    
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

void DeferredRenderer::updateLightSources(scene::SceneGraph* Graph, scene::Camera* ActiveCamera)
{
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_START(debTimer0)
    #endif
    
    /* Update each light source */
    f32 Color[4];
    s32 i = 0, iEx = 0;
    u32 ShadowCubeMapIndex = 0, ShadowMapIndex = 0;
    
    #ifdef _DEB_USE_LIGHT_CONSTANT_BUFFER_
    
    const s32 LightCount    = static_cast<s32>(Lights_.getCount());
    const s32 LightExCount  = static_cast<s32>(LightsEx_.getCount());
    
    SLightCB* Lit = 0;
    SLightExCB* LitEx = 0;
    
    #else
    
    const s32 LightCount    = static_cast<s32>(Lights_.size());
    const s32 LightExCount  = static_cast<s32>(LightsEx_.size());
    
    SLight* Lit = 0;
    SLightEx* LitEx = 0;
    
    #endif
    
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
        
        #ifdef _DEB_USE_LIGHT_CONSTANT_BUFFER_
        Lit = Lights_.getRef<SLightCB>(i);
        #else
        Lit = &(Lights_[i]);
        #endif
        
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
            #ifdef _DEB_USE_LIGHT_CONSTANT_BUFFER_
            LitEx = LightsEx_.getRef<SLightExCB>(iEx);
            #else
            LitEx = &(LightsEx_[iEx]);
            #endif
            
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
                LightObj->getVolumetricRadius()*2.0f
                //1.0f//!!!
            );
        }

        ++i;
    }
    
    if (UseShadow)
        GlbRenderSys->setGlobalShaderClass(0);
    
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_PRINT("Light Setup Time: ", debTimer0)
    PERFORMANCE_QUERY_START(debTimer1)
    #endif
    
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
    
    #ifdef _DEB_USE_LIGHT_CONSTANT_BUFFER_
    
    if (ISFLAG(USE_VPL_OPTIMIZATION))
    {
        /* Update low-resolution VPL shader constants */
        FragShd = LowResVPLShader_->getPixelShader();
        
        FragShd->setConstant("LightCount", i);
        
        FragShd->setConstantBuffer("BufferLight", Lights_.getArray());
        FragShd->setConstantBuffer("BufferLightEx", LightsEx_.getArray());
    }
    
    #endif
    
    /* Update deferred shader constants */
    FragShd = DeferredShader_->getPixelShader();
    
    if (RenderSys_ == RENDERER_DIRECT3D11)
    {
        ShadingDesc_.LightCount = i;
        FragShd->setConstantBuffer("BufferShading", &ShadingDesc_);
    }
    else
        FragShd->setConstant(LightDesc_.LightCountConstant, i);
    
    #ifdef _DEB_USE_LIGHT_CONSTANT_BUFFER_
    
    FragShd->setConstantBuffer("BufferLight", Lights_.getArray());
    FragShd->setConstantBuffer("BufferLightEx", LightsEx_.getArray());

    #else

    for (s32 c = 0; c < i; ++c)
    {
        const SLight& Lit = Lights_[c];
        
        FragShd->setConstant(Lit.Constants[0], dim::vector4df(Lit.Position, Lit.InvRadius)  );
        FragShd->setConstant(Lit.Constants[1], Lit.Color                                    );
        FragShd->setConstant(Lit.Constants[2], Lit.Type                                     );
        FragShd->setConstant(Lit.Constants[3], Lit.ShadowIndex                              );
        FragShd->setConstant(Lit.Constants[4], Lit.UsedForLightmaps                         );
    }
    
    for (s32 c = 0; c < iEx; ++c)
    {
        const SLightEx& Lit = LightsEx_[c];
        
        FragShd->setConstant(Lit.Constants[0], Lit.ViewProjection   );
        FragShd->setConstant(Lit.Constants[1], Lit.Direction        );
        FragShd->setConstant(Lit.Constants[2], Lit.SpotTheta        );
        FragShd->setConstant(Lit.Constants[3], Lit.SpotPhiMinusTheta);
        
        if (ISFLAG(GLOBAL_ILLUMINATION))
            FragShd->setConstant(Lit.Constants[4], Lit.InvViewProjection);
    }

    #endif

    /* Build light grid if tiled shading is used */
    if (ISFLAG(TILED_SHADING))
    {
        LightGrid_.updateLights(PointLightsPositionAndRadius_, i);
        LightGrid_.build(Graph, ActiveCamera);
    }
    
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_PRINT("Light Shader Upload Time: ", debTimer1)
    #endif
}

void DeferredRenderer::renderSceneIntoGBuffer(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, bool UseDefaultGBufferShader)
{
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_START(debTimer2)
    #endif
    
    ShaderClass* PrevShaderClass = 0;
    
    if (UseDefaultGBufferShader)
    {
        PrevShaderClass = GlbRenderSys->getGlobalShaderClass();
        GlbRenderSys->setGlobalShaderClass(GBufferShader_);
    }
    
    GBuffer_.bindRenderTargets();
    GlbRenderSys->clearBuffers();
    
    GlbEngineDev->setActiveSceneGraph(Graph);
    
    if (ActiveCamera)
        Graph->renderScene(ActiveCamera);
    else
        Graph->renderScene();
    
    if (UseDefaultGBufferShader)
        GlbRenderSys->setGlobalShaderClass(PrevShaderClass);
    
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_PRINT("GBuffer Render Time: ", debTimer2)
    #endif
}

void DeferredRenderer::renderLowResVPLShading()
{
    GlbRenderSys->setRenderTarget(GBuffer_.getTexture(GBuffer::RENDERTARGET_LOWRES_VPL));
    
    GlbRenderSys->setRenderMode(RENDERMODE_DRAWING_2D);
    LowResVPLShader_->bind();
    {
        /* Bind shadow map texture-array and draw low-resolution VPL deferred-shading */
        ShadowMapper_.bind(1);
        
        GBuffer_.drawLowResVPLDeferredShading();
        
        ShadowMapper_.unbind(1);
    }
    LowResVPLShader_->unbind();
    
    GlbRenderSys->setRenderTarget(0);
}

void DeferredRenderer::renderDeferredShading(Texture* RenderTarget)
{
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_START(debTimer3)
    #endif
    
    /* Get shadow map layer base index */
    s32 ShadowMapLayerBase = 2;
    s32 NextLayerBase = 0;
    
    if (ISFLAG(HAS_LIGHT_MAP))
        ++ShadowMapLayerBase;
    if (ISFLAG(USE_VPL_OPTIMIZATION))
        ++ShadowMapLayerBase;
    
    /* Draw deferred shading 2D quad */
    if (ISFLAG(BLOOM))
        BloomEffect_.bindRenderTargets();
    else
        GlbRenderSys->setRenderTarget(RenderTarget);
    
    GlbRenderSys->setRenderMode(RENDERMODE_DRAWING_2D);
    DeferredShader_->bind();
    {
        /* Bind texture layers for deferred-rendering */
        NextLayerBase = ShadowMapper_.bind(ShadowMapLayerBase);
        
        if (ISFLAG(TILED_SHADING))
            LightGrid_.bind(NextLayerBase);

        /* Draw the deferred shading 2D quad */
        GBuffer_.drawDeferredShading();
        
        /* Unbind texture layers for deferred-rendering */
        NextLayerBase = ShadowMapper_.unbind(ShadowMapLayerBase);

        if (ISFLAG(TILED_SHADING))
            LightGrid_.unbind(NextLayerBase);
    }
    DeferredShader_->unbind();
    
    GlbRenderSys->setRenderTarget(0);
    
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_PRINT("Deferred Shading Time: ", debTimer3)
    #endif
}

void DeferredRenderer::renderDebugVPLs(scene::Camera* ActiveCamera)
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

bool DeferredRenderer::buildShader(
    const io::stringc &Name,
    ShaderClass* &ShdClass,
    VertexFormat* VertFmt,
    
    const std::list<io::stringc>* ShdBufferVertex,
    const std::list<io::stringc>* ShdBufferPixel,
    
    const io::stringc &VertexMain,
    const io::stringc &PixelMain,
    
    s32 Flags)
{
    if (!ShaderClass::build(Name, ShdClass, VertFmt, ShdBufferVertex, ShdBufferPixel, VertexMain, PixelMain, Flags))
    {
        deleteShaders();
        return false;
    }
    return true;
}

void DeferredRenderer::deleteShaders()
{
    deleteShader(GBufferShader_     );
    deleteShader(DeferredShader_    );
    deleteShader(LowResVPLShader_   );
    deleteShader(ShadowShader_      );
    deleteShader(DebugVPL_.ShdClass );
}

void DeferredRenderer::deleteShader(ShaderClass* &ShdClass)
{
    GlbRenderSys->deleteShaderClass(ShdClass, true);
    ShdClass = 0;
}

void DeferredRenderer::createVertexFormats()
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
    
    /* Create 2D image vertex format */
    ImageVertexFormat_.clear();
    
    ImageVertexFormat_.addCoord(DATATYPE_FLOAT, 2);
    ImageVertexFormat_.addTexCoord();
}

void DeferredRenderer::printInfo()
{
    /* Print basic information */
    io::Log::message("=== Setup Deferred Renderer: === ");
    io::Log::message(
        "Max. Point Lights (" + io::stringc(MaxPointLightCount_) +
        "), Max. Spot Lights (" + io::stringc(MaxSpotLightCount_) + "):"
    );
    
    /* Print flags info */
    io::stringc FlagsStr;
    
    pushBackInfo(FlagsStr, DEFERREDFLAG_DEBUG_GBUFFER,          "Debug"                 );
    pushBackInfo(FlagsStr, DEFERREDFLAG_HAS_LIGHT_MAP,          "Lightmaps"             );
    pushBackInfo(FlagsStr, DEFERREDFLAG_NORMAL_MAPPING,         "Bump Mapping"          );
    pushBackInfo(FlagsStr, DEFERREDFLAG_PARALLAX_MAPPING,       "Relief Mapping"        );
    pushBackInfo(FlagsStr, DEFERREDFLAG_SHADOW_MAPPING,         "Shadow Mapping"        );
    pushBackInfo(FlagsStr, DEFERREDFLAG_GLOBAL_ILLUMINATION,    "Global Illumination"   );
    pushBackInfo(FlagsStr, DEFERREDFLAG_TILED_SHADING,          "Tiled Shading"         );
    pushBackInfo(FlagsStr, DEFERREDFLAG_BLOOM,                  "Bloom"                 );
    
    if (FlagsStr.empty())
        io::Log::message("Flags { None }");
    else
        io::Log::message("Flags { " + FlagsStr + " }");
    
    /* Print tiled shading info */
    if (ISFLAG(TILED_SHADING))
    {
        const dim::size2di& GridSize(LightGrid_.getTileCount());
        io::Log::message(
            "Tiled Shading Raster (" + io::stringc(GridSize.Width) + " x " + io::stringc(GridSize.Height) + ")"
        );
    }
    
    io::Log::message("");
}

void DeferredRenderer::pushBackInfo(io::stringc &FlagsStr, u32 Flag, const io::stringc &Desc)
{
    if (Flags_ & Flag)
    {
        if (!FlagsStr.empty())
            FlagsStr += ", ";
        FlagsStr += Desc;
    }
}

#ifndef _DEB_USE_LIGHT_CONSTANT_BUFFER_

/*
 * SLight structure
 */

DeferredRenderer::SLight::SLight() :
    InvRadius       (0.001f ),
    Color           (1.0f   ),
    Type            (0      ),
    ShadowIndex     (-1     ),
    UsedForLightmaps(0      )
{
}
DeferredRenderer::SLight::~SLight()
{
}


/*
 * SLightEx structure
 */

DeferredRenderer::SLightEx::SLightEx() :
    Direction           (0.0f, 0.0f, 1.0f   ),
    SpotTheta           (0.0f               ),
    SpotPhiMinusTheta   (0.0f               )
{
}
DeferredRenderer::SLightEx::~SLightEx()
{
}

#endif


/*
 * SShadingDescCB structure
 */

DeferredRenderer::SShadingDescCB::SShadingDescCB() :
    AmbientColor    (0.07f  ),
    GIReflectivity  (0.1f   ),
    LightCount      (0      )
{
}
DeferredRenderer::SShadingDescCB::~SShadingDescCB()
{
}


/*
 * SDebugVPL structure
 */

DeferredRenderer::SDebugVPL::SDebugVPL() :
    ShdClass    (0      ),
    VtxFormat   (0      ),
    Enabled     (true   )
{
}
DeferredRenderer::SDebugVPL::~SDebugVPL()
{
}

void DeferredRenderer::SDebugVPL::load()
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
void DeferredRenderer::SDebugVPL::unload()
{
    if (VtxFormat)
    {
        Model.deleteMeshBuffer();
        GlbRenderSys->deleteVertexFormat(VtxFormat);
        VtxFormat = 0;
    }
}


#undef ISFLAG
#undef REMOVEFLAG


} // /namespace video

} // /namespace sp


#endif



// ================================================================================