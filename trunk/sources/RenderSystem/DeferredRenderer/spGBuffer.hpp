/*
 * GBuffer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GBUFFER_H__
#define __SP_GBUFFER_H__


#include "Base/spStandard.hpp"
#include "RenderSystem/spTextureBase.hpp"


namespace sp
{
namespace video
{


//! \todo This is unfinished
class SP_EXPORT GBuffer
{
    
    public:
        
        /* Enumerations */
        
        enum ERenderTargets
        {
            RENDERTARGET_COLORMAP = 0,
            RENDERTARGET_NORMALMAP,
            RENDERTARGET_DEPTHMAP,
            
            RENDERTARGET_COUNT
        };
        
        /* Constructor & destructor */
        
        GBuffer();
        virtual ~GBuffer();
        
        /* Functions */
        
        virtual void createGBuffer(const dim::size2di &Resolution, bool isMultiSampling = true);
        virtual void deleteGBuffer();
        
        /* Inline functions */
        
        inline dim::size2di getResolution() const
        {
            return Resolution_;
        }
        
        inline video::Texture* getTexture(const ERenderTargets Type)
        {
            return Type < RENDERTARGET_COUNT ? RenderTargets_[Type] : 0;
        }
        
    private:
        
        /* Members */
        
        dim::size2di Resolution_;
        
        video::Texture* RenderTargets_[RENDERTARGET_COUNT];
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
