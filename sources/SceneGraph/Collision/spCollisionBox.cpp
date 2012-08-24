/*
 * Collision box file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionBox.hpp"
#include "SceneGraph/Collision/spCollisionSphere.hpp"
#include "SceneGraph/Collision/spCollisionPlane.hpp"


namespace sp
{
namespace scene
{


CollisionBox::CollisionBox(
    CollisionMaterial* Material, SceneNode* Node, const dim::aabbox3df &Box) :
    CollisionNode   (Material, Node, COLLISION_BOX  ),
    Box_            (Box                            )
{
    Box_.repair();
}
CollisionBox::~CollisionBox()
{
}

s32 CollisionBox::getSupportFlags() const
{
    return COLLISIONSUPPORT_PLANE;
}

f32 CollisionBox::getMaxMovement() const
{
    return getBox().getMaxRadius().getMax();
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

bool CollisionBox::checkIntersection(const dim::line3df &Line, bool ExcludeCorners) const
{
    /* Store transformations */
    const dim::matrix4f Mat(getTransformation());
    const dim::matrix4f InvMat(Mat.getInverse());
    
    const dim::line3df InvLine(InvMat * Line.Start, InvMat * Line.End);
    
    /* Check for corners exclusion */
    if (ExcludeCorners)
    {
        dim::vector3df Point;
        if (math::CollisionLibrary::checkLineBoxIntersection(InvLine, Box_, Point))
            return checkCornerExlusion(Line, Point);
        return false;
    }
    
    /* Make intersection test */
    return math::CollisionLibrary::checkLineBoxOverlap(InvLine, Box_);
}


/*
 * ======= Private: =======
 */

bool CollisionBox::checkCollisionToPlane(const CollisionPlane* Rival, SCollisionContact &Contact) const
{
    if (!Rival)
        return false;
    
    /* Store transformation */
    const dim::plane3df RivalPlane(
        Rival->getTransformation().getPositionRotationMatrix() * Rival->getPlane()
    );
    const dim::plane3df RivalPlaneInv(getInverseTransformation() * RivalPlane);
    
    /* Check if this object and the other collide with each other */
    const f32 Distance = RivalPlaneInv.getAABBoxDistance(getBox());
    
    if (Distance < 0.0f)
    {
        /* Find nearest box corner */
        Contact.Point = getTransformation() * getBox().getClosestPoint(RivalPlaneInv);
        
        /* Compute impact and normal */
        Contact.Impact = -Distance;
        
        Contact.Normal = RivalPlane.Normal;
        Contact.Normal.normalize();
        
        return true;
    }
    
    return false;
}

void CollisionBox::performCollisionResolvingToPlane(const CollisionPlane* Rival)
{
    SCollisionContact Contact;
    if (checkCollisionToPlane(Rival, Contact))
        performDetectedContact(Rival, Contact);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
