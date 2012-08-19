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

extern SoftPixelDevice* __spDevice;
extern io::InputControl* __spInputControl;
extern video::RenderSystem* __spVideoDriver;

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
    if (!__spInputControl)
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
    const dim::point2df MouseSpeed(__spInputControl->getCursorSpeed().cast<f32>());
    
    Pitch_  += MouseSpeed.Y * TurnSpeed_;
    Yaw_    += MouseSpeed.X * TurnSpeed_;
    
    if (!math::Equal(MaxTurnDegree_, 0.0f))
        math::Clamp(Pitch_, -MaxTurnDegree_, MaxTurnDegree_);
    
    setRotation(dim::vector3df(Pitch_, Yaw_, 0));
    
    /* Catch cursor position */
    __spInputControl->setCursorPosition(
        dim::point2di(Viewport_.Left + Viewport_.Right/2, Viewport_.Top + Viewport_.Bottom/2)
    );
    
    #endif
}

void FirstPersonCamera::updateCameraMovement()
{
    #if defined(SP_PLATFORM_ANDROID) || defined(SP_PLATFORM_IOS)
    
    #else
    
    const bool UseArrowKeys = (Flags_ & FPCAMERAFLAG_USEARROWS);
    
    /* Control translation movement */
    if ( ( UseArrowKeys && __spInputControl->keyDown(io::KEY_RIGHT) ) || __spInputControl->keyDown(io::KEY_D) )
        move(dim::vector3df(MoveSpeed_, 0, 0));
    if ( ( UseArrowKeys && __spInputControl->keyDown(io::KEY_LEFT) ) || __spInputControl->keyDown(io::KEY_A) )
        move(dim::vector3df(-MoveSpeed_, 0, 0));
    if ( ( UseArrowKeys && __spInputControl->keyDown(io::KEY_UP) ) || __spInputControl->keyDown(io::KEY_W) )
        move(dim::vector3df(0, 0, MoveSpeed_));
    if ( ( UseArrowKeys && __spInputControl->keyDown(io::KEY_DOWN) ) || __spInputControl->keyDown(io::KEY_S) )
        move(dim::vector3df(0, 0, -MoveSpeed_));
    
    #endif
}


} // /namespace scene

} // /namespace sp



// ================================================================================
