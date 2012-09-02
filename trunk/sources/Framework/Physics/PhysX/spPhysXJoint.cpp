/*
 * PhysX joint file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/PhysX/spPhysXJoint.hpp"

//#ifdef SP_COMPILE_WITH_PHYSX


#include "Framework/Physics/PhysX/spPhysXRigidBody.hpp"


namespace sp
{
namespace physics
{


PhysXJoint::PhysXJoint(
    PxPhysics* PxDevice,
    const EPhysicsJoints Type, PhysXRigidBody* ObjectA,
    PhysicsBaseObject* ObjectB, const SPhysicsJointConstruct &Construct) :
    PhysicsJoint    (Type, ObjectA, ObjectB ),
    PxJoint_        (0                      ),
    PxRigidBody0_   (0                      ),
    PxRigidBody1_   (0                      )
{
    if (!PxDevice || !ObjectA)
        throw io::stringc("Invalid arguments for physics joint");
    
    /* Store PhysX rigid dynamic actors */
    if (ObjectA)
        PxRigidBody0_ = ObjectA->PxActor_;
    
    /* Construct local frame transformations */
    const dim::matrix4f MatInvA(ObjectA->getTransformation().getInverse());
    const dim::matrix4f MatInvB(ObjectB ? ObjectB->getTransformation().getInverse() : dim::matrix4f());
    
    const dim::vector3df PointInvA(MatInvA * Construct.PointA);
    const dim::vector3df PointInvB(MatInvB * Construct.PointB);
    
    PxTransform LocalFrame0, LocalFrame1;
    
    LocalFrame0.p = VecSp2Px(PointInvA);
    LocalFrame0.q = PxQuat::createIdentity();
    
    LocalFrame1.p = VecSp2Px(PointInvB);
    LocalFrame1.q = PxQuat::createIdentity();
    
    /* Create PhysX joint */
    switch (Type)
    {
        case JOINT_HINGE:
        {
            PxJoint_ = PxRevoluteJointCreate(
                *PxDevice,
                dynamic_cast<PhysXBaseObject*>(ObjectA)->PxBaseActor_,
                LocalFrame0,
                ObjectB ? dynamic_cast<PhysXBaseObject*>(ObjectB)->PxBaseActor_ : 0,
                LocalFrame1
            );
        }
        break;
        
        default:
            throw io::stringc("Unsupported physics joint");
    }
    
    if (!PxJoint_)
        throw io::stringc("Could not create PhysX joint");
}
PhysXJoint::~PhysXJoint()
{
}

void PhysXJoint::setPosition(const dim::vector3df &Position)
{
    //todo
}
dim::vector3df PhysXJoint::getPosition() const
{
    return 0.0f; //todo
}

void PhysXJoint::setLimit(bool Enable)
{
    //todo
}
bool PhysXJoint::getLimit() const
{
    return false; //todo
}

void PhysXJoint::setLimit(f32 Min, f32 Max, bool Enable)
{
    //todo
}
void PhysXJoint::getLimit(f32 &Min, f32 &Max) const
{
    //todo
}

void PhysXJoint::setMotor(bool Enable, f32 MotorPower)
{
    switch (Type_)
    {
        case JOINT_HINGE:
            static_cast<PxRevoluteJoint*>(PxJoint_)->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, Enable);
            break;
    }
}

bool PhysXJoint::getMotor() const
{
    return false; //todo
}

void PhysXJoint::runMotor(f32 Velocity)
{
    /* Wake up actors */
    if (PxRigidBody0_)
        PxRigidBody0_->wakeUp(100.0f);
    if (PxRigidBody1_)
        PxRigidBody1_->wakeUp(100.0f);
    
    /* Run joint motor */
    switch (Type_)
    {
        case JOINT_HINGE:
            static_cast<PxRevoluteJoint*>(PxJoint_)->setDriveVelocity(Velocity);
            break;
    }
}

f32 PhysXJoint::getLinearValue() const
{
    return 0.0f; //todo
}


} // /namespace physics

} // /namespace sp


//#endif



// ================================================================================
