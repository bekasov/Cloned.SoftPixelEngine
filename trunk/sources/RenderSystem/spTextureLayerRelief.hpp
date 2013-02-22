/*
 * Relief texture layer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TEXTURE_LAYER_RELIEF_H__
#define __SP_TEXTURE_LAYER_RELIEF_H__


#include "Base/spStandard.hpp"
#include "RenderSystem/spTextureLayer.hpp"


namespace sp
{
namespace video
{


/**
Relief (or rather parllax-occlusion) texture layer class. Particular used for deferred-renderer.
\note Realize that relief-mapping is a performance killer! But it looks really nice :-).
The problem with relief-mapping is, that you have both: lots of texture lookups in the shader,
and "dynamic-branching". These two effects are very time consuming also for moddern graphics hardware.
\see DeferredRenderer
\since Version 3.2
\ingroup group_texture
*/
class SP_EXPORT TextureLayerRelief : public TextureLayer
{
    
    public:
        
        TextureLayerRelief();
        virtual ~TextureLayerRelief();
        
        /* === Inline functions === */
        
        //! Enables or disables relief mapping. By default disabled.
        inline void setReliefEnable(bool Enable)
        {
            ReliefEnabled_ = Enable;
        }
        //! Returns true if relief mapping is enabled. By default disabled.
        inline bool getReliefEnable() const
        {
            return ReliefEnabled_;
        }
        
        /**
        Sets the minimal sample count. This is used when the view camera
        is far away from the surface and relief mapping would be an overkill.
        \param[in] MinSamples Specifies the count of minimal samples. By default 0.
        */
        inline void setMinSamples(s32 MinSamples)
        {
            MinSamples_ = MinSamples;
        }
        //! Returns the count of minimal samples. By default 0.
        inline s32 getMinSamples() const
        {
            return MinSamples_;
        }
        
        /**
        Sets the maximal sample count. This is used when the view camera
        is near to the surface and relief mapping produces nice graphics.
        \param[in] MaxSamples Specifies the count of minimal samples. By default 50.
        */
        inline void setMaxSamples(s32 MaxSamples)
        {
            MaxSamples_ = MaxSamples;
        }
        //! Returns the count of maximal samples. By default 50.
        inline s32 getMaxSamples() const
        {
            return MaxSamples_;
        }
        
        //! Sets the height-map scale factor.
        inline void setHeightMapScale(f32 HeightMapScale)
        {
            HeightMapScale_ = HeightMapScale;
        }
        //! Returns the height-map scale factor. By default 0.015.
        inline f32 getHeightMapScale() const
        {
            return HeightMapScale_;
        }
        
        /**
        Sets the view range. This indicates how quickly the quality decreaes
        when the camera move away from the surface.
        \param[in] ViewRange Specifies the view range. The greater the view range, the better
        the quality - and lower the performance ;-)
        */
        inline void setViewRange(f32 ViewRange)
        {
            ViewRange_ = ViewRange;
        }
        //! Returns the view range. By default 2.0.
        inline f32 getViewRange() const
        {
            return ViewRange_;
        }
        
    protected:
        
        /* === Members === */
        
        bool ReliefEnabled_;
        
        s32 MinSamples_;
        s32 MaxSamples_;
        
        f32 HeightMapScale_;
        f32 ViewRange_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
