/*
 * Cone 3D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_CONE3D_H__
#define __SP_DIMENSION_CONE3D_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionPlane3D.hpp"
#include "Base/spVectorArithmetic.hpp"


namespace sp
{
namespace dim
{


/**
Cone 3D class (Tip, Direction, Height, Radius). A cone is represented by its tip (a 3D vector),
the normalized axis direction (a 3D vector) pointing from its tip to the bottom, its height
and the radius at its bottom.
\since Version 3.3
\ingroup group_data_types
*/
template <typename T> class cone3d
{
    
    public:
        
        cone3d() :
            Height(0),
            Radius(0)
        {
        }
        cone3d(const vector3d<T> &ConeTip, const vector3d<T> &ConeDirection, const T &ConeHeight, const T &ConeRadius) :
            Tip         (ConeTip        ),
            Direction   (ConeDirection  ),
            Height      (ConeHeight     ),
            Radius      (ConeRadius     )
        {
            Direction.normalize();
        }
        cone3d(const cone3d<T> &Other) :
            Tip         (Other.Tip      ),
            Direction   (Other.Direction),
            Height      (Other.Height   ),
            Radius      (Other.Radius   )
        ~cone3d()
        {
        }
        
        /* === Functions === */
        
        /**
        Returns the closest point on the cone's bottom circle to the specified plane.
        \note This is not truely the closest point if the tip is closer to the plane than
        any point on the cone's bottom circle.
        */
        inline vector3d<T> getClosestPoint(const plane3d<T> &Plane) const
        {
            return Tip + Direction * Height + cross(cross(Plane.Normal, Direction), Direction) * Radius;
        }
        
        /**
        Returns true if this cone is in front of the specified plane.
        \todo This has not been tested yet.
        */
        inline bool isConeFrontSide(const plane3d<T> &Plane) const
        {
            return !Plane.isPointFrontSide(Tip) || !Plane.isPointFrontSide(getClosestPoint(Plane));
        }
        
        template <typename B> inline cone3d<B> cast() const
        {
            return cone3d<B>(Tip.cast<B>(), Direction.cast<B>(), static_cast<B>(Height), static_cast<B>(Radius));
        }
        
        /* === Members === */
        
        vector3d<T> Tip;        //!< Cone tip point.
        vector3d<T> Direction;  //!< Axis direction. Must always be normalized!
        T Height;               //!< Cone height.
        T Radius;               //!< Bottom radius.
        
};

typedef cone3d<s32> cone3di;
typedef cone3d<f32> cone3df;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
