/*
 * Shadow mapper header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHADOW_MAPPER_H__
#define __SP_SHADOW_MAPPER_H__


#include "Base/spStandard.hpp"


namespace sp
{

namespace scene
{
    class SceneGraph;
    class Light;
}

namespace video
{


class Texture;

//! \todo This is unfinished
class ShadowMapper
{
    
    public:
        
        ShadowMapper();
        virtual ~ShadowMapper();
        
        /* Functions */
        
        virtual void createShadowMaps(u32 MaxPointLightCount, u32 MaxSpotLightCount);
        virtual void deleteShadowMaps();
        
        virtual bool renderShadowMap(scene::SceneGraph* Graph, scene::Light* LightObj, u32 Index);
        
        /* Inline functions */
        
        inline video::Texture* getSpotLightShadowMapArray()
        {
            return SpotLightShadowMapArray_;
        }
        inline video::Texture* getPointLightShadowMapArray()
        {
            return PointLightShadowMapArray_;
        }
        
    protected:
        
        /* Functions */
        
        //! \warning Does not check for null pointers!
        bool renderPointLightShadowMap(scene::SceneGraph* Graph, scene::Light* LightObj, u32 Index);
        //! \warning Does not check for null pointers!
        bool renderSpotLightShadowMap(scene::SceneGraph* Graph, scene::Light* LightObj, u32 Index);
        
        /* Members */
        
        video::Texture* PointLightShadowMapArray_;
        video::Texture* SpotLightShadowMapArray_;
        
        u32 MaxPointLightCount_;
        u32 MaxSpotLightCount_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
