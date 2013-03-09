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
            
            RENDERTARGET_ILLUMINATION,              //!< Illumination (R) from light maps.
            
            RENDERTARGET_COUNT                      //!< Internal count constant. Don't use it to access a texture!
        };
        
        /* === Constructor & destructor === */
        
        GBuffer();
        virtual ~GBuffer();
        
        /* === Functions === */
        
        /**
        Creates the GBuffer textures.
        \param[in] Resolution Specifies the resolution. This should be the same as specified for the engine's graphics device.
        \param[in] MultiSampling Specifies the count of multi-samples. By default 0.
        \param[in] UseIllumination Specifies whether illumination should be used or not. This is used
        to combine dynamic lights with pre-computed static light-maps. By default false.
        \return True if the g-buffer could be created successful.
        */
        bool createGBuffer(
            const dim::size2di &Resolution, s32 MultiSampling = 0, bool UseIllumination = false
        );
        //! Deletes the GBuffer textures. When creating a new GBuffer the old textures will be deleted automatically.
        void deleteGBuffer();
        
        /**
        Binds the g-buffer's render targets.
        The following pixel shader pseudo code illustrates that:
        \code
        struct SPixelOutput
        {
            float4 DiffuseAndSpecular : COLOR0;
            float4 NormalAndDepth     : COLOR1;
        };
        
        // ...
        
        Out.DiffuseAndSpecular.rgb  = FinalPixelColor;
        Out.DiffuseAndSpecular.a    = FinalPixelSpecularFactor;
        
        Out.NormalAndDepth.xyz      = FinalPixelNormal;
        Out.NormalAndDepth.a        = distance(GlobalViewPosition, GlobalPixelPosition);
        \endcode
        */
        void bindRenderTargets();
        
        /**
        
        */
        void drawDeferredShading();
        
        /* === Inline functions === */
        
        //! Returns the resolution set after creating the GBuffer textures.
        inline const dim::size2di& getResolution() const
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
        
        inline bool useIllumination() const
        {
            return UseIllumination_;
        }
        
    private:
        
        /* === Functions === */
        
        bool setupMultiRenderTargets(s32 MultiSampling);
        
        /* === Members === */
        
        dim::size2di Resolution_;
        
        Texture* RenderTargets_[RENDERTARGET_COUNT];
        
        bool UseIllumination_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
