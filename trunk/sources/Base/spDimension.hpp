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

/* 2nd dimension */
#include "Base/spDimensionPoint2D.hpp"
#include "Base/spDimensionSize2D.hpp"
#include "Base/spDimensionRect2D.hpp"

/* 3rd dimension */
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionLine3D.hpp"
#include "Base/spDimensionTriangle3D.hpp"
#include "Base/spDimensionQuadrangle3D.hpp"
#include "Base/spDimensionPlane3D.hpp"

/* Matrix arithmetic */
#include "Base/spDimensionMatrix2.hpp"
#include "Base/spDimensionMatrix3.hpp"
#include "Base/spDimensionMatrix4.hpp"

/* Other dimension classes */
#include "Base/spDimensionQuaternion.hpp"
#include "Base/spDimensionAABB.hpp"
#include "Base/spDimensionOBB.hpp"
#include "Base/spDimensionContainer4.hpp"

/* Container class */
#include "Base/spDimensionSecureList.hpp"
#include "Base/spDimensionUniversalBuffer.hpp"


/* Extended defintions */ 

namespace sp
{
namespace dim
{


template <typename T> vector3d<T>::vector3d(const point2d<T> &Other) : X(Other.X), Y(Other.Y), Z(0)
{
}
template <typename T> vector3d<T>::vector3d(const size2d<T> &Other) : X(Other.Width), Y(Other.Height), Z(1)
{
}

template <typename T> point2d<T>::point2d(const vector3d<T> &Other) : X(Other.X), Y(Other.Y)
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
    return plane3d<T>(vector3d<T>(1, 0, 0), -Max.X);
}
template <typename T> inline plane3d<T> aabbox3d<T>::getTopPlane() const
{
    return plane3d<T>(vector3d<T>(0, 1, 0), -Max.Y);
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
    return plane3d<T>(vector3d<T>(0, 0, 1), -Min.Z);
}

template <typename T> inline bool obbox3d<T>::isPointInside(const vector3d<T> &Point) const
{
    return isInversePointInside(matrix4<T>(*this).getInverse() * Point);
}

template <typename T> inline bool obbox3d<T>::isBoxInside(const obbox3d<T> &Other) const
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


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
