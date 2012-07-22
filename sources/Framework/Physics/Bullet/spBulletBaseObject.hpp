/*
 * Bullet base object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_PHYSX_BASEOBJECT_H__
#define __SP_PHYSICS_PHYSX_BASEOBJECT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_BULLET


#include "Base/spDimension.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "Framework/Physics/Bullet/spBulletMotionState.hpp"
#include "Framework/Physics/spPhysicsRigidBodyConstruct.hpp"
#include "Framework/Physics/spPhysicsBaseObject.hpp"

#include <btBulletDynamicsCommon.h>


namespace sp
{
namespace physics
{


class SP_EXPORT BulletBaseObject : virtual public PhysicsBaseObject
{
    
    public:
        
        virtual ~BulletBaseObject();
        
        /* Functions */
        
        virtual void setTransformation(const dim::matrix4f &Transformation);
        virtual dim::matrix4f getTransformation() const;
        
    protected:
        
        friend class BulletSimulator;
        friend class BulletJoint;
        
        BulletBaseObject();
        
        /* Functions */
        
        btCollisionShape* createShape(const ERigidBodies Type, const SRigidBodyConstruction &Construct);
        btCollisionShape* createShape(scene::Mesh* Mesh);
        
        void createBody(
            f32 Mass, bool isDynamic, scene::SceneNode* Node,
            const ERigidBodies Type, const SRigidBodyConstruction &Construct
        );
        
        /* Members */
        
        btRigidBody* BtBody_;
        btCollisionShape* BtShape_;
        btTriangleMesh* BtMeshData_;
        
        BulletMotionState* MotionState_;
        
        dim::vector3df LocalInertia_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
