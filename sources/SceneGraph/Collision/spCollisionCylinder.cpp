/*
 * Collision cylinder file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionCylinder.hpp"


namespace sp
{
namespace scene
{


CollisionCylinder::CollisionCylinder(
    CollisionMaterial* Material, SceneNode* Node, f32 Radius, f32 Height) :
    CollisionLineBased(Material, Node, COLLISION_CYLINDER, Radius, Height)
{
}
CollisionCylinder::~CollisionCylinder()
{
}

s32 CollisionCylinder::getSupportFlags() const
{
    return COLLISIONSUPPORT_NONE;
}

bool CollisionCylinder::checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const
{
    return false; //todo
}

bool CollisionCylinder::checkIntersection(const dim::line3df &Line, bool ExcludeCorners) const
{
    return false; //todo
}


} // /namespace scene

} // /namespace sp



// ================================================================================
