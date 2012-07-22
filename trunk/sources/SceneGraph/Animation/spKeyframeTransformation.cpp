/*
 * Keyframe transformation header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spKeyframeTransformation.hpp"


namespace sp
{
namespace scene
{


KeyframeTransformation::KeyframeTransformation() :
    Scale_(1.0f)
{
}
KeyframeTransformation::KeyframeTransformation(const dim::matrix4f &Matrix) :
    Position_   (Matrix.getPosition()       ),
    Rotation_   (Matrix.getRotationMatrix() ),
    Scale_      (Matrix.getScale()          )
{
}
KeyframeTransformation::KeyframeTransformation(
    const dim::vector3df &Position, const dim::quaternion &Rotation, const dim::vector3df &Scale) :
    Position_   (Position   ),
    Rotation_   (Rotation   ),
    Scale_      (Scale      )
{
}
KeyframeTransformation::KeyframeTransformation(const KeyframeTransformation &Transformation) :
    Position_   (Transformation.Position_   ),
    Rotation_   (Transformation.Rotation_   ),
    Scale_      (Transformation.Scale_      )
{
}
KeyframeTransformation::~KeyframeTransformation()
{
}

void KeyframeTransformation::getMatrix(dim::matrix4f &Matrix) const
{
    Matrix.translate(Position_);
    Matrix *= Rotation_.getMatrixTransposed();
    Matrix.scale(Scale_);
}

dim::matrix4f KeyframeTransformation::getMatrix() const
{
    dim::matrix4f Mat;
    getMatrix(Mat);
    return Mat;
}

void KeyframeTransformation::interpolate(
    const KeyframeTransformation &From, const KeyframeTransformation &To, f32 Interpolation)
{
    Position_   = From.Position_    + (To.Position_ - From.Position_) * Interpolation;
    Scale_      = From.Scale_       + (To.Scale_    - From.Scale_   ) * Interpolation;
    Rotation_.slerp(From.Rotation_, To.Rotation_, Interpolation);
}

KeyframeTransformation KeyframeTransformation::getInverse() const
{
    dim::matrix4f Mat;
    getMatrix(Mat);
    
    Mat.setInverse();
    
    return KeyframeTransformation(Mat);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
