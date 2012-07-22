/*
 * Tree node BSP header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TREENODEBSP_H__
#define __SP_TREENODEBSP_H__


#include "Base/spStandard.hpp"
#include "Base/spTreeNode.hpp"


namespace sp
{
namespace scene
{


//! BSP tree node class used for BSP scenes.
class SP_EXPORT BSPTreeNode : public TreeNode
{
    
    public:
        
        BSPTreeNode(TreeNode* Parent = 0);
        ~BSPTreeNode();
        
        /* === Functions === */
        
        bool isLeaf() const;
        
        void addChildren();
        void removeChildren();
        
        /**
        Searchs each BSP node where the specified position is on the front side of the plane.
        Only those BSP nodes are searched which have no children.
        \param TreeNodeList: Resulting list with all found BSPTreeNode objects.
        This must be a reference and no function's return value because it will be called recursive for
        each next tree node child.
        */
        void findTreeNodes(
            std::list<const BSPTreeNode*> &TreeNodeList, const dim::vector3df &Pos
        ) const;
        
        /**
        Does the same like the first "findTreeNodes" function but considers the radius which occurs that sometimes
        more than one tree node can be found.
        */
        void findTreeNodes(
            std::list<const BSPTreeNode*> &TreeNodeList, const dim::vector3df &Pos, const f32 Radius
        ) const;
        
        /* === Inline functions === */
        
        //! Sets the BSP node plane.
        inline void setPlane(const dim::plane3df &Plane)
        {
            Plane_ = Plane;
        }
        
        //! Returns the BSP node plane.
        inline dim::plane3df getPlane() const
        {
            return Plane_;
        }
        
        //! Returns pointer to the BSPTreeNode child object which lies on the front side of the plane.
        inline BSPTreeNode* getChildFront() const
        {
            return ChildFront_;
        }
        
        //! Returns pointer to the BSPTreeNode child object which lies on the back side of the plane.
        inline BSPTreeNode* getChildBack() const
        {
            return ChildBack_;
        }
        
    private:
        
        /* Members */
        
        BSPTreeNode* ChildFront_;
        BSPTreeNode* ChildBack_;
        
        dim::plane3df Plane_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
