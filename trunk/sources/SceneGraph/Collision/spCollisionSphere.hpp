/*
 * Collision sphere header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_SPHERE_H__
#define __SP_COLLISION_SPHERE_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/Collision/spCollisionNode.hpp"


namespace sp
{
namespace scene
{


/**
CollisionSphere is one of the collision models and represents a perfect sphere with only a position and a radius.
Rotations and scaling does not effect the collision model.
\note This collision model can collide with any other collision model.
*/
class SP_EXPORT CollisionSphere : public CollisionNode
{
    
    public:
        
        CollisionSphere(CollisionMaterial* Material, SceneNode* Node, f32 Radius);
        ~CollisionSphere();
        
        /* Functions */
        
        bool checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const;
        bool checkIntersection(const dim::line3df &Line) const;
        
        /* Inline functions */
        
        inline void setRadius(f32 Radius)
        {
            Radius_ = Radius;
        }
        inline f32 getRadius() const
        {
            return Radius_;
        }
        
    private:
        
        /* Functions */
        
        bool checkCollisionToSphere (const CollisionSphere*     Rival, SCollisionContact &Contact) const;
        bool checkCollisionToCapsule(const CollisionCapsule*    Rival, SCollisionContact &Contact) const;
        bool checkCollisionToBox    (const CollisionBox*        Rival, SCollisionContact &Contact) const;
        bool checkCollisionToPlane  (const CollisionPlane*      Rival, SCollisionContact &Contact) const;
        bool checkCollisionToMesh   (const CollisionMesh*       Rival, SCollisionContact &Contact) const;
        
        bool checkAnyCollisionToMesh(const CollisionMesh* Rival) const;
        
        void performCollisionResolvingToSphere  (const CollisionSphere*     Rival);
        void performCollisionResolvingToCapsule (const CollisionCapsule*    Rival);
        void performCollisionResolvingToBox     (const CollisionBox*        Rival);
        void performCollisionResolvingToPlane   (const CollisionPlane*      Rival);
        void performCollisionResolvingToMesh    (const CollisionMesh*       Rival);
        
        void performDetectedContact(const CollisionNode* Rival, const SCollisionContact &Contact);
        
        /* Members */
        
        f32 Radius_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
