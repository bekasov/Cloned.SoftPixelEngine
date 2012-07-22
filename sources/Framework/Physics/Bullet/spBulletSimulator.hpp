/*
 * Bullet simulator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_BULLET_SIMULATOR_H__
#define __SP_PHYSICS_BULLET_SIMULATOR_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_BULLET


#include "Base/spDimension.hpp"
#include "Framework/Physics/spPhysicsSimulator.hpp"

#include <btBulletDynamicsCommon.h>


namespace sp
{
namespace physics
{


class SP_EXPORT BulletSimulator : public PhysicsSimulator
{
    
    public:
        
        BulletSimulator();
        virtual ~BulletSimulator();
        
        virtual io::stringc getVersion() const;
        
        virtual void updateSimulation(const f32 StepTime = 1.0f / 60.0f);
        
        virtual PhysicsMaterial* createMaterial(
            f32 StaticFriction = 0.5f, f32 DynamicFriction = 0.5f, f32 Restitution = 1.0f
        );
        
        virtual StaticPhysicsObject* createStaticObject(PhysicsMaterial* Material, scene::Mesh* MeshGeom);
        
        virtual RigidBody* createRigidBody(
            PhysicsMaterial* Material, const ERigidBodies Type, scene::SceneNode* RootNode,
            const SRigidBodyConstruction &Construct = SRigidBodyConstruction()
        );
        virtual RigidBody* createRigidBody(PhysicsMaterial* Material, scene::Mesh* Mesh);
        
        virtual PhysicsJoint* createJoint(
            const EPhysicsJoints Type, PhysicsBaseObject* Object, const SPhysicsJointConstruct &Construct
        );
        virtual PhysicsJoint* createJoint(
            const EPhysicsJoints Type, PhysicsBaseObject* ObjectA, PhysicsBaseObject* ObjectB, const SPhysicsJointConstruct &Construct
        );
        
        virtual void deleteJoint(PhysicsJoint* Object);
        
        virtual void clearScene(bool RigidBodies = true, bool StaticObjects = true, bool Joints = true);
        
    protected:
        
        /* === Functions === */
        
        
        /* === Members === */
        
        btBroadphaseInterface*                  BtBroadPhase_;
        btDefaultCollisionConfiguration*        BtCollisionConfig_;
        btCollisionDispatcher*                  BtDispatcher_;
        btSequentialImpulseConstraintSolver*    BtSolver_;
        btDiscreteDynamicsWorld*                BtWorld_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
