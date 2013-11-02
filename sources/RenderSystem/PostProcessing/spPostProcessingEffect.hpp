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


#include "Base/spDimensionSize2D.hpp"
#include "Base/spInputOutputLog.hpp"


namespace sp
{
namespace video
{


class ShaderClass;
class Texture;

//! Virtual base class for post-processing effects (such as bloom-effect).
class SP_EXPORT PostProcessingEffect
{
    
    public:
        
        virtual ~PostProcessingEffect()
        {
        }
        
        /* === Functions === */
        
        //! Returns the name of the post-processing effect.
        virtual const c8* getName() const = 0;

        /**
        Draws the post-processing effect onto the screen or into the render target.
        \param[in] InputTexture Pointer to the input texture. Some effects need more
        than one input texture. In this case set this to null, the input textures will
        be bound on a seperate way.
        \param[in] OutputTexture Pointer to a valid render target texture or null if the
        effect is to be renderd directly into the frame buffer (or rather onto the screen).
        */
        virtual void drawEffect(Texture* InputTexture, Texture* OutputTexture = 0) = 0;
        
        //! Sets the new resolution for this effect.
        virtual void setResolution(const dim::size2di &Resolution)
        {
            if (Resolution_ != Resolution)
            {
                Resolution_ = Resolution;
                if (valid())
                    adjustResolution();
            }
        }
        
        /* === Inline functions === */
        
        //! Returns true if the effect has been created correctly and is valid to be used.
        inline bool valid() const
        {
            return Valid_;
        }
        
        //! Returns the resolution which has been set after creating the resources for this effect.
        inline const dim::size2di& getResolution() const
        {
            return Resolution_;
        }
        
    protected:
        
        PostProcessingEffect() :
            ShdClass_   (0      ),
            Valid_      (false  )
        {
        }
        
        /* === Functions === */
        
        virtual void adjustResolution()
        {
            // Do nothing
        }

        bool errShaderNotSupported()
        {
            io::Log::error("Shaders for \"" + io::stringc(getName()) + "\" post-processing effect are not supported for this render system");
            return false;
        }
        
        /* === Members === */
        
        ShaderClass* ShdClass_;

        bool Valid_;
        dim::size2di Resolution_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
