/*
 * Newton joint header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_NEWTON_JOINT_H__
#define __SP_PHYSICS_NEWTON_JOINT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NEWTON


#include "Framework/Physics/spPhysicsJoint.hpp"


struct NewtonJoint;

namespace sp
{
namespace physics
{


class SP_EXPORT NewtonDynamicsJoint : public PhysicsJoint
{
    
    public:
        
        NewtonDynamicsJoint(
            const EPhysicsJoints Type, PhysicsBaseObject* ObjectA,
            PhysicsBaseObject* ObjectB, const SPhysicsJointConstruct &Construct
        );
        virtual ~NewtonDynamicsJoint();
        
        /* === Functions === */
        
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
        
    protected:
        
        /* === Members === */
        
        NewtonJoint* NtJoint_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
