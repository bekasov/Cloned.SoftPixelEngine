/*
 * Tree node oct file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spTreeNodeOct.hpp"
#include "Base/spMathCollisionLibrary.hpp"
#include "SceneGraph/spSceneMesh.hpp"


namespace sp
{
namespace scene
{


OcTreeNode::OcTreeNode(TreeNode* Parent)
    : TreeNode(Parent, TREENODE_OCTREE), Children_(0)
{
}
OcTreeNode::~OcTreeNode()
{
    removeChildren();
}

bool OcTreeNode::isLeaf() const
{
    return !Children_;
}

void OcTreeNode::addChildren()
{
    if (!Children_)
    {
        Children_ = new OcTreeNode*[8];
        for (s32 i = 0; i < 8; ++i)
            Children_[i] = new OcTreeNode(this);
    }
}
void OcTreeNode::removeChildren()
{
    if (Children_)
    {
        for (s32 i = 0; i < 8; ++i)
            MemoryManager::deleteMemory(Children_[i]);
        MemoryManager::deleteBuffer(Children_);
    }
}

void OcTreeNode::createTree(Mesh* Model, s8 ForksCount)
{
    if (!Model)
        return;
    
    // Limit count of tree forks
    clampTreeForks(ForksCount);
    
    // Temporary variables
    s32 s, i, c = (s32)(pow(2.0f, ForksCount) + 0.5f);
    
    dim::triangle3df Triangle;
    dim::line3df BoundBox;
    dim::line3di BoundBoxOffset;
    dim::vector3di OffsetPos;
    
    // Create all oct-tree-nodes
    Model->getMeshBoundingBox(Min_, Max_);
    
    Min_ -= EXT_BOUNDBOX_SIZE;
    Max_ += EXT_BOUNDBOX_SIZE;
    
    OcTreeNode** pTreeNodeList = new OcTreeNode*[c*c*c];
    memset(pTreeNodeList, 0, sizeof(OcTreeNode*)*c*c*c);
    
    createChildren(pTreeNodeList, ForksCount, Min_, Max_, c);
    
    // Loop for each mesh's triangle and add it to the right oct-tree-node
    for (s = 0; s < Model->getMeshBufferCount(); ++s)
    {
        video::MeshBuffer* Surface = Model->getMeshBuffer(s);
        
        for (i = 0; i < Surface->getTriangleCount(); ++i)
        {
            Triangle = Surface->getTriangleCoords(i);
            BoundBox = Triangle.getBox();
            
            BoundBoxOffset.Start    = getPositionOffset(BoundBox.Start, Min_, Max_, c);
            BoundBoxOffset.End      = getPositionOffset(BoundBox.End, Min_, Max_, c);
            
            for (OffsetPos.Z = BoundBoxOffset.Start.Z; OffsetPos.Z <= BoundBoxOffset.End.Z; ++OffsetPos.Z)
             for (OffsetPos.Y = BoundBoxOffset.Start.Y; OffsetPos.Y <= BoundBoxOffset.End.Y; ++OffsetPos.Y)
              for (OffsetPos.X = BoundBoxOffset.Start.X; OffsetPos.X <= BoundBoxOffset.End.X; ++OffsetPos.X)
                placeTriangle(pTreeNodeList, Surface->getTriangleReference(i), i, s, OffsetPos, c);
        }
    }
    
    // Clean up
    delete [] pTreeNodeList;
}

void OcTreeNode::createTree(const std::list<SceneNode*> &NodeList, s8 ForksCount)
{
    if (!NodeList.size())
        return;
    
    // Limit count of tree forks
    clampTreeForks(ForksCount);
    
    // Temporary variables
    s32 s, i, c = (s32)(pow(2.0f, ForksCount) + 0.5f);
    
    dim::vector3df Pos;
    dim::vector3di OffsetPos;
    
    // Compute the bounding box
    Min_ = 99999.f;
    Max_ = -99999.f;
    
    for (std::list<SceneNode*>::const_iterator it = NodeList.begin(); it != NodeList.end(); ++it)
    {
        Pos = (*it)->getTransformation(true).getPosition();
        
        if (Pos.X < Min_.X) Min_.X = Pos.X;
        if (Pos.Y < Min_.Y) Min_.Y = Pos.Y;
        if (Pos.Z < Min_.Z) Min_.Z = Pos.Z;
        
        if (Pos.X > Max_.X) Max_.X = Pos.X;
        if (Pos.Y > Max_.Y) Max_.Y = Pos.Y;
        if (Pos.Z > Max_.Z) Max_.Z = Pos.Z;
    }
    
    Min_ -= EXT_BOUNDBOX_SIZE;
    Max_ += EXT_BOUNDBOX_SIZE;
    
    // Create each oct-tree-node
    OcTreeNode** pTreeNodeList = new OcTreeNode*[c*c*c];
    memset(pTreeNodeList, 0, sizeof(OcTreeNode*)*c*c*c);
    
    createChildren(pTreeNodeList, ForksCount, Min_, Max_, c);
    
    // Loop for each node and add it to the right oct-tree-node
    for (std::list<SceneNode*>::const_iterator it = NodeList.begin(); it != NodeList.end(); ++it)
    {
        OffsetPos = getPositionOffset(
            (*it)->getTransformation(true).getPosition(), Min_, Max_, c
        );
        
        placeNode(pTreeNodeList, *it, OffsetPos, c);
    }
    
    // Clean up
    delete [] pTreeNodeList;
}

void OcTreeNode::findTreeNodes(
    std::list<const OcTreeNode*> &TreeNodeList, const dim::vector3df &Pos) const
{
    if (!isLeaf())
    {
        for (s32 i = 0; i < 8; ++i)
        {
            if ( Pos.X >= Min_.X && Pos.Y >= Min_.Y && Pos.Z >= Min_.Z &&
                 Pos.X <= Max_.X && Pos.Y <= Max_.Y && Pos.Z <= Max_.Z )
            {
                Children_[i]->findTreeNodes(TreeNodeList, Pos);
                return;
            }
        }
    }
    else if (getUserData())
        TreeNodeList.push_back(this);
}

void OcTreeNode::findTreeNodes(
    std::list<const OcTreeNode*> &TreeNodeList, const dim::vector3df &Pos, const dim::vector3df &Radius) const
{
    if (!isLeaf())
    {
        for (s32 i = 0; i < 8; ++i)
        {
            if ( Pos.X >= (Min_.X - Radius.X) &&
                 Pos.Y >= (Min_.Y - Radius.Y) &&
                 Pos.Z >= (Min_.Z - Radius.Z) &&
                 Pos.X <= (Max_.X + Radius.X) &&
                 Pos.Y <= (Max_.Y + Radius.Y) &&
                 Pos.Z <= (Max_.Z + Radius.Z) )
            {
                Children_[i]->findTreeNodes(TreeNodeList, Pos, Radius);
            }
        }
    }
    else if (getUserData())
        TreeNodeList.push_back(this);
}

void OcTreeNode::findTreeNodes(
    std::list<const OcTreeNode*> &TreeNodeList, const dim::line3df &Ray) const
{
    if (!isLeaf())
    {
        for (s32 i = 0; i < 8; ++i)
        {
            if (math::CollisionLibrary::checkLineBoxOverlap(Ray, dim::aabbox3df(Min_, Max_)))
                Children_[i]->findTreeNodes(TreeNodeList, Ray);
        }
    }
    else if (getUserData())
        TreeNodeList.push_back(this);
}


/*
 * ======= Private: =======
 */

