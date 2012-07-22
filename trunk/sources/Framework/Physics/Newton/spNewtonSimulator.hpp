/*
 * Newton simulator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_NEWTON_SIMULATOR_H__
#define __SP_PHYSICS_NEWTON_SIMULATOR_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NEWTON


#include "Base/spDimension.hpp"
#include "Framework/Physics/spPhysicsSimulator.hpp"

#include <boost/function.hpp>


class NewtonWorld;

namespace sp
{
namespace physics
{


//! Third party library extension: "Newton Game Dynamics" (v.2.0) physics simulation engine.
class SP_EXPORT NewtonSimulator : public PhysicsSimulator
{
    
    public:
        
        NewtonSimulator();
        virtual ~NewtonSimulator();
        
        virtual io::stringc getVersion() const;
        
        virtual void updateSimulation(const f32 StepTime = 1.0f / 60.0f);
        virtual void setGravity(const dim::vector3df &Gravity);
        
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
        
        virtual void setThreadCount(s32 Count);
        virtual s32 getThreadCount() const;
        
        virtual void setSolverModel(s32 Model);
        
        static void setContactCallback(const PhysicsContactCallback &Callback);
        
        //! Returns a pointer to the current NewtonWorld opject.
        static NewtonWorld* getNewtonWorld();
        
    protected:
        
        /* === Functions === */
        
        virtual RigidBody* allocRigidBody(const ERigidBodies Type, const SRigidBodyConstruction &Construct);
        virtual RigidBody* allocRigidBody(scene::Mesh* MeshGeom);
        
        /* === Members === */
        
        static NewtonWorld* NtWorld_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
