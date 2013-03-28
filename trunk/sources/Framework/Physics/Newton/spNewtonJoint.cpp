/*
 * Newton joint file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/Newton/spNewtonJoint.hpp"

#ifdef SP_COMPILE_WITH_NEWTON


#include "Framework/Physics/Newton/spNewtonSimulator.hpp"
#include "Framework/Physics/Newton/spNewtonRigidBody.hpp"

#include <newton.h>


namespace sp
{
namespace physics
{


static const c8* DEBUG_MSG = "Not yet implemented for newton physics simulator";

NewtonDynamicsJoint::NewtonDynamicsJoint(
    const EPhysicsJoints Type, PhysicsBaseObject* ObjectA,
    PhysicsBaseObject* ObjectB, const SPhysicsJointConstruct &Construct) :
    PhysicsJoint(Type, ObjectA, ObjectB ),
    NtJoint_    (0                      )
{
    if (!ObjectB)
    {
        io::Log::error("Invalid child object for newton physics joint");
        return;
    }
    
    /* Get newton relevant objects */
    NewtonBody* NtBodyA     = (ObjectA ? dynamic_cast<NewtonBaseObject*>(ObjectA)->NtBody_ : 0);
    NewtonBody* NtBodyB     = (ObjectB ? dynamic_cast<NewtonBaseObject*>(ObjectB)->NtBody_ : 0);
    NewtonWorld* NtWorld    = NewtonSimulator::getNewtonWorld();
    
    /* Create newton joint */
    switch (Type_)
    {
        case JOINT_BALL:
            NtJoint_ = NewtonConstraintCreateBall(NtWorld, &Construct.PointA.X, NtBodyB, NtBodyA);
            break;
        case JOINT_HINGE:
            NtJoint_ = NewtonConstraintCreateHinge(NtWorld, &Construct.PointA.X, &Construct.DirectionA.X, NtBodyB, NtBodyA);
            break;
        case JOINT_SLIDER:
            NtJoint_ = NewtonConstraintCreateSlider(NtWorld, &Construct.PointA.X, &Construct.DirectionA.X, NtBodyB, NtBodyA);
            break;
        case JOINT_CORKSCREW:
            NtJoint_ = NewtonConstraintCreateCorkscrew(NtWorld, &Construct.PointA.X, &Construct.DirectionA.X, NtBodyB, NtBodyA);
            break;
        case JOINT_UNIVERSAL:
            NtJoint_ = NewtonConstraintCreateUniversal(NtWorld, &Construct.PointA.X, &Construct.DirectionA.X, &Construct.DirectionB.X, NtBodyB, NtBodyA);
            break;
    }
}
NewtonDynamicsJoint::~NewtonDynamicsJoint()
{
    if (NtJoint_)
        NewtonDestroyJoint(NewtonSimulator::getNewtonWorld(), NtJoint_);
}

void NewtonDynamicsJoint::setPosition(const dim::vector3df &Position)
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("NewtonDynamicsJoint::setPosition", DEBUG_MSG);
    #endif
}
dim::vector3df NewtonDynamicsJoint::getPosition() const
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("NewtonDynamicsJoint::getPosition", DEBUG_MSG);
    #endif
    return 0.0f;
}

void NewtonDynamicsJoint::setLimit(bool Enable)
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("NewtonDynamicsJoint::setLimit", DEBUG_MSG);
    #endif
}
bool NewtonDynamicsJoint::getLimit() const
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("NewtonDynamicsJoint::getLimit", DEBUG_MSG);
    #endif
    return false;
}

void NewtonDynamicsJoint::setLimit(f32 Min, f32 Max, bool Enable)
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("NewtonDynamicsJoint::setLimit", DEBUG_MSG);
    #endif
}
void NewtonDynamicsJoint::getLimit(f32 &Min, f32 &Max) const
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("NewtonDynamicsJoint::getLimit", DEBUG_MSG);
    #endif
}

void NewtonDynamicsJoint::setMotor(bool Enable, f32 MotorPower)
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("NewtonDynamicsJoint::setMotor", DEBUG_MSG);
    #endif
}
bool NewtonDynamicsJoint::getMotor() const
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("NewtonDynamicsJoint::getMotor", DEBUG_MSG);
    #endif
    return false;
}

void NewtonDynamicsJoint::runMotor(f32 Velocity)
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("NewtonDynamicsJoint::runMotor", DEBUG_MSG);
    #endif
}

f32 NewtonDynamicsJoint::getLinearValue() const
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("NewtonDynamicsJoint::getLinearValue", DEBUG_MSG);
    #endif
    return 0.0f;
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
