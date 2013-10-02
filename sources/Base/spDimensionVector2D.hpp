/*
 * Vector 2D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_VECTOR2D_H__
#define __SP_DIMENSION_VECTOR2D_H__


#include "Base/spBaseTypes.hpp"
#include "Base/spVectorArithmetic.hpp"


namespace sp
{
namespace dim
{


template <typename T> class vector3d;

/**
Vector 2D class (X, Y).
\ingroup group_data_types
*/
template <typename T> class vector2d
{
    
    public:
        
        //! Number of components (2). This is used for vector arithmetic templates.
        static const size_t NUM = 2;
        
        vector2d() :
            X(0),
            Y(0)
        {
        }
        vector2d(const T &Size) :
            X(Size),
            Y(Size)
        {
        }
        vector2d(const T &PointX, const T &PointY) :
            X(PointX),
            Y(PointY)
        {
        }
        vector2d(const vector2d<T> &Other) :
            X(Other.X),
            Y(Other.Y)
        {
        }
        vector2d(const vector3d<T> &Other);
        ~vector2d()
        {
        }
        
        /* === Operators - addition, subtraction, division, multiplication === */
        
        inline vector2d<T> operator + (const vector2d<T> &Other) const
        {
            return vector2d<T>(X + Other.X, Y + Other.Y);
        }
        inline vector2d<T>& operator += (const vector2d<T> &Other)
        {
            X += Other.X; Y += Other.Y; return *this;
        }
        
        inline vector2d<T> operator - (const vector2d<T> &Other) const
        {
            return vector2d<T>(X - Other.X, Y - Other.Y);
        }
        inline vector2d<T>& operator -= (const vector2d<T> &Other)
        {
            X -= Other.X; Y -= Other.Y; return *this;
        }
        
        inline vector2d<T> operator / (const vector2d<T> &Other) const
        {
            return vector2d<T>(X / Other.X, Y / Other.Y);
        }
        inline vector2d<T>& operator /= (const vector2d<T> &Other)
        {
            X /= Other.X; Y /= Other.Y; return *this;
        }
        
        inline vector2d<T> operator * (const vector2d<T> &Other) const
        {
            return vector2d<T>(X * Other.X, Y * Other.Y);
        }
        inline vector2d<T>& operator *= (const vector2d<T> &Other)
        {
            X *= Other.X; Y *= Other.Y; return *this;
        }
        
        inline vector2d<T> operator / (T Size) const
        {
            return vector2d<T>(X / Size, Y / Size);
        }
        inline vector2d<T>& operator /= (T Size)
        {
            X /= Size; Y /= Size; return *this;
        }
        
        inline vector2d<T> operator * (T Size) const
        {
            return vector2d<T>(X * Size, Y * Size);
        }
        inline vector2d<T>& operator *= (T Size)
        {
            X *= Size; Y *= Size; return *this;
        }
        
        inline vector2d<T> operator - () const
        {
            return vector2d<T>(-X, -Y);
        }
        
        /* === Additional operators === */
        
        inline const T& operator [] (size_t i) const
        {
            return *(&X + i);
        }
        inline T& operator [] (size_t i)
        {
            return *(&X + i);
        }
        
        /* === Static functions === */
        
        //! \deprecated This should not be a member function.
        static inline void make3DProjection(T &NewX, T &NewY, s32 ScreenWidth, s32 ScreenHeight)
        {
            NewX = static_cast<T>( static_cast<f32>(  NewX - ScreenWidth /2) / (ScreenWidth /2) );
            NewY = static_cast<T>( static_cast<f32>(- NewY + ScreenHeight/2) / (ScreenHeight/2) );
        }
        
        /* === Extra functions === */
        
        inline T dot(const vector2d<T> &Other) const
        {
            return dim::dot(*this, Other);
        }
        
        inline T getLength() const
        {
            return dim::length(*this);
        }
        inline T getLengthSq() const
        {
            return dim::dot(*this, *this);
        }
        
        //! \deprecated This should not be a member function.
        inline void make3DProjection(s32 ScreenWidth, s32 ScreenHeight) // (for 2D graphics)
        {
            X = static_cast<T>( static_cast<f32>( X - ScreenWidth /2) / (ScreenWidth /2) );
            Y = static_cast<T>( static_cast<f32>(-Y + ScreenHeight/2) / (ScreenHeight/2) );
        }
        
        //! \deprecated This should not be a member function.
        inline void make3DProjectionOrigin(s32 ScreenWidth, s32 ScreenHeight)
        {
            X = static_cast<T>( static_cast<f32>(X) / (ScreenWidth /2) );
            Y = static_cast<T>( static_cast<f32>(Y) / (ScreenHeight/2) );
        }
        
        //! \deprecated This should not be a member function.
        inline void make3DFrustum(f32 Width, f32 Height)
        {
            const f32 aspect = Width / Height;
            const f32 stdasp = 4.0f / 3.0f;
            
            X = static_cast<T>( f32( X - Width /2) / (Width/2) * aspect / stdasp );
            Y = static_cast<T>( f32(-Y + Height/2) / (Width/2) * aspect / stdasp );
        }
        
        inline vector2d<T>& setAbs()
        {
            X = X > 0 ? X : -X;
            Y = Y > 0 ? Y : -Y;
            return *this;
        }
        inline vector2d<T> getAbs() const
        {
            return vector2d<T>(
                X > 0 ? X : -X,
                Y > 0 ? Y : -Y
            );
        }
        
        inline vector2d<T>& normalize()
        {
            dim::normalize(*this);
            return *this;
        }
        
        //! \deprecated
        inline void set(T NewX, T NewY)
        {
            X = NewX; Y = NewY;
        }
        //! \deprecated
        inline void get(T &NewX, T &NewY) const
        {
            NewX = X; NewY = Y;
        }
        
        inline void setLength(T Length)
        {
            normalize();
            *this *= Length;
        }
        
        //! \deprecated This should not be a member function.
        inline vector2d<T>& getCircleCollision(f32 ThisRadius, vector2d<T> &OtherPoint, f32 OtherRadius)
        {
            f32 Distance = sqrt( (OtherPoint.X - X)*(OtherPoint.X - X) + (OtherPoint.Y - Y)*(OtherPoint.Y - Y) );
            f32 Degree = asin( (OtherPoint.X - X) / Distance )*180.0f/M_PI;
            
            if (Y < OtherPoint.Y)
                Degree = 180 - Degree;
            
            if (Distance < ThisRadius + OtherRadius) {
                OtherPoint.X = X + sin(Degree*M_PI/180) * (ThisRadius + OtherRadius);
                OtherPoint.Y = Y + cos(Degree*M_PI/180) * (ThisRadius + OtherRadius);
            }
            
            return OtherPoint;
        }
        
        inline bool isPointInsideCircle(const vector2d<T> &Center, const f32 Radius) const
        {
            return (X - Center.X)*(X - Center.X) + (Y - Center.Y)*(Y - Center.Y) < Radius*Radius;
        }

        inline T getMin() const
        {
            return (X <= Y) ? X : Y;
        }
        inline T getMax() const
        {
            return (X >= Y) ? X : Y;
        }
        
        template <typename B> inline vector2d<B> cast() const
        {
            return vector2d<B>(static_cast<B>(X), static_cast<B>(Y));
        }
        
        /* === Members === */
        
        T X, Y;
        
};

typedef vector2d<s32> vector2di;
typedef vector2d<f32> vector2df;

typedef vector2d<s32> point2di;
typedef vector2d<f32> point2df;


/* === Relation operators === */

template <typename T> inline bool operator == (const vector2d<T> &A, const vector2d<T> &B)
{
    return compareVecEqual(A, B);
}
template <typename T> inline bool operator != (const vector2d<T> &A, const vector2d<T> &B)
{
    return compareVecNotEqual(A, B);
}

template <typename T> inline bool operator < (const vector2d<T> &A, const vector2d<T> &B)
{
    return compareVecLessThan(A, B);
}
template <typename T> inline bool operator > (const vector2d<T> &A, const vector2d<T> &B)
{
    return compareVecGreaterThan(A, B);
}

template <typename T> inline bool operator <= (const vector2d<T> &A, const vector2d<T> &B)
{
    return compareVecLessThanOrEqual(A, B);
}
template <typename T> inline bool operator >= (const vector2d<T> &A, const vector2d<T> &B)
{
    return compareVecGreaterThanOrEqual(A, B);
}


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
