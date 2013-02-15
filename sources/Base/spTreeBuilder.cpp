/*
 * Tree builder file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spTreeBuilder.hpp"
#include "SceneGraph/Collision/spCollisionMesh.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


namespace TreeBuilder
{

/*
 * Internal functions
 */

static void buildKdTreeNode(
    KDTreeNode* Node, std::list<SCollisionFace*> &Triangles,
    s32 ForkLevel, const EKDTreeBuildingConcepts Concept
);

static void buildKdTreeNode_ALT(
    KDTreeNode* Node, const std::vector<SCollisionFace> &Faces,
    s32 ForkLevel, const EKDTreeBuildingConcepts Concept
);

static void KdTreeRootNodeDestructorProc(TreeNode* Node)
{
    delete static_cast<std::list<SCollisionFace>*>(Node->getUserData());
}

static void KdTreeNodeDestructorProc(TreeNode* Node)
{
    delete static_cast<std::vector<SCollisionFace*>*>(Node->getUserData());
}

static void KdTreeNodeDestructorProc_ALT(TreeNode* Node)
{
    delete static_cast<std::vector<SCollisionFace>*>(Node->getUserData());
}

static void getMeshListBoundingBox(
    const std::list<Mesh*> &MeshList, dim::aabbox3df &BoundBox, u32 &TriangleCount, bool PreTransform)
{
    foreach (Mesh* Obj, MeshList)
    {
        /* Count triangles */
        TriangleCount += Obj->getTriangleCount();
        
        /* Update bounding box */
        BoundBox.insertBox(Obj->getMeshBoundingBox(PreTransform));
    }
}

static KDTreeNode* buildKdTree_ALT(
    const std::list<Mesh*> &MeshList, u8 MaxTreeLevel, const EKDTreeBuildingConcepts Concept, bool PreTransform
);


/*
 * Global functions
 */

SP_EXPORT KDTreeNode* buildKdTree(
    const std::list<Mesh*> &MeshList, u8 MaxTreeLevel, const EKDTreeBuildingConcepts Concept, bool PreTransform)
{
    #ifdef _DEB_NEW_KDTREE_
    
    return buildKdTree_ALT(MeshList, MaxTreeLevel, Concept, PreTransform);
    
    #else
    
    if (MeshList.empty())
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("TreeBuilder::buildKdTree", "Empty mesh list for collision mesh");
        #endif
        return 0;
    }
    
    /* Get whole count of triangles and construct bounding box */
    u32 TriangleCount = 0;
    dim::aabbox3df BoundBox(dim::aabbox3df::OMEGA);
    
    getMeshListBoundingBox(MeshList, BoundBox, TriangleCount, PreTransform);
    
    if (!TriangleCount)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("TreeBuilder::buildKdTree", "No triangles for collision mesh");
        #endif
        return 0;
    }
    
    /* Create collision triangle list */
    std::list<SCollisionFace>* TriangleList = new std::list<SCollisionFace>();
    SCollisionFace Face;
    
    foreach (Mesh* Obj, MeshList)
    {
        Face.Mesh = Obj;
        
        const dim::matrix4f Matrix(Obj->getTransformMatrix(true));
        
        for (Face.Surface = 0; Face.Surface < Obj->getMeshBufferCount(); ++Face.Surface)
        {
            video::MeshBuffer* Surface = Obj->getMeshBuffer(Face.Surface);
            
            for (Face.Index = 0; Face.Index < Surface->getTriangleCount(); ++Face.Index)
            {
                /* Fill collision triangle data */
                Face.Triangle = Surface->getTriangleCoords(Face.Index);
                
                if (PreTransform)
                    Face.Triangle = Matrix * Face.Triangle;
                
                TriangleList->push_back(Face);
            }
        }
    }
    
    /* Create tree root node */
    KDTreeNode* RootNode = MemoryManager::createMemory<KDTreeNode>("TreeBuilder::KDTreeNode");
    
    RootNode->setBox(BoundBox);
    RootNode->setUserData(TriangleList);
    RootNode->setDestructorCallback(KdTreeRootNodeDestructorProc);
    
    /* Build first level of children */
    std::list<SCollisionFace*> SubTriangleList;
    
    foreach (SCollisionFace &Face, *TriangleList)
        SubTriangleList.push_back(&Face);
    
    buildKdTreeNode(RootNode, SubTriangleList, MaxTreeLevel, Concept);
    
    return RootNode;
    
    #endif
}

