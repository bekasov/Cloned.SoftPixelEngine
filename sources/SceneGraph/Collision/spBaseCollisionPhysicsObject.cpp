/*
 * Base collision physics object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spBaseCollisionPhysicsObject.hpp"
#include "SceneGraph/Collision/spCollisionNode.hpp"


namespace sp
{
namespace scene
{


BaseCollisionPhysicsObject::BaseCollisionPhysicsObject() :
    Mass_(1.0f)
{
}
BaseCollisionPhysicsObject::~BaseCollisionPhysicsObject()
{
}


/*
 * ======= Protected: =======
 */

void BaseCollisionPhysicsObject::applyForces(CollisionNode* Node)
{
    addForce(Gravity_ * Mass_);
    if (Node)
        Node->translate(Force_);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
