/*
 * Tree node KD header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TREENODEKD_H__
#define __SP_TREENODEKD_H__


#include "Base/spStandard.hpp"
#include "Base/spTreeNode.hpp"


namespace sp
{
namespace scene
{


static const u8 DEF_KDTREE_LEVEL = 12;


//! kd-Tree node axles.
enum EKDTreeAxles
{
    KDTREE_XAXIS = 0,   //!< X axis.
    KDTREE_YAXIS,       //!< Y axis.
    KDTREE_ZAXIS,       //!< Z axis.
};


//! kd-Tree node class for universal usage.
class SP_EXPORT KDTreeNode : public TreeNode
{
    
    public:
        
        KDTreeNode(TreeNode* Parent = 0, const dim::aabbox3df &Box = dim::aabbox3df());
        ~KDTreeNode();
        
        /* === Functions === */
        
        u32 getNumChildren() const;
        bool isLeaf() const;
        
        void addChildren();
        void removeChildren();
        
        const TreeNode* findLeaf(const dim::vector3df &Point) const;
        void findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::vector3df &Point, f32 Radius) const;
        void findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::line3df &Line) const;
        void findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::line3df &Line, f32 Radius) const;
        
        const TreeNode* findLeafSub(const dim::vector3df &Point) const;
        void findLeafListSub(std::list<const TreeNode*> &TreeNodeList, const dim::vector3df &Point, f32 Radius) const;
        
        /* === Templates === */
        
        template <class A, class B> void addChildren()
        {
            removeChildren();
            
            dim::aabbox3df NearBox, FarBox;
            getBoxes(NearBox, FarBox);
            
            ChildNear_ = createChild<A>(NearBox);
            ChildFar_ = createChild<B>(FarBox);
        }
        
        template <class A, class B> void addChildren(const EKDTreeAxles Axis, f32 Distance)
        {
            Axis_       = Axis;
            Distance_   = Distance;
            addChildren<A, B>();
        }
        
        template <class T> void addChildren()
        {
            addChildren<T, T>();
        }
        template <class T> void addChildren(const EKDTreeAxles Axis, f32 Distance)
        {
            addChildren<T, T>(Axis, Distance);
        }
        
        /* === Inline functions === */
        
        inline TreeNode* getChildNear() const
        {
            return ChildNear_;
        }
        inline TreeNode* getChildFar() const
        {
            return ChildFar_;
        }
        
        inline void setAxis(const EKDTreeAxles Axis)
        {
            Axis_ = Axis;
        }
        inline EKDTreeAxles getAxis() const
        {
            return Axis_;
        }
        
        inline void setDistance(f32 Distance)
        {
            Distance_ = Distance;
        }
        inline f32 getDistance() const
        {
            return Distance_;
        }
        
        inline void setBox(const dim::aabbox3df &Box)
        {
            Box_ = Box;
        }
        inline dim::aabbox3df getBox() const
        {
            return Box_;
        }
        
    private:
        
        /* === Functions === */
        
        void getBoxes(dim::aabbox3df &NearBox, dim::aabbox3df &FarBox) const;
        
        /* === Members === */
        
        TreeNode* ChildNear_;
        TreeNode* ChildFar_;
        
        EKDTreeAxles Axis_;
        f32 Distance_;
        
        dim::aabbox3df Box_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
