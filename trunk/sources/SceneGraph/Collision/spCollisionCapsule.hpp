/*
 * Collision capsule header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_CAPSULE_H__
#define __SP_COLLISION_CAPSULE_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/Collision/spCollisionLineBased.hpp"


namespace sp
{
namespace scene
{


/**
CollisionCapsule is one of the collision models and represents a capsule often used for player controller.
\note This collision model can collide with any other collision model.
\ingroup group_collision
*/
class SP_EXPORT CollisionCapsule : public CollisionLineBased
{
    
    public:
        
        CollisionCapsule(CollisionMaterial* Material, SceneNode* Node, f32 Radius, f32 Height);
        ~CollisionCapsule();
        
        /* Functions */
        
        s32 getSupportFlags() const;
        
        bool checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const;
        bool checkIntersection(const dim::line3df &Line, bool ExcludeCorners = false) const;
        
    private:
        
        /* Functions */
        
        bool checkCollisionToSphere (const CollisionSphere*     Rival, SCollisionContact &Contact) const;
        bool checkCollisionToCapsule(const CollisionCapsule*    Rival, SCollisionContact &Contact) const;
        bool checkCollisionToMesh   (const CollisionMesh*       Rival, SCollisionContact &Contact) const;
        
        void performCollisionResolvingToSphere  (const CollisionSphere*     Rival);
        void performCollisionResolvingToCapsule (const CollisionCapsule*    Rival);
        void performCollisionResolvingToPlane   (const CollisionPlane*      Rival);
        void performCollisionResolvingToMesh    (const CollisionMesh*       Rival);
        
        void performDetectedContact(const CollisionNode* Rival, const SCollisionContact &Contact);
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
