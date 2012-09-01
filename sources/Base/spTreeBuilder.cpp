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

static void KdTreeRootNodeDestructorProc(TreeNode* Node)
{
    delete static_cast<std::list<SCollisionFace>*>(Node->getUserData());
}

static void KdTreeNodeDestructorProc(TreeNode* Node)
{
    delete static_cast<std::vector<SCollisionFace*>*>(Node->getUserData());
}


/*
 * Global functions
 */

SP_EXPORT KDTreeNode* buildKdTree(
    const std::list<Mesh*> &MeshList, u8 MaxTreeLevel, const EKDTreeBuildingConcepts Concept, bool PreTransform)
{
    if (MeshList.empty())
        return 0;
    
    /* Get whole count of triangles and construct bounding box */
    u32 TriangleCount = 0;
    dim::aabbox3df MeshListBoundBox(dim::aabbox3df::OMEGA);
    
    foreach (Mesh* Obj, MeshList)
    {
        /* Count triangles */
        TriangleCount += Obj->getTriangleCount();
        
        /* Update bounding box */
        MeshListBoundBox.insertBox(Obj->getMeshBoundingBox(PreTransform));
    }
    
    if (!TriangleCount)
        return 0;
    
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
    
    RootNode->setBox(MeshListBoundBox);
    RootNode->setUserData(TriangleList);
    RootNode->setDestructorCallback(KdTreeRootNodeDestructorProc);
    
    /* Build first level of children */
    std::list<SCollisionFace*> SubTriangleList;
    
    foreach (SCollisionFace &Face, *TriangleList)
        SubTriangleList.push_back(&Face);
    
    buildKdTreeNode(RootNode, SubTriangleList, MaxTreeLevel, Concept);
    
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
    if (ForkLevel > 0)
    {
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
                
                /* Check if further sub-division is pointless */
                if (PotSubTrianglesNear[Axis].size() == Triangles.size() && PotSubTrianglesFar[Axis].size() == Triangles.size())
                    ForkLevel = 0;
            }
            break;
            
            case KDTREECONCEPT_AVERAGE:
            {
                const u32 ListSize[3] = {
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
                
                /* Check if further sub-division is pointless */
                if (PotSubTrianglesNear[Axis].size() == Triangles.size() || PotSubTrianglesFar[Axis].size() == Triangles.size())
                    ForkLevel = 0;
            }
            break;
        }
        
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
    else
    {
        /* Create triangle reference list */
        std::vector<SCollisionFace*>* RefList = new std::vector<SCollisionFace*>(Triangles.size());
        
        u32 i = 0;
        foreach (SCollisionFace* Face, Triangles)
            (*RefList)[i++] = Face;
        
        /* Setup user data for tree node */
        Node->setDestructorCallback(KdTreeNodeDestructorProc);
        Node->setUserData(RefList);
    }
}

} // /namespace TreeBuilder


} // /namespace scene

} // /namespace sp



// ================================================================================
