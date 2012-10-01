/*
 * Tree builder header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TREEBUILDER_H__
#define __SP_TREEBUILDER_H__


#include "Base/spStandard.hpp"
#include "Base/spTreeNode.hpp"
#include "Base/spTreeNodeBSP.hpp"
#include "Base/spTreeNodeQuad.hpp"
#include "Base/spTreeNodeOct.hpp"
#include "Base/spTreeNodeKD.hpp"
#include "Base/spTreeNodeOBB.hpp"
#include "Base/spTreeNodeImage.hpp"
#include "Base/spMathTriangleCutter.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/Collision/spCollisionConfigTypes.hpp"


namespace sp
{
namespace scene
{


//! Concepts of building a kd-Tree.
enum EKDTreeBuildingConcepts
{
    KDTREECONCEPT_CENTER,   //!< Center will be used. This is similar to an OcTree.
    KDTREECONCEPT_AVERAGE,  //!< The average vertex position will be used to determine the next kd-Tree node construction.
};


/**
This tree builder namespace builds or rather constructs all hierarchical trees.
It builds particular trees for collision detection purposes.
\note Building a tree for a mesh can modify your geometry object. Sometimes triangles
will be cut and divided into two parts.
*/
namespace TreeBuilder
{

/**
Builds a kd-Tree with scene::SCollisionFace data packets as user data.
\param MeshList: Specifies the list of meshes. Only one large kd-Tree will be constructed out of several mesh objects.
\param MaxForkLevel: Specifies the maximal fork level. Some leaf nodes can have a lower level. Don't make it too large!
\param Concept: Specifies the concept of building the kd-Tree.
\param PreTransform: Specifies whether the triangles are to be pre-transformed or not. i.e. the triangles
will be transformed by the mesh matrices.
\return Pointer to the root tree node or 0 if the given meshes have no triangles.
*/
SP_EXPORT KDTreeNode* buildKdTree(
    const std::list<Mesh*> &MeshList, u8 MaxTreeLevel = 12,
    const EKDTreeBuildingConcepts Concept = KDTREECONCEPT_CENTER, bool PreTransform = true
);

//! Builds a kd-Tree only for one mesh object.
SP_EXPORT KDTreeNode* buildKdTree(
    Mesh* Object, u8 MaxTreeLevel = 12, const EKDTreeBuildingConcepts Concept = KDTREECONCEPT_CENTER
);

SP_EXPORT OcTreeNode* buildOcTree(Mesh* Object, u8 MaxTreeLevel = 4);
SP_EXPORT QuadTreeNode* buildQuadTree(Mesh* Object, u8 MaxTreeLevel = 6);
SP_EXPORT BSPTreeNode* buildBSPTree(Mesh* Object, u8 MaxTreeLevel = 12);
SP_EXPORT OBBTreeNode* buildOBBTree(const std::list<dim::obbox3df> &BoxList);

} // /namespace TreeBuilder


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
