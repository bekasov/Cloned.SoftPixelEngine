/*
 * Tree node KD file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spTreeNodeKD.hpp"
#include "Base/spMathCollisionLibrary.hpp"
#include "SceneGraph/Collision/spCollisionCapsule.hpp"


namespace sp
{
namespace scene
{


KDTreeNode::KDTreeNode(TreeNode* Parent, const dim::aabbox3df &Box) :
    TreeNode    (Parent, TREENODE_KDTREE),
    ChildNear_  (0                      ),
    ChildFar_   (0                      ),
    Axis_       (KDTREE_XAXIS           ),
    Distance_   (0.0f                   ),
    Box_        (Box                    )
{
}
KDTreeNode::~KDTreeNode()
{
    removeChildren();
}

bool KDTreeNode::isLeaf() const
{
    return !ChildNear_;
}

void KDTreeNode::addChildren()
{
    addChildren<KDTreeNode>();
}
void KDTreeNode::removeChildren()
{
    MemoryManager::deleteMemory(ChildNear_);
    MemoryManager::deleteMemory(ChildFar_);
}

const TreeNode* KDTreeNode::findLeaf(const dim::vector3df &Point) const
{
    if (Box_.isPointInside(Point))
        return findLeafSub(Point);
    return 0;
}

void KDTreeNode::findLeafList(
    std::list<const TreeNode*> &TreeNodeList, const dim::vector3df &Point, f32 Radius) const
{
    /* Check for early culling by a box-box intersection test first for performance reasons*/
    if (Box_.checkBoxBoxIntersection(dim::aabbox3df(Point - Radius, Point + Radius)) &&
        math::CollisionLibrary::getPointBoxDistanceSq(dim::obbox3df(Box_.Min, Box_.Max), Point) < math::Pow2(Radius))
    {
        findLeafListSub(TreeNodeList, Point, Radius);
    }
}

void KDTreeNode::findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::line3df &Line) const
{
    if (math::CollisionLibrary::checkLineBoxOverlap(Line, Box_))
    {
        if (ChildNear_)
        {
            ChildNear_->findLeafList(TreeNodeList, Line);
            ChildFar_->findLeafList(TreeNodeList, Line);
        }
        else
            TreeNodeList.push_back(this);
    }
}

void KDTreeNode::findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::line3df &Line, f32 Radius) const
{
    const dim::obbox3df ThisBox(Box_.Min, Box_.Max);
    const dim::obbox3df LineBox(CollisionCapsule::getBoundBoxFromLine(Line, Radius));
    
    /* Check for early culling by a box-box intersection test first for performance reasons*/
    if (math::CollisionLibrary::checkOBBoxOBBoxOverlap(ThisBox, LineBox))
    {
        if (ChildNear_)
        {
            ChildNear_->findLeafList(TreeNodeList, Line, Radius);
            ChildFar_->findLeafList(TreeNodeList, Line, Radius);
        }
        else
            TreeNodeList.push_back(this);
    }
}

const TreeNode* KDTreeNode::findLeafSub(const dim::vector3df &Point) const
{
    if (ChildNear_)
    {
        return (Point[Axis_] < Distance_) ?
            ChildNear_->findLeafSub(Point) :
            ChildFar_->findLeafSub(Point);
    }
    return this;
}

void KDTreeNode::findLeafListSub(
    std::list<const TreeNode*> &TreeNodeList, const dim::vector3df &Point, f32 Radius) const
{
    if (ChildNear_)
    {
        if (Point[Axis_] < Distance_ + Radius)
            ChildNear_->findLeafListSub(TreeNodeList, Point, Radius);
        if (Point[Axis_] > Distance_ - Radius)
            ChildFar_->findLeafListSub(TreeNodeList, Point, Radius);
    }
    else
        TreeNodeList.push_back(this);
}


/*
 * ======= Private: =======
 */

void KDTreeNode::getBoxes(dim::aabbox3df &NearBox, dim::aabbox3df &FarBox) const
{
    dim::vector3df NearMax, FarMin;
    
    switch (Axis_)
    {
        case KDTREE_XAXIS:
            NearMax = dim::vector3df(Distance_, Box_.Max.Y, Box_.Max.Z);
            FarMin  = dim::vector3df(Distance_, Box_.Min.Y, Box_.Min.Z);
            break;
        case KDTREE_YAXIS:
            NearMax = dim::vector3df(Box_.Max.X, Distance_, Box_.Max.Z);
            FarMin  = dim::vector3df(Box_.Min.X, Distance_, Box_.Min.Z);
            break;
        case KDTREE_ZAXIS:
            NearMax = dim::vector3df(Box_.Max.X, Box_.Max.Y, Distance_);
            FarMin  = dim::vector3df(Box_.Min.X, Box_.Min.Y, Distance_);
            break;
    }
    
    NearBox = dim::aabbox3df(Box_.Min, NearMax);
    FarBox  = dim::aabbox3df(FarMin, Box_.Max);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
