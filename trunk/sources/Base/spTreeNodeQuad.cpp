/*
 * Tree node quad file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spTreeNodeQuad.hpp"


namespace sp
{
namespace scene
{


QuadTreeNode::QuadTreeNode(TreeNode* Parent) :
    TreeNode    (Parent, TREENODE_QUADTREE  ),
    Children_   (0                          )
{
}
QuadTreeNode::~QuadTreeNode()
{
    removeChildren();
}

u32 QuadTreeNode::getNumChildren() const
{
    if (Children_ != 0)
    {
        u32 Num = 4;
        
        for (u32 i = 0; i < 4; ++i)
            Num += Children_[i]->getNumChildren();
        
        return Num;
    }
    return 0;
}

bool QuadTreeNode::isLeaf() const
{
    return !Children_;
}

void QuadTreeNode::addChildren()
{
    if (!Children_)
    {
        Children_ = new QuadTreeNode*[4];
        for (s32 i = 0; i < 4; ++i)
            Children_[i] = new QuadTreeNode(this);
    }    
}
void QuadTreeNode::removeChildren()
{
    if (Children_)
    {
        for (s32 i = 0; i < 4; ++i)
            MemoryManager::deleteMemory(Children_[i]);
        MemoryManager::deleteBuffer(Children_);
    }
}

void QuadTreeNode::findTreeNodes(
    std::list<const QuadTreeNode*> &TreeNodeList, const dim::point2df &Pos) const
{
    if (Children_)
    {
        for (s32 i = 0; i < 4; ++i)
        {
            if (Pos.X >= Min_.X && Pos.Y >= Min_.Y && Pos.X <= Max_.X && Pos.Y <= Max_.Y)
            {
                Children_[i]->findTreeNodes(TreeNodeList, Pos);
                return;
            }
        }
    }
    else if (getUserData())
        TreeNodeList.push_back(this);
}

void QuadTreeNode::findTreeNodes(
    std::list<const QuadTreeNode*> &TreeNodeList, const dim::point2df &Pos, const dim::size2df &Radius) const
{
    if (Children_)
    {
        for (s32 i = 0; i < 4; ++i)
        {
            if ( Pos.X >= (Min_.X - Radius.Width) &&
                 Pos.Y >= (Min_.Y - Radius.Height) &&
                 Pos.X <= (Max_.X + Radius.Width) &&
                 Pos.Y <= (Max_.Y + Radius.Height) )
            {
                Children_[i]->findTreeNodes(TreeNodeList, Pos, Radius);
            }
        }
    }
    else if (getUserData())
        TreeNodeList.push_back(this);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
