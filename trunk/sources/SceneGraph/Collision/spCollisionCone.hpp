/*
 * Collision cone header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_CONE_H__
#define __SP_COLLISION_CONE_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/Collision/spCollisionLineBased.hpp"


namespace sp
{
namespace scene
{


class SP_EXPORT CollisionCone : public CollisionLineBased
{
    
    public:
        
        CollisionCone(CollisionMaterial* Material, SceneNode* Node, f32 Radius, f32 Height);
        ~CollisionCone();
        
        /* Functions */
        
        s32 getSupportFlags() const;
        
        bool checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const;
        bool checkIntersection(const dim::line3df &Line) const;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
