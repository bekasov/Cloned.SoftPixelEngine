/*
 * Vector 3D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_VECTOR3D_H__
#define __SP_DIMENSION_VECTOR3D_H__


#include "Base/spStandard.hpp"
#include "Base/spMathCore.hpp"
#include "Base/spVectorArithmetic.hpp"


namespace sp
{
namespace dim
{


template <typename T> class vector4d;
template <typename T> class point2d;
template <typename T> class size2d;


//! Axis direction types.
enum EAxisTypes
{
    AXIS_X_POSITIVE = 0,
    AXIS_X_NEGATIVE,
    AXIS_Y_POSITIVE,
    AXIS_Y_NEGATIVE,
    AXIS_Z_POSITIVE,
    AXIS_Z_NEGATIVE,
};


/**
Vector 3D class which has the three components X, Y and Z. This is the main class
used for 3D scene directions, positions, scaling etc.
*/
template <typename T> class vector3d
{
    
    public:
        
        static const u32 NUM = 3;
        
        vector3d() :
            X(0),
            Y(0),
            Z(0)
        {
        }
        vector3d(const T &Size) :
            X(Size),
            Y(Size),
            Z(Size)
        {
        }
        vector3d(const T &VecX, const T &VecY, const T &VecZ) :
            X(VecX),
            Y(VecY),
            Z(VecZ)
        {
        }
        vector3d(const T &VecX, const T &VecY, const T &VecZ, const T &VecW) :
            X(VecX*VecW),
            Y(VecY*VecW),
            Z(VecZ*VecW)
        {
        }
        vector3d(const vector3d<T> &Other) :
            X(Other.X),
            Y(Other.Y),
            Z(Other.Z)
        {
        }
        vector3d(const vector4d<T> &Other);
        vector3d(const point2d<T> &Other);
        vector3d(const size2d<T> &Other);
        ~vector3d()
        {
        }
        
        /* === Operators - comparisions === */
        
        inline bool operator == (const vector3d<T> &Other) const
        {
            return math::equal(X, Other.X) && math::equal(Y, Other.Y) && math::equal(Z, Other.Z);
        }
        inline bool operator != (const vector3d<T> &Other) const
        {
            return !math::equal(X, Other.X) || !math::equal(Y, Other.Y) || !math::equal(Z, Other.Z);
        }
        
        inline bool operator <= (const vector3d<T> &Other) const
        {
            return
                ( X < Other.X || math::equal(X, Other.X) ) ||
                ( math::equal(X, Other.X) && ( Y < Other.Y || math::equal(Y, Other.Y) ) ) ||
                ( math::equal(X, Other.X) && math::equal(Y, Other.Y) && ( Z < Other.Z || math::equal(Z, Other.Z) ) );
        }
        inline bool operator >= (const vector3d<T> &Other) const
        {
            return
                ( X > Other.X || math::equal(X, Other.X) ) ||
                ( math::equal(X, Other.X) && (Y > Other.Y || math::equal(Y, Other.Y) ) ) ||
                ( math::equal(X, Other.X) && math::equal(Y, Other.Y) && ( Z > Other.Z || math::equal(Z, Other.Z) ) );
        }
        
        inline bool operator < (const vector3d<T> &Other) const
        {
            return
                ( X < Other.X && !math::equal(X, Other.X) ) ||
                ( math::equal(X, Other.X) && Y < Other.Y && !math::equal(Y, Other.Y) ) ||
                ( math::equal(X, Other.X) && math::equal(Y, Other.Y) && Z < Other.Z && !math::equal(Z, Other.Z) );
        }
        inline bool operator > (const vector3d<T> &Other) const
        {
            return
                ( X > Other.X && !math::equal(X, Other.X) ) ||
                ( math::equal(X, Other.X) && Y > Other.Y && !math::equal(Y, Other.Y) ) ||
                ( math::equal(X, Other.X) && math::equal(Y, Other.Y) && Z > Other.Z && !math::equal(Z, Other.Z) );
        }
        
        /* === Operators - addition, subtraction, division, multiplication === */
        
        //! Pre-increment operator.
        inline vector3d<T>& operator ++ ()
        {
            ++X; ++Y; ++Z; return *this;
        }
        //! Post-increment operator.
        inline vector3d<T>& operator ++ (int)
        {
            const vector3d<T> Tmp(*this);
            ++*this;
            return Tmp;
        }
        
        //! Pre-decrement operator.
        inline vector3d<T>& operator -- ()
        {
            --X; --Y; --Z; return *this;
        }
        //! Post-decrement operator.
        inline vector3d<T>& operator -- (int)
        {
            const vector3d<T> Tmp(*this);
            --*this;
            return Tmp;
        }
        
        inline vector3d<T> operator + (const vector3d<T> &Other) const
        {
            return vector3d<T>(X + Other.X, Y + Other.Y, Z + Other.Z);
        }
        inline vector3d<T>& operator += (const vector3d<T> &Other)
        {
            X += Other.X; Y += Other.Y; Z += Other.Z; return *this;
        }
        
        inline vector3d<T> operator - (const vector3d<T> &Other) const
        {
            return vector3d<T>(X - Other.X, Y - Other.Y, Z - Other.Z);
        }
        inline vector3d<T>& operator -= (const vector3d<T> &Other)
        {
            X -= Other.X; Y -= Other.Y; Z -= Other.Z; return *this;
        }
        
        inline vector3d<T> operator / (const vector3d<T> &Other) const
        {
            return vector3d<T>(X / Other.X, Y / Other.Y, Z / Other.Z);
        }
        inline vector3d<T>& operator /= (const vector3d<T> &Other)
        {
            X /= Other.X; Y /= Other.Y; Z /= Other.Z; return *this;
        }
        
        inline vector3d<T> operator * (const vector3d<T> &Other) const
        {
            return vector3d<T>(X * Other.X, Y * Other.Y, Z * Other.Z);
        }
        inline vector3d<T>& operator *= (const vector3d<T> &Other)
        {
            X *= Other.X; Y *= Other.Y; Z *= Other.Z; return *this;
        }
        
        inline vector3d<T> operator * (const T &Size) const
        {
            return vector3d<T>(X * Size, Y * Size, Z * Size);
        }
        inline vector3d<T>& operator *= (const T &Size)
        {
            X *= Size; Y *= Size; Z *= Size; return *this;
        }
        
        inline vector3d<T> operator / (const T &Size) const
        {
            return *this * (T(1) / Size);
        }
        inline vector3d<T>& operator /= (const T &Size)
        {
            return *this *= (T(1) / Size);
        }
        
        inline vector3d<T> operator - () const
        {
            return vector3d<T>(-X, -Y, -Z);
        }
        
        /* === Additional operators === */
        
        inline const T& operator [] (u32 i) const
        {
            return *(&X + i);
        }
        
        inline T& operator [] (u32 i)
        {
            return *(&X + i);
        }
        
        /* === Extra functions === */
        
        //! Returns the dot (or rather scalar) product between this and the given vector.
        inline T dot(const vector3d<T> &Other) const
        {
            return dim::dot(*this, Other);
        }
        
        //! Returns the cross (or rather vector) product between this and the given vector.
        inline vector3d<T> cross(const vector3d<T> &Other) const
        {
            return dim::cross(*this, Other);
        }
        
        //! Returns the vector's length.
        inline T getLength() const
        {
            return dim::length(*this);
        }
        
        //! Returns the square of the vector's length (Can be used for faster comparision between two distances).
        inline T getLengthSq() const
        {
            return dim::dot(*this, *this);
        }
        
        //! Returns the angle (in degrees) between this and the given vector.
        inline T getAngle(const vector3d<T> &Other) const
        {
            return dim::angle(*this, Other) * T(math::RAD64);
        }
        
        inline vector3d<T>& setInverse()
        {
            X = -X; Y = -Y; Z = -Z; return *this;
        }
        inline vector3d<T> getInverse() const
        {
            return vector3d<T>(-X, -Y, -Z);
        }
        
        inline bool equal(const vector3d<T> &Other, f32 Tolerance = math::ROUNDING_ERROR) const
        {
            return
                (X + Tolerance > Other.X) && (X - Tolerance < Other.X) &&
                (Y + Tolerance > Other.Y) && (Y - Tolerance < Other.Y) &&
                (Z + Tolerance > Other.Z) && (Z - Tolerance < Other.Z);
        }
        inline bool empty() const
        {
            return equal(0);
        }
        
        //! \todo This should be a global function in the "math" namespace.
        inline void make2DProjection(s32 ScreenWidth, s32 ScreenHeight)
        {
            X =   X * static_cast<f32>(ScreenWidth /2) + ScreenWidth /2;
            Y = - Y * static_cast<f32>(ScreenHeight/2) + ScreenHeight/2;
            Z = T(0);
        }
        //! \todo This should be a global function in the "math" namespace.
        inline void make2DProjection(f32 FOV, s32 ScreenWidth, s32 ScreenHeight)
        {
            X =   X / Z * FOV + ScreenWidth /2;
            Y = - Y / Z * FOV + ScreenHeight/2;
        }
        
        inline vector3d<T>& setAbs()
        {
            X = X > 0 ? X : -X;
            Y = Y > 0 ? Y : -Y;
            Z = Z > 0 ? Z : -Z;
            return *this;
        }
        inline vector3d<T> getAbs() const
        {
            return vector3d<T>(
                X > 0 ? X : -X,
                Y > 0 ? Y : -Y,
                Z > 0 ? Z : -Z
            );
        }
        
        //! Normalizes the vectors. After that the vector has the length of 1.
        inline vector3d<T>& normalize()
        {
            dim::normalize(*this);
            return *this;
        }

        //! Per-component signum function. \see math::sgn
        inline vector3d<T>& sgn()
        {
            math::sgn(X);
            math::sgn(Y);
            math::sgn(Z);
            return *this;
        }
        
        inline vector3d<T>& setLength(const T &Length)
        {
            normalize();
            *this *= Length;
            return *this;
        }
        
        inline T getDistanceFromSq(const vector3d<T> &Other) const
        {
            return vector3d<T>(X - Other.X, Y - Other.Y, Z - Other.Z).getLengthSq();
        }
        
        inline bool isBetweenPoints(const vector3d<T> &Begin, const vector3d<T> &End) const
        {
            const T Temp = (End - Begin).getLengthSq();
            return getDistanceFromSq(Begin) <= Temp && getDistanceFromSq(End) <= Temp;
        }
        
        //! \deprecated This should not be a member function!
        inline bool isPointInsideSphere(const vector3d<T> &Center, const f32 Radius) const
        {
            return math::pow2(X - Center.X) + math::pow2(Y - Center.Y) + math::pow2(Z - Center.Z) < math::pow2(Radius);
        }
        
        inline vector3d<T> getInterpolatedQuadratic(const vector3d<T> &v2, const vector3d<T> &v3, const T d) const
        {
            const T inv = static_cast<T>(1.0) - d;
            const T mul0 = inv * inv;
            const T mul1 = static_cast<T>(2.0) * d * inv;
            const T mul2 = d * d;
            
            return vector3d<T>(
                X * mul0 + v2.X * mul1 + v3.X * mul2,
                Y * mul0 + v2.Y * mul1 + v3.Y * mul2,
                Z * mul0 + v2.Z * mul1 + v3.Z * mul2
            );
        }
        
        inline vector3d<T> getRotatedAxis(const T &Angle, vector3d<T> Axis) const
        {
            if (Angle == T(0))
                return *this;
            
            Axis.normalize();
            
            vector3d<T> rotMatrixRow1, rotMatrixRow2, rotMatrixRow3;
            
            T sinAngle      = sin(Angle*math::DEG);
            T cosAngle      = cos(Angle*math::DEG);
            T cosAngleInv   = 1.0f - cosAngle;
            
            rotMatrixRow1.X = Axis.X*Axis.X + cosAngle*(1.0f - Axis.X*Axis.X);
            rotMatrixRow1.Y = Axis.X*Axis.Y*cosAngleInv - sinAngle*Axis.Z;
            rotMatrixRow1.Z = Axis.X*Axis.Z*cosAngleInv + sinAngle*Axis.Y;
            
            rotMatrixRow2.X = Axis.X*Axis.Y*cosAngleInv + sinAngle*Axis.Z;
            rotMatrixRow2.Y = Axis.Y*Axis.Y + cosAngle*(1.0f - Axis.Y*Axis.Y);
            rotMatrixRow2.Z = Axis.Y*Axis.Z*cosAngleInv - sinAngle*Axis.X;
            
            rotMatrixRow3.X = Axis.X*Axis.Z*cosAngleInv - sinAngle*Axis.Y;
            rotMatrixRow3.Y = Axis.Y*Axis.Z*cosAngleInv + sinAngle*Axis.X;
            rotMatrixRow3.Z = Axis.Z*Axis.Z + cosAngle*(1.0f - Axis.Z*Axis.Z);
            
            return vector3d<T>(
                dot(rotMatrixRow1),
                dot(rotMatrixRow2),
                dot(rotMatrixRow3)
            );
        }
        
        //! Returns the direction type of the dominant axis.
        inline EAxisTypes getAxisType() const
        {
            const dim::vector3d<T> AbsDir(getAbs());
            
            if (AbsDir.X >= AbsDir.Y && AbsDir.X >= AbsDir.Z)
                return (X > 0 ? AXIS_X_POSITIVE : AXIS_X_NEGATIVE);
            else if (AbsDir.Y >= AbsDir.X && AbsDir.Y >= AbsDir.Z)
                return (Y > 0 ? AXIS_Y_POSITIVE : AXIS_Y_NEGATIVE);
            
            return (Z > 0 ? AXIS_Z_POSITIVE : AXIS_Z_NEGATIVE);
        }
        
        //! Returns a normal vector to this vector.
        inline vector3d<T> getNormal() const
        {
            if (X > Y && X > Z)
                return vector3d<T>(Y, -X, 0);
            else if (Y > X && Y > Z)
                return vector3d<T>(0, Z, -Y);
            return vector3d<T>(-Z, 0, X);
        }
        
        //! Returns the smalest vector component.
        inline T getMin() const
        {
            if (X <= Y && X <= Z) return X;
            if (Y <= X && Y <= Z) return Y;
            return Z;
        }
        //! Returns the greatest vector component.
        inline T getMax() const
        {
            if (X >= Y && X >= Z) return X;
            if (Y >= X && Y >= Z) return Y;
            return Z;
        }
        
        //! Returns the volume of the bounding box clamped by this vector (X*Y*Z).
        inline T getVolume() const
        {
            return X*Y*Z;
        }
        
        /**
        Just returns this vector. This is only required that this call can be used for several templates.
        Write your own vertex class for example and add this function so that it can be used
        for polygon clipping as well.
        Some templates expect a class with this function (e.g. 'math::CollisionLibrary::clipPolygon').
        \see CollisionLibrary::clipPolygon
        */
        inline vector3d<T> getCoord() const
        {
            return *this;
        }
        
        template <typename B> inline vector3d<B> cast() const
        {
            return vector3d<B>(static_cast<B>(X), static_cast<B>(Y), static_cast<B>(Z));
        }
        
        static inline bool isPointOnSameSide(
            const vector3d<T> &P1, const vector3d<T> &P2, const vector3d<T> &A, const vector3d<T> &B)
        {
            vector3d<T> Difference(B - A);
            vector3d<T> P3 = Difference.cross(P1 - A);
            vector3d<T> P4 = Difference.cross(P2 - A);
            return P3.dot(P4) >= T(0);
        }
        
        /* === Members === */
        
        T X, Y, Z;
        
};

typedef vector3d<s32> vector3di;
typedef vector3d<f32> vector3df;


/*
 * vector4d class
 */

//! \todo Write this in an own class and don't inherit from "vector3d".
template <typename T> class vector4d : public vector3d<T>
{
    
    public:
        
        vector4d() : vector3d<T>(), W(1)
        {
        }
        vector4d(T Size) : vector3d<T>(Size), W(1)
        {
        }
        vector4d(T VecX, T VecY, T VecZ, T VecW = static_cast<T>(1)) : vector3d<T>(VecX, VecY, VecZ), W(VecW)
        {
        }
        vector4d(const vector3d<T> &Other, T VecW = static_cast<T>(1)) : vector3d<T>(Other.X, Other.Y, Other.Z), W(VecW)
        {
        }
        vector4d(const vector4d<T> &Other) : vector3d<T>(Other.X, Other.Y, Other.Z), W(Other.W)
        {
        }
        ~vector4d()
        {
        }
        
        /* Members */
        
        T W;
        
};

template <typename T> vector3d<T>::vector3d(const vector4d<T> &Other) :
    X(Other.X),
    Y(Other.Y),
    Z(Other.Z)
{
}

typedef vector4d<s32> vector4di;
typedef vector4d<f32> vector4df;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
