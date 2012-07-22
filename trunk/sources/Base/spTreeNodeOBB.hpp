/*
 * Tree node obbox header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TREENODEOBB_H__
#define __SP_TREENODEOBB_H__


#include "Base/spStandard.hpp"
#include "Base/spTreeNode.hpp"


namespace sp
{
namespace scene
{


//! Oriented bounding box tree node class used for bounding volume hierarchies.
class SP_EXPORT OBBTreeNode : public TreeNode
{
    
    public:
        
        OBBTreeNode(OBBTreeNode* Parent = 0, const dim::obbox3df &Box = dim::obbox3df());
        ~OBBTreeNode();
        
        /* === Functions === */
        
        bool isLeaf() const;
        
        const TreeNode* findLeaf(const dim::vector3df &Point) const;
        void findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::vector3df &Point, f32 Radius) const;
        void findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::line3df &Line) const;
        void findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::line3df &Line, f32 Radius) const;
        
        /**
        Inserts the specified bounding box into the tree. If this box is fully inside the given
        box, the new tree node will become the new parent node.
        \return Pointer to the new OBBTreeNode object if the new box could be inserted.
        \note The best way to insert new boxes is to do that at the root node.
        */
        OBBTreeNode* insertBoundingBox(const dim::obbox3df &Box);
        
        //! Inserts the given OBB tree node into this node.
        bool insertBoundingBox(OBBTreeNode* Node);
        
        //! Returns the oriented-bounding box.
        void setBox(const dim::obbox3df &Box);
        
        //! Updates the hierarchy of this node.
        void update(bool UpdateChildren = true);
        
        /* === Inline functions === */
        
        //! Returns the children list.
        inline const std::list<TreeNode*>& getChildren() const
        {
            return Children_;
        }
        
        //! Returns the bounding box of the OBB tree node.
        inline dim::obbox3df getBox() const
        {
            return Box_;
        }
        
    private:
        
        /* Functions */
        
        OBBTreeNode* createChild(const dim::obbox3df &Box);
        
        void removeFromParent();
        void insertThisUpper(OBBTreeNode* Parent);
        
        /* Inline functions */
        
        inline OBBTreeNode* addChild(OBBTreeNode* Child)
        {
            Children_.push_back(Child);
            return Child;
        }
        
        /* Members */
        
        std::list<TreeNode*> Children_;
        
        dim::obbox3df Box_;
        dim::matrix4f InvBoxTransformation_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
