/*
 * Physics rigid body file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/spPhysicsRigidBody.hpp"

#ifdef SP_COMPILE_WITH_PHYSICS


#include "Base/spMemoryManagement.hpp"


namespace sp
{
namespace physics
{


RigidBody::RigidBody(const ERigidBodies Type, const SRigidBodyConstruction &Construct) :
    DynamicPhysicsObject(           ),
    Type_               (Type       ),
    Construct_          (Construct  ),
    RootNode_           (0          )
{
}
RigidBody::~RigidBody()
{
}

void RigidBody::setRootNode(scene::SceneNode* Node)
{
    RootNode_ = Node;
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
