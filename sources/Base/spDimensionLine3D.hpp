/*
 * Line 3D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_LINE3D_H__
#define __SP_DIMENSION_LINE3D_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionPoint2D.hpp"


namespace sp
{
namespace dim
{


//! 3D line class for intersection tests or 3D drawing information.
template <typename T, template <typename> class Vec> class linekd
{
        
        typedef Vec<T> V;
        typedef linekd<T, Vec> L;
        
    public:
        
        linekd()
        {
        }
        linekd(const V &RayStart, const V &RayEnd)
            : Start(RayStart), End(RayEnd)
        {
        }
        linekd(const L &other)
            : Start(other.Start), End(other.End)
        {
        }
        virtual ~linekd()
        {
        }
        
        /* === Operators === */
        
        inline bool operator == (const L &other)
        {
            return Start == other.Start && End == other.End;
        }
        inline bool operator != (const L &other)
        {
            return Start != other.Start && End != other.End;
        }
        
        inline L operator + (const L &other) const
        {
            return L(Start + other.Start, End + other.End);
        }
        inline L& operator += (const L &other)
        {
            Start += other.Start; End += other.End; return *this;
        }
        
        inline L operator - (const L &other) const
        {
            return L(Start - other.Start, End - other.End);
        }
        inline L& operator -= (const L &other)
        {
            Start -= other.Start; End -= other.End; return *this;
        }
        
        inline L operator / (const L &other) const
        {
            return L(Start / other.Start, End / other.End);
        }
        inline L& operator /= (const L &other)
        {
            Start /= other.Start; End /= other.End; return *this;
        }
        
        inline L operator * (const L &other) const
        {
            return L(Start * other.Start, End * other.End);
        }
        inline L& operator *= (const L &other)
        {
            Start *= other.Start; End *= other.End; return *this;
        }
        
        inline L operator - () const
        {
            return L(-Start, -End);
        }
        
        /* === Extra functions === */
        
        //! Returns the line's center ((Start + End) / 2).
        inline V getCenter() const
        {
            return (Start + End) / 2;
        }
        
        //! Returns the line's direction (End - Start).
        inline V getDirection() const
        {
            return End - Start;
        }
        
        //! Returns this line as vice-versa variant.
        inline L getViceVersa() const
        {
            return L(End, Start);
        }
        
        //! Returns true if the two lines (understood as axis-aligned-bounding-boxes) are intersecting.
        inline bool checkBoxBoxIntersection(const L &Line) const
        {
            return Start <= Line.End && End >= Line.Start;
        }
        
        //! Returns true if the specified point lies between the line's start and end point.
        inline bool isPointInside(const V &Point) const
        {
            return Point.isBetweenPoints(Start, End);
        }
        
        //! Returns the closest point on the line between the specfied point and the line.
        inline V getClosestPoint(const V &Point) const
        {
            V Pos = Point - Start;
            V Dir = End - Start;
            
            T Len = T(Dir.getLength());
            Dir /= Len;
            T Factor = Dir.dot(Pos);
            
            if (Factor < T(0))
                return Start;
            if (Factor > Len)
                return End;
            
            Dir *= Factor;
            
            return Start + Dir;
        }
        
        //! Returns the distance between the line and the specified point.
        inline T getPointDistance(const V &Point) const
        {
            return (getClosestPoint(Point) - Point).getLength();
        }
        
        /* Members */
        
        V Start, End;
        
};

template <typename T> class line3d : public linekd<T, vector3d>
{
    
    public:
        
        line3d() :
            linekd<T, vector3d>()
        {
        }
        line3d(const vector3d<T> &Start, const vector3d<T> &End) :
            linekd<T, vector3d>(Start, End)
        {
        }
        line3d(const linekd<T, vector3d> &Other) :
            linekd<T, vector3d>(Other)
        {
        }
        ~line3d()
        {
        }
        
};

template <typename T> class line2d : public linekd<T, point2d>
{
    
    public:
        
        line2d() :
            linekd<T, point2d>()
        {
        }
        line2d(const point2d<T> &Start, const point2d<T> &End) :
            linekd<T, point2d>(Start, End)
        {
        }
        line2d(const linekd<T, point2d> &Other) :
            linekd<T, point2d>(Other)
        {
        }
        ~line2d()
        {
        }
        
};

typedef line3d<s32> line3di;
typedef line3d<f32> line3df;

typedef line2d<s32> line2di;
typedef line2d<f32> line2df;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
