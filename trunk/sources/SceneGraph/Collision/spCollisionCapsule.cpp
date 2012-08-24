/*
 * Collision sphere file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionCapsule.hpp"
#include "SceneGraph/Collision/spCollisionSphere.hpp"
#include "SceneGraph/Collision/spCollisionBox.hpp"
#include "SceneGraph/Collision/spCollisionPlane.hpp"
#include "SceneGraph/Collision/spCollisionMesh.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


CollisionCapsule::CollisionCapsule(
    CollisionMaterial* Material, SceneNode* Node, f32 Radius, f32 Height) :
    CollisionLineBased(Material, Node, COLLISION_CAPSULE, Radius, Height)
{
}
CollisionCapsule::~CollisionCapsule()
{
}

s32 CollisionCapsule::getSupportFlags() const
{
    return COLLISIONSUPPORT_SPHERE | COLLISIONSUPPORT_CAPSULE | COLLISIONSUPPORT_MESH;
}

bool CollisionCapsule::checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const
{
    dim::vector3df PointP, PointQ;
    const dim::line3df CapsuleLine(getLine());
    
    /* Make an intersection test with both lines */
    const f32 DistanceSq = math::CollisionLibrary::getLineLineDistanceSq(CapsuleLine, Line, PointP, PointQ);
    
    if (DistanceSq < math::Pow2(getRadius()))
    {
        Contact.Normal  = (PointQ - PointP).normalize();
        Contact.Point   = PointP + Contact.Normal * getRadius();
        Contact.Object  = this;
        return true;
    }
    
    return false;
}

bool CollisionCapsule::checkIntersection(const dim::line3df &Line, bool ExcludeCorners) const
{
    /* Make an intersection test with both lines */
    dim::vector3df PointP, PointQ;
    
    if (math::CollisionLibrary::getLineLineDistanceSq(getLine(), Line, PointP, PointQ) < math::Pow2(getRadius()))
    {
        if (ExcludeCorners)
        {
            dim::vector3df Dir(PointQ - PointP);
            Dir.setLength(getRadius());
            return checkCornerExlusion(Line, PointP + Dir);
        }
        return true;
    }
    
    return false;
}


/*
 * ======= Private: =======
 */

bool CollisionCapsule::checkCollisionToSphere(const CollisionSphere* Rival, SCollisionContact &Contact) const
{
    if (!Rival)
        return false;
    
    /* Store transformation */
    const dim::vector3df SpherePos(Rival->getPosition());
    const dim::line3df CapsuleLine(getLine());
    
    const f32 MaxRadius = Radius_ + Rival->getRadius();
    
    /* Get the closest point from this sphere to the capsule */
    const dim::vector3df ClosestPoint = CapsuleLine.getClosestPoint(SpherePos);
    
    /* Check if this object and the other collide with each other */
    if (math::getDistanceSq(SpherePos, ClosestPoint) < math::Pow2(MaxRadius))
        return setupCollisionContact(ClosestPoint, SpherePos, MaxRadius, Rival->getRadius(), Contact);
    
    return false;
}

bool CollisionCapsule::checkCollisionToCapsule(const CollisionCapsule* Rival, SCollisionContact &Contact) const
{
    if (!Rival)
        return false;
    
    /* Store transformation */
    const dim::vector3df SpherePos(Rival->getPosition());
    const dim::line3df CapsuleLine(getLine());
    
    const f32 MaxRadius = Radius_ + Rival->getRadius();
    
    /* Get the closest points between this and the rival capsule */
    dim::vector3df PointP, PointQ;
    const f32 DistanceSq = math::CollisionLibrary::getLineLineDistanceSq(getLine(), Rival->getLine(), PointP, PointQ);
    
    /* Check if this object and the other collide with each other */
    if (DistanceSq < math::Pow2(MaxRadius))
        return setupCollisionContact(PointP, PointQ, MaxRadius, Rival->getRadius(), Contact);
    
    return false;
}

bool CollisionCapsule::checkCollisionToPlane(const CollisionPlane* Rival, SCollisionContact &Contact) const
{
    if (!Rival)
        return false;
    
    /* Store transformation */
    const dim::line3df CapsuleLine(getLine());
    const dim::plane3df RivalPlane(
        Rival->getTransformation().getPositionRotationMatrix() * Rival->getPlane()
    );
    
    /* Check if this object and the other collide with each other */
    const f32 DistA = RivalPlane.getPointDistance(CapsuleLine.Start);
    const f32 DistB = RivalPlane.getPointDistance(CapsuleLine.End);
    
    if ( DistA > 0.0f && DistB > 0.0f && ( DistA < getRadius() || DistB < getRadius() ) )
    {
        Contact.Normal  = RivalPlane.Normal;
        Contact.Point   = Contact.Normal;
        
        if (DistA <= DistB)
        {
            Contact.Point *= -DistA;
            Contact.Point += CapsuleLine.Start;
            
            Contact.Impact = getRadius() - DistA;
        }
        else
        {
            Contact.Point *= -DistB;
            Contact.Point += CapsuleLine.End;
            
            Contact.Impact = getRadius() - DistB;
        }
        
        return true;
    }
    
    return false;
}

