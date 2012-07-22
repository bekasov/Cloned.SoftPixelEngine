/*
 * Newton rigid body header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_NEWTON_RIGIDBODY_H__
#define __SP_PHYSICS_NEWTON_RIGIDBODY_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NEWTON


#include "Base/spDimension.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "Framework/Physics/spPhysicsRigidBody.hpp"
#include "Framework/Physics/Newton/spNewtonBaseObject.hpp"


namespace sp
{
namespace physics
{


class SP_EXPORT NewtonRigidBody : public RigidBody, public NewtonBaseObject
{
    
    public:
        
        NewtonRigidBody(const ERigidBodies Type, const SRigidBodyConstruction &Construct);
        NewtonRigidBody(scene::Mesh* MeshGeom);
        virtual ~NewtonRigidBody();
        
        /* === Functions === */
        
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
        
        /* === Functions === */
        
        void createBox();
        void createCone();
        void createSphere();
        void createCylinder();
        void createChamferCylinder();
        void createCapsule();
        
        void createMesh(scene::Mesh* MeshGeom);
        
        void setupCollision(NewtonCollision* NtCollision);
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
