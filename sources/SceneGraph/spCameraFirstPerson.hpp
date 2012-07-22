/*
 * First person camera header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_CAMERA_FIRSTPERSON_H__
#define __SP_SCENE_CAMERA_FIRSTPERSON_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/spSceneCamera.hpp"


namespace sp
{
namespace scene
{


enum EFirstPersonCameraFlags
{
    FPCAMERAFLAG_NONE       = 0x00,
    FPCAMERAFLAG_USEARROWS  = 0x01,
};


class SP_EXPORT FirstPersonCamera : public Camera
{
    
    public:
        
        FirstPersonCamera();
        virtual ~FirstPersonCamera();
        
        /* Functions */
        
        virtual void updateControl();
        virtual void drawMenu();
        
        /* Inline functions */
        
        inline void setFreeMovement(bool isEnable)
        {
            isFreeMovement_ = isEnable;
        }
        inline bool getFreeMovement() const
        {
            return isFreeMovement_;
        }
        
        inline void setFlags(s32 Flags)
        {
            Flags_ = Flags;
        }
        inline s32 getFlags() const
        {
            return Flags_;
        }
        
        inline void setMoveSpeed(f32 Speed)
        {
            MoveSpeed_ = Speed;
        }
        inline f32 getMoveSpeed() const
        {
            return MoveSpeed_;
        }
        
        inline void setTurnSpeed(f32 Speed)
        {
            TurnSpeed_ = Speed;
        }
        inline f32 getTurnSpeed() const
        {
            return TurnSpeed_;
        }
        
        inline void setMaxTurnDegree(f32 Degree)
        {
            MaxTurnDegree_ = Degree;
        }
        inline f32 getMaxTurnDegree() const
        {
            return MaxTurnDegree_;
        }
        
    protected:
        
        /* Functions */
        
        void updateCameraRotation();
        void updateCameraMovement();
        
        /* Members */
        
        s32 Flags_;
        
        bool isFreeMovement_;
        f32 Pitch_, Yaw_;
        
        f32 MoveSpeed_, TurnSpeed_;
        f32 MaxTurnDegree_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
