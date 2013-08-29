/*
 * Tree node quad header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TREENODEQUAD_H__
#define __SP_TREENODEQUAD_H__


#include "Base/spStandard.hpp"
#include "Base/spTreeNode.hpp"


namespace sp
{
namespace scene
{


//! Quad tree node class used for terrain patches and optimization purposes.
class SP_EXPORT QuadTreeNode : public TreeNode
{
    
    public:
        
        QuadTreeNode(TreeNode* Parent = 0);
        ~QuadTreeNode();
        
        /* === Functions === */
        
        u32 getNumChildren() const;
        bool isLeaf() const;
        
        void addChildren();
        void removeChildren();
        
        /**
        Searchs each quad node where the specified position is inside.
        Only those quad nodes are searched which have no children.
        \param TreeNodeList: Resulting list with all found QuadTreeNode objects.
        This must be a reference and no function's return value because it will be called recursive for
        each next tree node child.
        */
        void findTreeNodes(
            std::list<const QuadTreeNode*> &TreeNodeList, const dim::point2df &Pos
        ) const;
        
        /**
        Does the same like the first "findTreeNodes" function but considers the radius which occurs that sometimes
        more than one tree node can be found.
        */
        void findTreeNodes(
            std::list<const QuadTreeNode*> &TreeNodeList, const dim::point2df &Pos, const dim::size2df &Radius
        ) const;
        
        /* === Inline functions === */
        
        //! \return Pointer the the specified QuadTreeNode child object(index must be in a range of [0, 3]).
        inline QuadTreeNode* getChild(u32 Index) const
        {
            return (Children_ && Index < 4) ? Children_[Index] : 0;
        }
        
    private:
        
        /* === Members === */
        
        QuadTreeNode** Children_;
        
        dim::point2df Min_, Max_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
