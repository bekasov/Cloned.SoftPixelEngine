/*
 * Dimensions header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_H__
#define __SP_DIMENSION_H__


/*
 * All dimension classes:
 *   point2d, size2d, rect2d, vector3d, line3d, triangle3d, quadrangle3d, plane3d,
 *   matrix2, matrix4, matrix4, quaternion, view-frustum, array, universal-buffer
 */

/* Vector arithmetic */
#include "Base/spDimensionVector2D.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionVector4D.hpp"

/* Matrix arithmetic */
#include "Base/spDimensionMatrix2.hpp"
#include "Base/spDimensionMatrix3.hpp"
#include "Base/spDimensionMatrix4.hpp"

/* 2nd dimension */
#include "Base/spDimensionSize2D.hpp"
#include "Base/spDimensionRect2D.hpp"

/* 3rd dimension */
#include "Base/spDimensionLine3D.hpp"
#include "Base/spDimensionTriangle3D.hpp"
#include "Base/spDimensionQuadrangle3D.hpp"
#include "Base/spDimensionPlane3D.hpp"

/* Other dimension classes */
#include "Base/spDimensionQuaternion.hpp"
#include "Base/spDimensionAABB.hpp"
#include "Base/spDimensionOBB.hpp"

/* Container class */
#include "Base/spDimensionSecureList.hpp"
#include "Base/spDimensionUniversalBuffer.hpp"


/* Extended defintions */ 

