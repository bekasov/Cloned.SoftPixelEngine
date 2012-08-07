/*
 * Convex polyhedron header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_CONVEXPOLYHEDRON_H__
#define __SP_MATH_CONVEXPOLYHEDRON_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionMatrix4.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionPlane3D.hpp"
#include "Base/spDimensionAABB.hpp"


namespace sp
{
namespace math
{


/**
A convex polyhedron represents a set of planes (or rather half-spaces)
where the plane-normals must point out of the polyhedron.
*/
template <typename T, s32 PlaneCount> class ConvexPolyhedron
{
    
    public:
        
        ConvexPolyhedron()
        {
        }
        virtual ~ConvexPolyhedron()
        {
        }
        
        /* === Functions === */
        
        //! Returns true if the specified 3D point is inside the frustum.
        bool isPointInside(const dim::vector3d<T> &Point, const T Radius = 0.0f) const
        {
            for (s32 i = 0; i < PlaneCount; ++i)
            {
                if (Planes_[i].getPointDistance(Point) > Radius)
                    return false;
            }
            return true;
        }
        
        //! Returns true if the specified box is inside the frustum.
        bool isBoundBoxInside(const dim::aabbox3d<T> &Box, const dim::matrix4<T> &Matrix) const
        {
            const dim::matrix4<T> InvMatrix(Matrix.getInverse());
            
            for (s32 i = 0; i < PlaneCount; ++i)
            {
                if ((InvMatrix * Planes_[i]).getAABBoxRelation(Box) == dim::PLANE_RELATION_FRONT)
                    return false;
            }
            return true;
        }
        
        /**
        Makes an intersection tests with the specified line and this convex polyhedron.
        \param Line: Specifies the line which is to be tested against this convex polyhedron.
        \param Intersection: Specifies the reference where the resulting intersection line will be stored.
        \return True if the intersection test succeeded. Otherwise false.
        */
        bool checkLineIntersection(const dim::line3d<T> &Line, dim::line3d<T> &Intersection) const
        {
            const dim::vector3d<T> Direction(Line.getDirection());
            
            T IntervalFirst = T(0);
            T IntervalLast  = T(1);
            
            /* Test line against each plane */
            for (s32 i = 0; i < PlaneCount; ++i)
            {
                T Denom = Planes_[i].Normal.dot(Direction);
                T Dist  = Planes_[i].Distance - Planes_[i].Normal.dot(Line.Start);
                
                /* Check if line runs parallel to the plane */
                if (Denom == T(0))
                {
                    if (Dist > T(0))
                        return false;
                }
                else
                {
                    T t = Dist / Denom;
                    
                    if (Denom < T(0))
                    {
                        if (t > IntervalFirst)
                            IntervalFirst = t;
                    }
                    else
                    {
                        if (t < IntervalLast)
                            IntervalLast = t;
                    }
                    
                    /* Exit with no intersection if intersection becomes empty */
                    if (IntervalFirst > IntervalLast)
                        return false;
                }
            }
            
            /* Return the intersection */
            Intersection.Start  = Line.Start + Direction * IntervalFirst;
            Intersection.End    = Line.Start + Direction * IntervalLast;
            
            return true;
        }
        
        //! Normalizes all planes.
        void normalize()
        {
            for (s32 i = 0; i < PlaneCount; ++i)
            {
                const T Len = 1.0f / Planes_[i].Normal.getLength();
                
                Planes_[i].Normal   *= Len;
                Planes_[i].Distance *= Len;
            }
        }
        
        /* === Inline functions === */
        
        //! Returns a reference of the specified plane.
        inline const dim::plane3d<T>& getPlane(u32 Index) const
        {
            return Planes_[Index];
        }
        inline dim::plane3d<T>& getPlane(u32 Index)
        {
            return Planes_[Index];
        }
        
    protected:
        
        /* === Members === */
        
        dim::plane3d<T> Planes_[PlaneCount];
        
};


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
