/*
 * Collision mesh header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_MESH_H__
#define __SP_COLLISION_MESH_H__


#include "Base/spStandard.hpp"
#include "Base/spTreeBuilder.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/Collision/spCollisionNode.hpp"


namespace sp
{
namespace scene
{


/**
CollisionMesh is one of the collision models and represents a complete mesh and has its own kd-Tree for fast collision detection.
Each kd-Tree node leaf stores a list of SCollisionFace instances. Thus modifying your mesh does not effect the collision model
after it has been already created.
*/
class SP_EXPORT CollisionMesh : public CollisionNode
{
    
    public:
        
        CollisionMesh(CollisionMaterial* Material, scene::Mesh* Mesh, u8 MaxTreeLevel = DEF_KDTREE_LEVEL);
        CollisionMesh(CollisionMaterial* Material, const std::list<Mesh*> &MeshList, u8 MaxTreeLevel = DEF_KDTREE_LEVEL);
        ~CollisionMesh();
        
        /* Functions */
        
        void findIntersections(const dim::line3df &Line, std::list<SIntersectionContact> &ContactList) const;
        bool checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const;
        bool checkIntersection(const dim::line3df &Line) const;
        
        /* Inline functions */
        
        inline KDTreeNode* getRootTreeNode() const
        {
            return RootTreeNode_;
        }
        
        //! Sets the collidable face side. By default video::FACE_FRONT.
        inline void setCollFace(const video::EFaceTypes Type)
        {
            CollFace_ = Type;
        }
        //! Returns the collidable face side.
        inline video::EFaceTypes getCollFace() const
        {
            return CollFace_;
        }
        
        /* Types */
        
        typedef std::vector<SCollisionFace*> TreeNodeDataType;
        
    private:
        
        /* Functions */
        
        void createCollisionModel(const std::list<Mesh*> &MeshList, u8 MaxTreeLevel, bool PreTransform);
        
        /* Members */
        
        KDTreeNode* RootTreeNode_;
        video::EFaceTypes CollFace_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
