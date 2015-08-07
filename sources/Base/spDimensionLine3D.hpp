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
#include "Base/spDimensionVector2D.hpp"


namespace sp
{
namespace dim
{


//! Relations between closest point on line to point.
enum ELinePointRelations
{
    LINE_RELATION_START,      //!< The closest point on line to point is the line start vector.
    LINE_RELATION_END,        //!< The closest point on line to point is the line end vector.
    LINE_RELATION_BETWEEN,    //!< The closest point on line to point is between the line start- and end vectors.
};


/**
Line kd class (Start, End). Used for intersection tests or 3D drawing information.
This can be used for 2D or 3D lines.
\tparam T Specifies the data type.
\tparam Vec Specifies the vector type (should be vector2d, vector3d or vector4d).
\ingroup group_data_types
*/
template <typename T, template <typename> class Vec> class linekd
{
        
        typedef Vec<T> VecT;
        typedef linekd<T, Vec> L;
        
    public:
        
        linekd()
        {
        }
        linekd(const VecT &RayStart, const VecT &RayEnd) :
            Start   (RayStart   ),
            End     (RayEnd     )
        {
        }
        linekd(const L &Other) :
            Start   (Other.Start),
            End     (Other.End  )
        {
        }
        virtual ~linekd()
        {
        }
        
        /* === Operators === */
        
        inline bool operator == (const L &Other)
        {
            return Start == Other.Start && End == Other.End;
        }
        inline bool operator != (const L &Other)
        {
            return Start != Other.Start && End != Other.End;
        }
        
        inline L operator + (const L &Other) const
        {
            return L(Start + Other.Start, End + Other.End);
        }
        inline L& operator += (const L &Other)
        {
            Start += Other.Start; End += Other.End; return *this;
        }
        
        inline L operator - (const L &Other) const
        {
            return L(Start - Other.Start, End - Other.End);
        }
        inline L& operator -= (const L &Other)
        {
            Start -= Other.Start; End -= Other.End; return *this;
        }
        
        inline L operator / (const L &Other) const
        {
            return L(Start / Other.Start, End / Other.End);
        }
        inline L& operator /= (const L &Other)
        {
            Start /= Other.Start; End /= Other.End; return *this;
        }
        
        inline L operator * (const L &Other) const
        {
            return L(Start * Other.Start, End * Other.End);
        }
        inline L& operator *= (const L &Other)
        {
            Start *= Other.Start; End *= Other.End; return *this;
        }
        
        inline L operator - () const
        {
            return L(-Start, -End);
        }

        /* === Functions === */
        
        //! Returns the closest point on the line between the specfied point and the line.
        ELinePointRelations getClosestPointStraight(const VecT &Point, VecT &ClosestPoint) const
        {
            VecT Pos(Point - Start);
            VecT Dir(End - Start);
            
            T Len(Dir.getLength());
            Dir *= (T(1) / Len);
            T Factor(Dir.dot(Pos));
            
            Dir *= Factor;
            ClosestPoint = Start + Dir;
            
            if (Factor < T(0))
                return LINE_RELATION_START;
            if (Factor > Len)
                return LINE_RELATION_END;
            
            return LINE_RELATION_BETWEEN;
        }
        
        //! Returns the closest point on the line between the specfied point and the line.
        ELinePointRelations getClosestPoint(const VecT &Point, VecT &ClosestPoint) const
        {
            VecT Pos(Point - Start);
            VecT Dir(End - Start);
            
            T Len(Dir.getLength());
            Dir *= (T(1) / Len);
            T Factor(Dir.dot(Pos));
            
            if (Factor < T(0))
            {
                ClosestPoint = Start;
                return LINE_RELATION_START;
            }
            if (Factor > Len)
            {
                ClosestPoint = End;
                return LINE_RELATION_END;
            }
            
            Dir *= Factor;
            
            ClosestPoint = Start + Dir;
            return LINE_RELATION_BETWEEN;
        }
        
        /* === Inline functions === */
        
        //! Returns the line's center ((Start + End) / 2).
        inline VecT getCenter() const
        {
            return (Start + End) / 2;
        }
        
        //! Returns the line's direction (End - Start).
        inline VecT getDirection() const
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
        inline bool isPointInside(const VecT &Point) const
        {
            return Point.isBetweenPoints(Start, End);
        }
        
        //! Returns the closest point on the line between the specfied point and the line.
        inline VecT getClosestPointStraight(const VecT &Point) const
        {
            VecT ClosestPoint;
            getClosestPointStraight(Point, ClosestPoint);
            return ClosestPoint;
        }
        
        //! Returns the closest point on the line between the specfied point and the line.
        inline VecT getClosestPoint(const VecT &Point) const
        {
            VecT ClosestPoint;
            getClosestPoint(Point, ClosestPoint);
            return ClosestPoint;
        }
        
        //! Returns the distance between the line and the specified point.
        inline T getPointDistance(const VecT &Point) const
        {
            return (getClosestPoint(Point) - Point).getLength();
        }
        
        //! Returns the squared distance between the line and the specified point.
        inline T getPointDistanceSq(const VecT &Point) const
        {
            return (getClosestPoint(Point) - Point).getLengthSq();
        }
        
        /* === Members === */
        
        VecT Start, End;
        
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

template <typename T> class line2d : public linekd<T, vector2d>
{
    
    public:
        
        line2d() :
            linekd<T, vector2d>()
        {
        }
        line2d(const vector2d<T> &Start, const vector2d<T> &End) :
            linekd<T, vector2d>(Start, End)
        {
        }
        line2d(const linekd<T, vector2d> &Other) :
            linekd<T, vector2d>(Other)
        {
        }
        ~line2d()
        {
        }
        
};

typedef linekd<s32, vector3d> line3di;
typedef linekd<f32, vector3d> line3df;

typedef linekd<s32, vector2d> line2di;
typedef linekd<f32, vector2d> line2df;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
