/*
 * Mesh animation file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spSkeletalAnimation.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern scene::SceneGraph* __spSceneManager;

namespace scene
{


MeshAnimation::MeshAnimation(const EAnimationTypes Type) :
    Animation(Type)
{
}
MeshAnimation::~MeshAnimation()
{
}


/*
 * ======= Protected: =======
 */

bool MeshAnimation::checkFrustumCulling(scene::Mesh* Object) const
{
    /* Check frustum culling for each visible camera */
    if (!__spSceneManager || !Object)
        return false;
    
    const dim::matrix4f Transformation(Object->getTransformMatrix(true));
    
    foreach (const Camera* Cam, __spSceneManager->getCameraList())
    {
        if (Cam->getVisible() && Object->getBoundingVolume().checkFrustumCulling(Cam->getViewFrustum(), Transformation))
            return true;
    }
    
    return false;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
