/*
 * Tree node oct header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TREENODEOCT_H__
#define __SP_TREENODEOCT_H__


#include "Base/spStandard.hpp"
#include "Base/spTreeNode.hpp"


namespace sp
{
namespace scene
{


//! Oct tree (or rather OcTree) node class used for collision detection and intersection optimization.
class SP_EXPORT OcTreeNode : public TreeNode
{
    
    public:
        
        OcTreeNode(TreeNode* Parent = 0);
        ~OcTreeNode();
        
        /* === Functions === */
        
        u32 getNumChildren() const;
        bool isLeaf() const;
        
        void addChildren();
        void removeChildren();
        
        /**
        Creates a complete tree for a Mesh object with the specified count of forks.
        By this creation the tree nodes' user data will be "std::list<STreeNodeTriangleData>*".
        \param Model: Pointer to the Mesh object for which the tree is to be created.
        \param ForksCount: Count of forks the tree shall has. If the count is 0 only one OcTreeNode object
        will be created. This would be pointless for optimization purposes. By default the count amounts 3.
        It can not be greater than 8 because this will occurs a tree with 8^8 (16.777.216) tree nodes!
        \deprecated Since 3.2
        */
        void createTree(Mesh* Model, s8 ForksCount = DEF_TREENODE_FORKSCOUNT);
        
        /**
        Creates a complete tree for the Node objects with the specified count of forks.
        By this creation the tree nodes' user data will be "std::list<SceneNode*>*".
        This function does not effect the Node objects but it will result a tree hierarchy where the Node
        objects are sorted by their position.
        \param NodeList: List of all Node objects which shall be placed into a tree hierarchy.
        \param ForksCount: Count of forks the tree shall has.
        \deprecated Since 3.2
        */
        void createTree(const std::list<SceneNode*> &NodeList, s8 ForksCount = DEF_TREENODE_FORKSCOUNT);
        
        /**
        Searchs each oct node where the specified position is inside.
        Only those oct nodes are searched which have no children.
        \param TreeNodeList: Resulting list with all found OcTreeNode objects.
        This must be a reference and no function's return value because it will be called recursive for
        each next tree node child.
        \deprecated Since 3.2
        */
        void findTreeNodes(
            std::list<const OcTreeNode*> &TreeNodeList, const dim::vector3df &Pos
        ) const;
        
        /**
        Does the same like the first "findTreeNodes" function but considers the radius which occurs that sometimes
        more than one tree node can be found.
        \deprecated Since 3.2
        */
        void findTreeNodes(
            std::list<const OcTreeNode*> &TreeNodeList, const dim::vector3df &Pos, const dim::vector3df &Radius
        ) const;
        
        /**
        Searchs each oct node where the specified line intersects with. Only those oct nodes are
        searched which have no children. This function is used for mesh picking.
        \param TreeNodeList: Resulting list with all found OcTreeNode objects.
        \param Ray: Intersection ray (or rather line).
        \deprecated Since 3.2
        */
        void findTreeNodes(
            std::list<const OcTreeNode*> &TreeNodeList, const dim::line3df &Ray
        ) const;
        
        /* === Inline functions === */
        
        //! Returns pointer the the specified OcTreeNode child object (index must be in a range of [0, 7]).
        inline OcTreeNode* getChild(u32 Index) const
        {
            return (Children_ && Index < 8) ? Children_[Index] : 0;
        }
        
    private:
        
        /* === Functions === */
        
        void createChildren(
            OcTreeNode** pTreeNodeList, s8 &ForksCount,
            const dim::vector3df &Min, const dim::vector3df &Max, const s32 LineCount
        );
        
        void placeTriangle(
            OcTreeNode** pTreeNodeList,
            const dim::ptriangle3df &TriangleLink, const u32 TriangleIndex, const u32 CurrentSurface,
            const dim::vector3di OffsetPos, const s32 LineCount
        );
        
        void placeNode(
            OcTreeNode** pTreeNodeList, SceneNode* ObjNode,
            const dim::vector3di OffsetPos, const s32 LineCount
        );
        
        dim::vector3di getPositionOffset(
            dim::vector3df Pos, const dim::vector3df &Min, const dim::vector3df &Max, const s32 LineCount
        ) const;
        
        void clampTreeForks(s8 &ForksCount);
        
        /* === Inline functions === */
        
        inline s32 getOffset(const dim::vector3di &Pos, const s32 LineCount) const
        {
            return Pos.Z * LineCount * LineCount + Pos.Y * LineCount + Pos.X;
        }
        
        /* === Members === */
        
        OcTreeNode** Children_;
        
        dim::vector3df Min_, Max_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
