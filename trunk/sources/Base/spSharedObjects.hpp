/*
 * Shared objects header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHAREDOBJECTS_H__
#define __SP_SHAREDOBJECTS_H__


#include "Base/spStandard.hpp"


namespace sp
{


/* === Declarations === */

class SoftPixelDevice;

namespace video
{
    class RenderSystem;
    class RenderContext;
    #ifdef SP_COMPILE_WITH_CG
    class CgShaderContext;
    #endif
}
namespace io
{
    class InputControl;
    class OSInformator;
}
namespace scene
{
    class SceneGraph;
}
namespace audio
{
    class SoundDevice;
}


/* === Structures === */

struct SSharedObjects
{
    SSharedObjects() :
        Engine          (0),
        ActiveContext   (0),
        ActiveScene     (0),
        Input           (0),
        OSInfo          (0),
        AudioDevice     (0),
        
        #ifdef SP_COMPILE_WITH_CG
        CgContext       (0),
        #endif
        
        ScreenWidth     (0),
        ScreenHeight    (0),
        ScreenOffsetX   (0),
        ScreenOffsetY   (0),
        CursorSpeedX    (0),
        CursorSpeedY    (0),
        MouseWheel      (0),
        KeyRecordCount  (0)
    {
        for (s32 i = 0; i < 10; ++i)
            KeyRecordList[i] = 0;
    }
    ~SSharedObjects()
    {
    }
    
    /* Members */
    SoftPixelDevice*        Engine;
    video::RenderSystem*    Renderer;
    video::RenderContext*   ActiveContext;
    scene::SceneGraph*      ActiveScene;
    io::InputControl*       Input;
    io::OSInformator*       OSInfo;
    audio::SoundDevice*     AudioDevice;
    
    #ifdef SP_COMPILE_WITH_CG
    video::CgShaderContext* CgContext;
    #endif
    
    s32 ScreenWidth;
    s32 ScreenHeight;
    
    s32 ScreenOffsetX;
    s32 ScreenOffsetY;
    
    s32 CursorSpeedX;
    s32 CursorSpeedY;
    
    s32 MouseWheel;
    u32 KeyRecordCount;
    u32 KeyRecordList[10];
};

// Global shared objects (only used internally)
extern SSharedObjects gSharedObjects;


} // /namespace sp


#endif



// ================================================================================
