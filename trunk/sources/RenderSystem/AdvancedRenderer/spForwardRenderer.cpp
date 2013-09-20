/*
 * Forward renderer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/AdvancedRenderer/spForwardRenderer.hpp"

#if defined(SP_COMPILE_WITH_ADVANCEDRENDERER)


#include "RenderSystem/AdvancedRenderer/spDeferredRendererShaderCallbacks.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Platform/spSoftPixelDevice.hpp"


namespace sp
{

extern SoftPixelDevice* GlbEngineDev;
extern video::RenderSystem* GlbRenderSys;
extern scene::SceneGraph* GlbSceneGraph;

namespace video
{


#define ISFLAG(n)       ((Config_.Flags & RENDERERFLAG_##n) != 0)
#define ISRENDERER(n)   (RenderSys_ == RENDERER_##n)
#define ADDOP(n)        Shader::addOption(CompilerOp, n)

extern s32 GlbDfRnFlags;
extern s32 GlbDfRnLightGridRowSize;


ForwardRenderer::ForwardRenderer() :
    AdvancedRenderer    (ADVANCEDRENDERER_FORWARD   ),
    ForwardShader_      (0                          ),
    ShadowMapStartSlot_ (0                          )
{
}
ForwardRenderer::~ForwardRenderer()
{
    releaseResources();
}

io::stringc ForwardRenderer::getDescription() const
{
    return "Forward Renderer";
}

void ForwardRenderer::renderScene(scene::SceneGraph* Graph, scene::Camera* ActiveCamera)
{
    if (!Graph || !ForwardShader_)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug(
            "ForwardRenderer::renderScene",
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
    
    renderSceneIntoFrameBuffer(Graph, ActiveCamera);
    
    if (ISFLAG(BLOOM))
        BloomEffect_.drawEffect(RenderTarget);
    
    if (ISFLAG(DEBUG_VIRTUALPOINTLIGHTS) && DebugVPL_.Enabled)
        renderDebugVPLs(ActiveCamera);
}

void ForwardRenderer::setGIReflectivity(f32 Reflectivity)
{
    /* Store new GI reflectivity setting */
    AdvancedRenderer::setGIReflectivity(Reflectivity);
    
    /* Update shader constant */
    Shader* FragShd = ForwardShader_->getPixelShader();
    
    if (RenderSys_ == RENDERER_DIRECT3D11)
        FragShd->setConstantBuffer("BufferShading", &ShadingDesc_);
    else
        FragShd->setConstant("GIReflectivity", ShadingDesc_.GIReflectivity);
}

void ForwardRenderer::setAmbientColor(const dim::vector3df &ColorVec)
{
    /* Store new color setting */
    AdvancedRenderer::setAmbientColor(ColorVec);
    
    //todo ...
}


/*
 * ======= Protected: =======
 */

void ForwardRenderer::updateLightSources(scene::SceneGraph* Graph, scene::Camera* ActiveCamera)
{
    s32 i = 0, iEx = 0;
    AdvancedRenderer::updateLightSources(Graph, ActiveCamera, 0, i, iEx);
    
    Shader* FragShd = 0;
    
    /* Update deferred shader constants */
    FragShd = ForwardShader_->getPixelShader();
    
    /*if (RenderSys_ == RENDERER_DIRECT3D11)
    {
        ShadingDesc_.LightCount = i;
        FragShd->setConstantBuffer("BufferShading", &ShadingDesc_);
    }
    else*/
        FragShd->setConstant(LightDesc_.LightCountConstant, i);
    
    FragShd->setConstantBuffer("BufferLight", Lights_.getArray());
    FragShd->setConstantBuffer("BufferLightEx", LightsEx_.getArray());
}

void ForwardRenderer::renderSceneIntoFrameBuffer(scene::SceneGraph* Graph, scene::Camera* ActiveCamera)
{
    GlbEngineDev->setActiveSceneGraph(Graph);
    
    GlbRenderSys->setGlobalShaderClass(ForwardShader_);
    {
        if (ISFLAG(SHADOW_MAPPING))
            ShadowMapper_.bind(ShadowMapStartSlot_);
        
        Graph->renderScene(ActiveCamera);
        
        if (ISFLAG(SHADOW_MAPPING))
            ShadowMapper_.unbind(ShadowMapStartSlot_);
    }
    GlbRenderSys->setGlobalShaderClass(0);
}

bool ForwardRenderer::loadAllShaders()
{
    return
        AdvancedRenderer::loadAllShaders() &&
        loadForwardShader               ();
}

void ForwardRenderer::deleteAllShaders()
{
    AdvancedRenderer::deleteAllShaders();
    deleteShader(ForwardShader_);
}

