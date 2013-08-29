/*
 * Tree node BSP file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spTreeNodeBSP.hpp"


namespace sp
{
namespace scene
{


BSPTreeNode::BSPTreeNode(TreeNode* Parent) :
    TreeNode    (Parent, TREENODE_BSPTREE   ),
    ChildFront_ (0                          ),
    ChildBack_  (0                          )
{
}
BSPTreeNode::~BSPTreeNode()
{
    removeChildren();
}

u32 BSPTreeNode::getNumChildren() const
{
    return ChildFront_ != 0 ? (2 + ChildFront_->getNumChildren() + ChildBack_->getNumChildren()) : 0;
}

bool BSPTreeNode::isLeaf() const
{
    return !ChildFront_;
}

void BSPTreeNode::addChildren()
{
    if (!ChildFront_)
    {
        ChildFront_ = new BSPTreeNode(this);
        ChildBack_  = new BSPTreeNode(this);
    }
}
void BSPTreeNode::removeChildren()
{
    MemoryManager::deleteMemory(ChildFront_);
    MemoryManager::deleteMemory(ChildBack_);
}

void BSPTreeNode::findTreeNodes(
    std::list<const BSPTreeNode*> &TreeNodeList, const dim::vector3df &Pos) const
{
    if (ChildFront_)
    {
        if (Plane_.isPointFrontSide(Pos))
            ChildFront_->findTreeNodes(TreeNodeList, Pos);
        else
            ChildBack_->findTreeNodes(TreeNodeList, Pos);
    }
    else if (getUserData())
        TreeNodeList.push_back(this);
}

void BSPTreeNode::findTreeNodes(
    std::list<const BSPTreeNode*> &TreeNodeList, const dim::vector3df &Pos, const f32 Radius) const
{
    if (ChildFront_)
    {
        const f32 Distance = Plane_.getPointDistance(Pos);
        
        if (Distance > -Radius)
            ChildFront_->findTreeNodes(TreeNodeList, Pos, Radius);
        if (Distance < Radius)
            ChildBack_->findTreeNodes(TreeNodeList, Pos, Radius);
    }
    else if (getUserData())
        TreeNodeList.push_back(this);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
