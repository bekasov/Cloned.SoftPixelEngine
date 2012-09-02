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
        
        void setLimit(bool Enable);
        bool getLimit() const;
        
        void setLimit(f32 Min, f32 Max, bool Enable = true);
        void getLimit(f32 &Min, f32 &Max) const;
        
        void setMotor(bool Enable, f32 MotorPower = 100.0f);
        bool getMotor() const;
        
        void runMotor(f32 Velocity);
        
        f32 getLinearValue() const;
        
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
