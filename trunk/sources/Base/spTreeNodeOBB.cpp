/*
 * Tree node obbox file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spTreeNodeOBB.hpp"
#include "Base/spMathCollisionLibrary.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


OBBTreeNode::OBBTreeNode(OBBTreeNode* Parent, const dim::obbox3df &Box) :
    TreeNode    (Parent, TREENODE_OBBTREE   ),
    Box_        (Box                        )
{
}
OBBTreeNode::~OBBTreeNode()
{
    MemoryManager::deleteList(Children_);
}

bool OBBTreeNode::isLeaf() const
{
    return Children_.empty();
}

const TreeNode* OBBTreeNode::findLeaf(const dim::vector3df &Point) const
{
    if (Box_.isPointInside(Point))
    {
        foreach (TreeNode* Node, Children_)
        {
            const TreeNode* Leaf = Node->findLeaf(Point);
            if (Leaf)
                return Leaf;
        }
        return this;
    }
    return 0;
}

void OBBTreeNode::findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::vector3df &Point, f32 Radius) const
{
    if (math::CollisionLibrary::getPointBoxDistanceSq(Box_, Point) < math::pow2(Radius))
    {
        if (!Children_.empty())
        {
            foreach (TreeNode* Node, Children_)
                Node->findLeafList(TreeNodeList, Point, Radius);
        }
        else
            TreeNodeList.push_back(this);
    }
}

void OBBTreeNode::findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::line3df &Line) const
{
    if (math::CollisionLibrary::checkLineBoxOverlap(InvBoxTransformation_ * Line, dim::aabbox3df::IDENTITY))
    {
        if (!Children_.empty())
        {
            foreach (TreeNode* Node, Children_)
                Node->findLeafList(TreeNodeList, Line);
        }
        else
            TreeNodeList.push_back(this);
    }
}

void OBBTreeNode::findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::line3df &Line, f32 Radius) const
{
    if (math::CollisionLibrary::getLineBoxDistanceSq(dim::aabbox3df::IDENTITY, InvBoxTransformation_ * Line) < math::pow2(Radius))
    {
        if (!Children_.empty())
        {
            foreach (TreeNode* Node, Children_)
                Node->findLeafList(TreeNodeList, Line, Radius);
        }
        else
            TreeNodeList.push_back(this);
    }
}

OBBTreeNode* OBBTreeNode::insertBoundingBox(const dim::obbox3df &Box)
{
    /* Check if the box can be inserted into this box */
    if (Box_.isBoxInside(Box))
    {
        /* Check if the box can be inserted into a child */
        foreach (TreeNode* Child, Children_)
        {
            if (Child->getType() == TREENODE_OBBTREE)
            {
                OBBTreeNode* Node = static_cast<OBBTreeNode*>(Child)->insertBoundingBox(Box);
                if (Node)
                    return Node;
            }
        }
        
        /* Create a new child */
        return createChild(Box);
    }
    return 0;
}

bool OBBTreeNode::insertBoundingBox(OBBTreeNode* Node)
{
    /* Check if the box can be inserted into this box */
    if (Node && Box_.isBoxInside(Node->getBox()))
    {
        /* Check if the box can be inserted into a child */
        foreach (TreeNode* Child, Children_)
        {
            if (Child->getType() == TREENODE_OBBTREE && static_cast<OBBTreeNode*>(Child)->insertBoundingBox(Node))
                return true;
        }
        
        /* Add to this children list */
        addChild(Node);
        
        return true;
    }
    return false;
}

void OBBTreeNode::setBox(const dim::obbox3df &Box)
{
    Box_ = Box;
    InvBoxTransformation_ = dim::matrix4f(Box_).getInverse();
}

void OBBTreeNode::update(bool UpdateChildren)
{
    /* Root nodes must not be updated */
    if (!Parent_ || Parent_->getType() != TREENODE_OBBTREE)
        return;
    
    OBBTreeNode* Parent = static_cast<OBBTreeNode*>(Parent_);
    
    if (UpdateChildren)
    {
        /* Check for each child if it's no longer inside this box and must be updated */
        for (std::list<TreeNode*>::iterator it = Children_.begin(); it != Children_.end();)
        {
            if ((*it)->getType() == TREENODE_OBBTREE && !Box_.isBoxInside(static_cast<OBBTreeNode*>(*it)->getBox()))
            {
                /* Exchange the child to the upper parent */
                Parent->addChild(static_cast<OBBTreeNode*>(*it));
                it = Children_.erase(it);
            }
            else
                ++it;
        }
    }
    
    /* Check if this box is still inside it's parent box */
    insertThisUpper(Parent);
}


/*
 * ======= Private: =======
 */

OBBTreeNode* OBBTreeNode::createChild(const dim::obbox3df &Box)
{
    return addChild(new OBBTreeNode(this, Box));
}

void OBBTreeNode::removeFromParent()
{
    if (Parent_ && Parent_->getType() == TREENODE_OBBTREE)
    {
        TreeNode* Child = this;
        MemoryManager::removeElement(static_cast<OBBTreeNode*>(Parent_)->Children_, Child);
    }
}

void OBBTreeNode::insertThisUpper(OBBTreeNode* Parent)
{
    if (Parent->getBox().isBoxInside(Box_))
    {
        /* Check if the box can be inserted into a parent's child */
        foreach (TreeNode* Child, Parent->Children_)
        {
            if (Child && Child != this && Child->getType() == TREENODE_OBBTREE &&
                static_cast<OBBTreeNode*>(Child)->insertBoundingBox(this))
            {
                removeFromParent();
                return;
            }
        }
    }
    else if (Parent->getParent() && Parent->getParent()->getType() == TREENODE_OBBTREE)
    {
        insertThisUpper(static_cast<OBBTreeNode*>(Parent->getParent()));
        return;
    }
    
    /* Check if this box must be inserted into the current parent */
    if (Parent != Parent_)
    {
        removeFromParent();
        Parent->addChild(this);
    }
}


} // /namespace scene

} // /namespace sp



// ================================================================================
