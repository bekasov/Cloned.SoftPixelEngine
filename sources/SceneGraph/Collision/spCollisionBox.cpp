/*
 * Collision box file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionBox.hpp"
#include "SceneGraph/Collision/spCollisionSphere.hpp"


namespace sp
{
namespace scene
{


CollisionBox::CollisionBox(
    CollisionMaterial* Material, SceneNode* Node, const dim::aabbox3df &Box) :
    CollisionNode   (Material, Node, COLLISION_BOX  ),
    Box_            (Box                            )
{
}
CollisionBox::~CollisionBox()
{
}

bool CollisionBox::checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const
{
    /* Store transformations */
    const dim::matrix4f Mat(getTransformation());
    const dim::matrix4f InvMat(Mat.getInverse());
    
    const dim::line3df InvLine(InvMat * Line.Start, InvMat * Line.End);
    
    /* Make an intersection test with the inverse-transformed line and the aab-box */
    if (math::CollisionLibrary::checkLineBoxIntersection(InvLine, Box_, Contact.Point))
    {
        /* Calculate normal and final contact point */
             if (Contact.Point.X > Box_.Max.X - math::ROUNDING_ERROR) Contact.Normal = dim::vector3df( 1,  0,  0);
        else if (Contact.Point.X < Box_.Min.X + math::ROUNDING_ERROR) Contact.Normal = dim::vector3df(-1,  0,  0);
        else if (Contact.Point.Y > Box_.Max.Y - math::ROUNDING_ERROR) Contact.Normal = dim::vector3df( 0,  1,  0);
        else if (Contact.Point.Y < Box_.Min.Y + math::ROUNDING_ERROR) Contact.Normal = dim::vector3df( 0, -1,  0);
        else if (Contact.Point.Z > Box_.Max.Z - math::ROUNDING_ERROR) Contact.Normal = dim::vector3df( 0,  0,  1);
        else                                                          Contact.Normal = dim::vector3df( 0,  0, -1);
        
        Contact.Point   = Mat * Contact.Point;
        Contact.Normal  = (Mat.getRotationMatrix() * Contact.Normal).normalize();
        Contact.Object  = this;
        
        return true;
    }
    
    return false;
}

bool CollisionBox::checkIntersection(const dim::line3df &Line) const
{
    /* Store transformations */
    const dim::matrix4f Mat(getTransformation());
    const dim::matrix4f InvMat(Mat.getInverse());
    
    const dim::line3df InvLine(InvMat * Line.Start, InvMat * Line.End);
    
    /* Make intersection test */
    return math::CollisionLibrary::checkLineBoxOverlap(InvLine, Box_);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
