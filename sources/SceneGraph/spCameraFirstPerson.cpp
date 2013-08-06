/*
 * First person camera file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spCameraFirstPerson.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern SoftPixelDevice* GlbEngineDev;
extern io::InputControl* GlbInputCtrl;
extern video::RenderSystem* GlbRenderSys;

namespace scene
{


FirstPersonCamera::FirstPersonCamera() :
    Camera          (                       ),
    Flags_          (FPCAMERAFLAG_USEARROWS ),
    isFreeMovement_ (false                  ),
    Pitch_          (0.0f                   ),
    Yaw_            (0.0f                   ),
    MoveSpeed_      (0.25f                  ),
    TurnSpeed_      (0.25f                  ),
    MaxTurnDegree_  (90.0f                  )
{
}
FirstPersonCamera::~FirstPersonCamera()
{
}

void FirstPersonCamera::updateControl()
{
    if (!GlbInputCtrl)
        return;
    
    if (isFreeMovement_)
        updateCameraMovement();
    updateCameraRotation();
}

void FirstPersonCamera::drawMenu()
{
    #if defined(SP_PLATFORM_ANDROID) || defined(SP_PLATFORM_IOS)
    
    #endif
}


/*
 * ======= Protected: =======
 */

void FirstPersonCamera::updateCameraRotation()
{
    #if defined(SP_PLATFORM_ANDROID) || defined(SP_PLATFORM_IOS)
    
    #else
    
    /* Update camera rotation */
    const dim::point2df MouseSpeed(GlbInputCtrl->getCursorSpeed().cast<f32>());
    
    Pitch_  += MouseSpeed.Y * TurnSpeed_;
    Yaw_    += MouseSpeed.X * TurnSpeed_;
    
    if (!math::equal(MaxTurnDegree_, 0.0f))
        math::clamp(Pitch_, -MaxTurnDegree_, MaxTurnDegree_);
    
    setRotation(dim::vector3df(Pitch_, Yaw_, 0));
    
    /* Catch cursor position */
    GlbInputCtrl->setCursorPosition(
        dim::point2di(
            getViewport().Left + getViewport().Right/2,
            getViewport().Top + getViewport().Bottom/2
        )
    );
    
    #endif
}

void FirstPersonCamera::updateCameraMovement()
{
    #if defined(SP_PLATFORM_ANDROID) || defined(SP_PLATFORM_IOS)
    
    #else
    
    const bool UseArrowKeys = (Flags_ & FPCAMERAFLAG_USEARROWS);
    
    /* Control translation movement */
    if ( ( UseArrowKeys && GlbInputCtrl->keyDown(io::KEY_RIGHT) ) || GlbInputCtrl->keyDown(io::KEY_D) )
        move(dim::vector3df(MoveSpeed_, 0, 0));
    if ( ( UseArrowKeys && GlbInputCtrl->keyDown(io::KEY_LEFT) ) || GlbInputCtrl->keyDown(io::KEY_A) )
        move(dim::vector3df(-MoveSpeed_, 0, 0));
    if ( ( UseArrowKeys && GlbInputCtrl->keyDown(io::KEY_UP) ) || GlbInputCtrl->keyDown(io::KEY_W) )
        move(dim::vector3df(0, 0, MoveSpeed_));
    if ( ( UseArrowKeys && GlbInputCtrl->keyDown(io::KEY_DOWN) ) || GlbInputCtrl->keyDown(io::KEY_S) )
        move(dim::vector3df(0, 0, -MoveSpeed_));
    
    #endif
}


} // /namespace scene

} // /namespace sp



// ================================================================================
