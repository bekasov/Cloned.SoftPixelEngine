/*
 * Newton base object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/Newton/spNewtonBaseObject.hpp"

#ifdef SP_COMPILE_WITH_NEWTON


#include "Framework/Physics/Newton/spNewtonSimulator.hpp"

#include <newton.h>


namespace sp
{
namespace physics
{


NewtonBaseObject::NewtonBaseObject() :
    //PhysicsBaseObject   (   ),
    NtCollision_        (0  ),
    NtBody_             (0  )
{
}
NewtonBaseObject::~NewtonBaseObject()
{
    if (NtCollision_)
        NewtonReleaseCollision(NewtonSimulator::getNewtonWorld(), NtCollision_);
    if (NtBody_)
        NewtonDestroyBody(NewtonSimulator::getNewtonWorld(), NtBody_);
}

void NewtonBaseObject::setTransformation(const dim::matrix4f &Transformation)
{
    NewtonBodySetMatrix(NtBody_, Transformation.getPositionRotationMatrix().getArray());
}
dim::matrix4f NewtonBaseObject::getTransformation() const
{
    dim::matrix4f Transformation;
    NewtonBodyGetMatrix(NtBody_, Transformation.getArray());
    return Transformation;
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
