/*
 * Collision line based file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionLineBased.hpp"


namespace sp
{
namespace scene
{


CollisionLineBased::CollisionLineBased(
    CollisionMaterial* Material, SceneNode* Node, const ECollisionModels Type, f32 Radius, f32 Height) :
    CollisionNode   (Material, Node, Type   ),
    Radius_         (Radius                 ),
    Height_         (Height                 )
{
    if (Radius_ < math::ROUNDING_ERROR)
        throw "Line-based collision nodes must have a radius larger than 0.0";
}
CollisionLineBased::~CollisionLineBased()
{
}

f32 CollisionLineBased::getMaxMovement() const
{
    return getRadius() * 0.8f;
}

dim::line3df CollisionLineBased::getLine() const
{
    const dim::matrix4f Mat(getTransformation());
    return dim::line3df(Mat.getPosition(), Mat * dim::vector3df(0, Height_, 0));
}

dim::obbox3df CollisionLineBased::getBoundBoxFromLine(const dim::line3df &Line, f32 Radius)
{
    /* Convert the line with radius into an OBB */
    dim::vector3df Axis[3];
    
    Axis[0] = Line.getDirection() * 0.5f;
    Axis[1] = Axis[0].getNormal();
    Axis[2] = Axis[0].cross(Axis[1]);
    
    Axis[1].setLength(Radius);
    Axis[2].setLength(Radius);
    
    const dim::vector3df Dir(Line.getDirection().normalize());
    
    return dim::obbox3df(
        Line.Start + Axis[0], Axis[0] + Dir * Radius, Axis[1], Axis[2]
    );
}


} // /namespace scene

} // /namespace sp



// ================================================================================
