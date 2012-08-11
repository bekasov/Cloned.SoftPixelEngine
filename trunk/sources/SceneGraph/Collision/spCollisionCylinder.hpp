/*
 * Collision cylinder header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_CYLINDER_H__
#define __SP_COLLISION_CYLINDER_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/Collision/spCollisionLineBased.hpp"


namespace sp
{
namespace scene
{


/**
- not yet documented -
\ingroup group_collision
*/
class SP_EXPORT CollisionCylinder : public CollisionLineBased
{
    
    public:
        
        CollisionCylinder(CollisionMaterial* Material, SceneNode* Node, f32 Radius, f32 Height);
        ~CollisionCylinder();
        
        /* Functions */
        
        s32 getSupportFlags() const;
        
        bool checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const;
        bool checkIntersection(const dim::line3df &Line, bool ExcludeCorners = false) const;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
