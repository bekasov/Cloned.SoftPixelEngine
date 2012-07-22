/*
 * Rect 2D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_RECT2D_H__
#define __SP_DIMENSION_RECT2D_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionSize2D.hpp"


namespace sp
{
namespace dim
{


template <typename T> class rect2d
{
    
    public:
        
        rect2d()
            : Left(0), Right(0), Top(0), Bottom(0)
        {
        }
        rect2d(T NewLeft, T NewTop)
            : Left(NewLeft), Right(0), Top(NewTop), Bottom(0)
        {
        }
        rect2d(T NewLeft, T NewTop, T NewRight, T NewBottom)
            : Left(NewLeft), Right(NewRight), Top(NewTop), Bottom(NewBottom)
        {
        }
        rect2d(const rect2d<T> &other)
            : Left(other.Left), Right(other.Right), Top(other.Top), Bottom(other.Bottom)
        {
        }
        ~rect2d()
        {
        }
        
        /* Operators */
        
        inline bool operator == (const rect2d<T> &other) const
        {
            return Left == other.Left && Top == other.Top && Right == other.Right && Bottom == other.Bottom;
        }
        inline bool operator != (const rect2d<T> &other) const
        {
            return Left != other.Left && Top != other.Top && Right != other.Right && Bottom != other.Bottom;
        }
        
        inline rect2d<T> operator + (const rect2d<T> &other) const
        {
            return rect2d<T>(Left + other.Left, Top + other.Top, Right + other.Right, Bottom + other.Bottom);
        }
        inline rect2d<T>& operator += (const rect2d<T> &other)
        {
            Left += other.Left; Top += other.Top; Right += other.Right; Bottom += other.Bottom; return *this;
        }
        
        inline rect2d<T> operator - (const rect2d<T> &other) const
        {
            return rect2d<T>(Left - other.Left, Top - other.Top, Right - other.Right, Bottom - other.Bottom);
        }
        inline rect2d<T>& operator -= (const rect2d<T> &other)
        {
            Left -= other.Left; Top -= other.Top; Right -= other.Right; Bottom -= other.Bottom; return *this;
        }
        
        inline rect2d<T> operator / (const rect2d<T> &other) const
        {
            return rect2d<T>(Left / other.Left, Top / other.Top, Right / other.Right, Bottom / other.Bottom);
        }
        inline rect2d<T>& operator /= (const rect2d<T> &other)
        {
            Left /= other.Left; Top /= other.Top; Right /= other.Right; Bottom /= other.Bottom; return *this;
        }
        
        inline rect2d<T> operator * (const rect2d<T> &other) const
        {
            return rect2d<T>(Left * other.Left, Top * other.Top, Right * other.Right, Bottom * other.Bottom);
        }
        inline rect2d<T>& operator *= (const rect2d<T> &other)
        {
            Left *= other.Left; Top *= other.Top; Right *= other.Right; Bottom *= other.Bottom; return *this;
        }
        
        inline rect2d<T> operator - () const
        {
            return rect2d<T>(-Left, -Top, -Right, -Bottom);
        }
        
        /* Extra functions */
        
        inline size2d<T> getSize() const
        {
            return size2d<T>(Right - Left, Bottom - Top);
        }
        inline point2d<T> getCenter() const
        {
            return point2d<T>((Right + Left)/2, (Bottom + Top)/2);
        }
        
        //! Returns the rectangle's width (Right - Left).
        inline T getWidth() const
        {
            return Right - Left;
        }
        //! Returns the rectangle's height (Bottom - Top).
        inline T getHeight() const
        {
            return Bottom - Top;
        }
        //! Returns true if all four components are 0.
        inline bool empty() const
        {
            return Left == Right == Top == Bottom == 0;
        }
        //! Returns true if this is a valid rectangle.
        inline bool valid() const
        {
            return getWidth() >= 0 && getHeight() >= 0;
        }
        
        inline void repair()
        {
            if (Right < Left)
            {
                T tmp   = Right;
                Right   = Left;
                Left    = tmp;
            }
            if (Bottom < Top)
            {
                T tmp   = Bottom;
                Bottom  = Top;
                Top     = tmp;
            }
        }
        
        inline bool isPointCollided(const point2d<T> &Point) const
        {
            return (Point.X >= Left && Point.X < Right && Point.Y >= Top && Point.Y < Bottom);
        }
        inline bool isRectCollided(const rect2d<T> &other) const
        {
            return (Bottom > other.Top && Top < other.Bottom && Right > other.Left && Left < other.Right);
        }
        
        template <typename B> inline rect2d<B> cast() const
        {
            return rect2d<B>(static_cast<B>(Left), static_cast<B>(Right), static_cast<B>(Top), static_cast<B>(Bottom));
        }
        
        /* Members */
        
        T Left, Right, Top, Bottom;
        
};

typedef rect2d<s32> rect2di;
typedef rect2d<f32> rect2df;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
