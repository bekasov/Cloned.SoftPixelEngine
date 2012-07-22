/*
 * Bullet joint file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/Bullet/spBulletJoint.hpp"

#ifdef SP_COMPILE_WITH_BULLET


#include "Framework/Physics/Bullet/spBulletSimulator.hpp"
#include "Framework/Physics/Bullet/spBulletRigidBody.hpp"


namespace sp
{
namespace physics
{


#define GetBallConstraint   static_cast<btPoint2PointConstraint*>(BtConstraint_)
#define GetHingeConstraint  static_cast<btHingeConstraint*      >(BtConstraint_)
#define GetSliderConstraint static_cast<btSliderConstraint*     >(BtConstraint_)

BulletJoint::BulletJoint(
    const EPhysicsJoints Type, PhysicsBaseObject* ObjectA,
    PhysicsBaseObject* ObjectB, const SPhysicsJointConstruct &Construct) :
    PhysicsJoint    (Type, ObjectA, ObjectB ),
    BtConstraint_   (0                      ),
    HingeLimitMin_  (0.0f                   ),
    HingeLimitMax_  (0.0f                   ),
    SliderLimitMin_ (0.0f                   ),
    SliderLimitMax_ (0.0f                   )
{
    if (!ObjectA || !ObjectB)
    {
        io::Log::error("Invalid objects for physics joint");
        return;
    }
    
    /* Get local pivot points */
    const dim::matrix4f MatInvA(ObjectA->getTransformation().getInverse());
    const dim::matrix4f MatInvB(ObjectB->getTransformation().getInverse());
    
    const dim::vector3df PointInvA(MatInvA * Construct.PointA);
    const dim::vector3df PointInvB(MatInvB * Construct.PointB);
    
    const btVector3 DirA(Construct.DirectionA.X, Construct.DirectionA.Y, Construct.DirectionA.Z);
    const btVector3 DirB(Construct.DirectionB.X, Construct.DirectionB.Y, Construct.DirectionB.Z);
    
    btRigidBody* BtBodyA = dynamic_cast<BulletBaseObject*>(ObjectA)->BtBody_;
    btRigidBody* BtBodyB = dynamic_cast<BulletBaseObject*>(ObjectB)->BtBody_;
    
    /* Create respective constraint */
    switch (Type_)
    {
        case JOINT_BALL:
            BtConstraint_ = new btPoint2PointConstraint(
                *BtBodyA, *BtBodyB,
                btVector3(PointInvA.X, PointInvA.Y, PointInvA.Z),
                btVector3(PointInvB.X, PointInvB.Y, PointInvB.Z)
            );
            break;
            
        case JOINT_HINGE:
            BtConstraint_ = new btHingeConstraint(
                *BtBodyA, *BtBodyB,
                btVector3(PointInvA.X, PointInvA.Y, PointInvA.Z),
                btVector3(PointInvB.X, PointInvB.Y, PointInvB.Z),
                DirA, DirB
            );
            break;
            
        case JOINT_SLIDER:
            BtConstraint_ = new btSliderConstraint(
                *BtBodyA, *BtBodyB,
                BulletMotionState::getTransform(MatInvA * Construct.TransformA),
                BulletMotionState::getTransform(MatInvB * Construct.TransformB),
                true
            );
            break;
            
        case JOINT_CORKSCREW:
            break;
        case JOINT_UNIVERSAL:
            break;
    }
}
BulletJoint::BulletJoint(
    const EPhysicsJoints Type, PhysicsBaseObject* Object, const SPhysicsJointConstruct &Construct) :
    PhysicsJoint    (Type, Object, 0),
    BtConstraint_   (0              ),
    HingeLimitMin_  (0.0f           ),
    HingeLimitMax_  (0.0f           ),
    SliderLimitMin_ (0.0f           ),
    SliderLimitMax_ (0.0f           )
{
    if (!Object)
    {
        io::Log::error("Invalid object for physics joint");
        return;
    }
    
    /* Get local pivot points */
    const dim::matrix4f MatInv(Object->getTransformation().getInverse());
    const dim::vector3df PointInv(MatInv * Construct.PointA);
    
    const btVector3 Dir(Construct.DirectionA.X, Construct.DirectionA.Y, Construct.DirectionA.Z);
    
    btRigidBody* BtBody = dynamic_cast<BulletBaseObject*>(Object)->BtBody_;
    
    /* Create respective constraint */
    switch (Type_)
    {
        case JOINT_BALL:
            BtConstraint_ = new btPoint2PointConstraint(
                *BtBody, btVector3(PointInv.X, PointInv.Y, PointInv.Z)
            );
            break;
            
        case JOINT_HINGE:
            BtConstraint_ = new btHingeConstraint(
                *BtBody, btVector3(PointInv.X, PointInv.Y, PointInv.Z), Dir
            );
            break;
            
        case JOINT_SLIDER:
            BtConstraint_ = new btSliderConstraint(
                *BtBody, BulletMotionState::getTransform(MatInv * Construct.TransformA), true
            );
            break;
            
        case JOINT_CORKSCREW:
            break;
        case JOINT_UNIVERSAL:
            break;
    }
}
BulletJoint::~BulletJoint()
{
    MemoryManager::deleteMemory(BtConstraint_);
}