//!TODO! -> this is still in progress
/*SP_EXPORT */static KDTreeNode* buildKdTree_ALT(
    const std::list<Mesh*> &MeshList, u8 MaxTreeLevel, const EKDTreeBuildingConcepts Concept, bool PreTransform)
{
    if (MeshList.empty())
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("TreeBuilder::buildKdTree", "Empty mesh list for collision mesh");
        #endif
        return 0;
    }
    
    /* Get whole count of triangles and construct bounding box */
    u32 TriangleCount = 0;
    dim::aabbox3df BoundBox(dim::aabbox3df::OMEGA);
    
    getMeshListBoundingBox(MeshList, BoundBox, TriangleCount, PreTransform);
    
    if (!TriangleCount)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("TreeBuilder::buildKdTree", "No triangles for collision mesh");
        #endif
        return 0;
    }
    
    /* Create collision triangle list */
    std::vector<SCollisionFace> FaceList(TriangleCount);
    SCollisionFace* Face = 0;
    
    u32 f = 0;
    
    foreach (Mesh* Obj, MeshList)
    {
        const dim::matrix4f Matrix(Obj->getTransformMatrix(true));
        
        for (u32 s = 0; s < Obj->getMeshBufferCount(); ++s)
        {
            video::MeshBuffer* Surface = Obj->getMeshBuffer(s);
            
            for (u32 i = 0, c = Surface->getTriangleCount(); i < c; ++i)
            {
                Face = &(FaceList[f++]);
                
                /* Fill collision triangle data */
                Face->Mesh      = Obj;
                Face->Surface   = s;
                Face->Index     = i;
                Face->Triangle  = Surface->getTriangleCoords(i);
                
                if (PreTransform)
                    Face->Triangle = Matrix * Face->Triangle;
            }
        }
    }
    
    /* Create tree root node */
    KDTreeNode* RootNode = MemoryManager::createMemory<KDTreeNode>("TreeBuilder::KDTreeNode");
    
    RootNode->setBox(BoundBox);
    
    /* Build first level of children */
    buildKdTreeNode_ALT(RootNode, FaceList, MaxTreeLevel, Concept);
    
    return RootNode;
}

SP_EXPORT KDTreeNode* buildKdTree(Mesh* Object, u8 MaxTreeLevel, const EKDTreeBuildingConcepts Concept)
{
    std::list<Mesh*> MeshList;
    MeshList.push_back(Object);
    return buildKdTree(MeshList, MaxTreeLevel, Concept);
}

SP_EXPORT OcTreeNode* buildOcTree(Mesh* Object, u8 MaxTreeLevel)
{
    return 0; // todo
}

SP_EXPORT QuadTreeNode* buildQuadTree(Mesh* Object, u8 MaxTreeLevel)
{
    return 0; // todo
}

SP_EXPORT BSPTreeNode* buildBSPTree(Mesh* Object, u8 MaxTreeLevel)
{
    return 0; // todo
}

SP_EXPORT OBBTreeNode* buildOBBTree(const std::list<dim::obbox3df> &BoxList)
{
    if (BoxList.empty())
        return 0;
    
    OBBTreeNode* RootNode = new OBBTreeNode(0, dim::obbox3df(-math::OMEGA, math::OMEGA));
    
    foreach (const dim::obbox3df &Box, BoxList)
        RootNode->insertBoundingBox(Box);
    
    return RootNode;
}


/*
 * Internal functions
 */

