/*
 * Convex polyhedron header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_CONVEXPOLYHEDRON_H__
#define __SP_CONVEXPOLYHEDRON_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionMatrix4.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionPlane3D.hpp"
#include "Base/spDimensionAABB.hpp"


namespace sp
{
namespace scene
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
        ConvexPolyhedron(const ConvexPolyhedron<T, PlaneCount> &Other)
        {
            for (s32 i = 0; i < PlaneCount; ++i)
                Planes_[i] = Other.Planes_[i];
        }
        virtual ~ConvexPolyhedron()
        {
        }
        
        /* === Functions === */
        
        //! Returns true if the specified 3D point is inside the polyhedron.
        bool isPointInside(const dim::vector3d<T> &Point, const T Radius = T(0)) const
        {
            for (s32 i = 0; i < PlaneCount; ++i)
            {
                if (Planes_[i].getPointDistance(Point) > Radius)
                    return false;
            }
            return true;
        }
        
        /**
        Returns true if the specified box (with inverse transformation matrix) is inside the polyhedron.
        It is not required that the box is completely inside the polyhedron.
        */
        bool isBoundBoxInsideInv(const dim::aabbox3d<T> &Box, const dim::matrix4<T> &InvMatrix) const
        {
            for (s32 i = 0; i < PlaneCount; ++i)
            {
                if ((InvMatrix * Planes_[i]).getAABBoxRelation(Box) == dim::PLANE_RELATION_FRONT)
                    return false;
            }
            return true;
        }
        
        /**
        Returns true if the specified box is inside the polyhedron.
        It is not required that the box is completely inside the polyhedron.
        */
        bool isBoundBoxInside(const dim::aabbox3d<T> &Box, const dim::matrix4<T> &Matrix) const
        {
            return isBoundBoxInsideInv(Box, Matrix.getInverse());
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
                Planes_[i].normalize();
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
        
        inline void setPlane(u32 Index, const dim::plane3d<T> &Plane)
        {
            if (Index < PlaneCount)
                Planes_[Index] = Plane;
        }
        
    protected:
        
        /* === Members === */
        
        dim::plane3d<T> Planes_[PlaneCount];
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
