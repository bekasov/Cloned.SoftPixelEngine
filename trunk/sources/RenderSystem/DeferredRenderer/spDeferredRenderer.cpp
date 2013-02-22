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

static void GBufferObjectShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    /* Get vertex- and pixel shaders */
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    /* Setup transformations */
    const dim::vector3df ViewPosition(
        __spSceneManager->getActiveCamera()->getPosition(true)
    );
    
    dim::matrix4f WVPMatrix(__spVideoDriver->getProjectionMatrix());
    WVPMatrix *= __spVideoDriver->getViewMatrix();
    WVPMatrix *= __spVideoDriver->getWorldMatrix();
    
    /* Setup shader constants */
    VertShd->setConstant("WorldViewProjectionMatrix", WVPMatrix);
    VertShd->setConstant("WorldMatrix", __spVideoDriver->getWorldMatrix());
    VertShd->setConstant("ViewPosition", ViewPosition);
    
    FragShd->setConstant("ViewPosition", ViewPosition);
}

static void GBufferSurfaceShaderCallback(ShaderClass* ShdClass, const std::vector<TextureLayer*> &TextureLayers)
{
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    u32 TexCount = TextureLayers.size();
    
    if (DefRendererFlags & DEFERREDFLAG_USE_TEXTURE_MATRIX)
    {
        /*if (TexCount > 0)
            VertShd->setConstant("TextureMatrix", TextureLayers.front().Matrix);
        else*/
            VertShd->setConstant("TextureMatrix", dim::matrix4f());
    }
    
    if ((DefRendererFlags & DEFERREDFLAG_HAS_SPECULAR_MAP) == 0)
        ++TexCount;
    
    if (DefRendererFlags & DEFERREDFLAG_HAS_LIGHT_MAP)
        FragShd->setConstant("EnableLightMap", TexCount >= ((DefRendererFlags & DEFERREDFLAG_PARALLAX_MAPPING) != 0 ? 5u : 4u));
    
    if (DefRendererFlags & DEFERREDFLAG_PARALLAX_MAPPING)
    {
        FragShd->setConstant("EnablePOM", TexCount >= 4);//!!!
        FragShd->setConstant("MinSamplesPOM", 0);//!!!
        FragShd->setConstant("MaxSamplesPOM", 50);//!!!
        FragShd->setConstant("HeightMapScale", 0.015f);//!!!
        FragShd->setConstant("ParallaxViewRange", 2.0f);//!!!
    }
    
    FragShd->setConstant("SpecularFactor", 1.0f);//!!!
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
    LightsEx_       (DeferredRenderer::MAX_EX_LIGHTS),
    AmbientColor_   (0.07f                          )
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
    LayerModel_.clear();
    
    const bool IsGL = (__spVideoDriver->getRendererType() == RENDERER_OPENGL);
    const dim::size2di Resolution(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight);
    
    const bool CompileGLSL = IsGL && true;//!!!
    
    std::list<io::stringc> GBufferCompilerOp, DeferredCompilerOp;
    setupCompilerOptions(GBufferCompilerOp, DeferredCompilerOp);
    
    /* Delete old shaders and shadow maps */
    deleteShaders();
    ShadowMapper_.deleteShadowMaps();
    
    /* Create new vertex formats */
    createVertexFormats();
    
    /* Setup g-buffer shader source code */
    std::list<io::stringc> GBufferShdBufVert(GBufferCompilerOp), GBufferShdBufFrag(GBufferCompilerOp);
    
    if (CompileGLSL)
    {
        GBufferShdBufVert.push_back(
            #include "RenderSystem/DeferredRenderer/spGBufferShaderStr.glvert"
        );
        GBufferShdBufFrag.push_back(
            #include "RenderSystem/DeferredRenderer/spGBufferShaderStr.glfrag"
        );
    }
    else
    {
        GBufferShdBufVert.push_back(
            #include "RenderSystem/DeferredRenderer/spGBufferShaderStr.cg"
        );
    }
    
    /* Generate g-buffer shader */
    if (!buildShader(
            "g-buffer", GBufferShader_, &VertexFormat_, &GBufferShdBufVert,
            CompileGLSL ? &GBufferShdBufFrag : &GBufferShdBufVert,
            "VertexMain", "PixelMain", CompileGLSL ? BUILD_GLSL : BUILD_CG))
    {
        return false;
    }
    
    GBufferShader_->setObjectCallback(GBufferObjectShaderCallback);
    GBufferShader_->setSurfaceCallback(GBufferSurfaceShaderCallback);
    
    if (CompileGLSL)
        setupGBufferSampler(GBufferShader_->getPixelShader());
    
    /* Setup deferred shader source code */
    std::list<io::stringc> DeferredShdBufVert(DeferredCompilerOp), DeferredShdBufFrag(DeferredCompilerOp);
    
    if (CompileGLSL)
    {
        DeferredShdBufVert.push_back(
            #include "RenderSystem/DeferredRenderer/spDeferredShaderStr.glvert"
        );
        DeferredShdBufFrag.push_back(
            #include "RenderSystem/DeferredRenderer/spDeferredShaderStr.glfrag"
        );
    }
    else
    {
        DeferredShdBufVert.push_back(
            #include "RenderSystem/DeferredRenderer/spDeferredShaderStr.cg"
        );
    }
    
    /* Generate deferred shader */
    if (!buildShader(
            "deferred", DeferredShader_, &ImageVertexFormat_, &DeferredShdBufVert,
            CompileGLSL ? &DeferredShdBufFrag : &DeferredShdBufVert,
            "VertexMain", "PixelMain", CompileGLSL ? BUILD_GLSL : BUILD_CG))
    {
        return false;
    }
    
    DeferredShader_->setObjectCallback(DeferredShaderCallback);
    
    if (CompileGLSL)
        setupDeferredSampler(DeferredShader_->getPixelShader());
    
    /* Generate bloom filter shader */
    if (Flags_ & DEFERREDFLAG_BLOOM)
    {
        std::list<io::stringc> BloomShdBufVert, BloomShdBufFrag;
        
        if (CompileGLSL)
        {
            BloomShdBufVert.push_back(
                #include "RenderSystem/DeferredRenderer/spBloomFilterStr.glvert"
            );
            BloomShdBufFrag.push_back(
                #include "RenderSystem/DeferredRenderer/spBloomFilterStr.glfrag"
            );
        }
        else
        {
            BloomShdBufVert.push_back(
                #include "RenderSystem/DeferredRenderer/spBloomFilterStr.cg"
            );
        }
        
        if (!buildShader(
                "bloom", BloomShaderHRP_, &ImageVertexFormat_, &BloomShdBufVert,
                CompileGLSL ? &BloomShdBufFrag : &BloomShdBufVert,
                "VertexMain", "PixelMainHRP", CompileGLSL ? BUILD_GLSL : BUILD_CG))
        {
            return false;
        }
        
        if (CompileGLSL)
            BloomShdBufFrag.insert(BloomShdBufFrag.begin(), "#define HORZ_RENDER_PASS\n");
        
        if (!buildShader(
                "bloom", BloomShaderVRP_, &ImageVertexFormat_, &BloomShdBufVert,
                CompileGLSL ? &BloomShdBufFrag : &BloomShdBufVert,
                "VertexMain", "PixelMainVRP", CompileGLSL ? BUILD_GLSL : BUILD_CG))
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
        std::list<io::stringc> ShadowShdBuf;
        
        Shader::addOption(ShadowShdBuf, "USE_VSM");
        Shader::addOption(ShadowShdBuf, "USE_TEXTURE");
        
        //if (Flags_ & DEFERREDFLAG_USE_TEXTURE_MATRIX)
        //    Shader::addOption(ShadowShdBuf, "USE_TEXTURE_MATRIX");
        
        /* Build shadow shader */
        ShadowShdBuf.push_back(
            #include "RenderSystem/DeferredRenderer/spShadowShaderStr.cg"
        );
        
        if (!buildShader("shadow", ShadowShader_, &VertexFormat_, &ShadowShdBuf, &ShadowShdBuf, "VertexMain", "PixelMain"))
            return false;
        
        ShadowShader_->setObjectCallback(ShadowShaderCallback);
    }
    
    /* Build g-buffer */
    return GBuffer_.createGBuffer(
        Resolution, false, false, (Flags_ & DEFERREDFLAG_BLOOM) != 0, (Flags_ & DEFERREDFLAG_HAS_LIGHT_MAP) != 0
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
    
    std::vector<scene::Light*>::const_iterator it = Graph->getLightList().begin(), itEnd = Graph->getLightList().end();
    
    const bool UseShadow = ((Flags_ & DEFERREDFLAG_SHADOW_MAPPING) != 0);
    
    if (UseShadow)
        __spVideoDriver->setGlobalShaderClass(ShadowShader_);
    
    for (; it != itEnd && i < DeferredRenderer::MAX_LIGHTS; ++it)
    {
        /* Get current light source object */
        scene::Light* LightObj = *it;
        
        if ( !LightObj->getVisible() || ( LightObj->getLightModel() != scene::LIGHT_POINT && static_cast<u32>(iEx) >= LightsEx_.size() ) )
            continue;
        
        SLight* Lit = &(Lights_[i]);
        
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
        Lit->Radius             = (LightObj->getVolumetric() ? LightObj->getVolumetricRadius() : 1000.0f);
        Lit->Color              = dim::vector3df(Color[0], Color[1], Color[2]);
        Lit->Type               = static_cast<u8>(LightObj->getLightModel());
        Lit->UsedForLightmaps   = (LightObj->getShadow() ? 0 : 1);//!!!
        
        if (Lit->Type != scene::LIGHT_POINT)
        {
            SLightEx* LitEx = &(LightsEx_[iEx]);
            
            /* Copy extended data */
            const scene::Transformation Transform(LightObj->getTransformation(true));
            
            if (Lit->Type == scene::LIGHT_SPOT)
            {
                //LitEx->Projection = LightObj->getProjectionMatrix() * LightObj->getTransformMatrix(true).getInverse();
                dim::matrix4f ProjMat;
                ProjMat.setPerspectiveRH(90.0f, 1.0f, 0.01f, 1000.0f);
                
                LitEx->Projection = ProjMat * Transform.getInverseMatrix();
            }
            
            LitEx->Direction = Transform.getDirection();
            LitEx->Direction.normalize();
            
            LitEx->SpotTheta            = LightObj->getSpotConeInner() * math::DEG;
            LitEx->SpotPhiMinusTheta    = LightObj->getSpotConeOuter() * math::DEG - LitEx->SpotTheta;
            
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
    
    #if 1 //!!!
    
    for (s32 c = 0; c < i; ++c)
    {
        const io::stringc n = "Lights[" + io::stringc(c) + "].";
        FragShd->setConstant(n + "PositionAndRadius", dim::vector4df(Lights_[c].Position, Lights_[c].Radius));
        FragShd->setConstant(n + "Color", Lights_[c].Color);
        FragShd->setConstant(n + "Type", Lights_[c].Type);
        FragShd->setConstant(n + "ShadowIndex", Lights_[c].ShadowIndex);
        FragShd->setConstant(n + "UsedForLightmaps", Lights_[c].UsedForLightmaps);
    }
    
    for (s32 c = 0; c < iEx; ++c)
    {
        const io::stringc n = "LightsEx[" + io::stringc(c) + "].";
        FragShd->setConstant(n + "Projection", LightsEx_[c].Projection);
        FragShd->setConstant(n + "Direction", LightsEx_[c].Direction);
        FragShd->setConstant(n + "SpotTheta", LightsEx_[c].SpotTheta);
        FragShd->setConstant(n + "SpotPhiMinusTheta", LightsEx_[c].SpotPhiMinusTheta);
    }
    
    #else
    FragShd->setConstant("Lights", &(Lights_[0].Position.X), sizeof(SLight) / sizeof(f32) * i);
    FragShd->setConstant("LightsEx", LightsEx_[0].Projection.getArray(), sizeof(SLightEx) / sizeof(f32) * iEx);
    #endif
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
    
    const s32 ShadowMapLayerBase = ((Flags_ & DEFERREDFLAG_HAS_LIGHT_MAP) != 0 ? 3 : 2);
    
    __spVideoDriver->beginDrawing2D();
    {
        DeferredShader_->bind();
        {
            DeferredShader_->getPixelShader()->setConstant("AmbientColor", AmbientColor_);
            
            /* Bind shadow map texture-array and draw deferred-shading */
            ShadowMapper_.bind(ShadowMapLayerBase, ShadowMapLayerBase + 1);
            
            GBuffer_.drawDeferredShading();
            
            ShadowMapper_.unbind(ShadowMapLayerBase, ShadowMapLayerBase + 1);
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

EShaderVersions DeferredRenderer::getShaderVersionFromFlags(s32 Flags) const
{
    if (Flags & BUILD_GLSL)
        return GLSL_VERSION_1_20;
    
    if (Flags & BUILD_CG)
        return CG_VERSION_2_0;
    
    if (Flags & BUILD_HLSL3)
    {
        if (Flags & BUILD_VERTEX) return HLSL_VERTEX_3_0;
        if (Flags & BUILD_PIXEL ) return HLSL_PIXEL_3_0;
    }
    
    if (Flags & BUILD_HLSL5)
    {
        if (Flags & BUILD_VERTEX    ) return HLSL_VERTEX_5_0;
        if (Flags & BUILD_PIXEL     ) return HLSL_PIXEL_5_0;
        if (Flags & BUILD_GEOMETRY  ) return HLSL_GEOMETRY_5_0;
        if (Flags & BUILD_HULL      ) return HLSL_HULL_5_0;
        if (Flags & BUILD_DOMAIN    ) return HLSL_DOMAIN_5_0;
    }
    
    return DUMMYSHADER_VERSION;
}

bool DeferredRenderer::buildShader(
    const io::stringc &Name,
    ShaderClass* &ShdClass,
    VertexFormat* VertFmt,
    
    const std::list<io::stringc>* ShdBufferVert,
    const std::list<io::stringc>* ShdBufferFrag,
    
    const io::stringc &VertexMain,
    const io::stringc &PixelMain,
    
    s32 Flags)
{
    /* Create shader class */
    if (Flags & BUILD_CG)
        ShdClass = __spVideoDriver->createCgShaderClass(&ImageVertexFormat_);
    else
        ShdClass = __spVideoDriver->createShaderClass(&ImageVertexFormat_);
    
    if (!ShdClass)
        return false;
    
    /* Create vertex- and pixel shaders */
    if (Flags & BUILD_CG)
    {
        __spVideoDriver->createCgShader(
            ShdClass, SHADER_VERTEX, getShaderVersionFromFlags(Flags), *ShdBufferVert, VertexMain
        );
        __spVideoDriver->createCgShader(
            ShdClass, SHADER_PIXEL, getShaderVersionFromFlags(Flags), *ShdBufferFrag, PixelMain
        );
    }
    else
    {
        __spVideoDriver->createShader(
            ShdClass, SHADER_VERTEX, getShaderVersionFromFlags(Flags), *ShdBufferVert, VertexMain
        );
        __spVideoDriver->createShader(
            ShdClass, SHADER_PIXEL, getShaderVersionFromFlags(Flags), *ShdBufferFrag, PixelMain
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
        /* Add texture-coordinates for normal-mapping (tangent and binormal is texture-coordinates) */
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 3);
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 3);
    }
    
    if (Flags_ & DEFERREDFLAG_HAS_LIGHT_MAP)
    {
        /* Add texture-coordinates for lightmaps */
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 2);
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

void DeferredRenderer::setupCompilerOptions(
    std::list<io::stringc> &GBufferCompilerOp, std::list<io::stringc> &DeferredCompilerOp)
{
    const bool IsGL = (__spVideoDriver->getRendererType() == RENDERER_OPENGL);
    
    if (Flags_ & DEFERREDFLAG_USE_TEXTURE_MATRIX)
        Shader::addOption(GBufferCompilerOp, "USE_TEXTURE_MATRIX");
    if (Flags_ & DEFERREDFLAG_HAS_SPECULAR_MAP)
        Shader::addOption(GBufferCompilerOp, "HAS_SPECULAR_MAP");
    
    if (Flags_ & DEFERREDFLAG_HAS_LIGHT_MAP)
    {
        Shader::addOption(GBufferCompilerOp, "HAS_LIGHT_MAP");
        Shader::addOption(DeferredCompilerOp, "HAS_LIGHT_MAP");
    }
    
    if (Flags_ & DEFERREDFLAG_ALLOW_OVERBLENDING)
        Shader::addOption(DeferredCompilerOp, "ALLOW_OVERBLENDING");
    
    if (Flags_ & DEFERREDFLAG_NORMAL_MAPPING)
    {
        Shader::addOption(GBufferCompilerOp, "NORMAL_MAPPING");
        
        if (Flags_ & DEFERREDFLAG_PARALLAX_MAPPING)
        {
            Shader::addOption(GBufferCompilerOp, "PARALLAX_MAPPING");
            if (Flags_ & DEFERREDFLAG_NORMALMAP_XYZ_H)
                Shader::addOption(GBufferCompilerOp, "NORMALMAP_XYZ_H");
        }
    }
    
    if (Flags_ & DEFERREDFLAG_DEBUG_GBUFFER)
    {
        Shader::addOption(GBufferCompilerOp, "DEBUG_GBUFFER");
        Shader::addOption(DeferredCompilerOp, "DEBUG_GBUFFER");
        
        if (Flags_ & DEFERREDFLAG_DEBUG_GBUFFER_TEXCOORDS)
            Shader::addOption(GBufferCompilerOp, "DEBUG_GBUFFER_TEXCOORDS");
        if (Flags_ & DEFERREDFLAG_DEBUG_GBUFFER_WORLDPOS)
            Shader::addOption(DeferredCompilerOp, "DEBUG_GBUFFER_WORLDPOS");
    }
    
    if (Flags_ & DEFERREDFLAG_BLOOM)
    {
        Shader::addOption(DeferredCompilerOp, "BLOOM_FILTER");
        
        if (IsGL)
            Shader::addOption(DeferredCompilerOp, "FLIP_Y_AXIS");
    }
    
    if (Flags_ & DEFERREDFLAG_SHADOW_MAPPING)
    {
        Shader::addOption(GBufferCompilerOp, "SHADOW_MAPPING");
        Shader::addOption(DeferredCompilerOp, "SHADOW_MAPPING");
    }
}

void DeferredRenderer::setupGBufferSampler(video::Shader* PixelShader)
{
    if (!PixelShader)
        return;
    
    s32 SamplerIndex = 0;
    
    LayerModel_.DiffuseMap = static_cast<u8>(SamplerIndex);
    PixelShader->setConstant("DiffuseMap", SamplerIndex++);
    
    if (Flags_ & DEFERREDFLAG_HAS_SPECULAR_MAP)
    {
        LayerModel_.SpecularMap = static_cast<u8>(SamplerIndex);
        PixelShader->setConstant("SpecularMap", SamplerIndex++);
    }
    
    if (Flags_ & DEFERREDFLAG_NORMAL_MAPPING)
    {
        LayerModel_.NormalMap = static_cast<u8>(SamplerIndex);
        PixelShader->setConstant("NormalMap", SamplerIndex++);
        
        if ((Flags_ & DEFERREDFLAG_PARALLAX_MAPPING))
        {
            if (!(Flags_ & DEFERREDFLAG_NORMALMAP_XYZ_H))
            {
                LayerModel_.HeightMap = static_cast<u8>(SamplerIndex);
                PixelShader->setConstant("HeightMap", SamplerIndex++);
            }
            else
                LayerModel_.HeightMap = LayerModel_.NormalMap;
        }
    }
    
    if (Flags_ & DEFERREDFLAG_HAS_LIGHT_MAP)
    {
        LayerModel_.LightMap = static_cast<u8>(SamplerIndex);
        PixelShader->setConstant("LightMap", SamplerIndex++);
    }
}

void DeferredRenderer::setupDeferredSampler(video::Shader* PixelShader)
{
    if (!PixelShader)
        return;
    
    s32 SamplerIndex = 0;
    
    PixelShader->setConstant("DiffuseAndSpecularMap", SamplerIndex++);
    PixelShader->setConstant("NormalAndDepthMap", SamplerIndex++);
    
    if (Flags_ & DEFERREDFLAG_HAS_LIGHT_MAP)
        PixelShader->setConstant("IlluminationMap", SamplerIndex++);
    
    if (Flags_ & DEFERREDFLAG_SHADOW_MAPPING)
    {
        PixelShader->setConstant("DirLightShadowMaps", SamplerIndex++);
        PixelShader->setConstant("PointLightShadowMaps", SamplerIndex++);
    }
}


/*
 * SLight structure
 */

DeferredRenderer::SLight::SLight() :
    Radius          (1000.0f),
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
