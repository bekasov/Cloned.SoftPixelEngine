/*
 * Transformation 2D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_TRANSFORMATION_2D_H__
#define __SP_SCENE_TRANSFORMATION_2D_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector2D.hpp"
#include "Base/spDimensionSize2D.hpp"
#include "Base/spDimensionMatrix4.hpp"


namespace sp
{
namespace scene
{


/**
2D transformation class. This is not a base class. Unlike 3D transformation in 2D, most transformations
are done without matrices or some kind of that. But it can be used for a more structured transformation.
This transformation is stored as position (2D Point), rotation (Single value) and scale (2D Size).
\see point2d
\see size2d
\since Version 3.2
*/
template <typename T> class Transformation2D
{
    
    public:
        
        Transformation2D() :
            Rotation_   (T(0)),
            Scale_      (T(1)),
            HasChanged_ (true)
        {
        }
        Transformation2D(const dim::matrix4<T> &Matrix) :
            Position_   (Matrix.getPosition()   ),
            Rotation_   (Matrix.getRotation().Z ),
            Scale_      (Matrix.getScale()      ),
            Matrix_     (Matrix                 ),
            HasChanged_ (false                  )
        {
        }
        Transformation2D(
            const dim::vector2d<T> &Position, const T &Rotation, const dim::size2d<T> &Scale) :
            Position_   (Position   ),
            Rotation_   (Rotation   ),
            Scale_      (Scale      ),
            HasChanged_ (true       )
        {
        }
        Transformation2D(const Transformation2D<T> &Other) :
            Position_   (Other.Position_    ),
            Rotation_   (Other.Rotation_    ),
            Scale_      (Other.Scale_       ),
            Matrix_     (Other.Matrix_      ),
            HasChanged_ (Other.HasChanged_  )
        {
        }
        ~Transformation2D()
        {
        }
        
        /* === Operators === */
        
        Transformation2D<T>& operator = (const Transformation2D<T> &Other)
        {
            Position_   = Other.Position_;
            Rotation_   = Other.Rotation_;
            Scale_      = Other.Scale_;
            Matrix_     = Other.Matrix_;
            HasChanged_ = Other.HasChanged_;
            return *this;
        }
        
        Transformation2D<T>& operator *= (const Transformation2D<T> &Other)
        {
            operator = (getMatrix() * Other.getMatrix());
            return *this;
        }
        
        Transformation2D<T> operator * (const Transformation2D<T> &Other) const
        {
            Transformation2D<T> Temp(*this);
            Temp *= Other;
            return Temp;
        }
        
        /* === Functions === */
        
        /**
        Returns the transformation as a 4x4 matrix.
        \note Can not be used for multi-threading because of mutable members.
        */
        const dim::matrix4<T>& getMatrix() const
        {
            if (HasChanged_)
            {
                Matrix_.reset(dim::vector3d<T>(Position_.X, Position_.Y, T(0)));
                Matrix_.rotateZ(Rotation_);
                Matrix_.scale(dim::vector3d<T>(Scale_.Width, Scale_.Height, T(1)));
                HasChanged_ = false;
            }
            return Matrix_;
        }
        
        //! Returns the inverse transformation.
        Transformation2D<T> getInverse() const
        {
            dim::matrix4<T> Mat;
            getMatrix(Mat);
            Mat.setInverse();
            return Transformation2D<T>(Mat);
        }
        
        //! Interpolates this transformation between the given ones with the specified factor which is in the range [0.0 .. 1.0].
        void interpolate(const Transformation2D<T> &From, const Transformation2D<T> &To, const T &Interpolation)
        {
            math::lerp(Position_,   From.Position_, To.Position_,   Interpolation);
            math::lerp(Rotation_,   From.Rotation_, To.Rotation_,   Interpolation);
            math::lerp(Scale_,      From.Scale_,    To.Scale_,      Interpolation);
            HasChanged_ = true;
        }
        
        //! Moves the transformation into the specified direction. This depends on the current rotation.
        void move(const dim::vector2d<T> &Direction)
        {
            Position_ += (getRotationMatrix() * Direction);
            HasChanged_ = true;
        }
        //! Turns the transformation into the specified rotation.
        void turn(const T &Rotation)
        {
            Rotation_ += Rotation;
            HasChanged_ = true;
        }
        
        /* === Inline functions === */
        
        //! Sets the position vector.
        inline void setPosition(const dim::vector2d<T> &Position)
        {
            Position_ = Position;
            HasChanged_ = true;
        }
        //! Returns the position vector.
        inline dim::vector2d<T> getPosition() const
        {
            return Position_;
        }
        
        //! Sets the rotation quaternion.
        inline void setRotation(const T &Rotation)
        {
            Rotation_ = Rotation;
            HasChanged_ = true;
        }
        //! Returns the rotation quaternion.
        inline T getRotation() const
        {
            return Rotation_;
        }
        //! Returns the rotation as 4x4 matrix.
        inline dim::matrix4<T> getRotationMatrix() const
        {
            dim::matrix4<T> Mat;
            Mat.rotateZ(Rotation_);
            return Mat;
        }
        
        //! Sets the scaling vector.
        inline void setScale(const dim::size2d<T> &Scale)
        {
            Scale_ = Scale;
            HasChanged_ = true;
        }
        //! Returns the scaling vector. By default ( 1 | 1 | 1 ).
        inline dim::size2d<T> getScale() const
        {
            return Scale_;
        }
        
        //! Adds the specified direction to the position vector.
        inline void translate(const dim::vector2d<T> &Direction)
        {
            Position_ += Direction;
            HasChanged_ = true;
        }
        //! Adds the specified size to the scaling vector.
        inline void transform(const dim::size2d<T> &Size)
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
        
    private:
        
        /* === Members === */
        
        dim::vector2d<T> Position_;
        T Rotation_;
        dim::size2d<T> Scale_;
        
        mutable dim::matrix4<T> Matrix_;
        mutable bool HasChanged_;
        
};


typedef Transformation2D<f32> ScreenSpaceTransformation;


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