bool ForwardRenderer::setupFinalResources()
{
    if (!AdvancedRenderer::setupFinalResources())
        return false;
    
    #if 0
    
    /* Bind light grid shader resoruces to forwerd shader */
    if (ISFLAG(TILED_SHADING))
    {
        ForwardShader_->addShaderResource(LightGrid_.getLGShaderResource());
        ForwardShader_->addShaderResource(LightGrid_.getTLIShaderResource());
    }
    
    #endif
    
    return true;
}


/*
 * ======= Private: =======
 */

bool ForwardRenderer::loadForwardShader()
{
    const bool IsGL = ISRENDERER(OPENGL);
    
    s32 Flags = 0;
    
    /* Setup shader compilation options */
    std::list<io::stringc> CompilerOp;
    setupForwardCompilerOptions(CompilerOp);
    
    /* Setup forward shader source code */
    std::list<io::stringc> ForwardShdBufVert(CompilerOp), ForwardShdBufFrag(CompilerOp);
    
    Shader::addOption(ForwardShdBufVert, "VARYING_OUT out");
    
    switch (RenderSys_)
    {
        case RENDERER_OPENGL:
        {
            /* Use g-buffer vertex shader and forward fragment shader */
            #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
            ForwardShdBufVert.push_back(
                #include "Resources/spGBufferShaderStr.glvert"
            );
            ForwardShdBufFrag.push_back(
                #include "Resources/spForwardShaderStr.glfrag"
            );
            #else
            io::FileSystem fsys;
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spGBufferShader.glvert", ForwardShdBufVert);
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spForwardShader.glfrag", ForwardShdBufFrag);
            #endif
            
            Flags = SHADERBUILD_GLSL;
        }
        break;
        
        #if 0
        case RENDERER_DIRECT3D11:
        {
            #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
            ForwardShdBufVert.push_back(
                #include "Resources/spForwardShaderStr.hlsl"
            );
            #else
            io::FileSystem fsys;
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spForwardShader.hlsl", ForwardShdBufVert);
            #endif
            
            Flags = SHADERBUILD_HLSL5;
        }
        break;
        #endif
        
        default:
        {
            io::Log::error("Forward shader is not provided for this render system");
            return false;
        }
    }
    
    /* Generate forward shader */
    if (!buildShader(
            "forward", ForwardShader_, getVertexFormat(), &ForwardShdBufVert,
            IsGL ? &ForwardShdBufFrag : &ForwardShdBufVert,
            "VertexMain", "PixelMain", Flags))
    {
        return false;
    }
    
    /*if (ISRENDERER(DIRECT3D11))
    {
        ForwardShader_->setObjectCallback(FwRnForwardObjectShaderCallbackCB);
        ForwardShader_->setSurfaceCallback(DfRnGBufferSurfaceShaderCallbackCB);
    }
    else*/
    {
        ForwardShader_->setObjectCallback(FwRnForwardObjectShaderCallback);
        ForwardShader_->setSurfaceCallback(DfRnGBufferSurfaceShaderCallback);
    }
    
    /* Setup uniforms/ constant buffers */
    if (IsGL)
        setupForwardSampler(ForwardShader_->getPixelShader());
    
    setupLightShaderConstants(ForwardShader_->getPixelShader());
    
    if (ISFLAG(GLOBAL_ILLUMINATION))
    {
        setAmbientColor(ShadingDesc_.AmbientColor);
        setupVPLOffsets(ForwardShader_->getPixelShader(), "BufferVPL", AdvancedRenderer::VPL_COUNT);
    }
    
    return true;
}

void ForwardRenderer::setupForwardCompilerOptions(std::list<io::stringc> &CompilerOp)
{
    setupGeometryCompilerOptions(CompilerOp);
    setupShadingCompilerOptions(CompilerOp, true);
}

void ForwardRenderer::setupForwardSampler(Shader* ShaderObj)
{
    /* Setup geometry sampler first */
    s32 SamplerIndex = 0;
    setupGeometrySampler(ShaderObj, SamplerIndex);
    
    /* Then setup shadow map sampler */
    ShadowMapStartSlot_ = SamplerIndex;
    
    if (ISFLAG(SHADOW_MAPPING))
    {
        ShaderObj->setConstant("DirLightShadowMaps", SamplerIndex++);
        ShaderObj->setConstant("PointLightShadowMaps", SamplerIndex++);
        
        if (ISFLAG(GLOBAL_ILLUMINATION))
        {
            ShaderObj->setConstant("DirLightDiffuseMaps", SamplerIndex++);
            ShaderObj->setConstant("PointLightDiffuseMaps", SamplerIndex++);
        }
    }
}


#undef ISFLAG
#undef ISRENDERER
#undef ADDOP


} // /namespace video

} // /namespace sp


#endif



// ================================================================================