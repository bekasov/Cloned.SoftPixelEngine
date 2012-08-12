/*
 * Shadow mapper file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spShadowMapper.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "SceneGraph/spSceneGraph.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


ShadowMapper::ShadowMapper() :
    SpotLightShadowMapArray_    (0),
    PointLightShadowMapArray_   (0),
    MaxPointLightCount_         (0),
    MaxSpotLightCount_          (0)
{
}
ShadowMapper::~ShadowMapper()
{
    deleteShadowMaps();
}

void ShadowMapper::createShadowMaps(u32 MaxPointLightCount, u32 MaxSpotLightCount)
{
    /* Delete old shadow maps and setup light count */
    deleteShadowMaps();
    
    MaxPointLightCount_ = MaxPointLightCount;
    MaxSpotLightCount_  = MaxSpotLightCount;
    
    /* Create new point light shadow map */
    if (MaxPointLightCount_ > 0)
    {
        //todo
    }
    
    /* Create new spot light shadow map */
    if (MaxSpotLightCount_ > 0)
    {
        //todo
    }
}

void ShadowMapper::deleteShadowMaps()
{
    __spVideoDriver->deleteTexture(PointLightShadowMapArray_);
    __spVideoDriver->deleteTexture(SpotLightShadowMapArray_);
}

bool ShadowMapper::renderShadowMap(scene::SceneGraph* Graph, scene::Light* LightObj, u32 Index)
{
    if (Graph && LightObj)
    {
        switch (LightObj->getLightingType())
        {
            case scene::LIGHT_POINT:
                return renderPointLightShadowMap(Graph, LightObj, Index);
            case scene::LIGHT_SPOT:
                return renderSpotLightShadowMap(Graph, LightObj, Index);
        }
    }
    return false;
}


/*
 * ======= Private: =======
 */

bool ShadowMapper::renderPointLightShadowMap(scene::SceneGraph* Graph, scene::Light* LightObj, u32 Index)
{
    if (!PointLightShadowMapArray_ || Index >= MaxPointLightCount_)
        return false;
    
    //todo
    
    return true;
}

bool ShadowMapper::renderSpotLightShadowMap(scene::SceneGraph* Graph, scene::Light* LightObj, u32 Index)
{
    if (!SpotLightShadowMapArray_ || Index >= MaxSpotLightCount_)
        return false;
    
    //todo
    
    return true;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
