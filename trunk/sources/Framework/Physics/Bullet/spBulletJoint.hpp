/*
 * Bullet joint header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_BULLET_JOINT_H__
#define __SP_PHYSICS_BULLET_JOINT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_BULLET


#include "Framework/Physics/spPhysicsJoint.hpp"

#include <btBulletDynamicsCommon.h>


namespace sp
{
namespace physics
{


class SP_EXPORT BulletJoint : public PhysicsJoint
{
    
    public:
        
        BulletJoint(
            const EPhysicsJoints Type, PhysicsBaseObject* ObjectA,
            PhysicsBaseObject* ObjectB, const SPhysicsJointConstruct &Construct
        );
        BulletJoint(
            const EPhysicsJoints Type, PhysicsBaseObject* Object, const SPhysicsJointConstruct &Construct
        );
        ~BulletJoint();
        
        /* Functions */
        
        void setPosition(const dim::vector3df &Position);
        dim::vector3df getPosition() const;
        
        /* Hinge functions */
        
        void setHingeLimit(bool Enable);
        bool getHingeLimit() const;
        
        void setHingeLimit(f32 MinAngle, f32 MaxAngle, bool Enable = true);
        void getHingeLimit(f32 &MinAngle, f32 &MaxAngle) const;
        
        void setHingeMotor(bool Enable, f32 Velocity = 0.0f, f32 MotorPower = 100.0f);
        bool getHingeMotor() const;
        
        f32 getHingeAngle() const;
        
        /* Slider functions */
        
        void setSliderLimit(bool Enable);
        bool getSliderLimit() const;
        
        void setSliderLimit(f32 MinLinear, f32 MaxLinear, bool Enable = true);
        void getSliderLimit(f32 &MinLinear, f32 &MaxLinear) const;
        
        void setSliderMotor(bool Enable, f32 Velocity = 0.0f, f32 MotorPower = 100.0f);
        bool getSliderMotor() const;
        
        f32 getSliderLinear() const;
        
    private:
        
        friend class BulletSimulator;
        
        /* === Members === */
        
        btTypedConstraint* BtConstraint_;
        
        f32 HingeLimitMin_, HingeLimitMax_;
        f32 SliderLimitMin_, SliderLimitMax_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