static void buildKdTreeNode(
    KDTreeNode* Node, std::list<SCollisionFace*> &Triangles, s32 ForkLevel, const EKDTreeBuildingConcepts Concept)
{
    if (!Node || Triangles.empty())
        return;
    
    /* Check if tree node is a leaf */
    if (ForkLevel <= 0)
    {
        /* Create triangle reference list */
        std::vector<SCollisionFace*>* RefList = new std::vector<SCollisionFace*>(Triangles.size());
        
        u32 i = 0;
        foreach (SCollisionFace* Face, Triangles)
            (*RefList)[i++] = Face;
        
        /* Setup user data for tree node */
        Node->setDestructorCallback(KdTreeNodeDestructorProc);
        Node->setUserData(RefList);
        
        return;
    }
    
    const dim::aabbox3df BoundBox(Node->getBox());
    
    /* Compute average vertex position */
    dim::vector3df AvgVertPos;
    
    switch (Concept)
    {
        case KDTREECONCEPT_CENTER:
        {
            AvgVertPos = BoundBox.getCenter();
        }
        break;
        
        case KDTREECONCEPT_AVERAGE:
        {
            foreach (SCollisionFace* Face, Triangles)
                AvgVertPos += Face->Triangle.getCenter();
            
            AvgVertPos /= dim::vector3df(static_cast<f32>(Triangles.size()));
        }
        break;
    }
    
    /* Fill potentially sub triangle lists */
    std::list<SCollisionFace*> PotSubTrianglesNear[3], PotSubTrianglesFar[3];
    
    foreach (SCollisionFace* Face, Triangles)
    {
        for (s32 i = 0; i < 3; ++i)
        {
            #if 1
            
            if (Face->Triangle.PointA[i] < AvgVertPos[i] ||
                Face->Triangle.PointB[i] < AvgVertPos[i] ||
                Face->Triangle.PointC[i] < AvgVertPos[i])
            {
                PotSubTrianglesNear[i].push_back(Face);
            }
            if (Face->Triangle.PointA[i] >= AvgVertPos[i] ||
                Face->Triangle.PointB[i] >= AvgVertPos[i] ||
                Face->Triangle.PointC[i] >= AvgVertPos[i])
            {
                PotSubTrianglesFar[i].push_back(Face);
            }
            
            #else
            
            dim::aabbox3df NearBox(BoundBox);
            NearBox.Max[i] = AvgVertPos[i];
            
            dim::aabbox3df FarBox(BoundBox);
            NearBox.Min[i] = AvgVertPos[i];
            
            if (math::CollisionLibrary::checkTriangleBoxOverlap(Face->Triangle, NearBox))
                PotSubTrianglesNear[i].push_back(Face);
            if (math::CollisionLibrary::checkTriangleBoxOverlap(Face->Triangle, FarBox))
                PotSubTrianglesFar[i].push_back(Face);
            
            #endif
        }
    }
    
    /* Search for optimal tree partitioning */
    EKDTreeAxles Axis = KDTREE_XAXIS;
    
    switch (Concept)
    {
        case KDTREECONCEPT_CENTER:
        {
            const dim::vector3df BoxSize(BoundBox.getSize());
            
            if (BoxSize.X >= BoxSize.Y && BoxSize.X >= BoxSize.Z)
                Axis = KDTREE_XAXIS;
            else if (BoxSize.Y >= BoxSize.X && BoxSize.Y >= BoxSize.Z)
                Axis = KDTREE_YAXIS;
            else
                Axis = KDTREE_ZAXIS;
        }
        break;
        
        case KDTREECONCEPT_AVERAGE:
        {
            const u32 ListSize[3] =
            {
                PotSubTrianglesNear[0].size() + PotSubTrianglesFar[0].size(),
                PotSubTrianglesNear[1].size() + PotSubTrianglesFar[1].size(),
                PotSubTrianglesNear[2].size() + PotSubTrianglesFar[2].size()
            };
            
            if (ListSize[0] == ListSize[1] && ListSize[0] == ListSize[2])
            {
                const dim::vector3df BoxSize(BoundBox.getSize());
                
                if (BoxSize.X >= BoxSize.Y && BoxSize.X >= BoxSize.Z)
                    Axis = KDTREE_XAXIS;
                else if (BoxSize.Y >= BoxSize.X && BoxSize.Y >= BoxSize.Z)
                    Axis = KDTREE_YAXIS;
                else
                    Axis = KDTREE_ZAXIS;
            }
            else if (ListSize[0] <= ListSize[1] && ListSize[0] <= ListSize[2])
                Axis = KDTREE_XAXIS;
            else if (ListSize[1] <= ListSize[0] && ListSize[1] <= ListSize[2])
                Axis = KDTREE_YAXIS;
            else
                Axis = KDTREE_ZAXIS;
        }
        break;
    }
    
    /* Check if further sub-division is pointless */
    if (PotSubTrianglesNear[Axis].size() == Triangles.size() && PotSubTrianglesFar[Axis].size() == Triangles.size())
        ForkLevel = 0;
    
    #if 0
    /* Check if further sub-division is pointless */
    if (PotSubTrianglesNear[Axis].size() <= 1 && PotSubTrianglesFar[Axis].size() <= 1)
        io::Log::message("DEB: Only one triangle");
    #endif
    
    /* Create children tree nodes */
    Node->setAxis(Axis);
    Node->setDistance(AvgVertPos[Axis]);
    
    Node->addChildren();
    
    KDTreeNode* TreeNodeNear = static_cast<KDTreeNode*>(Node->getChildNear());
    KDTreeNode* TreeNodeFar = static_cast<KDTreeNode*>(Node->getChildFar());
    
    /* Build next fork level */
    buildKdTreeNode(TreeNodeNear, PotSubTrianglesNear[Axis], ForkLevel - 1, Concept);
    buildKdTreeNode(TreeNodeFar, PotSubTrianglesFar[Axis], ForkLevel - 1, Concept);
}

