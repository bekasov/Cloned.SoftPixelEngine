/*
 * **********************************
 *                                   *
 * HLC Games - Software Entertainment *
 *                                     *
 * (c) Copyright 2003 - Lukas Hermanns  *
 *                                       *
 * ****************************************
 *  ___    ___    ___          _____
 * /\  \  /\  \  /\  \        /  ___\
 * \ \  \_\_\  \ \ \  \      /\  \__/
 *  \ \   ___   \ \ \  \     \ \  \
 *   \ \  \__/\  \ \ \  \____ \ \  \___
 *    \ \__\ \ \__\ \ \______\ \ \_____\
 *     \/__/  \/__/  \/______/  \/_____/
 *
 * ****************************************
 *
 * ********************** HLC SoftPixelEngine - (3.1.2008) ************************
 *
 * (c) Copyright 2008 - Lukas Hermanns
 *
 * container4 header
 *
 */

#ifndef __SP_DIMENSION_CONTAINER4_H__
#define __SP_DIMENSION_CONTAINER4_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionQuaternion.hpp"

#include <math.h>


namespace sp
{
namespace dim
{


template <typename T> class container4
{
    
    public:
        
        container4(const T Scalar = 0) : X(Scalar), Y(Scalar), Z(Scalar), W(Scalar)
        {
        }
        container4(const T InitX, const T InitY, const T InitZ = 0, const T InitW = 1) : X(InitX), Y(InitY), Z(InitZ), W(InitW)
        {
        }
        container4(const vector3d<T> &Vector) : X(Vector.X), Y(Vector.Y), Z(Vector.Z), W(1)
        {
        }
        container4(const vector4d<T> &Vector) : X(Vector.X), Y(Vector.Y), Z(Vector.Z), W(Vector.W)
        {
        }
        container4(const quaternion &Vector) : X((T)Vector.X), Y((T)Vector.Y), Z((T)Vector.Z), W((T)Vector.W)
        {
        }
        
        /* === Operators - addition, subtraction, division, multiplication === */
        
        inline container4& operator = (const container4 &other)
        {
            X = other.X; Y = other.Y; Z = other.Z; W = other.W; return *this;
        }
        
        inline container4<T> operator + (const container4<T> &other) const
        {
            return vector3d<T>(X + other.X, Y + other.Y, Z + other.Z, W + other.W);
        }
        inline container4<T>& operator += (const container4<T> &other)
        {
            X += other.X; Y += other.Y; Z += other.Z; W += other.W; return *this;
        }
        
        inline container4<T> operator - (const container4<T> &other) const
        {
            return vector3d<T>(X - other.X, Y - other.Y, Z - other.Z, W - other.W);
        }
        inline container4<T>& operator -= (const container4<T> &other)
        {
            X -= other.X; Y -= other.Y; Z -= other.Z; W -= other.W; return *this;
        }
        
        inline container4<T> operator / (const container4<T> &other) const
        {
            return container4<T>(X / other.X, Y / other.Y, Z / other.Z, W / other.W);
        }
        inline container4<T>& operator /= (const container4<T> &other)
        {
            X /= other.X; Y /= other.Y; Z /= other.Z; W /= other.W; return *this;
        }
        
        inline container4<T> operator / (const T Size) const
        {
            return *this * (1.0 / Size);
        }
        inline container4<T>& operator /= (const T Size)
        {
            return *this *= (1.0 / Size);
        }
        
        inline container4<T> operator * (const container4<T> &other) const
        {
            return container4<T>(X * other.X, Y * other.Y, Z * other.Z, W * other.W);
        }
        inline container4<T>& operator *= (const container4<T> &other)
        {
            X *= other.X; Y *= other.Y; Z *= other.Z; W *= other.W; return *this;
        }
        
        inline container4<T> operator * (const T Size) const
        {
            return container4<T>(X * Size, Y * Size, Z * Size, W * Size);
        }
        inline container4<T>& operator *= (const T Size)
        {
            X *= Size; Y *= Size; Z *= Size; W *= Size; return *this;
        }
        
        /* === Additional operators === */
        
        inline const T operator [] (u32 i) const
        {
            return i < 4 ? *(&X + i) : (T)0;
        }
        
        inline T& operator [] (u32 i)
        {
            return *(&X + i);
        }
        
        /* Members */
        
        T X, Y, Z, W;
        
};

typedef container4<f32> float4;
typedef container4<s32> int4;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
