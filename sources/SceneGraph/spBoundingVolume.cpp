/*
 * Bounding volume file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spBoundingVolume.hpp"


namespace sp
{
namespace scene
{


BoundingVolume::BoundingVolume() :
    Type_   (BOUNDING_NONE  ),
    Box_    (-0.5f, 0.5f    ),
    Radius_ (0.5f           )
{
}
BoundingVolume::BoundingVolume(const BoundingVolume &Other) :
    Type_   (Other.Type_    ),
    Box_    (Other.Box_     ),
    Radius_ (Other.Radius_  )
{
}
BoundingVolume::~BoundingVolume()
{
}

bool BoundingVolume::checkFrustumCulling(const scene::ViewFrustum &Frustum, const dim::matrix4f &Transformation) const
{
    /* Make frustum-culling tests */
    return !(
        ( Type_ == BOUNDING_SPHERE && !Frustum.isPointInside(Transformation.getPosition(), Radius_) ) ||
        ( Type_ == BOUNDING_BOX && !Frustum.isBoundBoxInside(Box_, Transformation) )
    );
}


} // /namespace scene

} // /namespace sp



// ================================================================================
