/*
 * Vector 4D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_VECTOR4D_H__
#define __SP_DIMENSION_VECTOR4D_H__


#include "Base/spStandard.hpp"
#include "Base/spMathCore.hpp"
#include "Base/spVectorArithmetic.hpp"


namespace sp
{
namespace dim
{


/**
Vector 4D class (X, Y, Z, W).
The most operators work equivalent as for the vector3d class, i.e. the + operator
will only modify X, Y and Z, but not the W component.
\ingroup group_data_types
*/
template <typename T> class vector4d
{
    
    public:
        
        static const u32 NUM = 4;
        
        vector4d() :
            X(0),
            Y(0),
            Z(0),
            W(1)
        {
        }
        vector4d(const T &Size) :
            X(Size  ),
            Y(Size  ),
            Z(Size  ),
            W(1     )
        {
        }
        vector4d(const T &x, const T &y, const T &z) :
            X(x),
            Y(y),
            Z(z),
            W(1)
        {
        }
        vector4d(const T &x, const T &y, const T &z, const T &w) :
            X(x),
            Y(y),
            Z(z),
            W(w)
        {
        }
        vector4d(const vector3d<T> &Other, const T &w = T(1)) :
            X(Other.X   ),
            Y(Other.Y   ),
            Z(Other.Z   ),
            W(w         )
        {
        }
        vector4d(const vector4d<T> &Other) :
            X(Other.X),
            Y(Other.Y),
            Z(Other.Z),
            W(Other.W)
        {
        }
        ~vector4d()
        {
        }
        
        /* === Operators - addition, subtraction, division, multiplication === */
        
        //! Pre-increment operator.
        inline vector4d<T>& operator ++ ()
        {
            ++X; ++Y; ++Z; return *this;
        }
        //! Post-increment operator.
        inline vector4d<T>& operator ++ (int)
        {
            const vector4d<T> Tmp(*this);
            ++*this;
            return Tmp;
        }
        
        //! Pre-decrement operator.
        inline vector4d<T>& operator -- ()
        {
            --X; --Y; --Z; return *this;
        }
        //! Post-decrement operator.
        inline vector4d<T>& operator -- (int)
        {
            const vector4d<T> Tmp(*this);
            --*this;
            return Tmp;
        }
        
        inline vector4d<T> operator + (const vector4d<T> &Other) const
        {
            return vector4d<T>(X + Other.X, Y + Other.Y, Z + Other.Z, W);
        }
        inline vector4d<T>& operator += (const vector4d<T> &Other)
        {
            X += Other.X; Y += Other.Y; Z += Other.Z; return *this;
        }
        
        inline vector4d<T> operator - (const vector4d<T> &Other) const
        {
            return vector4d<T>(X - Other.X, Y - Other.Y, Z - Other.Z);
        }
        inline vector4d<T>& operator -= (const vector4d<T> &Other)
        {
            X -= Other.X; Y -= Other.Y; Z -= Other.Z; return *this;
        }
        
        inline vector4d<T> operator / (const vector4d<T> &Other) const
        {
            return vector4d<T>(X / Other.X, Y / Other.Y, Z / Other.Z, W);
        }
        inline vector4d<T>& operator /= (const vector4d<T> &Other)
        {
            X /= Other.X; Y /= Other.Y; Z /= Other.Z; return *this;
        }
        
        inline vector4d<T> operator * (const vector4d<T> &Other) const
        {
            return vector4d<T>(X * Other.X, Y * Other.Y, Z * Other.Z, W);
        }
        inline vector4d<T>& operator *= (const vector4d<T> &Other)
        {
            X *= Other.X; Y *= Other.Y; Z *= Other.Z; return *this;
        }
        
        inline vector4d<T> operator * (const T &Size) const
        {
            return vector4d<T>(X * Size, Y * Size, Z * Size, W);
        }
        inline vector4d<T>& operator *= (const T &Size)
        {
            X *= Size; Y *= Size; Z *= Size; return *this;
        }
        
        inline vector4d<T> operator / (const T &Size) const
        {
            return vector4d<T>(X / Size, Y / Size, Z / Size, W);
        }
        inline vector4d<T>& operator /= (const T &Size)
        {
            X /= Size; Y /= Size; Z /= Size; return *this;
        }
        
        inline vector4d<T> operator - () const
        {
            return vector4d<T>(-X, -Y, -Z, W);
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
        inline T dot(const vector4d<T> &Other) const
        {
            return dim::dot<3, T>(*this, Other);
        }
        
        //! Returns the cross (or rather vector) product between this and the given vector.
        inline vector4d<T> cross(const vector4d<T> &Other) const
        {
            return dim::cross(*this, Other);
        }
        
        //! Returns the vector's length.
        inline T getLength() const
        {
            return dim::length<3, T>(*this);
        }
        
        //! Returns the square of the vector's length (Can be used for faster comparision between two distances).
        inline T getLengthSq() const
        {
            return dim::dot<3, T>(*this, *this);
        }
        
        //! Returns the angle (in degrees) between this and the given vector.
        inline T getAngle(const vector4d<T> &Other) const
        {
            return dim::angle<3, T>(*this, Other) * T(math::RAD64);
        }
        
        inline vector4d<T>& setInverse()
        {
            X = -X; Y = -Y; Z = -Z; return *this;
        }
        inline vector4d<T> getInverse() const
        {
            return vector4d<T>(-X, -Y, -Z, W);
        }
        
        inline bool empty() const
        {
            return *this == 0;
        }
        
        inline vector4d<T>& setAbs()
        {
            X = std::abs(X);
            Y = std::abs(Y);
            Z = std::abs(Z);
            return *this;
        }
        inline vector4d<T> getAbs() const
        {
            return vector4d<T>(std::abs(X), std::abs(Y), std::abs(Z), W);
        }
        
        //! Normalizes the vectors. After that the vector has the length of 1.
        inline vector4d<T>& normalize()
        {
            dim::normalize<3, T>(*this);
            return *this;
        }
        
        //! Per-component signum function. \see math::sgn
        inline vector4d<T>& sgn()
        {
            math::sgn(X);
            math::sgn(Y);
            math::sgn(Z);
            return *this;
        }
        
        inline vector4d<T>& setLength(const T &Length)
        {
            normalize();
            *this *= Length;
            return *this;
        }
        
        //! Returns a constant pointer to the first element of this vector.
        inline const T* ptr() const
        {
            return &X;
        }
        //! Returns a pointer to the first element of this vector.
        inline T* ptr()
        {
            return &X;
        }
        
        /**
        Returns the direction type of the dominant axis.
        \todo Rename to "getDominantAxis".
        */
        inline EAxisTypes getDominantAxis() const
        {
            return dim::getDominantAxis(ptr());
        }
        
        //! Returns a normal vector to this vector.
        inline vector4d<T> getNormal() const
        {
            if (X > Y && X > Z)
                return vector4d<T>(Y, -X, 0, W);
            else if (Y > X && Y > Z)
                return vector4d<T>(0, Z, -Y, W);
            return vector4d<T>(-Z, 0, X, W);
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
        
        template <typename B> inline vector4d<B> cast() const
        {
            return vector4d<B>(static_cast<B>(X), static_cast<B>(Y), static_cast<B>(Z), static_cast<B>(W));
        }
        
        /* === Members === */
        
        T X, Y, Z, W;
        
};

typedef vector4d<s32> vector4di;
typedef vector4d<f32> vector4df;


/* === Relation operators === */

template <typename T> inline bool operator == (const vector4d<T> &A, const vector4d<T> &B)
{
    return compareVecEqual(A, B);
}
template <typename T> inline bool operator != (const vector4d<T> &A, const vector4d<T> &B)
{
    return compareVecNotEqual(A, B);
}

template <typename T> inline bool operator < (const vector4d<T> &A, const vector4d<T> &B)
{
    return compareVecLessThan(A, B);
}
template <typename T> inline bool operator > (const vector4d<T> &A, const vector4d<T> &B)
{
    return compareVecGreaterThan(A, B);
}

template <typename T> inline bool operator <= (const vector4d<T> &A, const vector4d<T> &B)
{
    return compareVecLessThanOrEqual(A, B);
}
template <typename T> inline bool operator >= (const vector4d<T> &A, const vector4d<T> &B)
{
    return compareVecGreaterThanOrEqual(A, B);
}


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
