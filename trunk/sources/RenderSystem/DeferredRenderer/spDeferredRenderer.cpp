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

static void GBufferShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
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
        FragShd->setConstant("MinSamplesPOM", 0);
        FragShd->setConstant("MaxSamplesPOM", 50);
        FragShd->setConstant("HeightMapScale", 0.015f);
        FragShd->setConstant("ParallaxViewRange", 2.0f);
    }
    
    FragShd->setConstant("ViewPosition", ViewPosition);
    FragShd->setConstant("SpecularFactor", 1.0f);
}

static void DeferredShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    const dim::matrix4f ViewTransform(
        __spSceneManager->getActiveCamera()->getTransformMatrix(true)
    );
    
    VertShd->setConstant("ProjectionMatrix", __spVideoDriver->getProjectionMatrix());
    
    FragShd->setConstant("ViewTransform", ViewTransform);
    FragShd->setConstant("ViewPosition", ViewTransform.getPosition());
    FragShd->setConstant("ScreenWidth", static_cast<f32>(gSharedObjects.ScreenWidth));
    FragShd->setConstant("ScreenHeight", static_cast<f32>(gSharedObjects.ScreenHeight));
}

static void ShadowShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
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
    
    FragShd->setConstant("ViewPosition", ViewPosition);
}


DeferredRenderer::DeferredRenderer() :
    GBufferShader_  (0                              ),
    DeferredShader_ (0                              ),
    BloomShaderHRP_ (0                              ),
    BloomShaderVRP_ (0                              ),
    ShadowShader_   (0                              ),
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

bool DeferredRenderer::generateResources(
    s32 Flags, s32 ShadowTexSize, u32 MaxPointLightCount, u32 MaxSpotLightCount)
{
    /* Setup shader compilation options */
    Flags_ = Flags;
    
    const bool IsGL = (__spVideoDriver->getRendererType() == RENDERER_OPENGL);
    const dim::size2di Resolution(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight);
    
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
    {
        DeferredCompilerOp.push_back("-DBLOOM_FILTER");
        if (IsGL)
            DeferredCompilerOp.push_back("-DFLIP_Y_AXIS");
    }
    if (Flags_ & DEFERREDFLAG_SHADOW_MAPPING)
    {
        GBufferCompilerOp.push_back("-DSHADOW_MAPPING");
        DeferredCompilerOp.push_back("-DSHADOW_MAPPING");
    }
    
    GBufferCompilerOp.push_back(0);
    DeferredCompilerOp.push_back(0);
    
    /* Delete old shaders and shadow maps */
    deleteShaders();
    ShadowMapper_.deleteShadowMaps();
    
    /* Create new vertex formats */
    createVertexFormats();
    
    /* Get shader buffers */
    std::vector<io::stringc> GBufferShdBuf(1), DeferredShdBuf(1);
    
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
        std::vector<io::stringc> BloomShdBuf(1);
        
        BloomShdBuf[0] = (
            #include "RenderSystem/DeferredRenderer/spBloomFilterStr.h"
        );
        
        if ( !buildShader("bloom", BloomShaderHRP_, &ImageVertexFormat_, BloomShdBuf, 0, "VertexMain", "PixelMainHRP") ||
             !buildShader("bloom", BloomShaderVRP_, &ImageVertexFormat_, BloomShdBuf, 0, "VertexMain", "PixelMainVRP") )
        {
            return false;
        }
        
        /* Compute bloom filter offsets and weights */
        BloomFilter_.computeWeights();
        BloomFilter_.computeOffsets(Resolution);
        
        /* Setup gaussian shader constants */
        Shader* VertShdH = BloomShaderHRP_->getVertexShader();
        Shader* FragShdH = BloomShaderHRP_->getPixelShader();
        
        Shader* VertShdV = BloomShaderVRP_->getVertexShader();
        Shader* FragShdV = BloomShaderVRP_->getPixelShader();
        
        dim::matrix4f ProjMat;
        ProjMat.make2Dimensional(
            gSharedObjects.ScreenWidth,
            gSharedObjects.ScreenHeight,
            gSharedObjects.ScreenWidth,
            gSharedObjects.ScreenHeight
        );
        
        VertShdH->setConstant("ProjectionMatrix", ProjMat);
        FragShdH->setConstant("BlurOffsets", BloomFilter_.BlurOffsets, SBloomFilter::FILTER_SIZE*2);
        FragShdH->setConstant("BlurWeights", BloomFilter_.BlurWeights, SBloomFilter::FILTER_SIZE);
        
        VertShdV->setConstant("ProjectionMatrix", ProjMat);
        FragShdV->setConstant("BlurOffsets", BloomFilter_.BlurOffsets, SBloomFilter::FILTER_SIZE*2);
        FragShdV->setConstant("BlurWeights", BloomFilter_.BlurWeights, SBloomFilter::FILTER_SIZE);
    }
    
    /* Generate shadow shader */
    if (Flags_ & DEFERREDFLAG_SHADOW_MAPPING)
    {
        /* Create the shadow maps */
        ShadowMapper_.createShadowMaps(ShadowTexSize, MaxPointLightCount, MaxSpotLightCount, true);
        
        /* Setup shader compilation options */
        std::vector<const c8*> ShadowCompilerOp;
        
        ShadowCompilerOp.push_back("-DUSE_VSM");
        ShadowCompilerOp.push_back("-DUSE_TEXTURE");
        
        if (Flags_ & DEFERREDFLAG_USE_TEXTURE_MATRIX)
            ShadowCompilerOp.push_back("-DUSE_TEXTURE_MATRIX");
        
        ShadowCompilerOp.push_back(0);
        
        /* Build shadow shader */
        std::vector<io::stringc> ShadowShdBuf(1);
        
        ShadowShdBuf[0] = (
            #include "RenderSystem/DeferredRenderer/spShadowShaderStr.h"
        );
        
        if (!buildShader(
                "shadow", ShadowShader_, &VertexFormat_, ShadowShdBuf,
                &ShadowCompilerOp[0], "VertexMain", "PixelMain"))
        {
            return false;
        }
        
        ShadowShader_->setObjectCallback(ShadowShaderCallback);
    }
    
    /* Build g-buffer */
    return GBuffer_.createGBuffer(
        Resolution, false, false, (Flags_ & DEFERREDFLAG_BLOOM) != 0
    );
}