void OcTreeNode::createChildren(
    OcTreeNode** pTreeNodeList, s8 &ForksCount,
    const dim::vector3df &Min, const dim::vector3df &Max, const s32 LineCount)
{
    #define mcrAddChild(i, p)                                                           \
        Children_[i] = new OcTreeNode();                                                \
        Children_[i]->Min_ = p;                                                         \
        Children_[i]->Max_ = p + Size;                                                  \
        Children_[i]->createChildren(pTreeNodeList, ForksCount, Min, Max, LineCount);
    
    if (--ForksCount >= 0)
    {
        const dim::vector3df Size((Max_ - Min_)/2);
        
        addChildren();
        
        mcrAddChild(0, dim::vector3df(Min_.X         , Min_.Y         , Min_.Z         ));
        mcrAddChild(1, dim::vector3df(Min_.X + Size.X, Min_.Y         , Min_.Z         ));
        mcrAddChild(2, dim::vector3df(Min_.X         , Min_.Y + Size.Y, Min_.Z         ));
        mcrAddChild(3, dim::vector3df(Min_.X + Size.X, Min_.Y + Size.Y, Min_.Z         ));
        mcrAddChild(4, dim::vector3df(Min_.X         , Min_.Y         , Min_.Z + Size.Z));
        mcrAddChild(5, dim::vector3df(Min_.X + Size.X, Min_.Y         , Min_.Z + Size.Z));
        mcrAddChild(6, dim::vector3df(Min_.X         , Min_.Y + Size.Y, Min_.Z + Size.Z));
        mcrAddChild(7, dim::vector3df(Min_.X + Size.X, Min_.Y + Size.Y, Min_.Z + Size.Z));
    }
    else
    {
        const s32 Offset = getOffset(
            getPositionOffset((Min_ + Max_)/2, Min, Max, LineCount), LineCount
        );
        
        pTreeNodeList[Offset] = this;
    }
    
    ++ForksCount;
    
    #undef mcrAddChild
}

