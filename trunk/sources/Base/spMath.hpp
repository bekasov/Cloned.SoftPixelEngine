/*
 * Math header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_H__
#define __SP_MATH_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionPoint2D.hpp"
#include "Base/spDimensionRect2D.hpp"
#include "Base/spDimensionLine3D.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionTriangle3D.hpp"
#include "Base/spDimensionPlane3D.hpp"
#include "Base/spMathRadianDegree.hpp"
#include "Base/spMaterialColor.hpp"
#include "Base/spMathCore.hpp"

#include <stdlib.h>
#include <vector>


namespace sp
{

namespace dim
{
template <typename T> class matrix4;
typedef matrix4<f32> matrix4f;
}

namespace math
{


//! Returns the distance between the two given 2D points.
template <typename T> inline T getDistance(const dim::point2d<T> &PosA, const dim::point2d<T> &PosB)
{
    return sqrt(Pow2(PosB.X - PosA.X) + Pow2(PosB.Y - PosA.Y));
}
//! Returns the distance between the two given 3D points.
template <typename T> inline T getDistance(const dim::vector3d<T> &PosA, const dim::vector3d<T> &PosB)
{
    return sqrt(Pow2(PosB.X - PosA.X) + Pow2(PosB.Y - PosA.Y) + Pow2(PosB.Z - PosA.Z));
}

//! \return Suqare distance between two 2D points (used for fast distance comparisions).
template <typename T> inline T getDistanceSq(const dim::point2d<T> &PosA, const dim::point2d<T> &PosB)
{
    return Pow2(PosB.X - PosA.X) + Pow2(PosB.Y - PosA.Y);
}
//! \return Suqare distance between two 3D points (used for fast distance comparisions).
template <typename T> inline T getDistanceSq(const dim::vector3d<T> &PosA, const dim::vector3d<T> &PosB)
{
    return Pow2(PosB.X - PosA.X) + Pow2(PosB.Y - PosA.Y) + Pow2(PosB.Z - PosA.Z);
}

/**
Returns the angle between the two given 2D points.
\param[in] A Specifies the first 2D point.
\param[in] B Specifies the second 2D point.
\return Angle between the two points. This is in the range [0.0 .. 360.0).
If B.X >= A.X and B.Y >= A.Y then the return value is in the range [0.0 .. 90.0).
If B.X >= A.X and B.Y < A.Y then the return value is in the range [90.0 .. 180.0).
If B.X < A.X and B.Y < A.Y then the return value is in the range [180.0 .. 270.0).
If B.X < A.X and B.Y >= A.Y then the return value is in the range [270.0 .. 360.0).
*/
template <typename T> T getAngle(const dim::point2d<T> &A, const dim::point2d<T> &B)
{
    const dim::point2d<T> Dir(B - A);
    const T Dist = math::getDistance(A, B);
    
    if (Dir.X >= T(0))
    {
        if (Dir.Y >= T(0))
            return math::ASin((B.X - A.X) / Dist);
        return T(180) - math::ASin((B.X - A.X) / Dist);
    }
    
    if (Dir.Y >= T(0))
        return T(360) - math::ASin((A.X - B.X) / Dist);
    return T(180) + math::ASin((A.X - B.X) / Dist);
}

/**
Returns the angle between the two given 2D points to the offset angle.
This is in the range [-180.0 .. 180.0].
*/
template <typename T> T getAngle(const dim::point2d<T> &A, const dim::point2d<T> &B, const T &OffsetAngle)
{
    T Angle = getAngle<T>(A, B) - OffsetAngle;
    
    while (Angle > T(180))
        Angle -= T(360);
    while (Angle < T(-180))
        Angle += T(360);
    
    return Angle;
}

//! Returns a bezier value used for bezier curves.
template <typename T> T getBezierValue(const f32 t, const T &Pos1, const T &Pos2, const T &Radial1, const T &Radial2)
{
    const f32 invt    = 1.0f - t;
    const f32 invt2   = invt*invt;
    const f32 invt3   = invt2*invt;
    const f32 t2      = t*t;
    const f32 t3      = t*t*t;
    
    return Pos1*invt3 + Radial1*3*t*invt2 + Radial2*3*t2*invt + Pos2*t3;
}

