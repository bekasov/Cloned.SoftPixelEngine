/*
 * Collision capsule header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_CAPSULE_H__
#define __SP_COLLISION_CAPSULE_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/Collision/spCollisionNode.hpp"


namespace sp
{
namespace scene
{


/**
CollisionCapsule is one of the collision models and represents a capsule often used for player controller.
\note This collision model can collide with any other collision model.
*/
class SP_EXPORT CollisionCapsule : public CollisionNode
{
    
    public:
        
        CollisionCapsule(CollisionMaterial* Material, SceneNode* Node, f32 Radius, f32 Height);
        ~CollisionCapsule();
        
        /* Functions */
        
        bool checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const;
        bool checkIntersection(const dim::line3df &Line) const;
        
        //! Returns the line representing the capsule.
        dim::line3df getLine() const;
        
        /* Static functions */
        
        static dim::obbox3df getBoundBoxFromLine(const dim::line3df &Line, f32 Radius);
        
        /* Inline functions */
        
        inline void setRadius(f32 Radius)
        {
            Radius_ = Radius;
        }
        inline f32 getRadius() const
        {
            return Radius_;
        }
        
        inline void setHeight(f32 Height)
        {
            Height_ = Height;
        }
        inline f32 getHeight() const
        {
            return Height_;
        }
        
    private:
        
        /* Functions */
        
        bool checkCollisionToSphere (const CollisionSphere*     Rival, SCollisionContact &Contact) const;
        bool checkCollisionToCapsule(const CollisionCapsule*    Rival, SCollisionContact &Contact) const;
        bool checkCollisionToBox    (const CollisionBox*        Rival, SCollisionContact &Contact) const;
        bool checkCollisionToMesh   (const CollisionMesh*       Rival, SCollisionContact &Contact) const;
        
        /* Members */
        
        f32 Radius_, Height_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
