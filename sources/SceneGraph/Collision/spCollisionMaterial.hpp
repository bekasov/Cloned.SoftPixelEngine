/*
 * Collision material header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_MATERIAL_H__
#define __SP_COLLISION_MATERIAL_H__


#include "Base/spStandard.hpp"
#include "Base/spBaseObject.hpp"
#include "SceneGraph/Collision/spCollisionNode.hpp"


namespace sp
{
namespace scene
{


//! Collision contact callback interface.
typedef boost::function<void (CollisionMaterial* Material, CollisionNode* Node, CollisionNode* Rival)> CollisionContactCallback;


/**
This is the collision material class. It's used to
define a collidable material with several rival materials.
*/
class SP_EXPORT CollisionMaterial : public BaseObject
{
    
    public:
        
        CollisionMaterial();
        ~CollisionMaterial();
        
        /* === Functions === */
        
        //! Adds the specified collision material as new rival.
        void addRivalCollisionMaterial(CollisionMaterial* Rival);
        //! Removes the specified collision material as rival.
        void removeRivalCollisionMaterial(CollisionMaterial* Rival);
        
        /* === Inline functions === */
        
        //! Sets the collision contact callback function. Use this to determine when a collision has been detected.
        inline void setCollisionContactCallback(const CollisionContactCallback &Callback)
        {
            CollContactCallback_ = Callback;
        }
        
        //! Returns the list of collision nodes which use this material.
        inline const std::vector<CollisionNode*>& getNodeList() const
        {
            return CollNodes_;
        }
        //! Retuns the rival collision material list.
        inline const std::vector<CollisionMaterial*>& getRivalList() const
        {
            return RivalCollMaterials_;
        }
        
    private:
        
        friend class CollisionNode;
        
        /* === Functions === */
        
        void addCollisionNode(CollisionNode* Node);
        void removeCollisionNode(CollisionNode* Node);
        
        /* === Members === */
        
        std::vector<CollisionNode*> CollNodes_;
        std::vector<CollisionMaterial*> RivalCollMaterials_;
        
        CollisionContactCallback CollContactCallback_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
