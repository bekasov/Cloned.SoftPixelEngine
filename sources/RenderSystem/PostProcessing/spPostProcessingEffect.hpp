/*
 * Post processing effect header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_POST_PROCESSING_EFFECT_H__
#define __SP_POST_PROCESSING_EFFECT_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_POSTPROCESSING)


namespace sp
{
namespace video
{


class Texture;

//! Virtual base class for post-processing effects (such as bloom-effect).
class SP_EXPORT PostProcessingEffect
{
    
    public:
        
        virtual ~PostProcessingEffect()
        {
        }
        
        /* === Functions === */
        
        /**
        Draws the post-processing effect onto the screen or into the render target.
        \param[in] RenderTarget Pointer to a valid render target texture or null if the
        effect is to be renderd directly into the frame buffer (or rather onto the screen).
        */
        virtual void drawEffect(Texture* RenderTarget = 0) = 0;
        
        /* === Inline functions === */
        
        //! Returns true if the effect has been created correctly and is valid to be used.
        inline bool valid() const
        {
            return Valid_;
        }
        
    protected:
        
        PostProcessingEffect() : Valid_(false)
        {
        }
        
        /* === Members === */
        
        bool Valid_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
