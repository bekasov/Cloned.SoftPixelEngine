/*
 * Physics simulator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_SYSTEM_H__
#define __SP_PHYSICS_SYSTEM_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PHYSICS


#include "Base/spDimensionVector3D.hpp"
#include "Framework/Physics/spPhysicsRigidBody.hpp"
#include "Framework/Physics/spPhysicsStaticObject.hpp"
#include "Framework/Physics/spPhysicsMaterial.hpp"
#include "Framework/Physics/spPhysicsJoint.hpp"

#include <boost/function.hpp>


namespace sp
{
namespace scene
{
    class Mesh;
}
namespace physics
{


typedef boost::function<void (const dim::vector3df &Point, const dim::vector3df &Normal, f32 Velocity)> PhysicsContactCallback;


//! Physics simulator types.
enum EPhysicsSimulators
{
    SIMULATOR_NEWTON,   //!< NewtonGameDynamics.
    SIMULATOR_PHYSX,    //!< NVIDIA PhysX.
    SIMULATOR_BULLET,   //!< Bullet Physics.
};


class SP_EXPORT PhysicsSimulator
{
    
    public:
        
        virtual ~PhysicsSimulator();
        
        //! Returns the version of the physics system.
        virtual io::stringc getVersion() const = 0;
        
        /**
        Updates the whole simulation. Call this in each frame of your game loop.
        \param StepTime: Specifies the speed of simulation for the current frame.
        */
        virtual void updateSimulation(const f32 StepTime = 1.0f / 60.0f) = 0;
        
        /**
        Sets the gravity for each object. You can also set the gravity individually for each rigid body.
        \param Gravity: Specifies the gravity vector which is to be set. By default (0 | -9.81 | 0).
        */
        virtual void setGravity(const dim::vector3df &Gravity);
        
        /*
        Creates a new physics material
        \param StaticFriction: Specifies the static friction factor in the range [0.0 .. 1.0].
        \param DynamicFriction: Specifies the dynamic friction factor in the range [0.0 .. 1.0].
        \param Restitution: Specifies the restitution factor in the range [0.0 .. 1.0].
        \return Pointer to the new PhysicsMaterial object.
        */
        virtual PhysicsMaterial* createMaterial(
            f32 StaticFriction = 0.5f, f32 DynamicFriction = 0.5f, f32 Restitution = 1.0f
        ) = 0;
        //! Deletes the specified physics material.
        virtual void deleteMaterial(PhysicsMaterial* Material);
        
        /**
        Creates a new physics static body. This is a none interactive environment object.
        Those objects represents the physics world.
        */
        virtual StaticPhysicsObject* createStaticObject(PhysicsMaterial* Material, scene::Mesh* Mesh) = 0;
        
        //! Deletes the specified static body.
        virtual void deleteStaticObject(StaticPhysicsObject* Object);
        
        /**
        Creates a new physics rigid body. This is an interactive physics object.
        \param Material: Specifies the physics material which is to be used for the rigid body.
        \param RootNode: Specifies the root scene node which is to be manipulated in its location and orientation.
        \param Type: Specifies the type of the body.
        \param Construct: Specifies the construction of the body. E.g. size, radius and/or height.
        */
        virtual RigidBody* createRigidBody(
            PhysicsMaterial* Material, const ERigidBodies Type, scene::SceneNode* RootNode,
            const SRigidBodyConstruction &Construct = SRigidBodyConstruction()
        ) = 0;
        
        /**
        Creates a new mesh physics rigid body. If you only want to use the mesh as the physics geometry you can
        change the root node by calling RigidBody::setRootNode".
        \param Material: Specifies the physics material which is to be used for the rigid body.
        \param MeshGeom: Specifies the root mesh node.
        \note Don't use to large mesh geometries for rigid bodies. This kind of physics simulation is very time consuming!
        */
        virtual RigidBody* createRigidBody(PhysicsMaterial* Material, scene::Mesh* Mesh) = 0;
        
        //! Delete the specified rigid body but not its root node.
        virtual void deleteRigidBody(RigidBody* Object);
        
        //! Creates a new physics joint (also called "constraint").
        virtual PhysicsJoint* createJoint(
            const EPhysicsJoints Type, PhysicsBaseObject* Object, const SPhysicsJointConstruct &Construct
        ) = 0;
        
        /**
        Adds a new joint connected child body to this rigid body.
        \param Child: Specifies the child rigid body which is to be added with a joint.
        If this parameter is 0 this rigid body is the child and will be appended to the physics world.
        \param Type: Specifies the type of the joint. E.g. when you want to have a physics door use the JOINT_HINGE type.
        \param PivotPoint: Specifies the global point where the joint is to be set.
        \return Pointer to the new BodyJoint object.
        */
        virtual PhysicsJoint* createJoint(
            const EPhysicsJoints Type, PhysicsBaseObject* ObjectA, PhysicsBaseObject* ObjectB, const SPhysicsJointConstruct &Construct
        ) = 0;
        
        virtual void deleteJoint(PhysicsJoint* Object);
        
        /**
        Deletes all physics objects
        \param RigidBodies: Specifies whether all rigid bodies are to be deleted.
        \param StaticObjects: Specifies whether all static objects are to be deleted.
        \param Joints: Specifies whether all joints are to be deleted.
        */
        virtual void clearScene(bool RigidBodies = true, bool StaticObjects = true, bool Joints = true);
        
        //! Sets the count of threads used for physics computations.
        virtual void setThreadCount(s32 Count);
        //! Returns the count of threads.
        virtual s32 getThreadCount() const;
        
        /**
        Sets the solver model. Use higher values if speed is more important than accuracy.
        \param Model: If 0 the accuracy is exact. If greater than 0 you can configure the accuracy if speed is more important.
        */
        virtual void setSolverModel(s32 Model);
        
        //! Sets the contact callback. This callback procedure will be called when a collision contact occured.
        static void setContactCallback(const PhysicsContactCallback &Callback);
        
        static PhysicsContactCallback getContactCallback();
        
        /* === Inline functions === */
        
        //! Returns the type of the simulator.
        inline EPhysicsSimulators getType() const
        {
            return Type_;
        }
        
        //! Returns the global gravity.
        inline dim::vector3df getGravity() const
        {
            return Gravity_;
        }
        
        inline std::list<RigidBody*> getRigidBodyList() const
        {
            return RigidBodyList_;
        }
        inline std::list<StaticPhysicsObject*> getStaticBodyList() const
        {
            return StaticBodyList_;
        }
        inline std::list<PhysicsMaterial*> getMaterialList() const
        {
            return MaterialList_;
        }
        inline std::list<PhysicsJoint*> getJointList() const
        {
            return JointList_;
        }
        
    protected:
        
        /* === Functions === */
        
        PhysicsSimulator(const EPhysicsSimulators Type);
        
        /* === Members === */
        
        EPhysicsSimulators Type_;
        
        std::list<RigidBody*> RigidBodyList_;
        std::list<StaticPhysicsObject*> StaticBodyList_;
        std::list<PhysicsMaterial*> MaterialList_;
        std::list<PhysicsJoint*> JointList_;
        
        dim::vector3df Gravity_;
        
        static PhysicsContactCallback ContactCallback_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