//! Returns a bernstein value used for bezier patch generation.
template <typename T> T getBernsteinValue(const f32 t, const T Points[4])
{
    const f32 invt = 1.0f - t;
    
    return
        Points[0] * pow(t, 3) +
        Points[1] * (T(3) * pow(t, 2) * invt) +
        Points[2] * (T(3) * pow(invt, 2) * t) +
        Points[3] * pow(invt, 3);
}

//! Returns a gaussian value used for gaussian blur.
template <typename T> T getGaussianValue(const T &X, const T &Mean, const T &StdDeviation)
{
    return (
        ( T(1) / sqrt( T(2) * static_cast<T>(math::PI) * StdDeviation * StdDeviation ) )
        * exp( ( -( ( X - Mean ) * ( X - Mean ) ) ) / ( T(2) * StdDeviation * StdDeviation ) )
    );
}

//! Returns normal vector computed by the three specified coordinates.
template <typename T> inline dim::vector3d<T> getNormalVectorSq(
    const dim::vector3d<T> &PosA, const dim::vector3d<T> &PosB, const dim::vector3d<T> &PosC)
{
    return (PosA - PosB).cross(PosB - PosC);
}

//! Returns normalized normal vector computed by the three specified coordinates.
template <typename T> inline dim::vector3d<T> getNormalVector(
    const dim::vector3d<T> &PosA, const dim::vector3d<T> &PosB, const dim::vector3d<T> &PosC)
{
    return getNormalVectorSq<T>(PosA, PosB, PosC).normalize();
}

//! \deprecated
template <typename T> void sortContainerConst(
    std::vector<T> &ObjectList, bool (*lpFuncCmp)(const T &obj1, const T &obj2))
{
    s32 changed, i;
    
    do
    {
        changed = 0;
        
        for (i = 0; i < ObjectList.size() - 1; ++i)
        {
            if (!lpFuncCmp(ObjectList[i], ObjectList[i + 1]))
            {
                changed = 1;
                Swap(ObjectList[i], ObjectList[i + 1]);
            }
        }
    }
    while (changed);
}

//! \deprecated
template <typename T> void sortContainer(
    std::vector<T> &ObjectList, bool (*lpFuncCmp)(T &obj1, T &obj2))
{
    s32 changed, i;
    
    do
    {
        changed = 0;
        
        for (i = 0; i < ObjectList.size() - 1; ++i)
        {
            if (!lpFuncCmp(ObjectList[i], ObjectList[i + 1]))
            {
                changed = 1;
                Swap(ObjectList[i], ObjectList[i + 1]);
            }
        }
    }
    while (changed);
}


/* === Vertex functions === */

//! \todo Develop this new
SP_EXPORT void getVertexInterpolation(
    const dim::triangle3df &Triangle, const dim::vector3df &Pos, f32 &Vert, f32 &Horz
);

//! \deprecated
template <class T> T getVertexInterpolation(
    const T &VertexA, const T &VertexB, const T &VertexC, const f32 Vert, const f32 Horz)
{
    return VertexA + (VertexB - VertexA) * Vert + (VertexC - VertexA) * Horz;
}


/* === Other distance and vector functions === */

SP_EXPORT dim::matrix4f getTangentSpace(
    const dim::vector3df PosA, const dim::vector3df PosB, const dim::vector3df PosC,
    const dim::point2df MapA, const dim::point2df MapB, const dim::point2df MapC,
    dim::vector3df &Tangent, dim::vector3df &Binormal, dim::vector3df &Normal
);


/* === Other math functions === */

//! Returns color in form of a 4D vector. The color components R, G, B, and A are transformed to the range [0.0, 1.0].
inline dim::vector4df Convert(const video::color &Color)
{
    return dim::vector4df(
        static_cast<f32>(Color.Red  ) / 255,
        static_cast<f32>(Color.Green) / 255,
        static_cast<f32>(Color.Blue ) / 255,
        static_cast<f32>(Color.Alpha) / 255
    );
}

//! Returns color in typical form. The color components R, G, B, and A are transformed to the range [0, 255].
inline video::color Convert(const dim::vector4df &Color)
{
    return video::color(
        static_cast<u8>(Color.X * 255),
        static_cast<u8>(Color.Y * 255),
        static_cast<u8>(Color.Z * 255),
        static_cast<u8>(Color.W * 255)
    );
}


} // /namespace math

} // /namespace sp


#endif



// ================================================================================
