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
    return COLLISIONSUPPORT_SPHERE | COLLISIONSUPPORT_CAPSULE | COLLISIONSUPPORT_BOX | COLLISIONSUPPORT_PLANE | COLLISIONSUPPORT_MESH;
}

bool CollisionCapsule::checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const
{
    dim::vector3df PointP, PointQ;
    const dim::line3df CapsuleLine(getLine());
    
    /* Make an intersection test with both lines */
    const f32 DistanceSq = math::CollisionLibrary::getLineLineDistanceSq(CapsuleLine, Line, PointP, PointQ);
    
    if (DistanceSq < math::pow2(getRadius()))
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
    
    if (math::CollisionLibrary::getLineLineDistanceSq(getLine(), Line, PointP, PointQ) < math::pow2(getRadius()))
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
    if (math::getDistanceSq(SpherePos, ClosestPoint) < math::pow2(MaxRadius))
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
    if (DistanceSq < math::pow2(MaxRadius))
        return setupCollisionContact(PointP, PointQ, MaxRadius, Rival->getRadius(), Contact);
    
    return false;
}

bool CollisionCapsule::checkCollisionToBox(const CollisionBox* Rival, SCollisionContact &Contact) const
{
    if (!Rival)
        return false;
    
    /* Store transformation */
    const dim::matrix4f Mat(Rival->getTransformation().getPositionRotationMatrix());
    const dim::matrix4f InvMat(Mat.getInverse());
    
    const dim::aabbox3df Box(Rival->getBox().getScaled(Rival->getScale()));
    const dim::line3df CapsuleLine(getLine());
    const dim::line3df CapsuleLineInv(
        InvMat * CapsuleLine.Start, InvMat * CapsuleLine.End
    );
    
    /* Get the closest point from this capsule and the box */
    if (Box.isPointInside(CapsuleLineInv.Start) || Box.isPointInside(CapsuleLineInv.End))
        return false;
    
    const dim::line3df Line = math::CollisionLibrary::getClosestLine(Box, CapsuleLineInv);
    
    /* Check if this object and the other collide with each other */
    if (math::getDistanceSq(Line.Start, Line.End) < math::pow2(getRadius()))
    {
        Contact.Point = Mat * Line.Start;
        
        /* Compute normal and impact together to avoid calling square-root twice */
        Contact.Normal = (Mat * Line.End) - Contact.Point;
        Contact.Impact = Contact.Normal.getLength();
        
        if (Contact.Impact < math::ROUNDING_ERROR)
            return false;
        
        Contact.Normal *= (1.0f / Contact.Impact);
        Contact.Impact = getRadius() - Contact.Impact;
        
        return true;
    }
    
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
    
    f32 DistanceSq = math::pow2(getRadius());
    SCollisionFace* ClosestFace = 0;
    dim::vector3df ClosestPoint;
    
    #ifndef _DEB_NEW_KDTREE_
    std::map<SCollisionFace*, bool> FaceMap;
    #endif
    
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
        #ifndef _DEB_NEW_KDTREE_
        foreach (SCollisionFace* Face, *TreeNodeData)
        #else
        foreach (SCollisionFace &NodeFace, *TreeNodeData)
        #endif
        {
            #ifndef _DEB_NEW_KDTREE_
            /* Check for unique usage */
            if (FaceMap.find(Face) != FaceMap.end())
                continue;
            
            FaceMap[Face] = true;
            #else
            SCollisionFace* Face = &NodeFace;
            #endif
            
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

bool CollisionCapsule::checkAnyCollisionToMesh(const CollisionMesh* Rival) const
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
    
    const f32 RadiusSq = math::pow2(getRadius());
    
    #ifndef _DEB_NEW_KDTREE_
    std::map<SCollisionFace*, bool> FaceMap;
    #endif
    
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
        #ifndef _DEB_NEW_KDTREE_
        foreach (SCollisionFace* Face, *TreeNodeData)
        #else
        foreach (SCollisionFace &NodeFace, *TreeNodeData)
        #endif
        {
            #ifndef _DEB_NEW_KDTREE_
            /* Check for unique usage */
            if (FaceMap.find(Face) != FaceMap.end())
                continue;
            
            FaceMap[Face] = true;
            #else
            SCollisionFace* Face = &NodeFace;
            #endif
            
            /* Check for face-culling */
            if (Face->isBackFaceCulling(CollFace, CapsuleLineInv))
                continue;
            
            /* Make sphere-triangle collision test */
            const dim::line3df CurClosestLine(
                math::CollisionLibrary::getClosestLine(Face->Triangle, CapsuleLineInv)
            );
            
            /* Check if the first collision has been detected and return on succeed */
            if (math::getDistanceSq(CurClosestLine.Start, CurClosestLine.End) < RadiusSq)
                return true;
        }
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

void CollisionCapsule::performCollisionResolvingToBox(const CollisionBox* Rival)
{
    SCollisionContact Contact;
    if (checkCollisionToBox(Rival, Contact))
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
    if (!Rival)
        return;
    
    /* Check if rival mesh has a tree-hierarchy */
    KDTreeNode* RootTreeNode = Rival->getRootTreeNode();
    
    if (!RootTreeNode)
        return;
    
    /* Store transformation */
    const video::EFaceTypes CollFace(Rival->getCollFace());
    
    const dim::matrix4f RivalMat(Rival->getTransformation());
    const dim::matrix4f RivalMatInv(RivalMat.getInverse());
    
    dim::line3df CapsuleLine(getLine());
    dim::line3df CapsuleLineInv(
        RivalMatInv * CapsuleLine.Start, RivalMatInv * CapsuleLine.End
    );
    
    dim::line3df ClosestLine;
    const f32 RadiusSq = math::pow2(getRadius());
    
    #ifndef _DEB_NEW_KDTREE_
    std::map<SCollisionFace*, bool> FaceMap, EdgeFaceMap;
    #endif
    
    /* Get tree node list */
    std::list<const TreeNode*> TreeNodeList;
    
    RootTreeNode->findLeafList(
        TreeNodeList, CapsuleLineInv, (RivalMatInv.getScale() * getRadius()).getMax()
    );
    
    /* Check collision with triangle faces of each tree-node */
    foreach (const TreeNode* Node, TreeNodeList)
    {
        /* Get tree node data */
        CollisionMesh::TreeNodeDataType* TreeNodeData = static_cast<CollisionMesh::TreeNodeDataType*>(Node->getUserData());
        
        if (!TreeNodeData)
            continue;
        
        /* Check collision with each triangle face */
        #ifndef _DEB_NEW_KDTREE_
        foreach (SCollisionFace* Face, *TreeNodeData)
        #else
        foreach (SCollisionFace &NodeFace, *TreeNodeData)
        #endif
        {
            #ifndef _DEB_NEW_KDTREE_
            /* Check for unique usage */
            if (FaceMap.find(Face) != FaceMap.end())
                continue;
            
            FaceMap[Face] = true;
            #else
            SCollisionFace* Face = &NodeFace;
            #endif
            
            /* Check for face-culling */
            if (Face->isBackFaceCulling(CollFace, CapsuleLineInv))
                continue;
            
            /* Make capsule-triangle collision test */
            const dim::triangle3df Triangle(RivalMat * Face->Triangle);
            
            if (!math::CollisionLibrary::getClosestLineStraight(Triangle, CapsuleLine, ClosestLine))
                continue;
            
            /* Check if this is a potentially new closest face */
            if (math::getDistanceSq(ClosestLine.Start, ClosestLine.End) < RadiusSq)
            {
                /* Perform detected collision contact */
                SCollisionContact Contact;
                {
                    Contact.Point       = ClosestLine.Start;
                    Contact.Normal      = Triangle.getNormal();
                    Contact.Impact      = getRadius() - (ClosestLine.End - Contact.Point).getLength();
                    Contact.Triangle    = Triangle;
                    Contact.Face        = Face;
                }
                performDetectedContact(Rival, Contact);
                
                if (getFlags() & COLLISIONFLAG_RESOLVE)
                {
                    /* Update capsule position */
                    CapsuleLine             = getLine();
                    CapsuleLineInv.Start    = RivalMatInv * CapsuleLine.Start;
                    CapsuleLineInv.End      = RivalMatInv * CapsuleLine.End;
                }
            }
        }
    }
    
    /* Check collision with triangle edges of each tree-node */
    foreach (const TreeNode* Node, TreeNodeList)
    {
        /* Get tree node data */
        CollisionMesh::TreeNodeDataType* TreeNodeData = static_cast<CollisionMesh::TreeNodeDataType*>(Node->getUserData());
        
        if (!TreeNodeData)
            continue;
        
        /* Check collision with each triangle edge */
        #ifndef _DEB_NEW_KDTREE_
        foreach (SCollisionFace* Face, *TreeNodeData)
        #else
        foreach (SCollisionFace &NodeFace, *TreeNodeData)
        #endif
        {
            #ifndef _DEB_NEW_KDTREE_
            /* Check for unique usage */
            if (EdgeFaceMap.find(Face) != EdgeFaceMap.end())
                continue;
            
            EdgeFaceMap[Face] = true;
            #else
            SCollisionFace* Face = &NodeFace;
            #endif
            
            /* Check for face-culling */
            if (Face->isBackFaceCulling(CollFace, CapsuleLineInv))
                continue;
            
            /* Make capsule-triangle collision test */
            const dim::triangle3df Triangle(RivalMat * Face->Triangle);
            
            ClosestLine = math::CollisionLibrary::getClosestLine(Triangle, CapsuleLine);
            
            /* Check if this is a potentially new closest face */
            if (math::getDistanceSq(ClosestLine.Start, ClosestLine.End) < RadiusSq)
            {
                /* Perform detected collision contact */
                SCollisionContact Contact;
                {
                    Contact.Point = ClosestLine.Start;
                    
                    Contact.Normal = ClosestLine.End;
                    Contact.Normal -= ClosestLine.Start;
                    Contact.Normal.normalize();
                    
                    Contact.Impact      = getRadius() - (ClosestLine.End - Contact.Point).getLength();
                    Contact.Triangle    = Triangle;
                    Contact.Face        = Face;
                }
                performDetectedContact(Rival, Contact);
                
                if (getFlags() & COLLISIONFLAG_RESOLVE)
                {
                    /* Update capsule position */
                    CapsuleLine             = getLine();
                    CapsuleLineInv.Start    = RivalMatInv * CapsuleLine.Start;
                    CapsuleLineInv.End      = RivalMatInv * CapsuleLine.End;
                }
            }
        }
    }
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
