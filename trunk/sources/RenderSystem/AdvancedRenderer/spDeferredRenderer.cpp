/*
 * Deferred renderer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/AdvancedRenderer/spDeferredRenderer.hpp"

#if defined(SP_COMPILE_WITH_ADVANCEDRENDERER)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Platform/spSoftPixelDevice.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern SoftPixelDevice* GlbEngineDev;
extern video::RenderSystem* GlbRenderSys;
extern scene::SceneGraph* GlbSceneGraph;

namespace video
{


#define ISFLAG(n) ((Config_.Flags & RENDERERFLAG_##n) != 0)

extern s32 GlbDfRnFlags;
extern s32 GlbDfRnLightGridRowSize;


DeferredRenderer::DeferredRenderer() :
    AdvancedRenderer        (ADVANCEDRENDERER_DEFERRED  ),
    GBufferShader_          (0                          ),
    DeferredShader_         (0                          ),
    LowResVPLShader_        (0                          ),
    UseDefaultGBufferShader_(true                       )
{
}
DeferredRenderer::~DeferredRenderer()
{
    releaseResources();
}

io::stringc DeferredRenderer::getDescription() const
{
    return "Deferred Renderer";
}

void DeferredRenderer::releaseResources()
{
    AdvancedRenderer::releaseResources();
    
    deleteAllShaders();
    GBuffer_.deleteGBuffer();
}

void DeferredRenderer::renderScene(scene::SceneGraph* Graph, scene::Camera* ActiveCamera)
{
    if (!Graph || !GBufferShader_ || !DeferredShader_)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug(
            "DeferredRenderer::renderScene",
            Graph != 0 ? "No valid shaders" : "No valid scene graph",
            io::LOG_UNIQUE
        );
        #endif
        return;
    }
    
    /* Get active camera */
    if (!getActiveCamera(Graph, ActiveCamera))
        return;
    
    /* Store flags in global space */
    GlbDfRnFlags = Config_.Flags;
    
    Texture* RenderTarget = GlbRenderSys->getRenderTarget();
    
    /* Update light sources and render scene */
    updateLightSources(Graph, ActiveCamera);
    
    renderSceneIntoGBuffer(Graph, ActiveCamera);
    
    if (ISFLAG(GLOBAL_ILLUMINATION) && ISFLAG(USE_VPL_OPTIMIZATION))
        renderLowResVPLShading();
    
    renderDeferredShading(RenderTarget);
    
    if (ISFLAG(BLOOM))
        BloomEffect_.drawEffect(RenderTarget);
    
    if (ISFLAG(DEBUG_VIRTUALPOINTLIGHTS) && DebugVPL_.Enabled)
        renderDebugVPLs(ActiveCamera);
}

void DeferredRenderer::setGIReflectivity(f32 Reflectivity)
{
    /* Store new GI reflectivity setting */
    AdvancedRenderer::setGIReflectivity(Reflectivity);
    
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
    AdvancedRenderer::setAmbientColor(ColorVec);
    
    /* Update shader constant */
    if (DeferredShader_)
    {
        Shader* FragShd = DeferredShader_->getPixelShader();
        
        if (RenderSys_ == RENDERER_DIRECT3D11)
            FragShd->setConstantBuffer("BufferShading", &ShadingDesc_);
        else
            FragShd->setConstant("AmbientColor", ShadingDesc_.AmbientColor);
    }
}

void DeferredRenderer::setResolution(const dim::size2di &Resolution)
{
    if (Resolution_ == Resolution)
        return;
    
    GBuffer_.setResolution(Resolution);
    
    AdvancedRenderer::setResolution(Resolution);
}


/*
 * ======= Protected: =======
 */

void DeferredRenderer::updateLightSources(scene::SceneGraph* Graph, scene::Camera* ActiveCamera)
{
    s32 i = 0, iEx = 0;
    AdvancedRenderer::updateLightSources(
        Graph, ActiveCamera,
        GBuffer_.getTexture(GBuffer::RENDERTARGET_NORMAL_AND_DEPTH),
        i, iEx
    );
    
    Shader* FragShd = 0;
    
    /* Update low-resolution VPL shader constants */
    if (ISFLAG(USE_VPL_OPTIMIZATION))
    {
        FragShd = LowResVPLShader_->getPixelShader();
        
        FragShd->setConstant("LightCount", i);
        
        FragShd->setConstantBuffer("BufferLight", Lights_.getArray());
        FragShd->setConstantBuffer("BufferLightEx", LightsEx_.getArray());
    }
    
    /* Update deferred shader constants */
    FragShd = DeferredShader_->getPixelShader();
    
    if (RenderSys_ == RENDERER_DIRECT3D11)
    {
        ShadingDesc_.LightCount = i;
        FragShd->setConstantBuffer("BufferShading", &ShadingDesc_);
    }
    else
        FragShd->setConstant(LightDesc_.LightCountConstant, i);
    
    FragShd->setConstantBuffer("BufferLight", Lights_.getArray());
    FragShd->setConstantBuffer("BufferLightEx", LightsEx_.getArray());
}

void DeferredRenderer::renderSceneIntoGBuffer(scene::SceneGraph* Graph, scene::Camera* ActiveCamera)
{
    ShaderClass* PrevShaderClass = 0;
    
    if (UseDefaultGBufferShader_)
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
    
    if (UseDefaultGBufferShader_)
        GlbRenderSys->setGlobalShaderClass(PrevShaderClass);
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
}

bool DeferredRenderer::loadAllShaders()
{
    return
        AdvancedRenderer::loadAllShaders() &&
        loadGBufferShader               () &&
        loadDeferredShader              () &&
        loadLowResVPLShader             ();
}

void DeferredRenderer::deleteAllShaders()
{
    AdvancedRenderer::deleteAllShaders();
    
    deleteShader(GBufferShader_     );
    deleteShader(DeferredShader_    );
    deleteShader(LowResVPLShader_   );
}

bool DeferredRenderer::setupFinalResources()
{
    if (!AdvancedRenderer::setupFinalResources())
        return false;
    
    /* Bind light grid shader resoruces to deferred shader */
    if (ISFLAG(TILED_SHADING))
    {
        DeferredShader_->addShaderResource(LightGrid_.getLGShaderResource(),    RESOURCE_ACCESS_READ);
        DeferredShader_->addShaderResource(LightGrid_.getTLIShaderResource(),   RESOURCE_ACCESS_READ);
        
        #ifdef _DEB_DEPTH_EXTENT_
        DeferredShader_->addShaderResource(_debDepthExt_, RESOURCE_ACCESS_READ);
        #endif
    }
    
    /* Build g-buffer */
    if ( !GBuffer_.createGBuffer(
            Resolution_, Config_.MultiSampling, ISFLAG(HAS_LIGHT_MAP),
            ISFLAG(GLOBAL_ILLUMINATION) && ISFLAG(USE_VPL_OPTIMIZATION)) )
    {
        return false;
    }
    
    return true;
}

void DeferredRenderer::createVertexFormats()
{
    AdvancedRenderer::createVertexFormats();
    
    /* Create 2D image vertex format */
    ImageVertexFormat_.clear();
    
    ImageVertexFormat_.addCoord(DATATYPE_FLOAT, 2);
    ImageVertexFormat_.addTexCoord();
}


#undef ISFLAG


} // /namespace video

} // /namespace sp


#endif



// ================================================================================