/*
 * Character controller file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCharacterController.hpp"
#include "SceneGraph/Collision/spCollisionMaterial.hpp"


namespace sp
{
namespace scene
{


static bool ChCtrlCollisionMaterial(
    CollisionMaterial* Material, CollisionNode* Node, const CollisionNode* Rival, const SCollisionContact &Contact)
{
    CharacterController* CharCtrl = static_cast<CharacterController*>(Node->getUserData());
    
    /* Check if contact resets gravity forces */
    dim::vector3df InvGravity(CharCtrl->getGravity());
    InvGravity.normalize();
    InvGravity.setInvert();
    
    if (InvGravity.dot(Contact.Normal) > 0.5f)
    {
        CharCtrl->resetGravityForces();
        CharCtrl->StayOnGround_ = true;
    }
    
    if (CharCtrl->CollContactCallback_)
        return CharCtrl->CollContactCallback_(CharCtrl, Rival, Contact);
    
    return true;
}

CharacterController::CharacterController(
    CollisionMaterial* Material, SceneNode* Node, f32 Radius, f32 Height) :
    BaseCollisionPhysicsObject  (                                           ),
    BaseObject                  (                                           ),
    ViewRotation_               (0.0f                                       ),
    MaxStepHeight_              (Radius*0.5f                                ),
    CollModel_                  (Material, Node, Radius, Height             ),
    CollStepDetector_           (0, Node, Radius*2, Height - MaxStepHeight_ ),
    StayOnGround_               (false                                      )
{
    if (!Material)
        throw "Collision character controller must have a valid collision material";
    
    Material->setContactCallback(ChCtrlCollisionMaterial);
    CollModel_.setUserData(this);
}
CharacterController::~CharacterController()
{
}

void CharacterController::update()
{
    
    integrate(&CollModel_);
    
    if (StayOnGround_)
        applyFriction();
    
    StayOnGround_ = false;
    
    CollModel_.updateCollisions();
    
}

void CharacterController::move(const dim::point2df &Direction, f32 MaxMoveSpeed)
{
    if (!StayOnGround_)
        return;
    
    /* Setup move direction */
    dim::matrix3f RotMat;
    RotMat.rotateY(ViewRotation_);
    
    dim::vector3df MoveDir(Direction.X, 0, Direction.Y);
    MoveDir = getOrientation() * (RotMat * MoveDir);
    
    /* Apply move force */
    if (getForce().getLength() < MaxMoveSpeed)
    {
        addForce(MoveDir);
        
        f32 ForceLen = getForce().getLength();
        
        //if (ForceLen > MaxMoveSpeed)
    }
}

void CharacterController::jump(f32 Force)
{
    addForce(getOrientation() * dim::vector3df(0, Force, 0));
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
