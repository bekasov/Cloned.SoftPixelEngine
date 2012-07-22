/*
 * Collision sphere file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionCapsule.hpp"
#include "SceneGraph/Collision/spCollisionSphere.hpp"
#include "SceneGraph/Collision/spCollisionBox.hpp"
#include "SceneGraph/Collision/spCollisionMesh.hpp"
#include "SceneGraph/Collision/spCollisionConfigTypes.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


CollisionCapsule::CollisionCapsule(
    CollisionMaterial* Material, SceneNode* Node, f32 Radius, f32 Height) :
    CollisionNode   (Material, Node, COLLISION_CAPSULE  ),
    Radius_         (Radius                             ),
    Height_         (Height                             )
{
}
CollisionCapsule::~CollisionCapsule()
{
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

bool CollisionCapsule::checkIntersection(const dim::line3df &Line) const
{
    /* Make an intersection test with both lines */
    dim::vector3df PointP, PointQ;
    return math::CollisionLibrary::getLineLineDistanceSq(getLine(), Line, PointP, PointQ) < math::Pow2(getRadius());
}

dim::line3df CollisionCapsule::getLine() const
{
    const dim::matrix4f Mat(getTransformation());
    return dim::line3df(Mat.getPosition(), Mat * dim::vector3df(0, Height_, 0));
}

dim::obbox3df CollisionCapsule::getBoundBoxFromLine(const dim::line3df &Line, f32 Radius)
{
    /* Convert the line with radius into an OBB */
    dim::vector3df Axis[3];
    
    Axis[0] = Line.getDirection() * 0.5f;
    Axis[1] = Axis[0].getNormal();
    Axis[2] = Axis[0].cross(Axis[1]);
    
    Axis[1].setLength(Radius);
    Axis[2].setLength(Radius);
    
    const dim::vector3df Dir(Line.getDirection().normalize());
    
    return dim::obbox3df(
        Line.Start + Axis[0], Axis[0] + Dir * Radius, Axis[1], Axis[2]
    );
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
    
    /* Get the closest point from this sphere to the capsule */
    const dim::vector3df ClosestPoint = CapsuleLine.getClosestPoint(SpherePos);
    
    /* Check if this object and the other collide with each other */
    if (math::getDistanceSq(SpherePos, ClosestPoint) < math::Pow2(Radius_ + Rival->getRadius()))
    {
        Contact.Normal  = (SpherePos - ClosestPoint).normalize();
        Contact.Point   = SpherePos - Contact.Normal * Rival->getRadius();
        return true;
    }
    
    return false;
}

bool CollisionCapsule::checkCollisionToCapsule(const CollisionCapsule* Rival, SCollisionContact &Contact) const
{
    if (!Rival)
        return false;
    
    /* Store transformation */
    const dim::vector3df SpherePos(Rival->getPosition());
    const dim::line3df CapsuleLine(getLine());
    
    /* Get the closest points between this and the rival capsule */
    dim::vector3df PointP, PointQ;
    const f32 DistanceSq = math::CollisionLibrary::getLineLineDistanceSq(getLine(), Rival->getLine(), PointP, PointQ);
    
    /* Check if this object and the other collide with each other */
    if (DistanceSq < math::Pow2(Radius_ + Rival->getRadius()))
    {
        Contact.Normal  = (PointQ - PointP).normalize();
        Contact.Point   = PointQ - Contact.Normal * Rival->getRadius();
        return true;
    }
    
    return false;
}

bool CollisionCapsule::checkCollisionToBox(const CollisionBox* Rival, SCollisionContact &Contact) const
{
    return false; // todo
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
            /* Check for face-culling */
            if (CollFace != video::FACE_BOTH)
            {
                const bool arePointsFront = (
                    dim::plane3df(Face->Triangle).isPointFrontSide(CapsuleLineInv.Start) &&
                    dim::plane3df(Face->Triangle).isPointFrontSide(CapsuleLineInv.End)
                );
                
                if ( ( CollFace == video::FACE_FRONT && !arePointsFront ) ||
                     ( CollFace == video::FACE_BACK && arePointsFront ) )
                {
                    continue;
                }
            }
            
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


} // /namespace scene

} // /namespace sp



// ================================================================================