namespace sp
{
namespace dim
{


/* === Vector typedefs for shader uniforms === */

typedef vector2di int2; //!< Typedef alternative for vector2di. Used for shader uniforms and constant buffers.
typedef vector3di int3; //!< Typedef alternative for vector3di. Used for shader uniforms and constant buffers.
typedef vector4di int4; //!< Typedef alternative for vector4di. Used for shader uniforms and constant buffers.

typedef vector2d<u32> uint2; //!< Typedef alternative for vector2d<u32>. Used for shader uniforms and constant buffers.
typedef vector3d<u32> uint3; //!< Typedef alternative for vector3d<u32>. Used for shader uniforms and constant buffers.
typedef vector4d<u32> uint4; //!< Typedef alternative for vector4d<u32>. Used for shader uniforms and constant buffers.

typedef vector2df float2; //!< Typedef alternative for vector2df. Used for shader uniforms and constant buffers.
typedef vector3df float3; //!< Typedef alternative for vector3df. Used for shader uniforms and constant buffers.
typedef vector4df float4; //!< Typedef alternative for vector4df. Used for shader uniforms and constant buffers.

typedef vector2d<f64> double2; //!< Typedef alternative for vector2d<f64>. Used for shader uniforms and constant buffers.
typedef vector3d<f64> double3; //!< Typedef alternative for vector3d<f64>. Used for shader uniforms and constant buffers.
typedef vector4d<f64> double4; //!< Typedef alternative for vector4d<f64>. Used for shader uniforms and constant buffers.


/* === Matrix typedefs for shader uniforms === */

typedef matrix2i int2x2; //!< Typedef alternative for matrix2i. Used for shader uniforms and constant buffers.
typedef matrix3i int3x3; //!< Typedef alternative for matrix3i. Used for shader uniforms and constant buffers.
typedef matrix4i int4x4; //!< Typedef alternative for matrix4i. Used for shader uniforms and constant buffers.

typedef matrix2<u32> uint2x2; //!< Typedef alternative for matrix2<u32>. Used for shader uniforms and constant buffers.
typedef matrix3<u32> uint3x3; //!< Typedef alternative for matrix3<u32>. Used for shader uniforms and constant buffers.
typedef matrix4<u32> uint4x4; //!< Typedef alternative for matrix4<u32>. Used for shader uniforms and constant buffers.

typedef matrix2f float2x2; //!< Typedef alternative for matrix2f. Used for shader uniforms and constant buffers.
typedef matrix3f float3x3; //!< Typedef alternative for matrix3f. Used for shader uniforms and constant buffers.
typedef matrix4f float4x4; //!< Typedef alternative for matrix4f. Used for shader uniforms and constant buffers.

typedef matrix2d double2x2; //!< Typedef alternative for matrix2d. Used for shader uniforms and constant buffers.
typedef matrix3d double3x3; //!< Typedef alternative for matrix3d. Used for shader uniforms and constant buffers.
typedef matrix4d double4x4; //!< Typedef alternative for matrix4d. Used for shader uniforms and constant buffers.


/* === Additional template functions === */

template <typename T> vector3d<T>::vector3d(const vector2d<T> &Other) :
    X(Other.X   ),
    Y(Other.Y   ),
    Z(0         )
{
}

template <typename T> vector2d<T>::vector2d(const vector3d<T> &Other) :
    X(Other.X),
    Y(Other.Y)
{
}

template <typename T> matrix4<T>::matrix4(const matrix3<T> &Other)
{
    M[0] = Other[0]; M[4] = Other[3]; M[ 8] = Other[6]; M[12] = 0;
    M[1] = Other[1]; M[5] = Other[4]; M[ 9] = Other[7]; M[13] = 0;
    M[2] = Other[2]; M[6] = Other[5]; M[10] = Other[8]; M[14] = 0;
    M[3] = 0;        M[7] = 0;        M[11] = 0;        M[15] = 1;
}

template <typename T> inline plane3d<T> aabbox3d<T>::getLeftPlane() const
{
    return plane3d<T>(vector3d<T>(-1, 0, 0), -Min.X);
}
template <typename T> inline plane3d<T> aabbox3d<T>::getRightPlane() const
{
    return plane3d<T>(vector3d<T>(1, 0, 0), Max.X);
}
template <typename T> inline plane3d<T> aabbox3d<T>::getTopPlane() const
{
    return plane3d<T>(vector3d<T>(0, 1, 0), Max.Y);
}
template <typename T> inline plane3d<T> aabbox3d<T>::getBottomPlane() const
{
    return plane3d<T>(vector3d<T>(0, -1, 0), -Min.Y);
}
template <typename T> inline plane3d<T> aabbox3d<T>::getFrontPlane() const
{
    return plane3d<T>(vector3d<T>(0, 0, -1), -Min.Z);
}
template <typename T> inline plane3d<T> aabbox3d<T>::getBackPlane() const
{
    return plane3d<T>(vector3d<T>(0, 0, 1), Max.Z);
}

template <typename T> inline bool obbox3d<T>::isPointInside(const vector3d<T> &Point) const
{
    return isInversePointInside(matrix4<T>(*this).getInverse() * Point);
}

template <typename T> bool obbox3d<T>::isBoxInside(const obbox3d<T> &Other) const
{
    const matrix4<T> OtherMat(Other);
    const matrix4<T> ThisMat(matrix4<T>(*this).getInverse());
    
    return
        isInversePointInside(ThisMat * (OtherMat * vector3d<T>( 1,  1,  1))) &&
        isInversePointInside(ThisMat * (OtherMat * vector3d<T>( 1,  1, -1))) &&
        isInversePointInside(ThisMat * (OtherMat * vector3d<T>( 1, -1,  1))) &&
        isInversePointInside(ThisMat * (OtherMat * vector3d<T>( 1, -1, -1))) &&
        isInversePointInside(ThisMat * (OtherMat * vector3d<T>(-1,  1,  1))) &&
        isInversePointInside(ThisMat * (OtherMat * vector3d<T>(-1,  1, -1))) &&
        isInversePointInside(ThisMat * (OtherMat * vector3d<T>(-1, -1,  1))) &&
        isInversePointInside(ThisMat * (OtherMat * vector3d<T>(-1, -1, -1)));
}

template <typename T> vector3d<T> aabbox3d<T>::getClosestPoint(const plane3d<T> &Plane) const
{
    vector3d<T> Result, Corner;
    
    T Distance = math::OMEGA;
    T Temp;
    
    for (s32 i = 0; i < 8; ++i)
    {
        Corner = getCorner(i);
        
        Temp = Plane.getPointDistance(Corner);
        
        if (Temp < Distance)
        {
            Result = Corner;
            Distance = Temp;
        }
    }
    
    return Result;
}


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
