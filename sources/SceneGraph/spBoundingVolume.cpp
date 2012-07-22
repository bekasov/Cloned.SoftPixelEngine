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
    Box_    (-1.0f, 1.0f    ),
    Radius_ (1.0f           )
{
}
BoundingVolume::~BoundingVolume()
{
}

bool BoundingVolume::checkFrustumCulling(const math::ViewFrustum &Frustum, const dim::matrix4f &Transformation) const
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
