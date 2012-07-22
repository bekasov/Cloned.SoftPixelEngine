/*
 * Newton simulator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/Newton/spNewtonSimulator.hpp"

#ifdef SP_COMPILE_WITH_NEWTON


#include "Framework/Physics/Newton/spNewtonRigidBody.hpp"
#include "Framework/Physics/Newton/spNewtonStaticObject.hpp"
#include "Framework/Physics/Newton/spNewtonJoint.hpp"
#include "Base/spMemoryManagement.hpp"

#include <newton.h>
#include <boost/foreach.hpp>


namespace sp
{
namespace physics
{


/*
 * Static callbacks
 */

void DefCallbackGenericContactProcess(const NewtonJoint* Contact, f32 TimeStep, s32 ThreadIndex)
{
    const NewtonMaterial* Material = NewtonContactGetMaterial(NewtonContactJointGetFirstContact(Contact));
    
    const f32 Velocity = NewtonMaterialGetContactNormalSpeed(Material);
    
    dim::vector3df Point, Normal;
    NewtonMaterialGetContactPositionAndNormal(Material, &Point.X, &Normal.X);
    
    (PhysicsSimulator::getContactCallback())(Point, Normal, Velocity);
}


/*
 * NewtonSimulator class
 */

NewtonWorld* NewtonSimulator::NtWorld_ = 0;

NewtonSimulator::NewtonSimulator() :
    PhysicsSimulator(SIMULATOR_NEWTON)
{
    /* Print newton library information */
    io::Log::message(getVersion(), 0);
    io::Log::message("Copyright (c) <2003-2011> - Julio Jerez and Alain Suero", 0);
    io::Log::message("", 0);
    
    /* Create the newton world */
    NtWorld_ = NewtonCreate();
    
    s32 MaterialGroupID = NewtonMaterialGetDefaultGroupID(NtWorld_);
    NewtonMaterialSetDefaultFriction    (NtWorld_, MaterialGroupID, MaterialGroupID, 0.8f, 0.4f);
    NewtonMaterialSetDefaultElasticity  (NtWorld_, MaterialGroupID, MaterialGroupID, 0.3f);
    NewtonMaterialSetDefaultSoftness    (NtWorld_, MaterialGroupID, MaterialGroupID, 0.05f);
}
NewtonSimulator::~NewtonSimulator()
{
    clearScene();
    NewtonDestroy(NtWorld_);
}

io::stringc NewtonSimulator::getVersion() const
{
    io::stringc Version(NewtonWorldGetVersion());
    Version = Version.left(1) + "." + Version.right(2);
    return "NewtonGameDynamics - v." + Version;
}

void NewtonSimulator::updateSimulation(const f32 StepTime)
{
    NewtonUpdate(NtWorld_, StepTime);
}

void NewtonSimulator::setGravity(const dim::vector3df &Gravity)
{
    Gravity_ = Gravity;
    foreach (RigidBody* Body, RigidBodyList_)
        Body->setGravity(Gravity);
}

PhysicsMaterial* NewtonSimulator::createMaterial(
    f32 StaticFriction, f32 DynamicFriction, f32 Restitution)
{
    return 0; //todo
}

StaticPhysicsObject* NewtonSimulator::createStaticObject(PhysicsMaterial* Material, scene::Mesh* Mesh)
{
    if (!Mesh)
    {
        io::Log::error("Invalid mesh object for static physics object");
        return 0;
    }
    
    StaticPhysicsObject* NewBody = new NewtonStaticObject(Mesh);
    StaticBodyList_.push_back(NewBody);
    
    return NewBody;
}

RigidBody* NewtonSimulator::createRigidBody(
    PhysicsMaterial* Material, const ERigidBodies Type, scene::SceneNode* RootNode, const SRigidBodyConstruction &Construct)
{
    if (!RootNode)
    {
        io::Log::error("Invalid root node for rigid body");
        return 0;
    }
    
    RigidBody* NewBody = 0;
    
    if (Type == RIGIDBODY_MESH)
    {
        if (RootNode->getType() != scene::NODE_MESH)
        {
            io::Log::error("Create mesh rigid body with no valid mesh object");
            return 0;
        }
        else
            NewBody = allocRigidBody(static_cast<scene::Mesh*>(RootNode));
    }
    else
    {
        NewBody = allocRigidBody(Type, Construct);
        NewBody->setRootNode(RootNode);
    }
    
    RootNode->setParent(0);
    NewBody->setMass(25.0f);
    NewBody->setGravity(Gravity_);
    NewBody->setTransformation(RootNode->getPositionMatrix() * RootNode->getRotationMatrix());
    
    RigidBodyList_.push_back(NewBody);
    
    return NewBody;
}

