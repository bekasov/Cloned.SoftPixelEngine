/*
 * Bloom effect file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/PostProcessing/spBloomEffect.hpp"

#if defined(SP_COMPILE_WITH_POSTPROCESSING)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "Base/spSharedObjects.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


BloomEffect::BloomEffect() :
    PostProcessingEffect(       ),
    GaussianMultiplier_ (0.6f   )
{
    memset(RenderTargets_, 0, sizeof(RenderTargets_));
    memset(BlurOffsets_, 0, sizeof(BlurOffsets_));
    memset(BlurWeights_, 0, sizeof(BlurWeights_));
}
BloomEffect::~BloomEffect()
{
    deleteResources();
}

bool BloomEffect::createResources(const dim::size2di &Resolution)
{
    /* Delete old textures */
    deleteResources();
    
    Resolution_ = Resolution;
    
    /* Create new resources */
    if (!createRenderTargets())
    {
        io::Log::error("Creating render targets for bloom-effect failed");
        deleteResources();
        return false;
    }
    
    if (!compileShaders())
    {
        io::Log::error("Compiling shaders for bloom-effect failed");
        deleteResources();
        return false;
    }
    
    /* Validate effect */
    Valid_ = true;
    
    return true;
}

void BloomEffect::deleteResources()
{
    /* Delete all render targets */
    for (u32 i = 0; i < RENDERTARGET_COUNT; ++i)
        GlbRenderSys->deleteTexture(RenderTargets_[i]);
    
    /* Delete shaders */
    GlbRenderSys->deleteShaderClass(ShdClass_, true);
    ShdClass_ = 0;
    
    Valid_ = false;
}

void BloomEffect::bindRenderTargets()
{
    GlbRenderSys->setRenderTarget(RenderTargets_[RENDERTARGET_INPUT_COLOR]);
}

const c8* BloomEffect::getName() const
{
    return "Bloom";
}

void BloomEffect::drawEffect(Texture* InputTexture, Texture* OutputTexture)
{
    /* Check if effect has already been created */
    if (!valid())
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("BloomEffect::drawEffect", "Effect is used but has not been created", io::LOG_UNIQUE);
        #endif
        return;
    }
    
    /* Check if the effect is not required to be drawn */
    if (active())
    {
        /* Down-sample gloss map */
        RenderTargets_[RENDERTARGET_INPUT_GLOSS]->generateMipMap();
        
        /* Render bloom filter: 1st pass */
        setupRenderPass(false);
        ShdClass_->bind();
        {
            GlbRenderSys->setRenderTarget(RenderTargets_[RENDERTARGET_GLOSS_1ST_PASS]);
            drawFullscreenImageStreched(RENDERTARGET_INPUT_GLOSS);
        }
        /* Render bloom filter: 2nd pass */
        setupRenderPass(true);
        ShdClass_->bind();
        {
            GlbRenderSys->setRenderTarget(RenderTargets_[RENDERTARGET_GLOSS_2ND_PASS]);
            drawFullscreenImage(RENDERTARGET_GLOSS_1ST_PASS);
        }
        ShdClass_->unbind();
        
        /* Draw final bloom filter over the deferred color result */
        GlbRenderSys->setRenderTarget(OutputTexture);
        {
            /* Draw input color result */
            drawFullscreenImage(RENDERTARGET_INPUT_COLOR);
            
            /* Add bloom gloss */
            GlbRenderSys->setBlending(BLEND_SRCALPHA, BLEND_ONE);
            {
                GlbRenderSys->draw2DImage(
                    RenderTargets_[RENDERTARGET_GLOSS_2ND_PASS],
                    dim::rect2di(0, 0, gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight)
                );
            }
            GlbRenderSys->setupDefaultBlending();
        }
        GlbRenderSys->setRenderTarget(0);
    }
    else
    {
        /* Draw input color result only */
        GlbRenderSys->setRenderTarget(OutputTexture);
        drawFullscreenImage(RENDERTARGET_INPUT_COLOR);
        GlbRenderSys->setRenderTarget(0);
    }
}

void BloomEffect::setFactor(f32 GaussianMultiplier)
{
    GaussianMultiplier_ = GaussianMultiplier;
    
    /* Update bloom weights only */
    computeWeights();
    
    if (ShdClass_)
        setupBlurWeights();
}


/*
 * ======= Private: =======
 */

bool BloomEffect::createRenderTargets()
{
    /* General texture flags */
    STextureCreationFlags CreationFlags;
    
    CreationFlags.Size              = Resolution_;
    CreationFlags.Format            = PIXELFORMAT_RGB;
    CreationFlags.HWFormat          = HWTEXFORMAT_UBYTE8;
    CreationFlags.Filter.WrapMode   = TEXWRAP_CLAMP;
    
    /* Create textures for bloom filter */
    CreationFlags.Filter.HasMIPMaps = false;
    CreationFlags.Filter.Min        = FILTER_LINEAR;
    CreationFlags.Filter.Mag        = FILTER_LINEAR;
    
    RenderTargets_[RENDERTARGET_INPUT_COLOR] = GlbRenderSys->createTexture(CreationFlags);
    
    /* Create base gloss map  */
    CreationFlags.Filter.HasMIPMaps = true;
    CreationFlags.Filter.Min        = FILTER_SMOOTH;
    CreationFlags.Filter.Mag        = FILTER_SMOOTH;
    
    RenderTargets_[RENDERTARGET_INPUT_GLOSS] = GlbRenderSys->createTexture(CreationFlags);
    
    /* Create temporary gloss map */
    CreationFlags.Size              /= BloomEffect::STRETCH_FACTOR;
    CreationFlags.Filter.HasMIPMaps = false;
    
    RenderTargets_[RENDERTARGET_GLOSS_1ST_PASS] = GlbRenderSys->createTexture(CreationFlags);
    RenderTargets_[RENDERTARGET_GLOSS_2ND_PASS] = GlbRenderSys->createTexture(CreationFlags);
    
    /* Make the texture to render targets */
    for (u32 i = 0; i < RENDERTARGET_COUNT; ++i)
    {
        if (RenderTargets_[i])
            RenderTargets_[i]->setRenderTarget(true);
        else
            return false;
    }
    
    /* Setup multi render targets for bloom filter */
    RenderTargets_[RENDERTARGET_INPUT_COLOR]->addMultiRenderTarget(
        RenderTargets_[RENDERTARGET_INPUT_GLOSS]
    );
    
    return true;
}