void DeferredRenderer::renderScene(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, Texture* RenderTarget, bool UseDefaultGBufferShader)
{
    DefRendererFlags = Flags_;
    
    if ( Graph && GBufferShader_ && DeferredShader_ && ( !RenderTarget || RenderTarget->getRenderTarget() ) )
    {
        updateLightSources(Graph, ActiveCamera);
        
        renderSceneIntoGBuffer(Graph, ActiveCamera, UseDefaultGBufferShader);
        renderDeferredShading(RenderTarget);
        
        if (Flags_ & DEFERREDFLAG_BLOOM)
            renderBloomFilter(RenderTarget);
    }
    #ifdef SP_DEBUGMODE
    else if ( !Graph || ( RenderTarget && !RenderTarget->getRenderTarget() ) )
        io::Log::debug("DeferredRenderer::renderScene");
    #endif
}

void DeferredRenderer::changeBloomFactor(f32 GaussianMultiplier)
{
    if (Flags_ & DEFERREDFLAG_BLOOM)
    {
        /* Update bloom weights only */
        BloomFilter_.computeWeights(GaussianMultiplier);
        
        BloomShaderHRP_->getPixelShader()->setConstant("BlurWeights", BloomFilter_.BlurWeights, SBloomFilter::FILTER_SIZE);
        BloomShaderVRP_->getPixelShader()->setConstant("BlurWeights", BloomFilter_.BlurWeights, SBloomFilter::FILTER_SIZE);
    }
}


/*
 * ======= Protected: =======
 */

