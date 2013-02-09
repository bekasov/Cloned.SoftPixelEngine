/*
 * Collision mesh file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionMesh.hpp"
#include "SceneGraph/Collision/spCollisionSphere.hpp"
#include "SceneGraph/Collision/spCollisionGraph.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Base/spTreeBuilder.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern scene::SceneGraph* __spSceneManager;

namespace scene
{


CollisionMesh::CollisionMesh(
    CollisionMaterial* Material, scene::Mesh* Mesh, u8 MaxTreeLevel) :
    CollisionNode   (Material, Mesh, COLLISION_MESH ),
    RootTreeNode_   (0                              ),
    CollFace_       (video::FACE_FRONT              )
{
    std::list<scene::Mesh*> MeshList;
    MeshList.push_back(Mesh);
    createCollisionModel(MeshList, MaxTreeLevel, false);
}
CollisionMesh::CollisionMesh(
    CollisionMaterial* Material, const std::list<Mesh*> &MeshList, u8 MaxTreeLevel) :
    CollisionNode   (Material, __spSceneManager->createNode(), COLLISION_MESH   ),
    RootTreeNode_   (0                                                          ),
    CollFace_       (video::FACE_FRONT                                          )
{
    createCollisionModel(MeshList, MaxTreeLevel, true);
}
CollisionMesh::~CollisionMesh()
{
    MemoryManager::deleteMemory(RootTreeNode_);
}

s32 CollisionMesh::getSupportFlags() const
{
    return COLLISIONSUPPORT_NONE;
}

f32 CollisionMesh::getMaxMovement() const
{
    return 0.0f;
}

void CollisionMesh::findIntersections(const dim::line3df &Line, std::list<SIntersectionContact> &ContactList) const
{
    if (!RootTreeNode_)
        return;
    
    /* Store transformation and a 'vice-versa' variant of the line */
    const dim::matrix4f Matrix(getTransformation());
    const dim::matrix4f InvMatrix(Matrix.getInverse());
    const dim::line3df InvLine(InvMatrix * Line);
    const dim::line3df InvLineVV(InvMatrix * Line.getViceVersa());
    
    const bool useFront = (CollFace_ == video::FACE_FRONT || CollFace_ == video::FACE_BOTH);
    const bool useBack = (CollFace_ == video::FACE_BACK || CollFace_ == video::FACE_BOTH);
    
    SIntersectionContact Contact;
    
    #ifndef _DEB_NEW_KDTREE_
    std::map<SCollisionFace*, bool> FaceMap;
    #endif
    
    /* Search tree node leafs */
    std::list<const TreeNode*> TreeNodeList;
    RootTreeNode_->findLeafList(TreeNodeList, InvLine);
    
    foreach (const TreeNode* Node, TreeNodeList)
    {
        if (!Node->getUserData())
            continue;
        
        TreeNodeDataType* TreeNodeData = static_cast<TreeNodeDataType*>(Node->getUserData());
        
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
            
            /* Check if an intersection between the line and the current triangle exists */
            if ( ( useFront && math::CollisionLibrary::checkLineTriangleIntersection(Face->Triangle, InvLine, Contact.Point) ) ||
                 ( useBack && math::CollisionLibrary::checkLineTriangleIntersection(Face->Triangle, InvLineVV, Contact.Point) ) )
            {
                /* Setup contact information */
                Contact.Point       = Matrix * Contact.Point;
                Contact.Triangle    = Matrix * Face->Triangle;
                Contact.Normal      = Contact.Triangle.getNormal();
                Contact.Face        = Face;
                Contact.Object      = this;
                
                /* Store intersection contact */
                if (useFront)
                    ContactList.push_back(Contact);
                if (useBack)
                {
                    Contact.Normal = -Contact.Normal;
                    ContactList.push_back(Contact);
                }
            }
        }
    }
}

bool CollisionMesh::checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const
{   
    /* Find all intersections and return the nearest one */
    std::list<SIntersectionContact> ContactList;
    findIntersections(Line, ContactList);
    
    if (!ContactList.empty())
    {
        /* Compute squared distance */
        foreach (SIntersectionContact &CurContact, ContactList)
            CurContact.DistanceSq = math::getDistanceSq(Line.Start, CurContact.Point);
        
        /* Sort contact list and store first element in the resulting contact */
        CollisionGraph::sortContactList(Line.Start, ContactList);
        Contact = *ContactList.begin();
        
        return true;
    }
    
    return false;
}

bool CollisionMesh::checkIntersection(const dim::line3df &Line, bool ExcludeCorners) const
{
    if (!RootTreeNode_)
        return false;
    
    /* Store transformation and a 'vice-versa' variant of the line */
    const dim::matrix4f InvMatrix(getInverseTransformation());
    const dim::line3df InvLine(InvMatrix * Line);
    const dim::line3df InvLineVV(InvMatrix * Line.getViceVersa());
    
    const bool useFront = (CollFace_ == video::FACE_FRONT || CollFace_ == video::FACE_BOTH);
    const bool useBack = (CollFace_ == video::FACE_BACK || CollFace_ == video::FACE_BOTH);
    
    dim::vector3df Point;
    
    /* Search tree node leafs */
    std::list<const TreeNode*> TreeNodeList;
    RootTreeNode_->findLeafList(TreeNodeList, InvLine);
    
    foreach (const TreeNode* Node, TreeNodeList)
    {
        if (!Node->getUserData())
            continue;
        
        TreeNodeDataType* TreeNodeData = static_cast<TreeNodeDataType*>(Node->getUserData());
        
        #ifndef _DEB_NEW_KDTREE_
        foreach (SCollisionFace* Face, *TreeNodeData)
        #else
        foreach (SCollisionFace &NodeFace, *TreeNodeData)
        #endif
        {
            #ifdef _DEB_NEW_KDTREE_
            SCollisionFace* Face = &NodeFace;
            #endif
            
            /* Check if an intersection between the line and the current triangle exists */
            if (useFront && math::CollisionLibrary::checkLineTriangleIntersection(Face->Triangle, InvLine, Point))
            {
                if (ExcludeCorners)
                {
                    if (checkCornerExlusion(InvLine, Point))
                        return true;
                }
                else
                    return true;
            }
            if (useBack && math::CollisionLibrary::checkLineTriangleIntersection(Face->Triangle, InvLineVV, Point))
            {
                if (ExcludeCorners)
                {
                    if (checkCornerExlusion(InvLine, Point))
                        return true;
                }
                else
                    return true;
            }
        }
    }
    
    return false;
}


/*
 * ======= Private: =======
 */

void CollisionMesh::createCollisionModel(
    const std::list<Mesh*> &MeshList, u8 MaxTreeLevel, bool PreTransform)
{
    RootTreeNode_ = TreeBuilder::buildKdTree(
        MeshList, MaxTreeLevel, KDTREECONCEPT_CENTER, PreTransform
    );
}


} // /namespace scene

} // /namespace sp



// ================================================================================
