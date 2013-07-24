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


bool ChCtrlCollisionMaterial(
    CollisionMaterial* Material, CollisionNode* Node, const CollisionNode* Rival, const SCollisionContact &Contact)
{
    /* Get character controller */
    CharacterController* CharCtrl = static_cast<CharacterController*>(Node->getUserData());
    
    /* Get normalize gravity and inverse gravity */
    dim::vector3df Gravity(CharCtrl->getGravity());
    Gravity.normalize();
    
    dim::vector3df InvGravity(Gravity);
    InvGravity.setInverse();
    
    /* Check if contact reduces jump forces */
    const f32 GdotN = Gravity.dot(Contact.Normal);
    
    if (GdotN > 0.5f)
        CharCtrl->reduceVelocity(dim::vector3df(1.0f) - Contact.Normal.getAbs());
    
    /* Check if contact arrests gravity forces */
    if (InvGravity.dot(Contact.Normal) > 0.5f)
    {
        CharCtrl->arrestGravityForces();
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
    /* Apply physics integration */
    integrate(&CollModel_);
    
    if (StayOnGround_)
        applyFriction();
    
    StayOnGround_ = false;
    
    /* Update collisions */
    CollModel_.updateCollisions();
    
    /* Update movement change */
    PrevPos_ = CurPos_;
    CurPos_ = CollModel_.getPosition();
}

void CharacterController::move(const dim::vector3df &Direction)
{
    /* Setup move direction */
    dim::matrix3f RotMat;
    RotMat.rotateY(ViewRotation_);
    
    const dim::vector3df MoveForce(
        getOrientation() * (RotMat * Direction)
    );
    
    /* Apply move force */
    addForce(MoveForce);
}

void CharacterController::jump(f32 Force)
{
    addForce(getOrientation() * dim::vector3df(0, Force, 0));
}

void CharacterController::setRadius(f32 Radius)
{
    CollModel_.setRadius(Radius);
}
void CharacterController::setHeight(f32 Height)
{
    CollModel_.setHeight(Height);
}

void CharacterController::setMaxStepHeight(f32 MaxHeight)
{
    
}


} // /namespace scene

} // /namespace sp



// ================================================================================