RigidBody* NewtonSimulator::createRigidBody(PhysicsMaterial* Material, scene::Mesh* Mesh)
{
    if (!Mesh)
    {
        io::Log::error("Invalid mesh for rigid body");
        return 0;
    }
    
    RigidBody* NewBody = allocRigidBody(Mesh);
    
    Mesh->setParent(0);
    NewBody->setMass(25.0f);
    NewBody->setGravity(Gravity_);
    NewBody->setTransformation(Mesh->getPositionMatrix() * Mesh->getRotationMatrix());
    
    RigidBodyList_.push_back(NewBody);
    
    return NewBody;
}

PhysicsJoint* NewtonSimulator::createJoint(
    const EPhysicsJoints Type, PhysicsBaseObject* Object, const SPhysicsJointConstruct &Construct)
{
    return createJoint(Type, 0, Object, Construct);
}

PhysicsJoint* NewtonSimulator::createJoint(
    const EPhysicsJoints Type, PhysicsBaseObject* ObjectA, PhysicsBaseObject* ObjectB, const SPhysicsJointConstruct &Construct)
{
    PhysicsJoint* NewJoint = new NewtonDynamicsJoint(Type, ObjectA, ObjectB, Construct);
    JointList_.push_back(NewJoint);
    return NewJoint;
}

void NewtonSimulator::setThreadCount(s32 Count)
{
    NewtonSetThreadsCount(NtWorld_, Count);
}
s32 NewtonSimulator::getThreadCount() const
{
    return NewtonGetThreadsCount(NtWorld_);
}

void NewtonSimulator::setSolverModel(s32 Model)
{
    NewtonSetSolverModel(NtWorld_, Model);
}

void NewtonSimulator::setContactCallback(const PhysicsContactCallback &Callback)
{
    PhysicsSimulator::setContactCallback(Callback);
    
    s32 MaterialGroupID = NewtonMaterialGetDefaultGroupID(NtWorld_);
    
    if (ContactCallback_)
        NewtonMaterialSetCollisionCallback(NtWorld_, MaterialGroupID, MaterialGroupID, 0, 0, DefCallbackGenericContactProcess);
    else
        NewtonMaterialSetCollisionCallback(NtWorld_, MaterialGroupID, MaterialGroupID, 0, 0, 0);
}

NewtonWorld* NewtonSimulator::getNewtonWorld()
{
    return NtWorld_;
}


/*
 * ======= Protected: =======
 */

RigidBody* NewtonSimulator::allocRigidBody(const ERigidBodies Type, const SRigidBodyConstruction &Construct)
{
    return new NewtonRigidBody(Type, Construct);
}
RigidBody* NewtonSimulator::allocRigidBody(scene::Mesh* MeshGeom)
{
    return new NewtonRigidBody(MeshGeom);
}


/*
 * ======= Physics callbacks =======
 */

void PhysicsTransformCallback(const NewtonBody* NtBody, const f32* Transformation, s32 ThreadIndex)
{
    /* Get the rigid body object */
    RigidBody* PhysicsObject = static_cast<RigidBody*>(NewtonBodyGetUserData(NtBody));
    
    if (!PhysicsObject)
        return;
    
    scene::SceneNode* RootNode = PhysicsObject->getRootNode();
    
    if (!RootNode)
        return;
    
    /* Apply the transformation */
    const dim::matrix4f* Matrix = (const dim::matrix4f*)Transformation;
    
    RootNode->setPosition(Matrix->getPosition());
    RootNode->setRotationMatrix(Matrix->getRotationMatrix());
}

void PhysicsForceAndTorqueCallback(const NewtonBody* NtBody, f32 TimeStep, s32 ThreadIndex)
{
    /* Get the rigid body object */
    RigidBody* PhysicsObject = static_cast<RigidBody*>(NewtonBodyGetUserData(NtBody));
    
    if (!PhysicsObject)
        return;
    
    /* Apply the forces */
    if (PhysicsObject->getGravityCallback())
        PhysicsObject->getGravityCallback()(PhysicsObject);
    else
        PhysicsObject->setForce(PhysicsObject->getGravity() * PhysicsObject->getMass());
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
