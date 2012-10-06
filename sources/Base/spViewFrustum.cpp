/*
 * ViewFrustum file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spViewFrustum.hpp"


namespace sp
{
namespace scene
{


ViewFrustum::ViewFrustum() :
    ConvexPolyhedron<f32, VIEWFRUSTUM_PLANE_COUNT>()
{
}
ViewFrustum::ViewFrustum(const dim::matrix4f &Matrix) :
    ConvexPolyhedron<f32, VIEWFRUSTUM_PLANE_COUNT>()
{
    setFrustum(Matrix);
}
ViewFrustum::ViewFrustum(const ViewFrustum &Other) :
    ConvexPolyhedron<f32, VIEWFRUSTUM_PLANE_COUNT>(Other)
{
}
ViewFrustum::~ViewFrustum()
{
}

void ViewFrustum::setFrustum(const dim::matrix4f &Matrix)
{
    // Near plane
    Planes_[VIEWFRUSTUM_NEAR    ].Normal.X = Matrix[ 3] + Matrix[ 2];
    Planes_[VIEWFRUSTUM_NEAR    ].Normal.Y = Matrix[ 7] + Matrix[ 6];
    Planes_[VIEWFRUSTUM_NEAR    ].Normal.Z = Matrix[11] + Matrix[10];
    Planes_[VIEWFRUSTUM_NEAR    ].Distance = Matrix[15] + Matrix[14];
    
    // Far plane
    Planes_[VIEWFRUSTUM_FAR     ].Normal.X = Matrix[ 3] - Matrix[ 2];
    Planes_[VIEWFRUSTUM_FAR     ].Normal.Y = Matrix[ 7] - Matrix[ 6];
    Planes_[VIEWFRUSTUM_FAR     ].Normal.Z = Matrix[11] - Matrix[10];
    Planes_[VIEWFRUSTUM_FAR     ].Distance = Matrix[15] - Matrix[14];
    
    // Left plane
    Planes_[VIEWFRUSTUM_LEFT    ].Normal.X = Matrix[ 3] + Matrix[ 0];
    Planes_[VIEWFRUSTUM_LEFT    ].Normal.Y = Matrix[ 7] + Matrix[ 4];
    Planes_[VIEWFRUSTUM_LEFT    ].Normal.Z = Matrix[11] + Matrix[ 8];
    Planes_[VIEWFRUSTUM_LEFT    ].Distance = Matrix[15] + Matrix[12];
    
    // Right plane
    Planes_[VIEWFRUSTUM_RIGHT   ].Normal.X = Matrix[ 3] - Matrix[ 0];
    Planes_[VIEWFRUSTUM_RIGHT   ].Normal.Y = Matrix[ 7] - Matrix[ 4];
    Planes_[VIEWFRUSTUM_RIGHT   ].Normal.Z = Matrix[11] - Matrix[ 8];
    Planes_[VIEWFRUSTUM_RIGHT   ].Distance = Matrix[15] - Matrix[12];
    
    // Bottom plane
    Planes_[VIEWFRUSTUM_BOTTOM  ].Normal.X = Matrix[ 3] + Matrix[ 1];
    Planes_[VIEWFRUSTUM_BOTTOM  ].Normal.Y = Matrix[ 7] + Matrix[ 5];
    Planes_[VIEWFRUSTUM_BOTTOM  ].Normal.Z = Matrix[11] + Matrix[ 9];
    Planes_[VIEWFRUSTUM_BOTTOM  ].Distance = Matrix[15] + Matrix[13];
    
    // Top plane
    Planes_[VIEWFRUSTUM_TOP     ].Normal.X = Matrix[ 3] - Matrix[ 1];
    Planes_[VIEWFRUSTUM_TOP     ].Normal.Y = Matrix[ 7] - Matrix[ 5];
    Planes_[VIEWFRUSTUM_TOP     ].Normal.Z = Matrix[11] - Matrix[ 9];
    Planes_[VIEWFRUSTUM_TOP     ].Distance = Matrix[15] - Matrix[13];
    
    // Normalize all planes
    normalize();
}

void ViewFrustum::setFrustum(const dim::matrix4f &ViewMatrix, const dim::matrix4f &ProjectionMatrix)
{
    dim::matrix4f Mat(ViewMatrix);
    Mat.setPosition(-Mat.getPosition());
    
    setFrustum(ProjectionMatrix * Mat);
}

dim::vector3df ViewFrustum::getLeftUp() const
{
    dim::vector3df Point;
    Planes_[VIEWFRUSTUM_FAR].checkMultiplePlaneIntersection(Planes_[VIEWFRUSTUM_TOP], Planes_[VIEWFRUSTUM_LEFT], Point);
    return Point;
}

dim::vector3df ViewFrustum::getLeftDown() const
{
    dim::vector3df Point;
    Planes_[VIEWFRUSTUM_FAR].checkMultiplePlaneIntersection(Planes_[VIEWFRUSTUM_BOTTOM], Planes_[VIEWFRUSTUM_LEFT], Point);
    return Point;
}

dim::vector3df ViewFrustum::getRightUp() const
{
    dim::vector3df Point;
    Planes_[VIEWFRUSTUM_FAR].checkMultiplePlaneIntersection(Planes_[VIEWFRUSTUM_TOP], Planes_[VIEWFRUSTUM_RIGHT], Point);
    return Point;
}

dim::vector3df ViewFrustum::getRightDown() const
{
    dim::vector3df Point;
    Planes_[VIEWFRUSTUM_FAR].checkMultiplePlaneIntersection(Planes_[VIEWFRUSTUM_BOTTOM], Planes_[VIEWFRUSTUM_RIGHT], Point);
    return Point;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
