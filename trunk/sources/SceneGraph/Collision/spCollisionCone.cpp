/*
 * Collision cone file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionCone.hpp"


namespace sp
{
namespace scene
{


CollisionCone::CollisionCone(
    CollisionMaterial* Material, SceneNode* Node, f32 Radius, f32 Height) :
    CollisionLineBased(Material, Node, COLLISION_CONE, Radius, Height)
{
}
CollisionCone::~CollisionCone()
{
}

s32 CollisionCone::getSupportFlags() const
{
    return COLLISIONSUPPORT_NONE;
}

bool CollisionCone::checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const
{
    return false; //todo
}

bool CollisionCone::checkIntersection(const dim::line3df &Line) const
{
    return false; //todo
}


} // /namespace scene

} // /namespace sp



// ================================================================================
