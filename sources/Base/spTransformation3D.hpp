/*
 * Transformation 3D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_TRANSFORMATION_3D_H__
#define __SP_SCENE_TRANSFORMATION_3D_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionMatrix4.hpp"
#include "Base/spDimensionQuaternion.hpp"


namespace sp
{
namespace scene
{


/**
Base class for 3d transformations. Used for scene nodes and animation joints.
This class was originally named "Transformation" but now this is the name of its typedef.
So actually you will only need the typedef named "Transformation".
This transformation is stored as position (3D Vector), rotation (Quaternion) and scale (3D vector).
\see vector3d
\see quaternion
\see matrix4
\since Version 3.2
*/
template <typename T> class Transformation3D
{
    
    public:
        
        Transformation3D() :
            Scale_      (T(1)),
            HasChanged_ (true)
        {
        }
        Transformation3D(const dim::matrix4<T> &Matrix) :
            Position_   (Matrix.getPosition()       ),
            Rotation_   (Matrix.getRotationMatrix() ),
            Scale_      (Matrix.getScale()          ),
            Matrix_     (Matrix                     ),
            HasChanged_ (false                      )
        {
        }
        Transformation3D(
            const dim::vector3d<T> &Position, const dim::quaternion4<T> &Rotation, const dim::vector3d<T> &Scale) :
            Position_   (Position   ),
            Rotation_   (Rotation   ),
            Scale_      (Scale      ),
            HasChanged_ (true       )
        {
        }
        Transformation3D(const Transformation3D<T> &Other) :
            Position_   (Other.Position_    ),
            Rotation_   (Other.Rotation_    ),
            Scale_      (Other.Scale_       ),
            Matrix_     (Other.Matrix_      ),
            HasChanged_ (Other.HasChanged_  )
        {
        }
        ~Transformation3D()
        {
        }
        
        /* === Operators === */
        
        Transformation3D<T>& operator = (const Transformation3D<T> &Other)
        {
            Position_   = Other.Position_;
            Rotation_   = Other.Rotation_;
            Scale_      = Other.Scale_;
            Matrix_     = Other.Matrix_;
            HasChanged_ = Other.HasChanged_;
            return *this;
        }
        
        Transformation3D<T>& operator *= (const Transformation3D<T> &Other)
        {
            operator = (getMatrix() * Other.getMatrix());
            return *this;
        }
        
        Transformation3D<T> operator * (const Transformation3D<T> &Other) const
        {
            Transformation3D<T> Temp(*this);
            Temp *= Other;
            return Temp;
        }
        
        /* === Functions === */
        
        //! Returns the transformation as a 4x4 matrix.
        dim::matrix4<T> getMatrix() const
        {
            if (HasChanged_)
            {
                Matrix_.reset(Position_);
                Matrix_ *= Rotation_.getMatrixTransposed();
                Matrix_.scale(Scale_);
                HasChanged_ = false;
            }
            return Matrix_;
        }
        
        //! Returns the inverse transformation.
        Transformation3D<T> getInverse() const
        {
            dim::matrix4<T> Mat;
            getMatrix(Mat);
            Mat.setInverse();
            return Transformation3D<T>(Mat);
        }
        
        //! Interpolates this transformation between the given ones with the specified factor which is in the range [0.0 .. 1.0].
        void interpolate(const Transformation3D<T> &From, const Transformation3D<T> &To, const T &Interpolation)
        {
            math::Lerp(Position_,   From.Position_, To.Position_,   Interpolation);
            math::Lerp(Scale_,      From.Scale_,    To.Scale_,      Interpolation);
            Rotation_.slerp(From.Rotation_, To.Rotation_, Interpolation);
            HasChanged_ = true;
        }
        
        //! Moves the transformation into the specified direction. This depends on the current rotation.
        void move(const dim::vector3d<T> &Direction)
        {
            Position_ += (Rotation_.getMatrixTransposed() * Direction);
            HasChanged_ = true;
        }
        //! Turns the transformation into the specified rotation.
        void turn(const dim::vector3d<T> &Rotation)
        {
            dim::matrix4<T> Mat;
            Mat.setRotation(Rotation);
            Rotation_ *= dim::quaternion4<T>(Mat);
            HasChanged_ = true;
        }
        
        /* === Inline functions === */
        
        //! Sets the position vector.
        inline void setPosition(const dim::vector3d<T> &Position)
        {
            Position_ = Position;
            HasChanged_ = true;
        }
        //! Returns the position vector.
        inline dim::vector3d<T> getPosition() const
        {
            return Position_;
        }
        
        //! Sets the rotation quaternion.
        inline void setRotation(const dim::quaternion4<T> &Rotation)
        {
            Rotation_ = Rotation;
            HasChanged_ = true;
        }
        //! Returns the rotation quaternion.
        inline dim::quaternion4<T> getRotation() const
        {
            return Rotation_;
        }
        //! Returns the rotation as 4x4 matrix.
        inline dim::matrix4<T> getRotationMatrix() const
        {
            return Rotation_.getMatrixTransposed();
        }
        
        //! Sets the scaling vector.
        inline void setScale(const dim::vector3d<T> &Scale)
        {
            Scale_ = Scale;
            HasChanged_ = true;
        }
        //! Returns the scaling vector. By default ( 1 | 1 | 1 ).
        inline dim::vector3d<T> getScale() const
        {
            return Scale_;
        }
        
        //! Adds the specified direction to the position vector.
        inline void translate(const dim::vector3d<T> &Direction)
        {
            Position_ += Direction;
            HasChanged_ = true;
        }
        //! Adds the specified size to the scaling vector.
        inline void transform(const dim::vector3d<T> &Size)
        {
            Scale_ += Size;
            HasChanged_ = true;
        }
        
        //! Transforms the given matrix by the current transformation.
        inline void getMatrix(dim::matrix4<T> &Matrix) const
        {
            Matrix *= getMatrix();
        }
        
        /**
        Returns the inverse matrix.
        \see getMatrix
        */
        inline dim::matrix4<T> getInverseMatrix() const
        {
            return getMatrix().getInverse();
        }
        
        /**
        Sets the final matrix transformation directly.
        \note This will be overwritten the next time "getMatrix" is called!
        */
        inline void setMatrixDirect(const dim::matrix4<T> &Matrix)
        {
            Matrix_ = Matrix;
        }
        //! Returns a reference of the matrix transformation.
        inline dim::matrix4<T>& getMatrixDirect()
        {
            return Matrix_;
        }
        //! Returns the matrix transformation directly.
        inline dim::matrix4<T> getMatrixDirect() const
        {
            return Matrix_;
        }
        
        //! Returns a vector which points into this transformation's direction.
        inline dim::vector3df getDirection(const dim::vector3df &upVector = dim::vector3df(0, 0, 1)) const
        {
            return Rotation_.getInverse() * upVector;
        }
        
    private:
        
        /* === Members === */
        
        dim::vector3d<T> Position_;
        dim::quaternion4<T> Rotation_;
        dim::vector3d<T> Scale_;
        
        mutable dim::matrix4<T> Matrix_;
        mutable bool HasChanged_;
        
};


typedef Transformation3D<f32> Transformation;


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
