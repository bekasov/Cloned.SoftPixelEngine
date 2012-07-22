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


class NewtonJoint;

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
        
    protected:
        
        /* Members */
        
        NewtonJoint* NtJoint_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
