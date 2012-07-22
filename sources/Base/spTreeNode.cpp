/*
 * Tree node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spTreeNode.hpp"


namespace sp
{
namespace scene
{


const f32 TreeNode::EXT_BOUNDBOX_SIZE = 0.01f;

TreeNode::TreeNode(TreeNode* Parent, const ETreeNodeTypes Type) :
    Type_               (Type   ),
    Parent_             (Parent ),
    DestructorCallback_ (0      )
{
}
TreeNode::~TreeNode()
{
    if (DestructorCallback_)
        DestructorCallback_(this);
}

const TreeNode* TreeNode::getRoot() const
{
    if (Parent_)
        return Parent_->getRoot();
    return this;
}
TreeNode* TreeNode::getRoot()
{
    if (Parent_)
        return Parent_->getRoot();
    return this;
}

u32 TreeNode::getLevel() const
{
    if (Parent_)
        return 1 + Parent_->getLevel();
    return 0;
}

void TreeNode::addChildren()
{
    // do nothing
}
void TreeNode::removeChildren()
{
    // do nothing
}

const TreeNode* TreeNode::findLeaf(const dim::vector3df &Point) const
{
    return 0; // do nothing
}
void TreeNode::findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::vector3df &Point, f32 Radius) const
{
    // do nothing
}
void TreeNode::findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::line3df &Line) const
{
    // do nothing
}
void TreeNode::findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::line3df &Line, f32 Radius) const
{
    // do nothing
}

const TreeNode* TreeNode::findLeafSub(const dim::vector3df &Point) const
{
    return findLeaf(Point);
}
void TreeNode::findLeafListSub(std::list<const TreeNode*> &TreeNodeList, const dim::vector3df &Point, f32 Radius) const
{
    return findLeafList(TreeNodeList, Point, Radius);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
