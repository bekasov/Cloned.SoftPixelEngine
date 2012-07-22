/*
 * Collision plane file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionPlane.hpp"


namespace sp
{
namespace scene
{


CollisionPlane::CollisionPlane(
    CollisionMaterial* Material, SceneNode* Node, const dim::plane3df &Plane) :
    CollisionNode   (Material, Node, COLLISION_PLANE),
    Plane_          (Plane                          )
{
}
CollisionPlane::~CollisionPlane()
{
}

bool CollisionPlane::checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const
{
    const dim::plane3df Plane(getTransformation() * Plane_);
    
    /* Make an intersection test with the line and this plane */
    if (Plane.checkLineIntersection(Line.Start, Line.End, Contact.Point))
    {
        Contact.Normal = Plane.Normal;
        Contact.Object = this;
        return true;
    }
    
    return false;
}

bool CollisionPlane::checkIntersection(const dim::line3df &Line) const
{
    /* Make an intersection test with the line and this plane */
    dim::vector3df Tmp;
    return (getTransformation() * Plane_).checkLineIntersection(Line.Start, Line.End, Tmp);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
