/*
 * PhysX base object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_PHYSX_BASEOBJECT_H__
#define __SP_PHYSICS_PHYSX_BASEOBJECT_H__


#include "Base/spStandard.hpp"

//#ifdef SP_COMPILE_WITH_PHYSX


#include "Base/spDimension.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "Framework/Physics/spPhysicsRigidBodyConstruct.hpp"
#include "Framework/Physics/spPhysicsBaseObject.hpp"
#include "Framework/Physics/PhysX/spPhysXCoreHeader.hpp"
#include "Framework/Physics/PhysX/spPhysXMaterial.hpp"


namespace sp
{
namespace physics
{


class SP_EXPORT PhysXBaseObject : virtual public PhysicsBaseObject
{
    
    public:
        
        virtual ~PhysXBaseObject();
        
        /* === Functions === */
        
        virtual void setTransformation(const dim::matrix4f &Transformation);
        virtual dim::matrix4f getTransformation() const;
        
    protected:
        
        friend class PhysXJoint;
        
        PhysXBaseObject(PhysXMaterial* Material);
        
        /* === Functions === */
        
        void addShape(const PxGeometry &Geometry);
        
        void createBox      (const SRigidBodyConstruction &Construct);
        void createSphere   (const SRigidBodyConstruction &Construct);
        void createCapsule  (const SRigidBodyConstruction &Construct);
        
        void createMesh(PxPhysics* PxDevice, PxCooking* PxCookDevice, scene::Mesh* Mesh);
        
        /* === Members === */
        
        PxRigidActor* PxBaseActor_;
        PxMaterial* PxMaterial_;
        std::list<PxShape*> Shapes_;
        
};


} // /namespace physics

} // /namespace sp


//#endif

#endif



// ================================================================================
