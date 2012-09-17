/*
 * GBuffer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GBUFFER_H__
#define __SP_GBUFFER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spTextureBase.hpp"


namespace sp
{
namespace video
{


//! GBuffer object which contains all textures which are required for a deferred renderer.
class SP_EXPORT GBuffer
{
    
    public:
        
        /* === Enumerations === */
        
        //! GBuffer render target names.
        enum ERenderTargets
        {
            RENDERTARGET_DIFFUSE_AND_SPECULAR = 0,  //!< Diffuse (RGB) and specular (A).
            RENDERTARGET_NORMAL_AND_DEPTH,          //!< Normal vectors (RGB) and depth distance (A).
            
            RENDERTARGET_DEFERRED_COLOR,            //!< Color result from deferred shading for bloom filter.
            RENDERTARGET_DEFERRED_GLOSS,            //!< Gloss result from deferred shading for bloom filter.
            RENDERTARGET_GLOSS_1ST_PASS,            //!< Temporary gloss texture for gaussian blur (1st render pass).
            RENDERTARGET_GLOSS_2ND_PASS,            //!< Temporary gloss texture for gaussian blur (2nd render pass).
            
            RENDERTARGET_COUNT                      //!< Internal count constant. Don't use it to access a texture!
        };
        
        /* === Constructor & destructor === */
        
        GBuffer();
        virtual ~GBuffer();
        
        /* === Functions === */
        
        /**
        Creates the GBuffer textures.
        \param Resolution: Specifies the resolution. This should be the same as specified for the engine's graphics device.
        \param UseMultiSampling: Specifies whether multi-sampling should be used or not. By default true.
        \param UseHDR: Specifies whether HDR rendering is required for the GBuffer or not.
        */
        bool createGBuffer(
            const dim::size2di &Resolution, bool UseMultiSampling = true, bool UseHDR = false, bool UseBloom = false
        );
        //! Deletes the GBuffer textures. When creating a new GBuffer the old textures will be deleted automatically.
        void deleteGBuffer();
        
        void bindRTDeferredShading();
        void drawDeferredShading();
        
        void bindRTBloomFilter();
        
        /* === Inline functions === */
        
        //! Returns the resolution set after creating the GBuffer textures.
        inline dim::size2di getResolution() const
        {
            return Resolution_;
        }
        
        /**
        Returns the specified GBuffer texture.
        \param Type: Specifies the texture name which is to be returned. This must not be RENDERTARGET_COUNT!
        \return Pointer to the specified Texture object.
        */
        inline Texture* getTexture(const ERenderTargets Type)
        {
            return Type < RENDERTARGET_COUNT ? RenderTargets_[Type] : 0;
        }
        
    private:
        
        /* === Functions === */
        
        bool setupMultiRenderTargets();
        
        void drawMRTImage(s32 FirstIndex, s32 LastIndex);
        
        /* === Members === */
        
        dim::size2di Resolution_;
        
        Texture* RenderTargets_[RENDERTARGET_COUNT];
        
        bool UseMultiSampling_;
        bool UseHDR_;
        bool UseBloom_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