void OcTreeNode::placeTriangle(
    OcTreeNode** pTreeNodeList,
    const dim::ptriangle3df &TriangleLink, const u32 TriangleIndex, const u32 CurrentSurface,
    const dim::vector3di OffsetPos, const s32 LineCount)
{
    const s32 Offset = getOffset(OffsetPos, LineCount);
    
    if (Offset >= LineCount*LineCount*LineCount)
    {
        io::Log::error("Offset in oct-tree out of range");
        return;
    }
    
    // Get the tree node
    OcTreeNode* TreeNode = pTreeNodeList[Offset];
    
    if (!TreeNode)
        return;
    
    // Determine if the triangle realy overlaps with the AABB oct-tree-node
    if (!math::CollisionLibrary::checkTriangleBoxOverlap(
            dim::triangle3df(*TriangleLink.PointA, *TriangleLink.PointB, *TriangleLink.PointC),
            dim::aabbox3df(TreeNode->Min_, TreeNode->Max_)))
    {
        return;
    }
    
    // Add the triangle to the node
    std::list<STreeNodeTriangleData>* TriangleData = (std::list<STreeNodeTriangleData>*)TreeNode->getUserData();
    
    if (!TriangleData)
        TreeNode->setUserData(TriangleData = new std::list<STreeNodeTriangleData>);
    
    STreeNodeTriangleData EntryData;
    {
        EntryData.Index      = TriangleIndex;
        EntryData.Surface    = CurrentSurface;
        EntryData.Triangle   = TriangleLink;
    }
    TriangleData->push_back(EntryData);
}

void OcTreeNode::placeNode(
    OcTreeNode** pTreeNodeList, SceneNode* ObjNode, const dim::vector3di OffsetPos, const s32 LineCount)
{
    const s32 Offset = getOffset(OffsetPos, LineCount);
    
    if (Offset >= LineCount*LineCount*LineCount)
    {
        io::Log::error("Offset in oct-tree out of range");
        return;
    }
    
    // Get the tree node
    OcTreeNode* TreeNode = pTreeNodeList[Offset];
    
    if (!TreeNode)
        return;
    
    // Add the triangle to the node
    std::list<SceneNode*>* NodeObjectData = (std::list<SceneNode*>*)TreeNode->getUserData();
    
    if (!NodeObjectData)
        TreeNode->setUserData(NodeObjectData = new std::list<SceneNode*>);
    
    NodeObjectData->push_back(ObjNode);
}

dim::vector3di OcTreeNode::getPositionOffset(
    dim::vector3df Pos, const dim::vector3df &Min, const dim::vector3df &Max, const s32 LineCount) const
{
    Pos -= Min;
    Pos /= (Max - Min);
    Pos *= LineCount;
    return dim::vector3di((s32)Pos.X, (s32)Pos.Y, (s32)Pos.Z);
}

void OcTreeNode::clampTreeForks(s8 &ForksCount)
{
    if (ForksCount < 0)
    {
        io::Log::warning("Cannot create oct-tree with less than 0 forks");
        ForksCount = 0;
    }
    else if (ForksCount > MAX_TREENODE_FORKSCOUNT)
    {
        io::Log::warning(
            "Cannot create oct-tree with more than " + io::stringc(MAX_TREENODE_FORKSCOUNT) +
            " forks (that are " + io::stringc(pow(8.0f, MAX_TREENODE_FORKSCOUNT)) + " nodes)"
        );
        ForksCount = MAX_TREENODE_FORKSCOUNT;
    }
}


} // /namespace scene

} // /namespace sp



// ================================================================================
