/*
 * Physics simulator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/spPhysicsSimulator.hpp"

#ifdef SP_COMPILE_WITH_PHYSICS


#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace physics
{


PhysicsContactCallback PhysicsSimulator::ContactCallback_ = 0;

PhysicsSimulator::PhysicsSimulator(const EPhysicsSimulators Type) :
    Type_   (Type           ),
    Gravity_(0, -9.81f, 0   )
{
}
PhysicsSimulator::~PhysicsSimulator()
{
    clearScene();
}

void PhysicsSimulator::setGravity(const dim::vector3df &Gravity)
{
    Gravity_ = Gravity;
    foreach (RigidBody* Body, RigidBodyList_)
        Body->setGravity(Gravity);
}

void PhysicsSimulator::deleteMaterial(PhysicsMaterial* Material)
{
    MemoryManager::removeElement(MaterialList_, Material, true);
}
void PhysicsSimulator::deleteStaticObject(StaticPhysicsObject* Object)
{
    MemoryManager::removeElement(StaticBodyList_, Object, true);
}
void PhysicsSimulator::deleteRigidBody(RigidBody* Object)
{
    MemoryManager::removeElement(RigidBodyList_, Object, true);
}
void PhysicsSimulator::deleteJoint(PhysicsJoint* Object)
{
    MemoryManager::removeElement(JointList_, Object, true);
}

void PhysicsSimulator::clearScene(bool RigidBodies, bool StaticObjects, bool Joints)
{
    if (RigidBodies)
        MemoryManager::deleteList(RigidBodyList_);
    if (StaticObjects)
        MemoryManager::deleteList(StaticBodyList_);
    if (Joints)
        MemoryManager::deleteList(JointList_);
}

void PhysicsSimulator::setThreadCount(s32 Count)
{
    // do nothing
}
s32 PhysicsSimulator::getThreadCount() const
{
    return 0; // do nothing
}

void PhysicsSimulator::setSolverModel(s32 Model)
{
    // do nothing
}

void PhysicsSimulator::setContactCallback(const PhysicsContactCallback &Callback)
{
    ContactCallback_ = Callback;
}
PhysicsContactCallback PhysicsSimulator::getContactCallback()
{
    return ContactCallback_;
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