bool CollisionCapsule::checkCollisionToMesh(const CollisionMesh* Rival, SCollisionContact &Contact) const
{
    if (!Rival)
        return false;
    
    /* Check if rival mesh has a tree-hierarchy */
    KDTreeNode* RootTreeNode = Rival->getRootTreeNode();
    
    if (!RootTreeNode)
        return false;
    
    /* Store transformation */
    const dim::line3df CapsuleLine(getLine());
    const video::EFaceTypes CollFace(Rival->getCollFace());
    
    const dim::matrix4f RivalMat(Rival->getTransformation());
    const dim::matrix4f RivalMatInv(RivalMat.getInverse());
    
    const dim::line3df CapsuleLineInv(
        RivalMatInv * CapsuleLine.Start, RivalMatInv * CapsuleLine.End
    );
    
    f32 DistanceSq = math::Pow2(getRadius());
    SCollisionFace* ClosestFace = 0;
    dim::vector3df ClosestPoint;
    
    std::map<SCollisionFace*, bool> FaceMap;
    
    /* Get tree node list */
    std::list<const TreeNode*> TreeNodeList;
    
    RootTreeNode->findLeafList(
        TreeNodeList, CapsuleLineInv, (RivalMatInv.getScale() * getRadius()).getMax()
    );
    
    /* Check collision with triangles of each tree-node */
    foreach (const TreeNode* Node, TreeNodeList)
    {
        /* Get tree node data */
        CollisionMesh::TreeNodeDataType* TreeNodeData = static_cast<CollisionMesh::TreeNodeDataType*>(Node->getUserData());
        
        if (!TreeNodeData)
            continue;
        
        /* Check collision with each triangle */
        foreach (SCollisionFace* Face, *TreeNodeData)
        {
            /* Check for unique usage */
            if (FaceMap.find(Face) != FaceMap.end())
                continue;
            
            FaceMap[Face] = true;
            
            /* Check for face-culling */
            if (Face->isBackFaceCulling(CollFace, CapsuleLineInv))
                continue;
            
            /* Make sphere-triangle collision test */
            const dim::line3df CurClosestLine(
                math::CollisionLibrary::getClosestLine(RivalMat * Face->Triangle, CapsuleLine)
            );
            
            /* Check if this is a potentially new closest face */
            const f32 CurDistSq = math::getDistanceSq(CurClosestLine.Start, CurClosestLine.End);
            
            if (CurDistSq < DistanceSq)
            {
                /* Store link to new closest face */
                DistanceSq      = CurDistSq;
                ClosestPoint    = CurClosestLine.Start;
                ClosestFace     = Face;
            }
        }
    }
    
    /* Check if a collision has been detected */
    if (ClosestFace)
    {
        Contact.Normal  = (RivalMat * ClosestFace->Triangle).getNormal();
        Contact.Point   = ClosestPoint;
        Contact.Face    = ClosestFace;
        return true;
    }
    
    return false;
}

void CollisionCapsule::performCollisionResolvingToSphere(const CollisionSphere* Rival)
{
    SCollisionContact Contact;
    if (checkCollisionToSphere(Rival, Contact))
        performDetectedContact(Rival, Contact);
}

void CollisionCapsule::performCollisionResolvingToCapsule(const CollisionCapsule* Rival)
{
    SCollisionContact Contact;
    if (checkCollisionToCapsule(Rival, Contact))
        performDetectedContact(Rival, Contact);
}

void CollisionCapsule::performCollisionResolvingToPlane(const CollisionPlane* Rival)
{
    SCollisionContact Contact;
    if (checkCollisionToPlane(Rival, Contact))
        performDetectedContact(Rival, Contact);
}

void CollisionCapsule::performCollisionResolvingToMesh(const CollisionMesh* Rival)
{
    //todo
}

bool CollisionCapsule::setupCollisionContact(
    const dim::vector3df &PointP, const dim::vector3df &PointQ,
    f32 MaxRadius, f32 RivalRadius, SCollisionContact &Contact) const
{
    /* Compute normal and impact together to avoid calling square-root twice */
    Contact.Normal = PointP;
    Contact.Normal -= PointQ;
    
    Contact.Impact = Contact.Normal.getLength();
    
    if (Contact.Impact < math::ROUNDING_ERROR)
        return false;
    
    Contact.Normal *= (1.0f / Contact.Impact);
    Contact.Impact = MaxRadius - Contact.Impact;
    
    Contact.Point = Contact.Normal;
    Contact.Point *= RivalRadius;
    Contact.Point += PointQ;
    
    return true;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
