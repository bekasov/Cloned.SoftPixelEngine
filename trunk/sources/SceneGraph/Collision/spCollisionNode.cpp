/*
 * Collision node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionNode.hpp"
#include "SceneGraph/Collision/spCollisionSphere.hpp"
#include "SceneGraph/Collision/spCollisionCapsule.hpp"
#include "SceneGraph/Collision/spCollisionCylinder.hpp"
#include "SceneGraph/Collision/spCollisionCone.hpp"
#include "SceneGraph/Collision/spCollisionBox.hpp"
#include "SceneGraph/Collision/spCollisionPlane.hpp"
#include "SceneGraph/Collision/spCollisionMesh.hpp"
#include "SceneGraph/Collision/spCollisionMaterial.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


CollisionNode::CollisionNode(
    CollisionMaterial* Material, SceneNode* Node, const ECollisionModels Type) :
    BaseObject      (                   ),
    Type_           (Type               ),
    Flags_          (COLLISIONFLAG_FULL ),
    Node_           (Node               ),
    Material_       (Material           ),
    UseOffsetTrans_ (false              )
{
    if (!Node_)
        throw io::stringc("Collision node must be linked to a valid scene node");
    
    if (Material_)
        Material_->addCollisionNode(this);
    
    updateTransformation();
    updatePrevPosition();
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
bool CollisionNode::checkIntersection(const dim::line3df &Line, bool ExcludeCorners) const
{
    return false; // do nothing
}

bool CollisionNode::checkCollision(const CollisionNode* Rival, SCollisionContact &Contact) const
{
    if (Rival)
    {
        switch (Rival->getType())
        {
            case COLLISION_SPHERE:      return checkCollisionToSphere   (static_cast<const CollisionSphere*     >(Rival), Contact);
            case COLLISION_CAPSULE:     return checkCollisionToCapsule  (static_cast<const CollisionCapsule*    >(Rival), Contact);
            case COLLISION_CYLINDER:    return checkCollisionToCylinder (static_cast<const CollisionCylinder*   >(Rival), Contact);
            case COLLISION_CONE:        return checkCollisionToCone     (static_cast<const CollisionCone*       >(Rival), Contact);
            case COLLISION_BOX:         return checkCollisionToBox      (static_cast<const CollisionBox*        >(Rival), Contact);
            case COLLISION_PLANE:       return checkCollisionToPlane    (static_cast<const CollisionPlane*      >(Rival), Contact);
            case COLLISION_MESH:        return checkCollisionToMesh     (static_cast<const CollisionMesh*       >(Rival), Contact);
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
        switch (Rival->getType())
        {
            case COLLISION_SPHERE:      return checkAnyCollisionToSphere    (static_cast<const CollisionSphere*     >(Rival));
            case COLLISION_CAPSULE:     return checkAnyCollisionToCapsule   (static_cast<const CollisionCapsule*    >(Rival));
            case COLLISION_CYLINDER:    return checkAnyCollisionToCylinder  (static_cast<const CollisionCylinder*   >(Rival));
            case COLLISION_CONE:        return checkAnyCollisionToCone      (static_cast<const CollisionCone*       >(Rival));
            case COLLISION_BOX:         return checkAnyCollisionToBox       (static_cast<const CollisionBox*        >(Rival));
            case COLLISION_PLANE:       return checkAnyCollisionToPlane     (static_cast<const CollisionPlane*      >(Rival));
            case COLLISION_MESH:        return checkAnyCollisionToMesh      (static_cast<const CollisionMesh*       >(Rival));
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
            case COLLISION_SPHERE:      performCollisionResolvingToSphere   (static_cast<const CollisionSphere*     >(Rival)); break;
            case COLLISION_CAPSULE:     performCollisionResolvingToCapsule  (static_cast<const CollisionCapsule*    >(Rival)); break;
            case COLLISION_CYLINDER:    performCollisionResolvingToCylinder (static_cast<const CollisionCylinder*   >(Rival)); break;
            case COLLISION_CONE:        performCollisionResolvingToCone     (static_cast<const CollisionCone*       >(Rival)); break;
            case COLLISION_BOX:         performCollisionResolvingToBox      (static_cast<const CollisionBox*        >(Rival)); break;
            case COLLISION_PLANE:       performCollisionResolvingToPlane    (static_cast<const CollisionPlane*      >(Rival)); break;
            case COLLISION_MESH:        performCollisionResolvingToMesh     (static_cast<const CollisionMesh*       >(Rival)); break;
            default:
                break;
        }
    }
}

void CollisionNode::updateTransformation()
{
    /* Update scene-node's global transformation */
    Trans_ = Node_->getTransformMatrix(true);
    NodePosition_ = Trans_.getPosition();
    
    /* Update offset transformation is enabled */
    if (UseOffsetTrans_)
        Trans_ *= OffsetTrans_;
    
    /* Store inverse transformation */
    Trans_.getInverse(InvTrans_);
}

void CollisionNode::setPosition(const dim::vector3df &Position, bool UpdatePrevPosition)
{
    Node_->setPosition(Position, true);
    updateTransformation();
    if (UpdatePrevPosition)
        updatePrevPosition();
}

