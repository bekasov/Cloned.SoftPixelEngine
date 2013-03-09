/*
 * Bloom effect header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_BLOOM_EFFECT_H__
#define __SP_BLOOM_EFFECT_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_POSTPROCESSING)


#include "Base/spDimensionSize2D.hpp"
#include "RenderSystem/PostProcessing/spPostProcessingEffect.hpp"


namespace sp
{
namespace video
{


//namespace PostProcessing { ...

class ShaderClass;

//! GBuffer object which contains all textures which are required for a deferred renderer.
class SP_EXPORT BloomEffect : public PostProcessingEffect
{
    
    public:
        
        /* === Enumerations === */
        
        //! Bloom effect render target names.
        enum ERenderTargets
        {
            RENDERTARGET_INPUT_COLOR,       //!< Color result from deferred- or forward renderer for bloom filter.
            RENDERTARGET_INPUT_GLOSS,       //!< Gloss result from deferred- or forward renderer for bloom filter.
            RENDERTARGET_GLOSS_1ST_PASS,    //!< Temporary gloss texture for gaussian blur (1st render pass).
            RENDERTARGET_GLOSS_2ND_PASS,    //!< Temporary gloss texture for gaussian blur (2nd render pass).
            
            RENDERTARGET_COUNT              //!< Internal count constant. Don't use it to access a texture!
        };
        
        /* === Constructor & destructor === */
        
        BloomEffect();
        ~BloomEffect();
        
        /* === Functions === */
        
        bool createResources(const dim::size2di &Resolution);
        void deleteResources();
        
        /**
        Binds the render targets for this effect. For the bloom effect a multi-render-target texture
        is used with exactly two textures. After calling this function you have to render into these
        two framebuffers. The first one stores the color information and the second one stores the gloss factor.
        Use render-target index 0 for the color (write to RGB components) and index 1 for the gloss
        factor (write to Alpha channel).
        The following pixel shader pseudo code illustrates that:
        \code
        struct SPixelOutput
        {
            float4 Color : COLOR0;
            float4 Gloss : COLOR1;
        };
        
        // ...
        
        Out.Color.rgb = BloomColorInput;
        Out.Gloss.a   = BloomGlossInput;
        \endcode
        */
        void bindRenderTargets();
        
        void drawEffect(Texture* RenderTarget = 0);
        
        /**
        Changes the gaussian multiplier if a bloom filter has already been generated.
        \param[in] GaussianMultiplier Specifies the gaussian multiplier for the bloom filter. By default 0.6.
        \note This has no effect until "createResources" was called.
        \see PostProcessingEffect::valid
        */
        void setFactor(f32 GaussianMultiplier);
        
        /* === Inline functions === */
        
        //! Returns the resolution which has been set after creating the resources for this effect.
        inline dim::size2di getResolution() const
        {
            return Resolution_;
        }
        
        /**
        Returns the specified effect texture.
        \param Type: Specifies the texture name which is to be returned. This must not be RENDERTARGET_COUNT!
        \return Pointer to the specified Texture object.
        */
        inline Texture* getTexture(const ERenderTargets Type)
        {
            return Type < RENDERTARGET_COUNT ? RenderTargets_[Type] : 0;
        }
        
        //! Returns the bloom factor or rather the gaussian multiplier. By default 0.6.
        inline f32 getFactor() const
        {
            return GaussianMultiplier_;
        }
        
        //! Returns true if this effect is active. The effect is active if bloom factor is greater than 0.0.
        inline bool active() const
        {
            return getFactor() > 0.0f;
        }
        
    private:
        
        /* === Macros === */
        
        static const s32 FILTER_SIZE = 9;
        
        /* === Functions === */
        
        bool createRenderTargets();
        bool compileShaders();
        
        void drawFullscreenImage(const ERenderTargets Type);
        void drawFullscreenImageStreched(const ERenderTargets Type);
        
        void computeWeights();
        void computeOffsets();
        
        /* === Members === */
        
        dim::size2di Resolution_;
        
        Texture* RenderTargets_[RENDERTARGET_COUNT];
        
        ShaderClass* BloomShaderHRP_; //!< Bloom shader class for the horizontal render pass.
        ShaderClass* BloomShaderVRP_; //!< Bloom shader class for the vertical render pass.
        
        f32 BlurOffsets_[FILTER_SIZE*2];
        f32 BlurWeights_[FILTER_SIZE];
        
        f32 GaussianMultiplier_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
