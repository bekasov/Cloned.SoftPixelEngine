/*
 * Rect 2D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_RECT2D_H__
#define __SP_DIMENSION_RECT2D_H__


#include "Base/spBaseTypes.hpp"
#include "Base/spDimensionPoint2D.hpp"
#include "Base/spDimensionSize2D.hpp"
#include "Base/spMathCore.hpp"


namespace sp
{
namespace dim
{


template <typename T> class rect2d
{
    
    public:
        
        rect2d() :
            Left    (0),
            Right   (0),
            Top     (0),
            Bottom  (0)
        {
        }
        rect2d(const T &X, const T &Y) :
            Left    (X),
            Right   (X),
            Top     (Y),
            Bottom  (Y)
        {
        }
        rect2d(const T &NewLeft, const T &NewTop, const T &NewRight, const T &NewBottom) :
            Left    (NewLeft    ),
            Right   (NewRight   ),
            Top     (NewTop     ),
            Bottom  (NewBottom  )
        {
        }
        rect2d(const rect2d<T> &Other) :
            Left    (Other.Left     ),
            Right   (Other.Right    ),
            Top     (Other.Top      ),
            Bottom  (Other.Bottom   )
        {
        }
        ~rect2d()
        {
        }
        
        /* === Operators === */
        
        inline bool operator == (const rect2d<T> &Other) const
        {
            return Left == Other.Left && Top == Other.Top && Right == Other.Right && Bottom == Other.Bottom;
        }
        inline bool operator != (const rect2d<T> &Other) const
        {
            return Left != Other.Left && Top != Other.Top && Right != Other.Right && Bottom != Other.Bottom;
        }
        
        inline rect2d<T> operator + (const rect2d<T> &Other) const
        {
            return rect2d<T>(Left + Other.Left, Top + Other.Top, Right + Other.Right, Bottom + Other.Bottom);
        }
        inline rect2d<T>& operator += (const rect2d<T> &Other)
        {
            Left += Other.Left; Top += Other.Top; Right += Other.Right; Bottom += Other.Bottom; return *this;
        }
        
        inline rect2d<T> operator - (const rect2d<T> &Other) const
        {
            return rect2d<T>(Left - Other.Left, Top - Other.Top, Right - Other.Right, Bottom - Other.Bottom);
        }
        inline rect2d<T>& operator -= (const rect2d<T> &Other)
        {
            Left -= Other.Left; Top -= Other.Top; Right -= Other.Right; Bottom -= Other.Bottom; return *this;
        }
        
        inline rect2d<T> operator / (const rect2d<T> &Other) const
        {
            return rect2d<T>(Left / Other.Left, Top / Other.Top, Right / Other.Right, Bottom / Other.Bottom);
        }
        inline rect2d<T>& operator /= (const rect2d<T> &Other)
        {
            Left /= Other.Left; Top /= Other.Top; Right /= Other.Right; Bottom /= Other.Bottom; return *this;
        }
        
        inline rect2d<T> operator * (const rect2d<T> &Other) const
        {
            return rect2d<T>(Left * Other.Left, Top * Other.Top, Right * Other.Right, Bottom * Other.Bottom);
        }
        inline rect2d<T>& operator *= (const rect2d<T> &Other)
        {
            Left *= Other.Left; Top *= Other.Top; Right *= Other.Right; Bottom *= Other.Bottom; return *this;
        }
        
        inline rect2d<T> operator - () const
        {
            return rect2d<T>(-Left, -Top, -Right, -Bottom);
        }
        
        /* === Functions === */
        
        //! Sets the rectangle's size.
        inline void setSize(const size2d<T> &Size)
        {
            Right   = Left  + Size.Width;
            Bottom  = Top   + Size.Height;
        }
        //! Returns the rectangle's size.
        inline size2d<T> getSize() const
        {
            return size2d<T>(Right - Left, Bottom - Top);
        }
        
        //! Sets the rectangle's center point.
        inline void setCenter(const vector2d<T> &Center)
        {
            *this += rect2d<T>(Center - getCenter());
        }
        //! Returns the rectangle's center point.
        inline vector2d<T> getCenter() const
        {
            return vector2d<T>((Right + Left)/T(2), (Bottom + Top)/T(2));
        }
        
        //! Sets the left-top point.
        inline void setLTPoint(const vector2d<T> &Point)
        {
            Left    = Point.X;
            Top     = Point.Y;
        }
        //! Returns the left-top point.
        inline vector2d<T> getLTPoint() const
        {
            return vector2d<T>(Left, Top);
        }
        
        //! Sets the right-bottom point.
        inline void setRBPoint(const vector2d<T> &Point)
        {
            Right   = Point.X;
            Bottom  = Point.Y;
        }
        //! Returns the right-bottom point.
        inline vector2d<T> getRBPoint() const
        {
            return vector2d<T>(Right, Bottom);
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
        //! Returns true if all four components are zero.
        inline bool empty() const
        {
            return Left == Right == Top == Bottom == T(0);
        }
        //! Returns true if this is a valid rectangle, i.e. width and height are greater than or equal to zero.
        inline bool valid() const
        {
            return getWidth() >= T(0) && getHeight() >= T(0);
        }
        
        //! Ensures that width and height are greater than or equal to zero.
        inline rect2d<T>& repair()
        {
            if (Left > Right)
                std::swap(Left, Right);
            if (Top > Bottom)
                std::swap(Top, Bottom);
            return *this;
        }
        
        //! Returns true if the specified point overlaps with this rectangle.
        inline bool overlap(const vector2d<T> &Point) const
        {
            return (Point.X >= Left && Point.X < Right && Point.Y >= Top && Point.Y < Bottom);
        }
        //! Returns true if the specified rectangle overlaps with this rectangle.
        inline bool overlap(const rect2d<T> &Other) const
        {
            return (Bottom > Other.Top && Top < Other.Bottom && Right > Other.Left && Left < Other.Right);
        }
        
        template <typename B> inline rect2d<B> cast() const
        {
            return rect2d<B>(static_cast<B>(Left), static_cast<B>(Top), static_cast<B>(Right), static_cast<B>(Bottom));
        }
        
        /* === Members === */
        
        T Left, Right, Top, Bottom;
        
};

typedef rect2d<s32> rect2di;
typedef rect2d<f32> rect2df;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
