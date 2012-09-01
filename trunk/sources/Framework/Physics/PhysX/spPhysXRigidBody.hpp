/*
 * PhysX rigid body header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_PHYSX_RIGIDBODY_H__
#define __SP_PHYSICS_PHYSX_RIGIDBODY_H__


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


class SP_EXPORT PhysXRigidBody : public RigidBody, public PhysXBaseObject
{
    
    public:
        
        PhysXRigidBody(
            PxPhysics* PxDevice, PhysXMaterial* Material, const ERigidBodies Type,
            scene::SceneNode* RootNode, const SRigidBodyConstruction &Construct
        );
        virtual ~PhysXRigidBody();
        
        /* === Functions === */
        
        virtual void setMass(f32 Mass);
        
        virtual void setMassCenter(const dim::vector3df &LocalPoint);
        virtual dim::vector3df getMassCenter() const;
        
        virtual void addVelocity(const dim::vector3df &Direction);
        virtual void setVelocity(const dim::vector3df &Direction);
        virtual dim::vector3df getVelocity() const;
        
        virtual void addImpulse(const dim::vector3df &Direction, const dim::vector3df &PivotPoint);
        virtual void setForce(const dim::vector3df &Direction);
        
        virtual PhysicsJoint* addJoint(
            PhysicsBaseObject* Child, const EPhysicsJoints Type, const dim::vector3df &PivotPoint,
            const dim::vector3df &Direction = dim::vector3df(0, 1, 0), const dim::vector3df &SecondDirection = dim::vector3df(0, 1, 0)
        );
        
    protected:
        
        friend class PhysXSimulator;
        
        /* === Members === */
        
        PxRigidDynamic* PxActor_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
