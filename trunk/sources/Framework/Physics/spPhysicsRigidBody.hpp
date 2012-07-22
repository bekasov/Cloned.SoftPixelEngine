/*
 * Physics rigid body header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_RIGIDBODY_H__
#define __SP_PHYSICS_RIGIDBODY_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PHYSICS


#include "SceneGraph/spSceneMesh.hpp"
#include "Framework/Physics/spPhysicsRigidBodyConstruct.hpp"
#include "Framework/Physics/spPhysicsDynamicObject.hpp"


namespace sp
{
namespace physics
{


class SP_EXPORT RigidBody : public DynamicPhysicsObject
{
    
    public:
        
        virtual ~RigidBody();
        
        /* Functions */
        
        //! Sets the root node which is to be located and oriented by the rigid body.
        virtual void setRootNode(scene::SceneNode* Node);
        
        /* Inline functions */
        
        //! Returns the rigid body's type.
        inline ERigidBodies getType() const
        {
            return Type_;
        }
        //! Returns the root node.
        inline scene::SceneNode* getRootNode() const
        {
            return RootNode_;
        }
        
    protected:
        
        friend class BodyJoint;
        
        /* Functions */
        
        RigidBody(const ERigidBodies Type, const SRigidBodyConstruction &Construct);
        
        /* Members */
        
        ERigidBodies Type_;
        SRigidBodyConstruction Construct_;
        
        scene::SceneNode* RootNode_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
