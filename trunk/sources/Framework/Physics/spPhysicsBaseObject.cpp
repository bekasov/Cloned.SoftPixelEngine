/*
 * Physics base object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/spPhysicsBaseObject.hpp"

#ifdef SP_COMPILE_WITH_PHYSICS


namespace sp
{
namespace physics
{


PhysicsBaseObject::PhysicsBaseObject()
{
}
PhysicsBaseObject::~PhysicsBaseObject()
{
}

void PhysicsBaseObject::setPosition(const dim::vector3df &Position)
{
    dim::matrix4f Transformation(getTransformation());
    Transformation.setPosition(Position);
    setTransformation(Transformation);
}
dim::vector3df PhysicsBaseObject::getPosition() const
{
    return getTransformation().getPosition();
}

void PhysicsBaseObject::setRotation(const dim::matrix4f &Rotation)
{
    const dim::vector3df Position(getPosition());
    dim::matrix4f Transformation(Rotation);
    Transformation.setPosition(Position);
    setTransformation(Transformation);
}
dim::matrix4f PhysicsBaseObject::getRotation() const
{
    return getTransformation().getRotationMatrix();
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
