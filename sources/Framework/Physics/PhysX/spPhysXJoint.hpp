/*
 * PhysX joint header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_PHYSX_JOINT_H__
#define __SP_PHYSICS_PHYSX_JOINT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PHYSX


#include "Base/spDimension.hpp"
#include "Framework/Physics/spPhysicsRigidBody.hpp"
#include "Framework/Physics/spPhysicsJoint.hpp"
#include "Framework/Physics/PhysX/spPhysXBaseObject.hpp"


namespace sp
{
namespace physics
{


class PhysXRigidBody;

class SP_EXPORT PhysXJoint : public PhysicsJoint
{
    
    public:
        
        PhysXJoint(
            PxPhysics* PxDevice,
            const EPhysicsJoints Type, PhysXRigidBody* ObjectA,
            PhysicsBaseObject* ObjectB, const SPhysicsJointConstruct &Construct
        );
        ~PhysXJoint();
        
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
        
    private:
        
        /* === Members === */
        
        PxJoint* PxJoint_;
        
        PxRigidDynamic* PxRigidBody0_;
        PxRigidDynamic* PxRigidBody1_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