void CollisionNode::setOffset(const dim::matrix4f &Matrix, bool Enable)
{
    OffsetTrans_    = Matrix;
    UseOffsetTrans_ = Enable;
    
    updateTransformation();
    updatePrevPosition();
}

void CollisionNode::updateCollisions()
{
    if (!(getFlags() & COLLISIONFLAG_DETECTION) || getSupportFlags() == COLLISIONSUPPORT_NONE || !Material_)
        return;
    
    /* Check for movement tolerance */
    dim::vector3df MoveDir(getNodePosition());
    MoveDir -= getPrevPosition();
    
    f32 Movement = MoveDir.getLengthSq();
    
    if (!(getFlags() & COLLISIONFLAG_PERMANENT_UPDATE) && Movement <= math::ROUNDING_ERROR)
        return;
    
    const f32 MaxMovement = getMaxMovement();
    
    if (Movement > math::Pow2(MaxMovement))
    {
        /* Adjust movement and direction */
        Movement = sqrt(Movement);
        
        MoveDir /= Movement;
        MoveDir *= MaxMovement;
        
        setPosition(getPrevPosition(), false);
        
        /* Perform collision resolving in several steps */
        do
        {
            translate(MoveDir);
            
            /* Perform simple collision resolving */
            foreach (const CollisionMaterial* RivalMaterial, Material_->RivalCollMaterials_)
            {
                foreach (const CollisionNode* Rival, RivalMaterial->CollNodes_)
                    performCollisionResolving(Rival);
            }
            
            /* Boost movement */
            Movement -= MaxMovement;
            if (Movement < MaxMovement)
                MoveDir.setLength(MaxMovement - Movement);
        }
        while (Movement > -math::ROUNDING_ERROR);
    }
    else
    {
        /* Perform simple collision resolving */
        foreach (const CollisionMaterial* RivalMaterial, Material_->RivalCollMaterials_)
        {
            foreach (const CollisionNode* Rival, RivalMaterial->CollNodes_)
                performCollisionResolving(Rival);
        }
    }
    
    updatePrevPosition();
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
bool CollisionNode::checkCollisionToCylinder(const CollisionCylinder* Rival, SCollisionContact &Contact) const
{
    return false; // do nothing
}
bool CollisionNode::checkCollisionToCone(const CollisionCone* Rival, SCollisionContact &Contact) const
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

bool CollisionNode::checkAnyCollisionToSphere(const CollisionSphere* Rival) const
{
    SCollisionContact Unused;
    return checkCollisionToSphere(Rival, Unused);
}
bool CollisionNode::checkAnyCollisionToCapsule(const CollisionCapsule* Rival) const
{
    SCollisionContact Unused;
    return checkCollisionToCapsule(Rival, Unused);
}
bool CollisionNode::checkAnyCollisionToCylinder(const CollisionCylinder* Rival) const
{
    SCollisionContact Unused;
    return checkCollisionToCylinder(Rival, Unused);
}
bool CollisionNode::checkAnyCollisionToCone(const CollisionCone* Rival) const
{
    SCollisionContact Unused;
    return checkCollisionToCone(Rival, Unused);
}
bool CollisionNode::checkAnyCollisionToBox(const CollisionBox* Rival) const
{
    SCollisionContact Unused;
    return checkCollisionToBox(Rival, Unused);
}
bool CollisionNode::checkAnyCollisionToPlane(const CollisionPlane* Rival) const
{
    SCollisionContact Unused;
    return checkCollisionToPlane(Rival, Unused);
}
bool CollisionNode::checkAnyCollisionToMesh(const CollisionMesh* Rival) const
{
    SCollisionContact Unused;
    return checkCollisionToMesh(Rival, Unused);
}

void CollisionNode::performCollisionResolvingToSphere(const CollisionSphere* Rival)
{
    // do nothing
}
void CollisionNode::performCollisionResolvingToCapsule(const CollisionCapsule* Rival)
{
    // do nothing
}
void CollisionNode::performCollisionResolvingToCylinder(const CollisionCylinder* Rival)
{
    // do nothing
}
void CollisionNode::performCollisionResolvingToCone(const CollisionCone* Rival)
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

bool CollisionNode::notifyCollisionContact(const CollisionNode* Rival, const SCollisionContact &Contact)
{
    /* Collision contact callback */
    if (Material_ && Material_->CollContactCallback_)
        return Material_->CollContactCallback_(Material_, this, Rival, Contact);
    return true;
}

bool CollisionNode::performDetectedContact(const CollisionNode* Rival, const SCollisionContact &Contact)
{
    /* Allways notify on collision detection */
    if (!notifyCollisionContact(Rival, Contact))
        return false;
    
    /* Only set the new position if collision-resolving is enabled */
    if (getFlags() & COLLISIONFLAG_RESOLVE)
    {
        translate(Contact.Normal * (Contact.Impact + math::ROUNDING_ERROR));
        return true;
    }
    
    return false;
}

bool CollisionNode::checkCornerExlusion(const dim::line3df &Line, const dim::vector3df &Point) const
{
    return (
        math::getDistanceSq(Line.Start, Point) > math::ROUNDING_ERROR &&
        math::getDistanceSq(Line.End, Point) > math::ROUNDING_ERROR
    );
}

void CollisionNode::updatePrevPosition()
{
    PrevPosition_ = Node_->getPosition(true);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
