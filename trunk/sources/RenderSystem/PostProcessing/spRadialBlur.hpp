/*
 * Radial blur header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RADIAL_BLUR_H__
#define __SP_RADIAL_BLUR_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_POSTPROCESSING)


#include "Base/spDimensionSize2D.hpp"
#include "RenderSystem/PostProcessing/spPostProcessingEffect.hpp"


namespace sp
{
namespace video
{


//! GBuffer object which contains all textures which are required for a deferred renderer.
class SP_EXPORT RadialBlur : public PostProcessingEffect
{
    
    public:
        
        RadialBlur();
        ~RadialBlur();
        
        /* === Functions === */
        
        bool createResources();
        void deleteResources();
        
        const c8* getName() const;

        void drawEffect(Texture* InputTexture, Texture* OutputTexture = 0);
        
        /* === Inline functions === */
        
        //! Sets the number of texture samples for the blur effect.
        inline void setNumSamples(u32 NumSamples)
        {
            NumSamples_ = NumSamples;
            setupConstBuffers();
        }
        //! Returns the number of texture samples for the blur effect. By default 8.
        inline u32 getNumSamples() const
        {
            return NumSamples_;
        }
        
        //! Sets the radial blur scaling factor.
        inline void setScaling(f32 Scaling)
        {
            Scaling_ = Scaling;
            setupConstBuffers();
        }
        //! Returns the radial blur scaling factor. By default 0.1.
        inline f32 getScaling() const
        {
            return Scaling_;
        }
        
        //! Returns true if this effect is active. The effect is active if scaling factor is greater than 0.0.
        inline bool active() const
        {
            return getScaling() > 0.0f;
        }
        
    private:
        
        /* === Functions === */
        
        bool compileShaders();
        void setupConstBuffers();
        
        /* === Members === */
        
        u32 NumSamples_;
        f32 Scaling_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
