/*
 * Bullet rigid body header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_BULLET_RIGIDBODY_H__
#define __SP_PHYSICS_BULLET_RIGIDBODY_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_BULLET


#include "Base/spDimension.hpp"
#include "Framework/Physics/spPhysicsRigidBody.hpp"
#include "Framework/Physics/Bullet/spBulletBaseObject.hpp"


namespace sp
{
namespace physics
{


class SP_EXPORT BulletRigidBody : public RigidBody, public BulletBaseObject
{
    
    public:
        
        BulletRigidBody(
            const ERigidBodies Type, scene::SceneNode* RootNode, const SRigidBodyConstruction &Construct
        );
        virtual ~BulletRigidBody();
        
        /* === Functions === */
        
        virtual void setGravity(const dim::vector3df &Gravity);
        virtual void setMass(f32 Mass);
        virtual void setAutoSleep(bool Enable);
        
        virtual void setMassCenter(const dim::vector3df &LocalPoint);
        virtual dim::vector3df getMassCenter() const;
        
        virtual void addVelocity(const dim::vector3df &Direction);
        virtual void setVelocity(const dim::vector3df &Direction);
        virtual dim::vector3df getVelocity() const;
        
        virtual void addImpulse(const dim::vector3df &Direction, const dim::vector3df &PivotPoint);
        virtual void setForce(const dim::vector3df &Direction);
        
    protected:
        
        /* === Members === */
        
        
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
