/*
 * ViewFrustum header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_VIEWFRUSTUM_H__
#define __SP_MATH_VIEWFRUSTUM_H__


#include "Base/spStandard.hpp"
#include "Base/spMathConvexPolyhedron.hpp"


namespace sp
{
namespace math
{


//! View frustum plane sides
enum EViewFrustumPlanes
{
    VIEWFRUSTUM_NEAR = 0,
    VIEWFRUSTUM_FAR,
    VIEWFRUSTUM_LEFT,
    VIEWFRUSTUM_RIGHT,
    VIEWFRUSTUM_BOTTOM,
    VIEWFRUSTUM_TOP,
    
    VIEWFRUSTUM_PLANE_COUNT,
};


/**
View frustum for Camera objects. Each Camera object has one view frustum which defines the whole area
which is visible for the user with all its near- and far clippings planes etc.
The view frustum can be used for "Frustum Culling" optimization where each object which is outside the view frustum
will not be rendered. This safes a lot of time because the renderer does not need to pass each triangle of the mesh
when it's anyway completly invisible.
\see Node::setBoundingType, Node::setBoundingBox, Node::setBoundingSphere
*/
class SP_EXPORT ViewFrustum : public ConvexPolyhedron<f32, VIEWFRUSTUM_PLANE_COUNT>
{
    
    public:
        
        ViewFrustum();
        ViewFrustum(const dim::matrix4f &Matrix);
        virtual ~ViewFrustum();
        
        /* === Functions === */
        
        //! Transforms the whole frustum by the specified 4x4 matrix.
        void setFrustum(const dim::matrix4f &Matrix);
        
        //! Transforms the whole frustum by the given view- and projection matrices.
        void setFrustum(const dim::matrix4f &ViewMatrix, const dim::matrix4f &ProjectionMatrix);
        
        //! Returns a 3D point along the left-up frustum edge.
        dim::vector3df getLeftUp() const;
        
        //! Returns a 3D point along the left-down frustum edge.
        dim::vector3df getLeftDown() const;
        
        //! Returns a 3D point along the right-up frustum edge.
        dim::vector3df getRightUp() const;
        
        //! Returns a 3D point along the right-down frustum edge.
        dim::vector3df getRightDown() const;
        
        /* === Inline functions === */
        
        //! Returns a constant reference of the specified frustum plane.
        inline const dim::plane3df& getPlane(const EViewFrustumPlanes Type) const
        {
            return Planes_[Type];
        }
        //! Returns a reference of the specified frustum plane.
        inline dim::plane3df& getPlane(const EViewFrustumPlanes Type)
        {
            return Planes_[Type];
        }
        
};


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
