/*
 * Collision node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionNode.hpp"
#include "SceneGraph/Collision/spCollisionSphere.hpp"
#include "SceneGraph/Collision/spCollisionCapsule.hpp"
#include "SceneGraph/Collision/spCollisionBox.hpp"
#include "SceneGraph/Collision/spCollisionPlane.hpp"
#include "SceneGraph/Collision/spCollisionMesh.hpp"
#include "SceneGraph/Collision/spCollisionMaterial.hpp"


namespace sp
{
namespace scene
{


CollisionNode::CollisionNode(
    CollisionMaterial* Material, SceneNode* Node, const ECollisionModels Type) :
    BaseObject  (                   ),
    Type_       (Type               ),
    Flags_      (COLLISIONFLAG_BOTH ),
    Node_       (Node               ),
    Material_   (Material           )
{
    if (!Node_)
        throw "Collision node must be linked to a valid scene node";
    if (Material_)
        Material_->addCollisionNode(this);
}
CollisionNode::~CollisionNode()
{
    if (Material_)
        Material_->removeCollisionNode(this);
}

void CollisionNode::setMaterial(CollisionMaterial* Material)
{
    if (Material_ != Material)
    {
        if (Material_)
            Material_->removeCollisionNode(this);
        
        Material_ = Material;
        
        if (Material_)
            Material_->addCollisionNode(this);
    }
}

void CollisionNode::findIntersections(const dim::line3df &Line, std::list<SIntersectionContact> &ContactList) const
{
    SIntersectionContact Contact;
    if (checkIntersection(Line, Contact))
        ContactList.push_back(Contact);
}

bool CollisionNode::checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const
{
    return false; // do nothing
}
bool CollisionNode::checkIntersection(const dim::line3df &Line) const
{
    return false; // do nothing
}

bool CollisionNode::checkCollision(const CollisionNode* Rival, SCollisionContact &Contact) const
{
    if (Rival)
    {
        switch (Rival->getType())
        {
            case COLLISION_SPHERE:
                return checkCollisionToSphere(static_cast<const CollisionSphere*>(Rival), Contact);
            case COLLISION_CAPSULE:
                return checkCollisionToCapsule(static_cast<const CollisionCapsule*>(Rival), Contact);
            case COLLISION_BOX:
                return checkCollisionToBox(static_cast<const CollisionBox*>(Rival), Contact);
            case COLLISION_PLANE:
                return checkCollisionToPlane(static_cast<const CollisionPlane*>(Rival), Contact);
            case COLLISION_MESH:
                return checkCollisionToMesh(static_cast<const CollisionMesh*>(Rival), Contact);
            default:
                break;
        }
    }
    return false;
}

bool CollisionNode::checkCollision(const CollisionNode* Rival) const
{
    if (Rival)
    {
        SCollisionContact Contact;
        
        switch (Rival->getType())
        {
            case COLLISION_SPHERE:
                return checkCollisionToSphere(static_cast<const CollisionSphere*>(Rival), Contact);
            case COLLISION_CAPSULE:
                return checkCollisionToCapsule(static_cast<const CollisionCapsule*>(Rival), Contact);
            case COLLISION_BOX:
                return checkCollisionToBox(static_cast<const CollisionBox*>(Rival), Contact);
            case COLLISION_PLANE:
                return checkCollisionToPlane(static_cast<const CollisionPlane*>(Rival), Contact);
            case COLLISION_MESH:
                return checkAnyCollisionToMesh(static_cast<const CollisionMesh*>(Rival));
            default:
                break;
        }
    }
    return false;
}

void CollisionNode::performCollisionResolving(const CollisionNode* Rival)
{
    if (Rival)
    {
        switch (Rival->getType())
        {
            case COLLISION_SPHERE:
                performCollisionResolvingToSphere(static_cast<const CollisionSphere*>(Rival));
                break;
            case COLLISION_CAPSULE:
                performCollisionResolvingToCapsule(static_cast<const CollisionCapsule*>(Rival));
                break;
            case COLLISION_BOX:
                performCollisionResolvingToBox(static_cast<const CollisionBox*>(Rival));
                break;
            case COLLISION_PLANE:
                performCollisionResolvingToPlane(static_cast<const CollisionPlane*>(Rival));
                break;
            case COLLISION_MESH:
                performCollisionResolvingToMesh(static_cast<const CollisionMesh*>(Rival));
                break;
            default:
                break;
        }
    }
}


/*
 * ======= Protected: =======
 */

bool CollisionNode::checkCollisionToSphere(const CollisionSphere* Rival, SCollisionContact &Contact) const
{
    return false; // do nothing
}
bool CollisionNode::checkCollisionToCapsule(const CollisionCapsule* Rival, SCollisionContact &Contact) const
{
    return false; // do nothing
}
bool CollisionNode::checkCollisionToBox(const CollisionBox* Rival, SCollisionContact &Contact) const
{
    return false; // do nothing
}
bool CollisionNode::checkCollisionToPlane(const CollisionPlane* Rival, SCollisionContact &Contact) const
{
    return false; // do nothing
}
bool CollisionNode::checkCollisionToMesh(const CollisionMesh* Rival, SCollisionContact &Contact) const
{
    return false; // do nothing
}

bool CollisionNode::checkAnyCollisionToMesh(const CollisionMesh* Rival) const
{
    return false; // do nothing
}

void CollisionNode::performCollisionResolvingToSphere(const CollisionSphere* Rival)
{
    // do nothing
}
void CollisionNode::performCollisionResolvingToCapsule(const CollisionCapsule* Rival)
{
    // do nothing
}
void CollisionNode::performCollisionResolvingToBox(const CollisionBox* Rival)
{
    // do nothing
}
void CollisionNode::performCollisionResolvingToPlane(const CollisionPlane* Rival)
{
    // do nothing
}
void CollisionNode::performCollisionResolvingToMesh(const CollisionMesh* Rival)
{
    // do nothing
}

void CollisionNode::notifyCollisionContact(const CollisionNode* Rival, const SCollisionContact &Contact)
{
    /* Collision contact callback */
    if (Material_ && Material_->CollContactCallback_)
        Material_->CollContactCallback_(Material_, this, Rival, Contact);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