static void buildKdTreeNodeLeaf_ALT(KDTreeNode* Node, const std::vector<SCollisionFace> &Faces)
{
    /* Create triangle reference list */
    std::vector<SCollisionFace>* RefList = new std::vector<SCollisionFace>(Faces.size());
    
    *RefList = Faces;
    
    //!TODO! -> swap in "math::CollisionLibrary::clipPolygon" function
    foreach (SCollisionFace &Face, *RefList)
        Face.Triangle.swap();
    
    /* Setup user data for tree node */
    Node->setDestructorCallback(KdTreeNodeDestructorProc_ALT);
    Node->setUserData(RefList);
}

static void buildKdTreeNode_ALT(
    KDTreeNode* Node, const std::vector<SCollisionFace> &Faces, s32 ForkLevel, const EKDTreeBuildingConcepts Concept)
{
    if (!Node || Faces.empty())
        return;
    
    /* Check if tree node is a leaf */
    if (ForkLevel <= 0)
    {
        buildKdTreeNodeLeaf_ALT(Node, Faces);
        return;
    }
    
    const dim::aabbox3df BoundBox(Node->getBox());
    
    /* Compute average vertex position */
    dim::vector3df AvgVertPos;
    
    switch (Concept)
    {
        case KDTREECONCEPT_CENTER:
        {
            AvgVertPos = BoundBox.getCenter();
        }
        break;
        
        case KDTREECONCEPT_AVERAGE:
        {
            #if 0
            
            foreach (const SCollisionFace &Face, Faces)
                AvgVertPos += Face.Triangle.getCenter();
            
            AvgVertPos /= dim::vector3df(static_cast<f32>(Faces.size()));
            
            #else
            
            /* Compute median position */
            std::vector<f32> MedianPos[3];
            
            for (s32 i = 0; i < 3; ++i)
            {
                MedianPos[i].resize(Faces.size());
                
                u32 j = 0;
                foreach (const SCollisionFace &Face, Faces)
                    (MedianPos[i])[j++] = Face.Triangle.getCenter()[i];
                
                std::sort(MedianPos[i].begin(), MedianPos[i].end());
                
                AvgVertPos[i] = (MedianPos[i])[MedianPos[i].size()/2];
            }
            
            #endif
        }
        break;
    }
    
    /* Fill potentially sub triangle lists */
    s32 PotFaceCountNear[3], PotFaceCountFar[3];
    
    foreach (const SCollisionFace &Face, Faces)
    {
        for (s32 i = 0; i < 3; ++i)
        {
            if (Face.Triangle.PointA[i] < AvgVertPos[i] ||
                Face.Triangle.PointB[i] < AvgVertPos[i] ||
                Face.Triangle.PointC[i] < AvgVertPos[i])
            {
                ++PotFaceCountNear[i];
            }
            if (Face.Triangle.PointA[i] >= AvgVertPos[i] ||
                Face.Triangle.PointB[i] >= AvgVertPos[i] ||
                Face.Triangle.PointC[i] >= AvgVertPos[i])
            {
                ++PotFaceCountFar[i];
            }
        }
    }
    
    /* Search for optimal tree partitioning */
    EKDTreeAxles Axis = KDTREE_XAXIS;
    
    switch (Concept)
    {
        case KDTREECONCEPT_CENTER:
        {
            const dim::vector3df BoxSize(BoundBox.getSize());
            
            if (BoxSize.X >= BoxSize.Y && BoxSize.X >= BoxSize.Z)
                Axis = KDTREE_XAXIS;
            else if (BoxSize.Y >= BoxSize.X && BoxSize.Y >= BoxSize.Z)
                Axis = KDTREE_YAXIS;
            else
                Axis = KDTREE_ZAXIS;
        }
        break;
        
        case KDTREECONCEPT_AVERAGE:
        {
            const u32 ListSize[3] =
            {
                PotFaceCountNear[0] + PotFaceCountFar[0],
                PotFaceCountNear[1] + PotFaceCountFar[1],
                PotFaceCountNear[2] + PotFaceCountFar[2]
            };
            
            if (ListSize[0] == ListSize[1] && ListSize[0] == ListSize[2])
            {
                const dim::vector3df BoxSize(BoundBox.getSize());
                
                if (BoxSize.X >= BoxSize.Y && BoxSize.X >= BoxSize.Z)
                    Axis = KDTREE_XAXIS;
                else if (BoxSize.Y >= BoxSize.X && BoxSize.Y >= BoxSize.Z)
                    Axis = KDTREE_YAXIS;
                else
                    Axis = KDTREE_ZAXIS;
            }
            else if (ListSize[0] <= ListSize[1] && ListSize[0] <= ListSize[2])
                Axis = KDTREE_XAXIS;
            else if (ListSize[1] <= ListSize[0] && ListSize[1] <= ListSize[2])
                Axis = KDTREE_YAXIS;
            else
                Axis = KDTREE_ZAXIS;
        }
        break;
    }
    
    /* Create children tree nodes */
    Node->setAxis(Axis);
    Node->setDistance(AvgVertPos[Axis]);
    
    /* Construct clipping plane */
    dim::plane3df ClipPlane;
    
    switch (Axis)
    {
        case KDTREE_XAXIS:
            ClipPlane.Normal = dim::vector3df(1, 0, 0);
            break;
        case KDTREE_YAXIS:
            ClipPlane.Normal = dim::vector3df(0, 1, 0);
            break;
        case KDTREE_ZAXIS:
            ClipPlane.Normal = dim::vector3df(0, 0, 1);
            break;
    }
    
    ClipPlane.Distance = Node->getDistance();
    
    /* Clip plolygons */
    std::vector<SCollisionFace> FacesNear, FacesFar;
    
    foreach (const SCollisionFace &Face, Faces)
    {
        /* Clip current face's triangle */
        dim::polygon3df Poly, PolyNear, PolyFar;
        
        Poly.push(Face.Triangle.PointA);
        Poly.push(Face.Triangle.PointB);
        Poly.push(Face.Triangle.PointC);
        
        math::CollisionLibrary::clipPolygon(Poly, ClipPlane, PolyFar, PolyNear);
        
        /* Fill new polygon into sub-lists */
        for (u32 i = 2; i < PolyNear.getCount(); ++i)
        {
            FacesNear.push_back(SCollisionFace(
                Face.Mesh, Face.Surface, Face.Index,
                dim::triangle3df(PolyNear[0], PolyNear[i - 1], PolyNear[i])
            ));
        }
        
        for (u32 i = 2; i < PolyFar.getCount(); ++i)
        {
            FacesFar.push_back(SCollisionFace(
                Face.Mesh, Face.Surface, Face.Index,
                dim::triangle3df(PolyFar[0], PolyFar[i - 1], PolyFar[i])
            ));
        }
    }
    
    /* Check if further sub-division is pointless */
    if (FacesNear.size() >= Faces.size() && FacesFar.size() >= Faces.size())
    {
        buildKdTreeNodeLeaf_ALT(Node, Faces);
        return;
    }
    
    /* Build next fork level */
    Node->addChildren();
    
    KDTreeNode* TreeNodeNear = static_cast<KDTreeNode*>(Node->getChildNear());
    KDTreeNode* TreeNodeFar = static_cast<KDTreeNode*>(Node->getChildFar());
    
    buildKdTreeNode_ALT(TreeNodeNear, FacesNear, ForkLevel - 1, Concept);
    buildKdTreeNode_ALT(TreeNodeFar, FacesFar, ForkLevel - 1, Concept);
}

} // /namespace TreeBuilder


} // /namespace scene

} // /namespace sp



// ================================================================================