void DeferredRenderer::updateLightSources(scene::SceneGraph* Graph, scene::Camera* ActiveCamera)
{
    /* Update each light source */
    f32 Color[4];
    s32 i = 0, iEx = 0;
    u32 ShadowCubeMapIndex = 0, ShadowMapIndex = 0;
    
    std::list<scene::Light*>::const_iterator it = Graph->getLightList().begin(), itEnd = Graph->getLightList().end();
    
    const bool UseShadow = ((Flags_ & DEFERREDFLAG_SHADOW_MAPPING) != 0);
    
    if (UseShadow)
        __spVideoDriver->setGlobalShaderClass(ShadowShader_);
    
    for (; it != itEnd && i < DeferredRenderer::MAX_LIGHTS; ++it)
    {
        /* Get current light source object */
        scene::Light* Node = *it;
        
        if (!Node->getVisible())
            continue;
        
        SLight* Lit = &(Lights_[i]);
        
        Node->getDiffuseColor().getFloatArray(Color);
        
        if (UseShadow && Node->getShadow())
        {
            /* Render shadow map */
            switch (Node->getLightModel())
            {
                case scene::LIGHT_POINT:
                    Lit->ShadowIndex = ShadowCubeMapIndex;
                    ShadowMapper_.renderShadowMap(Graph, ActiveCamera, Node, ShadowCubeMapIndex++);
                    break;
                case scene::LIGHT_SPOT:
                    Lit->ShadowIndex = ShadowMapIndex;
                    ShadowMapper_.renderShadowMap(Graph, ActiveCamera, Node, ShadowMapIndex++);
                    break;
                default:
                    break;
            }
        }
        
        /* Copy basic data */
        Lit->Position   = Node->getPosition(true);
        Lit->Radius     = (Node->getVolumetric() ? Node->getVolumetricRadius() : 1000.0f);
        Lit->Color      = dim::vector3df(Color[0], Color[1], Color[2]);
        Lit->Type       = static_cast<u8>(Node->getLightModel());
        
        if (Lit->Type != scene::LIGHT_POINT)
        {
            SLightEx* LitEx = &(LightsEx_[iEx]);
            
            /* Copy extended data */
            if (Lit->Type == scene::LIGHT_SPOT)
                LitEx->Projection = Node->getProjectionMatrix() * Node->getTransformMatrix(true).getInverse();
            
            LitEx->Direction = Node->getTransformation().getDirection();
            LitEx->Direction.normalize();
            
            LitEx->SpotTheta            = Node->getSpotConeInner() * math::DEG;
            LitEx->SpotPhiMinusTheta    = Node->getSpotConeOuter() * math::DEG - LitEx->SpotTheta;
            
            ++iEx;
        }
        
        ++i;
    }
    
    if (UseShadow)
        __spVideoDriver->setGlobalShaderClass(0);
    
    /* Update shader constants */
    Shader* FragShd = DeferredShader_->getPixelShader();
    
    FragShd->setConstant("LightCount", i);
    FragShd->setConstant("LightExCount", iEx);
    
    FragShd->setConstant("Lights", &(Lights_[0].Position.X), sizeof(SLight) / sizeof(f32) * i);
    FragShd->setConstant("LightsEx", LightsEx_[0].Projection.getArray(), sizeof(SLightEx) / sizeof(f32) * iEx);
}

void DeferredRenderer::renderSceneIntoGBuffer(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, bool UseDefaultGBufferShader)
{
    ShaderClass* PrevShaderClass = 0;
    
    if (UseDefaultGBufferShader)
    {
        PrevShaderClass = __spVideoDriver->getGlobalShaderClass();
        __spVideoDriver->setGlobalShaderClass(GBufferShader_);
    }
    
    GBuffer_.bindRTDeferredShading();
    __spVideoDriver->clearBuffers();
    
    __spDevice->setActiveSceneGraph(Graph);
    
    if (ActiveCamera)
        Graph->renderScene(ActiveCamera);
    else
        Graph->renderScene();
    
    if (UseDefaultGBufferShader)
        __spVideoDriver->setGlobalShaderClass(PrevShaderClass);
}

void DeferredRenderer::renderDeferredShading(Texture* RenderTarget)
{
    if (Flags_ & DEFERREDFLAG_BLOOM)
        GBuffer_.bindRTBloomFilter();
    else
        __spVideoDriver->setRenderTarget(RenderTarget);
    
    __spVideoDriver->beginDrawing2D();
    {
        DeferredShader_->bind();
        {
            ShadowMapper_.bind(2, 3);
            
            GBuffer_.drawDeferredShading();
            
            ShadowMapper_.unbind(2, 3);
        }
        DeferredShader_->unbind();
    }
    __spVideoDriver->endDrawing2D();
    
    __spVideoDriver->setRenderTarget(0);
}

