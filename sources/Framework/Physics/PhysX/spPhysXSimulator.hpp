/*
 * PhysX simulator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_PHYSX_SIMULATOR_H__
#define __SP_PHYSICS_PHYSX_SIMULATOR_H__


#include "Base/spStandard.hpp"

//#ifdef SP_COMPILE_WITH_PHYSX


#include "Base/spDimension.hpp"
#include "Framework/Physics/PhysX/spPhysXCoreHeader.hpp"
#include "Framework/Physics/spPhysicsSimulator.hpp"


namespace sp
{
namespace physics
{


class SP_EXPORT PhysXSimulator : public PhysicsSimulator
{
    
    public:
        
        PhysXSimulator();
        virtual ~PhysXSimulator();
        
        virtual io::stringc getVersion() const;
        
        virtual void updateSimulation(const f32 StepTime = 1.0f / 60.0f);
        
        virtual PhysicsMaterial* createMaterial(
            f32 StaticFriction = 0.5f, f32 DynamicFriction = 0.5f, f32 Restitution = 0.3f
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
        
    protected:
        
        /* === Functions === */
        
        PxScene* createScene();
        
        /* === Templates === */
        
        template <typename T> inline void releaseObject(T* &Object)
        {
            if (Object)
            {
                Object->release();
                Object = 0;
            }
        }
        
        inline PxVec3 convert(const dim::vector3df &Vec) const
        {
            return PxVec3(Vec.X, Vec.Y, Vec.Z);
        }
        
        static scene::Transformation convert(const PxTransform &Transform);
        
        /* === Members === */
        
        PxPhysics* PxDevice_;
        PxFoundation* PxFoundation_;
        PxProfileZoneManager* PxProfile_;
        PxCooking* PxCooking_;
        
        #ifdef _DEBUG
        PVD::PvdConnection* PxDebuggerConnection_;
        #endif
        
        PxScene* PxScene_;
        
};


} // /namespace physics

} // /namespace sp


//#endif

#endif



// ================================================================================