void BulletJoint::setPosition(const dim::vector3df &Position)
{
    switch (Type_)
    {
        case JOINT_BALL:
            GetBallConstraint->setPivotB(btVector3(Position.X, Position.Y, Position.Z));
            break;
    }
}
dim::vector3df BulletJoint::getPosition() const
{
    switch (Type_)
    {
        case JOINT_BALL:
            const btVector3 Pos = GetBallConstraint->getPivotInB();
            return dim::vector3df(Pos.x(), Pos.y(), Pos.z());
    }
    return 0.0f;
}

void BulletJoint::setHingeLimit(bool Enable)
{
    if (Type_ == JOINT_HINGE)
    {
        if (Enable)
            GetHingeConstraint->setLimit(HingeLimitMin_ * math::DEG, HingeLimitMax_ * math::DEG);
        else
            GetHingeConstraint->setLimit(1, 0);
    }
}
bool BulletJoint::getHingeLimit() const
{
    if (Type_ == JOINT_HINGE)
    {
        f32 MinAngle = 0.0f, MaxAngle = 0.0f;
        getHingeLimit(MinAngle, MaxAngle);
        return MinAngle <= MaxAngle;
    }
    return false;
}

void BulletJoint::setHingeLimit(f32 MinAngle, f32 MaxAngle, bool Enable)
{
    HingeLimitMin_ = MinAngle;
    HingeLimitMax_ = MaxAngle;
    
    setHingeLimit(Enable);
}
void BulletJoint::getHingeLimit(f32 &MinAngle, f32 &MaxAngle) const
{
    if (Type_ == JOINT_HINGE)
    {
        MinAngle = GetHingeConstraint->getLowerLimit() * math::RAD;
        MaxAngle = GetHingeConstraint->getUpperLimit() * math::RAD;
    }
}

void BulletJoint::setHingeMotor(bool Enable, f32 Velocity, f32 MotorPower)
{
    if (Type_ == JOINT_HINGE)
        GetHingeConstraint->enableAngularMotor(Enable, Velocity, MotorPower);
}
bool BulletJoint::getHingeMotor() const
{
    return Type_ == JOINT_HINGE ? GetHingeConstraint->getEnableAngularMotor() : false;
}

f32 BulletJoint::getHingeAngle() const
{
    return Type_ == JOINT_HINGE ? GetHingeConstraint->getHingeAngle() * math::RAD : 0.0f;
}

void BulletJoint::setSliderLimit(bool Enable)
{
    if (Type_ == JOINT_SLIDER)
    {
        if (Enable)
        {
            GetSliderConstraint->setLowerLinLimit(SliderLimitMin_);
            GetSliderConstraint->setUpperLinLimit(SliderLimitMax_);
        }
        else
        {
            GetSliderConstraint->setLowerLinLimit(1);
            GetSliderConstraint->setUpperLinLimit(0);
        }
    }
}
bool BulletJoint::getSliderLimit() const
{
    if (Type_ == JOINT_SLIDER)
    {
        f32 MinLin = 0.0f, MaxLin = 0.0f;
        getSliderLimit(MinLin, MaxLin);
        return MinLin <= MaxLin;
    }
    return false;
}

void BulletJoint::setSliderLimit(f32 MinLinear, f32 MaxLinear, bool Enable)
{
    SliderLimitMin_ = MinLinear;
    SliderLimitMax_ = MaxLinear;
    
    setSliderLimit(Enable);
}
void BulletJoint::getSliderLimit(f32 &MinLinear, f32 &MaxLinear) const
{
    if (Type_ == JOINT_SLIDER)
    {
        MinLinear = GetSliderConstraint->getLowerLinLimit();
        MaxLinear = GetSliderConstraint->getUpperLinLimit();
    }
}

void BulletJoint::setSliderMotor(bool Enable, f32 Velocity, f32 MotorPower)
{
    if (Type_ == JOINT_SLIDER)
    {
        if (Enable)
        {
            GetSliderConstraint->setPoweredLinMotor(true);
            GetSliderConstraint->setTargetLinMotorVelocity(Velocity);
            GetSliderConstraint->setMaxLinMotorForce(MotorPower);
        }
        else
            GetSliderConstraint->setPoweredLinMotor(false);
    }
}
bool BulletJoint::getSliderMotor() const
{
    return Type_ == JOINT_SLIDER ? GetSliderConstraint->getPoweredLinMotor() : false;
}

f32 BulletJoint::getSliderLinear() const
{
    return Type_ == JOINT_SLIDER ? GetSliderConstraint->getLinearPos() : 0.0f;
}

#undef GetBallConstraint
#undef GetHingeConstraint
#undef GetSliderConstraint


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
