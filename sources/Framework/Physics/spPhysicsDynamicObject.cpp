/*
 * Physics dynamic object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/spPhysicsDynamicObject.hpp"

#ifdef SP_COMPILE_WITH_PHYSICS


namespace sp
{
namespace physics
{


DynamicPhysicsObject::DynamicPhysicsObject() :
    Gravity_            (0, -9.81, 0),
    Mass_               (1          ),
    AutoSleep_          (false      )
{
}
DynamicPhysicsObject::~DynamicPhysicsObject()
{
}

void DynamicPhysicsObject::setGravity(const dim::vector3df &Gravity)
{
    Gravity_ = Gravity;
}
void DynamicPhysicsObject::setMass(f32 Mass)
{
    Mass_ = Mass;
}
void DynamicPhysicsObject::setGravityCallback(const PhysicsGravityCallback &Callback)
{
    GravityCallback_ = Callback;
}
void DynamicPhysicsObject::setAutoSleep(bool Enable)
{
    AutoSleep_ = Enable;
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
