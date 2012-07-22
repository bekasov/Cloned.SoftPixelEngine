/*
 * Bullet simulator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/Bullet/spBulletSimulator.hpp"

#ifdef SP_COMPILE_WITH_BULLET


#include "Framework/Physics/Bullet/spBulletRigidBody.hpp"
#include "Framework/Physics/Bullet/spBulletStaticObject.hpp"
//#include "Framework/Physics/Bullet/spBulletMaterial.hpp"
#include "Framework/Physics/Bullet/spBulletJoint.hpp"
#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace physics
{


BulletSimulator::BulletSimulator() :
    PhysicsSimulator    (SIMULATOR_BULLET   ),
    BtBroadPhase_       (0                  ),
    BtCollisionConfig_  (0                  ),
    BtDispatcher_       (0                  ),
    BtSolver_           (0                  ),
    BtWorld_            (0                  )
{
    /* Print newton library information */
    io::Log::message(getVersion(), 0);
    io::Log::message("Copyright (c) 2012 - Game Physics Simulation", 0);
    io::Log::message("", 0);
    
    /* Build Bullet broadphase */
    BtBroadPhase_ = new btDbvtBroadphase();
    
    /* Setup collision configuration and dispatcher */
    BtCollisionConfig_ = new btDefaultCollisionConfiguration();
    BtDispatcher_ = new btCollisionDispatcher(BtCollisionConfig_);
    
    /* Create physics solver */
    BtSolver_ = new btSequentialImpulseConstraintSolver();
    
    /* Create the dynamics world */
    BtWorld_ = new btDiscreteDynamicsWorld(BtDispatcher_, BtBroadPhase_, BtSolver_, BtCollisionConfig_);
    BtWorld_->setGravity(btVector3(Gravity_.X, Gravity_.Y, Gravity_.Z));
}
BulletSimulator::~BulletSimulator()
{
    clearScene();
    
    MemoryManager::deleteMemory(BtWorld_);
    MemoryManager::deleteMemory(BtSolver_);
    MemoryManager::deleteMemory(BtDispatcher_);
    MemoryManager::deleteMemory(BtCollisionConfig_);
    MemoryManager::deleteMemory(BtBroadPhase_);
    
    MemoryManager::deleteList(MaterialList_);
}

io::stringc BulletSimulator::getVersion() const
{
    return "Bullet Physics - v.2.80";
}

void BulletSimulator::updateSimulation(const f32 StepTime)
{
    BtWorld_->stepSimulation(StepTime, 3);
}

PhysicsMaterial* BulletSimulator::createMaterial(
    f32 StaticFriction, f32 DynamicFriction, f32 Restitution)
{
    return 0; //todo
}

StaticPhysicsObject* BulletSimulator::createStaticObject(PhysicsMaterial* Material, scene::Mesh* Mesh)
{
    BulletStaticObject* NewStaticObject = new BulletStaticObject(Mesh);
    StaticBodyList_.push_back(NewStaticObject);
    
    if (NewStaticObject->BtBody_)
        BtWorld_->addRigidBody(NewStaticObject->BtBody_);
    
    return NewStaticObject;
}

RigidBody* BulletSimulator::createRigidBody(
    PhysicsMaterial* Material, const ERigidBodies Type, scene::SceneNode* RootNode, const SRigidBodyConstruction &Construct)
{
    BulletRigidBody* NewRigidBody = new BulletRigidBody(Type, RootNode, Construct);
    RigidBodyList_.push_back(NewRigidBody);
    
    if (NewRigidBody->BtBody_)
    {
        BtWorld_->addRigidBody(NewRigidBody->BtBody_);
        NewRigidBody->setAutoSleep(false);
    }
    
    return NewRigidBody;
}

RigidBody* BulletSimulator::createRigidBody(PhysicsMaterial* Material, scene::Mesh* Mesh)
{
    return 0; //todo
}

PhysicsJoint* BulletSimulator::createJoint(
    const EPhysicsJoints Type, PhysicsBaseObject* Object, const SPhysicsJointConstruct &Construct)
{
    if (!Object)
        return 0;
    
    BulletJoint* NewJoint = new BulletJoint(Type, Object, Construct);
    JointList_.push_back(NewJoint);
    
    BtWorld_->addConstraint(NewJoint->BtConstraint_, true);
    
    return NewJoint;
}

PhysicsJoint* BulletSimulator::createJoint(
    const EPhysicsJoints Type, PhysicsBaseObject* ObjectA, PhysicsBaseObject* ObjectB, const SPhysicsJointConstruct &Construct)
{
    if (!ObjectA || !ObjectB)
        return 0;
    
    BulletJoint* NewJoint = new BulletJoint(Type, ObjectA, ObjectB, Construct);
    JointList_.push_back(NewJoint);
    
    BtWorld_->addConstraint(NewJoint->BtConstraint_, true);
    
    return NewJoint;
}

void BulletSimulator::deleteJoint(PhysicsJoint* Object)
{
    if (Object)
    {
        BtWorld_->removeConstraint(static_cast<BulletJoint*>(Object)->BtConstraint_);
        PhysicsSimulator::deleteJoint(Object);
    }
}

void BulletSimulator::clearScene(bool RigidBodies, bool StaticObjects, bool Joints)
{
    if (RigidBodies)
    {
        foreach (RigidBody* Obj, RigidBodyList_)
        {
            btRigidBody* BtBody = dynamic_cast<BulletBaseObject*>(Obj)->BtBody_;
            if (BtBody)
                BtWorld_->removeRigidBody(BtBody);
        }
    }
    if (StaticObjects)
    {
        foreach (StaticPhysicsObject* Obj, StaticBodyList_)
        {
            btRigidBody* BtBody = dynamic_cast<BulletBaseObject*>(Obj)->BtBody_;
            if (BtBody)
                BtWorld_->removeRigidBody(BtBody);
        }
    }
    if (Joints)
    {
        foreach (PhysicsJoint* Obj, JointList_)
        {
            btTypedConstraint* BtConstraint = static_cast<BulletJoint*>(Obj)->BtConstraint_;
            if (BtConstraint)
                BtWorld_->removeConstraint(BtConstraint);
        }
    }
    
    PhysicsSimulator::clearScene(RigidBodies, StaticObjects, Joints);
}


/*
 * ======= Protected: =======
 */



} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
