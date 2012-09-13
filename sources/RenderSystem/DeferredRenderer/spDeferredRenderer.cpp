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
#include "SceneGraph/spSceneGraph.hpp"
#include "Platform/spSoftPixelDevice.hpp"
#include "Base/spSharedObjects.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace video
{


static s32 DefRendererFlags = 0;

static void GBufferShaderCallback(video::ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    video::Shader* VertShd = ShdClass->getVertexShader();
    video::Shader* FragShd = ShdClass->getPixelShader();
    
    const dim::vector3df ViewPosition(
        __spSceneManager->getActiveCamera()->getPosition(true)
    );
    
    VertShd->setConstant(
        "WorldViewProjectionMatrix",
        __spVideoDriver->getProjectionMatrix() * __spVideoDriver->getViewMatrix() * __spVideoDriver->getWorldMatrix()
    );
    VertShd->setConstant(
        "WorldMatrix",
        __spVideoDriver->getWorldMatrix()
    );
    VertShd->setConstant("ViewPosition", ViewPosition);
    
    if (DefRendererFlags & DEFERREDFLAG_PARALLAX_MAPPING)
    {
        FragShd->setConstant("EnablePOM", true);
        FragShd->setConstant("MinSamplesPOM", 50);
        FragShd->setConstant("MaxSamplesPOM", 50);
        FragShd->setConstant("HeightMapScale", 0.015f);
        FragShd->setConstant("ParallaxViewRange", 2.0f);
    }
    
    FragShd->setConstant("ViewPosition", ViewPosition);
    FragShd->setConstant("SpecularFactor", 1.0f);
    
}

static void DeferredShaderCallback(video::ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    video::Shader* VertShd = ShdClass->getVertexShader();
    video::Shader* FragShd = ShdClass->getPixelShader();
    
    const dim::matrix4f ViewTransform(
        __spSceneManager->getActiveCamera()->getTransformMatrix(true)
    );
    
    VertShd->setConstant("ProjectionMatrix", __spVideoDriver->getProjectionMatrix());
    
    FragShd->setConstant("ViewTransform", ViewTransform);
    FragShd->setConstant("ViewPosition", ViewTransform.getPosition());
    FragShd->setConstant("ScreenWidth", static_cast<f32>(gSharedObjects.ScreenWidth));
    FragShd->setConstant("ScreenHeight", static_cast<f32>(gSharedObjects.ScreenHeight));
    
}


DeferredRenderer::DeferredRenderer() :
    GBufferShader_  (0                              ),
    DeferredShader_ (0                              ),
    BloomShaderHRP_ (0                              ),
    BloomShaderVRP_ (0                              ),
    Flags_          (0                              ),
    Lights_         (DeferredRenderer::MAX_LIGHTS   ),
    LightsEx_       (DeferredRenderer::MAX_EX_LIGHTS)
{
    if (!gSharedObjects.CgContext)
        __spDevice->createCgShaderContext();
}
DeferredRenderer::~DeferredRenderer()
{
    deleteShaders();
    GBuffer_.deleteGBuffer();
}

bool DeferredRenderer::generateResources(s32 Flags)
{
    /* Setup shader compilation options */
    Flags_ = Flags;
    
    std::vector<const c8*> GBufferCompilerOp, DeferredCompilerOp;
    
    if (Flags_ & DEFERREDFLAG_USE_TEXTURE_MATRIX)
        GBufferCompilerOp.push_back("-DUSE_TEXTURE_MATRIX");
    if (Flags_ & DEFERREDFLAG_HAS_SPECULAR_MAP)
        GBufferCompilerOp.push_back("-DHAS_SPECULAR_MAP");
    if (Flags_ & DEFERREDFLAG_NORMAL_MAPPING)
    {
        GBufferCompilerOp.push_back("-DNORMAL_MAPPING");
        if (Flags_ & DEFERREDFLAG_PARALLAX_MAPPING)
            GBufferCompilerOp.push_back("-DPARALLAX_MAPPING");
    }
    if (Flags_ & DEFERREDFLAG_DEBUG_GBUFFER)
        DeferredCompilerOp.push_back("-DDEBUG_GBUFFER");
    if (Flags_ & DEFERREDFLAG_BLOOM)
        DeferredCompilerOp.push_back("-DBLOOM_FILTER");
    
    GBufferCompilerOp.push_back(0);
    DeferredCompilerOp.push_back(0);
    
    /* Create new vertex formats and delete old shaders */
    createVertexFormats();
    deleteShaders();
    
    /* Get shader buffers */
    std::vector<io::stringc> GBufferShdBuf(1), DeferredShdBuf(1), BloomShdBuf(1);
    
    GBufferShdBuf[0] = (
        #include "RenderSystem/DeferredRenderer/spGBufferShaderStr.h"
    );
    
    DeferredShdBuf[0] = (
        #include "RenderSystem/DeferredRenderer/spDeferredShaderStr.h"
    );
    
    /* Generate g-buffer shader */
    if (!buildShader(
            "g-buffer", GBufferShader_, &VertexFormat_, GBufferShdBuf, &GBufferCompilerOp[0],
            "VertexMain", "PixelMain", (Flags_ & DEFERREDFLAG_TESSELLATION) != 0))
    {
        return false;
    }
    
    GBufferShader_->setObjectCallback(GBufferShaderCallback);
    
    /* Generate deferred shader */
    if (!buildShader("deferred", DeferredShader_, &ImageVertexFormat_, DeferredShdBuf, &DeferredCompilerOp[0]))
        return false;
    
    DeferredShader_->setObjectCallback(DeferredShaderCallback);
    
    /* Generate bloom filter shader */
    if (Flags_ & DEFERREDFLAG_BLOOM)
    {
        BloomShdBuf[0] = (
            #include "RenderSystem/DeferredRenderer/spBloomFilterStr.h"
        );
        
        if ( !buildShader("bloom", BloomShaderHRP_, &ImageVertexFormat_, BloomShdBuf, 0, "VertexMain", "PixelMainHRP") ||
             !buildShader("bloom", BloomShaderVRP_, &ImageVertexFormat_, BloomShdBuf, 0, "VertexMain", "PixelMainVRP") )
        {
            return false;
        }
    }
    
    /* Build g-buffer */
    return GBuffer_.createGBuffer(
        dim::size2di(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight)
    );
}

void DeferredRenderer::renderScene(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, video::Texture* RenderTarget, bool UseDefaultGBufferShader)
{
    DefRendererFlags = Flags_;
    
    if ( Graph && GBufferShader_ && DeferredShader_ && ( !RenderTarget || RenderTarget->getRenderTarget() ) )
    {
        updateLightSources(Graph, ActiveCamera);
        renderSceneIntoGBuffer(Graph, ActiveCamera, UseDefaultGBufferShader);
        renderDeferredShading(RenderTarget);
    }
    #ifdef SP_DEBUGMODE
    else if ( !Graph || ( RenderTarget && !RenderTarget->getRenderTarget() ) )
        io::Log::debug("DeferredRenderer::renderScene");
    #endif
}


/*
 * ======= Protected: =======
 */

void DeferredRenderer::updateLightSources(scene::SceneGraph* Graph, scene::Camera* ActiveCamera)
{
    /* Update each light source */
    f32 Color[4];
    s32 i = 0, iEx = 0;
    
    std::list<scene::Light*>::const_iterator it = Graph->getLightList().begin(), itEnd = Graph->getLightList().end();
    
    for (; it != itEnd && i < DeferredRenderer::MAX_LIGHTS; ++it)
    {
        scene::Light* Node = *it;
        
        if (!Node->getVisible())
            continue;
        
        SLight* Lit = &(Lights_[i]);
        
        Node->getDiffuseColor().getFloatArray(Color);
        
        /* Copy basic data */
        Lit->Position   = Node->getPosition(true);
        Lit->Radius     = (Node->getVolumetric() ? Node->getVolumetricRadius() : 1000.0f);
        Lit->Color      = dim::vector3df(Color[0], Color[1], Color[2]);
        //Lit->Type       = static_cast<u8>(Node->getLightModel());
        
        /* Copy extended data */
        
        //...
        
        ++i;
    }
    
    /* Update shader constants */
    video::Shader* FragShd = DeferredShader_->getPixelShader();
    
    FragShd->setConstant("LightCount", i);
    FragShd->setConstant("LightExCount", iEx);
    
    FragShd->setConstant("Lights", &(Lights_[0].Position.X), sizeof(SLight) / sizeof(f32) * i);
    FragShd->setConstant("LightsEx", &(LightsEx_[0].Direction.X), sizeof(SLightEx) / sizeof(f32) * iEx);
}

void DeferredRenderer::renderSceneIntoGBuffer(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, bool UseDefaultGBufferShader)
{
    video::ShaderClass* PrevShaderClass = 0;
    
    if (UseDefaultGBufferShader)
    {
        PrevShaderClass = __spVideoDriver->getGlobalShaderClass();
        __spVideoDriver->setGlobalShaderClass(GBufferShader_);
    }
    
    GBuffer_.bindRenderTarget();
    __spVideoDriver->clearBuffers();
    
    __spDevice->setActiveSceneGraph(Graph);
    
    if (ActiveCamera)
        Graph->renderScene(ActiveCamera);
    else
        Graph->renderScene();
    
    if (UseDefaultGBufferShader)
        __spVideoDriver->setGlobalShaderClass(PrevShaderClass);
}

void DeferredRenderer::renderDeferredShading(video::Texture* RenderTarget)
{
    __spVideoDriver->setRenderTarget(RenderTarget);
    {
        __spVideoDriver->beginDrawing2D();
        {
            DeferredShader_->bind();
            {
                GBuffer_.draw2DImage();
            }
            DeferredShader_->unbind();
        }
        __spVideoDriver->endDrawing2D();
    }
    __spVideoDriver->setRenderTarget(0);
}

bool DeferredRenderer::buildShader(
    const io::stringc &Name, ShaderClass* &ShdClass, VertexFormat* VertFmt,
    const std::vector<io::stringc> &ShdBuffer, const c8** CompilerOptions,
    const io::stringc &VertexMain, const io::stringc &PixelMain,
    bool HasTessellation)
{
    /* Create shader class */
    ShdClass = __spVideoDriver->createCgShaderClass(&ImageVertexFormat_);
    
    if (!ShdClass)
        return false;
    
    /* Create vertex- and pixel shaders */
    __spVideoDriver->createCgShader(
        ShdClass, video::SHADER_VERTEX, video::CG_VERSION_2_0, ShdBuffer, VertexMain, CompilerOptions
    );
    __spVideoDriver->createCgShader(
        ShdClass, video::SHADER_PIXEL, video::CG_VERSION_2_0, ShdBuffer, PixelMain, CompilerOptions
    );
    
    if (HasTessellation)
    {
        /* Create hull- and domain shaders */
        __spVideoDriver->createCgShader(
            ShdClass, video::SHADER_HULL, video::CG_VERSION_2_0, ShdBuffer, "HullMain", CompilerOptions
        );
        __spVideoDriver->createCgShader(
            ShdClass, video::SHADER_DOMAIN, video::CG_VERSION_2_0, ShdBuffer, "DomainMain", CompilerOptions
        );
    }
    
    /* Compile and link shaders */
    if (!ShdClass->link())
    {
        io::Log::error("Compiling " + Name + " shader failed");
        deleteShaders();
        return false;
    }
    
    return true;
}

void DeferredRenderer::deleteShaders()
{
    __spVideoDriver->deleteShaderClass(GBufferShader_, true);
    __spVideoDriver->deleteShaderClass(DeferredShader_, true);
    __spVideoDriver->deleteShaderClass(BloomShaderHRP_, true);
    __spVideoDriver->deleteShaderClass(BloomShaderVRP_, true);
    
    GBufferShader_ = 0;
    DeferredShader_ = 0;
    BloomShaderHRP_ = 0;
    BloomShaderVRP_ = 0;
}

void DeferredRenderer::createVertexFormats()
{
    /* Create object vertex format */
    VertexFormat_.clear();
    
    VertexFormat_.addCoord();
    VertexFormat_.addNormal();
    VertexFormat_.addTexCoord();
    
    if (Flags_ & DEFERREDFLAG_NORMAL_MAPPING)
    {
        /* Add information for normal-mapping (tangent and binormal is texture-coordinates) */
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 3);
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 3);
    }
    
    /* Create 2D image vertex format */
    ImageVertexFormat_.clear();
    
    ImageVertexFormat_.addCoord(DATATYPE_FLOAT, 2);
    ImageVertexFormat_.addTexCoord();
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