bool BloomEffect::compileShaders()
{
    const bool IsGL = (GlbRenderSys->getRendererType() == RENDERER_OPENGL);
    
    std::list<io::stringc> BloomShdBufVert, BloomShdBufFrag;
    
    if (IsGL)
    {
        BloomShdBufVert.push_back(
            #include "Resources/spBloomFilterStr.glvert"
        );
        BloomShdBufFrag.push_back(
            #include "Resources/spBloomFilterStr.glfrag"
        );
    }
    else
    {
        BloomShdBufVert.push_back(
            #include "Resources/spBloomFilterStr.cg"
        );
    }
    
    if (!ShaderClass::build(
            "bloom", ShdClass_, GlbRenderSys->getVertexFormatReduced(),
            &BloomShdBufVert, IsGL ? &BloomShdBufFrag : &BloomShdBufVert,
            "VertexMain", "PixelMain", IsGL ? SHADERBUILD_GLSL : SHADERBUILD_CG))
    {
        return false;
    }
    
    /* Compute bloom filter offsets and weights */
    computeWeights();
    computeOffsets();
    
    /* Setup gaussian shader constants */
    setupBlurOffsets();
    setupBlurWeights();
    
    setupProjectionMatrix();
    
    return true;
}

void BloomEffect::drawFullscreenImage(const ERenderTargets Type)
{
    GlbRenderSys->draw2DImage(RenderTargets_[Type], dim::point2di(0));
}

void BloomEffect::drawFullscreenImageStreched(const ERenderTargets Type)
{
    Texture* Tex = RenderTargets_[Type];
    const dim::size2di Size(Tex->getSize() / BloomEffect::STRETCH_FACTOR);
    GlbRenderSys->draw2DImage(Tex, dim::rect2di(0, 0, Size.Width, Size.Height));
}

void BloomEffect::computeWeights()
{
    const f32 HalfWidth = static_cast<f32>((BloomEffect::FILTER_SIZE - 1)/2);
    
    for (s32 i = 0; i < BloomEffect::FILTER_SIZE; ++i)
    {
        f32 f = static_cast<f32>(i) - HalfWidth;
        
        BlurWeights_[i] = math::getGaussianValue(f / HalfWidth, 0.0f, 0.8f) * GaussianMultiplier_;
    }
}

void BloomEffect::computeOffsets()
{
    const f32 HalfWidth = static_cast<f32>((BloomEffect::FILTER_SIZE - 1)/2);
    
    for (s32 i = 0; i < BloomEffect::FILTER_SIZE; ++i)
    {
        f32 f = static_cast<f32>(i) - HalfWidth;
        
        BlurOffsets_[i*2    ] = f * (HalfWidth / Resolution_.Width);
        BlurOffsets_[i*2 + 1] = f * (HalfWidth / Resolution_.Height);
    }
}

void BloomEffect::adjustResolution()
{
    RenderTargets_[RENDERTARGET_INPUT_COLOR]->setSize(Resolution_);
    RenderTargets_[RENDERTARGET_INPUT_GLOSS]->setSize(Resolution_);
    RenderTargets_[RENDERTARGET_GLOSS_1ST_PASS]->setSize(Resolution_ / BloomEffect::STRETCH_FACTOR);
    RenderTargets_[RENDERTARGET_GLOSS_2ND_PASS]->setSize(Resolution_ / BloomEffect::STRETCH_FACTOR);
    setupProjectionMatrix();
}

void BloomEffect::setupProjectionMatrix()
{
    dim::matrix4f ProjMat;
    ProjMat.make2Dimensional(
        Resolution_.Width, Resolution_.Height,
        Resolution_.Width, Resolution_.Height
    );
    
    ShdClass_->getVertexShader()->setConstant("ProjectionMatrix", ProjMat);
}

void BloomEffect::setupBlurOffsets()
{
    ShdClass_->getPixelShader()->setConstant("BlurOffsets", BlurOffsets_, BloomEffect::FILTER_SIZE);
}

void BloomEffect::setupBlurWeights()
{
    ShdClass_->getPixelShader()->setConstant("BlurWeights", BlurWeights_, BloomEffect::FILTER_SIZE);
}

void BloomEffect::setupRenderPass(bool IsVertical)
{
    ShdClass_->getPixelShader()->setConstant("VertRenderPass", IsVertical ? 1 : 0);
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
