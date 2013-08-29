/*
 * Tree node header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TREENODE_H__
#define __SP_TREENODE_H__


#include "Base/spStandard.hpp"
#include "Base/spBaseObject.hpp"
#include "Base/spMemoryManagement.hpp"
#include "Base/spDimension.hpp"
#include "Base/spMath.hpp"

#include <list>
#include <boost/function.hpp>


namespace sp
{
namespace scene
{


class SceneNode;
class Mesh;
class TreeNode;

/*
 * Macros & typedefinitions
 */

static const s32 DEF_TREENODE_FORKSCOUNT = 3;
static const s32 MAX_TREENODE_FORKSCOUNT = 8;

typedef boost::function<void (TreeNode* Node)> TreeNodeDestructorCallback;


/*
 * Enumerations
 */

//! Node types for tree-hierarchy.
enum ETreeNodeTypes
{
    TREENODE_QUADTREE,      //!< QuadTree node (four children).
    TREENODE_POINTQUADTREE, //!< PointQuadTree node (four children).
    TREENODE_OCTREE,        //!< OcTree node (or rather OcTree node, eight children)
    TREENODE_BSPTREE,       //!< BSP (binary-space-partion) tree node (two children).
    TREENODE_KDTREE,        //!< kd-Tree node (k dimensional binary tree).
    TREENODE_OBBTREE,       //!< Oriented Bounding Box tree node (list of children).
};


/*
 * Structures
 */

/**
\todo -> This deprecated structure is only used in the 'OcTreeNode' class.
\see OcTreeNode
\deprecated
*/
struct STreeNodeTriangleData
{
    u32 Surface;
    u32 Index;
    dim::ptriangle3df Triangle;
};


//! This is the tree node base class.
class SP_EXPORT TreeNode : public BaseObject
{
    
    public:
        
        virtual ~TreeNode();
        
        /* === Functions === */
        
        //! Returns a constant pointer to the tree root node.
        virtual const TreeNode* getRoot() const;
        //! Returns a pointer to the tree root node.
        virtual TreeNode* getRoot();
        
        //! Returns the tree level. The root node has level 0, its children 1 and so on.
        virtual u32 getLevel() const;
        
        /**
        Returns the number of children. If this is the root node,
        'getNumChildren() + 1' will be the number of all tree nodes in the whole tree hierarchy.
        */
        virtual u32 getNumChildren() const = 0;
        
        //! Returns true if this is a leaf node. In this case it has no children.
        virtual bool isLeaf() const = 0;
        
        //! Creates the children if it currently has none.
        virtual void addChildren();
        
        //! Deletes the children if it currently has some.
        virtual void removeChildren();
        
        /**
        Searches for a leaf TreeNode object that contains the specified point.
        \param Point: Specifies the point where the node is you are searching for.
        \return Pointer to the TreeNode object which has been found. If no tree node has been found the return value is null.
        */
        virtual const TreeNode* findLeaf(const dim::vector3df &Point) const;
        
        /**
        Searches all leaf TreeNode objects that contains the specified ellipsoid.
        \param TreeNodeList: Specifies the list where the result will be stored.
        \param Point: Specifies the ellipsoid's point.
        \param Radius: Specifies the ellipsoid's volumetric radius.
        */
        virtual void findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::vector3df &Point, f32 Radius) const;
        
        /**
        Searches all leaf TreeNode objects which the specified line intersects.
        \param TreeNodeList: Specifies the list where the result will be stored.
        \param Line: Specifies the line which is to be used for intersection tests.
        When you want to transform the whole tree transform the line inverse instead.
        */
        virtual void findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::line3df &Line) const;
        
        /**
        Searches all leaf TreeNode objects which the specified volumetric line intersects.
        \param TreeNodeList: Specifies the list where the result will be stored.
        \param Line: Specifies the line which is to be used for intersection tests.
        \param Radius: Specifise the line's radius (or rather size).
        */
        virtual void findLeafList(std::list<const TreeNode*> &TreeNodeList, const dim::line3df &Line, f32 Radius) const;
        
        /**
        Used internally.
        \see findLeaf
        */
        virtual const TreeNode* findLeafSub(const dim::vector3df &Point) const;
        /**
        Used internally.
        \see findLeafList
        */
        virtual void findLeafListSub(std::list<const TreeNode*> &TreeNodeList, const dim::vector3df &Point, f32 Radius) const;
        
        /* === Inline functions === */
        
        //! Returns the type of tree node (QuadTree, OcTree, BSP-Tree, kd-Tree).
        inline ETreeNodeTypes getType() const
        {
            return Type_;
        }
        
        //! Returns the parent node.
        inline TreeNode* getParent() const
        {
            return Parent_;
        }
        
        //! Returns true if this node is a root node (when it has no parent).
        inline bool isRoot() const
        {
            return Parent_ == 0;
        }
        
        inline void setDestructorCallback(const TreeNodeDestructorCallback &Callback)
        {
            DestructorCallback_ = Callback;
        }
        
    protected:
        
        /* === Macros === */
        
        static const f32 EXT_BOUNDBOX_SIZE;
        
        /* === Functions === */
        
        TreeNode(TreeNode* Parent, const ETreeNodeTypes Type);
        
        /* === Templates === */
        
        template <class T> TreeNode* createChild(const dim::aabbox3df &Box)
        {
            return new T(this, Box);
        }
        
        /* === Members === */
        
        ETreeNodeTypes Type_;
        TreeNode* Parent_;
        
        TreeNodeDestructorCallback DestructorCallback_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
