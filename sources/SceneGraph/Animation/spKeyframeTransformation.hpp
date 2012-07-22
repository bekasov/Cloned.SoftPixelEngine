/*
 * Keyframe transformation header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_KEYFRAME_TRANSFORMATION_H__
#define __SP_KEYFRAME_TRANSFORMATION_H__


#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"


namespace sp
{
namespace scene
{


class SP_EXPORT KeyframeTransformation
{
    
    public:
        
        KeyframeTransformation();
        KeyframeTransformation(const dim::matrix4f &Matrix);
        KeyframeTransformation(const dim::vector3df &Position, const dim::quaternion &Rotation, const dim::vector3df &Scale);
        KeyframeTransformation(const KeyframeTransformation &Transformation);
        ~KeyframeTransformation();
        
        /* Function */
        
        //! Transforms the given matrix by the current transformation.
        void getMatrix(dim::matrix4f &Matrix) const;
        
        //! Returns the transformation as one matrix object.
        dim::matrix4f getMatrix() const;
        
        //! Interpolates this transformation between the given ones with the specified factor which is in the range [0.0 .. 1.0].
        void interpolate(
            const KeyframeTransformation &From, const KeyframeTransformation &To, f32 Interpolation
        );
        
        //! Returns the inverse transformation.
        KeyframeTransformation getInverse() const;
        
        /* Inline functions */
        
        inline void setPosition(const dim::vector3df &Position)
        {
            Position_ = Position;
        }
        inline dim::vector3df getPosition() const
        {
            return Position_;
        }
        
        inline void setRotation(const dim::quaternion &Rotation)
        {
            Rotation_ = Rotation;
        }
        inline dim::quaternion getRotation() const
        {
            return Rotation_;
        }
        
        inline void setScale(const dim::vector3df &Scale)
        {
            Scale_ = Scale;
        }
        inline dim::vector3df getScale() const
        {
            return Scale_;
        }
        
    private:
        
        /* Members */
        
        dim::vector3df Position_;
        dim::quaternion Rotation_;
        dim::vector3df Scale_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