void DeferredRenderer::renderBloomFilter(Texture* RenderTarget)
{
    /* Down-sample gloss map */
    GBuffer_.getTexture(GBuffer::RENDERTARGET_DEFERRED_GLOSS)->generateMipMap();
    
    /* Render bloom filter: 1st pass */
    __spVideoDriver->setRenderTarget(GBuffer_.getTexture(GBuffer::RENDERTARGET_GLOSS_1ST_PASS));
    {
        BloomShaderHRP_->bind();
        {
            drawFullscreenImageStreched(GBuffer_.getTexture(GBuffer::RENDERTARGET_DEFERRED_GLOSS));
        }
        BloomShaderHRP_->unbind();
    }
    __spVideoDriver->setRenderTarget(0);
    
    /* Render bloom filter: 2nd pass */
    __spVideoDriver->setRenderTarget(GBuffer_.getTexture(GBuffer::RENDERTARGET_GLOSS_2ND_PASS));
    {
        BloomShaderVRP_->bind();
        {
            drawFullscreenImage(GBuffer_.getTexture(GBuffer::RENDERTARGET_GLOSS_1ST_PASS));
        }
        BloomShaderVRP_->unbind();
    }
    __spVideoDriver->setRenderTarget(0);
    
    /* Draw final bloom filter over the deferred color result */
    __spVideoDriver->setRenderTarget(RenderTarget);
    __spVideoDriver->beginDrawing2D();
    {
        /* Draw deferred color result */
        __spVideoDriver->draw2DImage(GBuffer_.getTexture(GBuffer::RENDERTARGET_DEFERRED_COLOR), dim::point2di(0));
        
        /* Add bloom gloss */
        __spVideoDriver->setBlending(BLEND_SRCALPHA, BLEND_ONE);
        {
            __spVideoDriver->draw2DImage(
                GBuffer_.getTexture(GBuffer::RENDERTARGET_GLOSS_2ND_PASS),
                dim::rect2di(0, 0, gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight)
            );
        }
        __spVideoDriver->setDefaultAlphaBlending();
    }
    __spVideoDriver->endDrawing2D();
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
        ShdClass, SHADER_VERTEX, CG_VERSION_2_0, ShdBuffer, VertexMain, CompilerOptions
    );
    __spVideoDriver->createCgShader(
        ShdClass, SHADER_PIXEL, CG_VERSION_2_0, ShdBuffer, PixelMain, CompilerOptions
    );
    
    if (HasTessellation)
    {
        /* Create hull- and domain shaders */
        __spVideoDriver->createCgShader(
            ShdClass, SHADER_HULL, CG_VERSION_2_0, ShdBuffer, "HullMain", CompilerOptions
        );
        __spVideoDriver->createCgShader(
            ShdClass, SHADER_DOMAIN, CG_VERSION_2_0, ShdBuffer, "DomainMain", CompilerOptions
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
    __spVideoDriver->deleteShaderClass(GBufferShader_,  true);
    __spVideoDriver->deleteShaderClass(DeferredShader_, true);
    __spVideoDriver->deleteShaderClass(BloomShaderHRP_, true);
    __spVideoDriver->deleteShaderClass(BloomShaderVRP_, true);
    __spVideoDriver->deleteShaderClass(ShadowShader_,   true);
    
    GBufferShader_  = 0;
    DeferredShader_ = 0;
    BloomShaderHRP_ = 0;
    BloomShaderVRP_ = 0;
    ShadowShader_   = 0;
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

void DeferredRenderer::drawFullscreenImage(Texture* Tex)
{
    __spVideoDriver->beginDrawing2D();
    __spVideoDriver->draw2DImage(Tex, dim::point2di(0));
    __spVideoDriver->endDrawing2D();
}

void DeferredRenderer::drawFullscreenImageStreched(Texture* Tex)
{
    const dim::size2di Size(Tex->getSize()/4);
    
    __spVideoDriver->beginDrawing2D();
    __spVideoDriver->draw2DImage(Tex, dim::rect2di(0, 0, Size.Width, Size.Height));
    __spVideoDriver->endDrawing2D();
}


/*
 * SBloomFilter structure
 */

DeferredRenderer::SBloomFilter::SBloomFilter()
{
    memset(BlurOffsets, 0, sizeof(BlurOffsets));
    memset(BlurWeights, 0, sizeof(BlurWeights));
}
DeferredRenderer::SBloomFilter::~SBloomFilter()
{
}

void DeferredRenderer::SBloomFilter::computeWeights(f32 GaussianMultiplier)
{
    const f32 HalfWidth = static_cast<f32>((SBloomFilter::FILTER_SIZE - 1)/2);
    
    for (s32 i = 0; i < SBloomFilter::FILTER_SIZE; ++i)
    {
        f32 f = static_cast<f32>(i) - HalfWidth;
        
        BlurWeights[i] = math::getGaussianValue(f / HalfWidth, 0.0f, 0.8f) * GaussianMultiplier;
    }
}

void DeferredRenderer::SBloomFilter::computeOffsets(const dim::size2di &Resolution)
{
    const f32 HalfWidth = static_cast<f32>((SBloomFilter::FILTER_SIZE - 1)/2);
    
    for (s32 i = 0; i < SBloomFilter::FILTER_SIZE; ++i)
    {
        f32 f = static_cast<f32>(i) - HalfWidth;
        
        BlurOffsets[i*2    ] = f * (HalfWidth / Resolution.Width);
        BlurOffsets[i*2 + 1] = f * (HalfWidth / Resolution.Height);
    }
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
