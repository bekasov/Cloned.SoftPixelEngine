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

//! \return Degree (or rather angle) between two 2D vectors.
template <typename T> inline T getDegree(const dim::point2d<T> &VecA, const dim::point2d<T> &VecB)
{
    T Degree;
    
    if (VecA != VecB)
    {
        Degree = math::ASin( (VecB.X - VecA.X) / getDistance(VecA, VecB) );
        Degree = VecA.Y > VecB.Y ? T(180) - Degree : Degree;
    }
    else
        Degree = 0;
    
    return Degree;
}
template <typename T> inline T getDegree(const T X1, const T Y1, const T X2, const T Y2)
{
    T Degree;
    
    if (X1 != X2 || Y1 != Y2)
    {
        Degree = math::ASin( (X2 - X1) / getDistance(X1, Y1, X2, Y2) );
        Degree = Y1 > Y2 ? T(180) - Degree : Degree;
    }
    else
        Degree = 0;
    
    return Degree;
}

//! Returns bezier value used for bezier curves.
template <typename T> inline T getBezierValue(const f32 t, const T &Pos1, const T &Pos2, const T &Radial1, const T &Radial2)
{
    const f32 invt    = 1.0f - t;
    const f32 invt2   = invt*invt;
    const f32 invt3   = invt2*invt;
    const f32 t2      = t*t;
    const f32 t3      = t*t*t;
    
    return Pos1*invt3 + Radial1*3*t*invt2 + Radial2*3*t2*invt + Pos2*t3;
}

//! Returns bernstein value used for bezier patch generation.
template <typename T> inline T getBernsteinValue(const f32 t, const T Points[4])
{
    const f32 invt = 1.0f - t;
    
    return
        Points[0] * pow(t, 3) +
        Points[1] * (T(3) * pow(t, 2) * invt) +
        Points[2] * (T(3) * pow(invt, 2) * t) +
        Points[3] * pow(invt, 3);
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

SP_EXPORT void getVertexInterpolation(
    const dim::triangle3df &Triangle, const dim::vector3df &Pos, f32 &Vert, f32 &Horz
);

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
