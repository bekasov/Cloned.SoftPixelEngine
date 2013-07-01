/*
 * Math file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spMath.hpp"
#include "Base/spInputOutputLog.hpp"
#include "Base/spDimension.hpp"
#include "Base/spMathCollisionLibrary.hpp"


namespace sp
{
namespace math
{


SP_EXPORT void getVertexInterpolation(
    const dim::triangle3df &Triangle, const dim::vector3df &Pos, f32 &Vert, f32 &Horz)
{
    // Temporary variables
    const f32 lenu = getDistance(Triangle.PointA, Triangle.PointB);
    const f32 lenv = getDistance(Triangle.PointA, Triangle.PointC);
    
    if (lenu < ROUNDING_ERROR || lenv < ROUNDING_ERROR)
        return;
    
    // Compute the vertex interpolation
    dim::vector3df u(Triangle.PointB - Triangle.PointA);
    dim::vector3df v(Triangle.PointC - Triangle.PointA);
    
    dim::vector3df pa, pb;
    
    math::CollisionLibrary::getLineLineIntersection(
        dim::line3df(Triangle.PointA, Triangle.PointB), dim::line3df(Pos, Pos - v), pa
    );
    math::CollisionLibrary::getLineLineIntersection(
        dim::line3df(Triangle.PointA, Triangle.PointC), dim::line3df(Pos, Pos - u), pb
    );
    
    dim::point2df map(getDistance(Pos, pb), getDistance(Pos, pa));
    
    Vert = map.X / lenu;
    Horz = map.Y / lenv;
}

SP_EXPORT dim::matrix4f getTangentSpace(
    const dim::vector3df PosA, const dim::vector3df PosB, const dim::vector3df PosC,
    const dim::point2df MapA, const dim::point2df MapB, const dim::point2df MapC,
    dim::vector3df &Tangent, dim::vector3df &Binormal, dim::vector3df &Normal)
{
    const dim::vector3df v1(PosB - PosA);
    const dim::vector3df v2(PosC - PosA);
    
    const dim::point2df st1(MapB - MapA);
    const dim::point2df st2(MapC - MapA);
    
    Tangent     = (v1 * st2.Y) - (v2 * st1.Y);
    Binormal    = (v1 * st2.X) - (v2 * st1.X);
    Normal      = v1.cross(v2);
    
    Tangent .normalize();
    Binormal.normalize();
    Normal  .normalize();
    
    if (Tangent.cross(Binormal).dot(Normal) > 0.0f)
    {
        Tangent     = -Tangent;
        Binormal    = -Binormal;
    }
    
    return dim::matrix4f(
        Tangent.X, Binormal.X, Normal.X, 0,
        Tangent.Y, Binormal.Y, Normal.Y, 0,
        Tangent.Z, Binormal.Z, Normal.Z, 0,
        0,         0,          0,        1
    );
}

SP_EXPORT f32 updateEloNumber(f32 Ra, f32 Rb, f32 Sa, const f32 k)
{
    const f32 E = 1.0f / (1.0f + pow(10.0f, (Rb - Ra) / 400.0f));
    return max(0.0f, Ra + k * (Sa - E));
}


} // /namespace math

} // /namespace sp



// ================================================================================
