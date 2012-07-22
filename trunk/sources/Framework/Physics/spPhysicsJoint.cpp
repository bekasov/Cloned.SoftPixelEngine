/*
 * Physics joint file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/spPhysicsJoint.hpp"

#ifdef SP_COMPILE_WITH_PHYSICS


namespace sp
{
namespace physics
{


PhysicsJoint::PhysicsJoint(
    const EPhysicsJoints Type, PhysicsBaseObject* ObjectA, PhysicsBaseObject* ObjectB) :
    Type_   (Type   ),
    ObjectA_(ObjectA),
    ObjectB_(ObjectB)
{
}
PhysicsJoint::~PhysicsJoint()
{
}

void PhysicsJoint::translate(const dim::vector3df &Direction)
{
    setPosition(getPosition() + Direction);
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
