/*
 * Character controller file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCharacterController.hpp"


namespace sp
{
namespace scene
{


CharacterController::CharacterController(
    CollisionMaterial* Material, SceneNode* Node, f32 Radius, f32 Height) :
    ViewRotation_       (0.0f                                       ),
    MaxStepHeight_      (Radius*0.5f                                ),
    CollModel_          (Material, Node, Radius, Height             ),
    CollStepDetector_   (0, Node, Radius*2, Height - MaxStepHeight_ )
{
}
CharacterController::~CharacterController()
{
}

void CharacterController::update()
{
    
    applyForces(&CollModel_);
    
}

void CharacterController::move(const dim::point2df &Direction, f32 MaxMoveSpeed)
{
    
}

void CharacterController::jump(f32 Force)
{
    
}

void CharacterController::setRadius(f32 Radius)
{
    
}
void CharacterController::setHeight(f32 Height)
{
    
}

void CharacterController::setMaxStepHeight(f32 MaxHeight)
{
    
}


} // /namespace scene

} // /namespace sp



// ================================================================================
