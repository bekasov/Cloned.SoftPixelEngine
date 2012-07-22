/*
 * SoftPixelDevice header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SOFTPIXELDEVICE_FLAGS_H__
#define __SP_SOFTPIXELDEVICE_FLAGS_H__


#include "Base/spStandard.hpp"


namespace sp
{


/*
 * Macros
 */

static const s32 DEF_SCRIPT_OBJECTCOUNT = 100;
static const s32 DEF_MULTISAMPLE        = 2;
static const s32 DEF_COLORDEPTH         = 32;
static const f32 DEF_FRAMERATE          = 70.0f;


/*
 * Structures
 */

struct SDeviceFlags
{
    SDeviceFlags(
        bool ResizAble = false, bool Vsync = true, bool AntiAlias = false,
        s32 AntiAliasSamples = DEF_MULTISAMPLE, bool DropFileAccept = false)
        : isResizAble(ResizAble), isVsync(Vsync), isAntiAlias(AntiAlias),
        isDropFileAccept(DropFileAccept), MultiSamples(AntiAliasSamples)
    {
    }
    ~SDeviceFlags()
    {
    }
    
    /* Members */
    
    bool isResizAble;
    bool isVsync;
    bool isAntiAlias;
    bool isDropFileAccept;
    s32 MultiSamples;
};

static const SDeviceFlags DEVICEFLAG_HQ = SDeviceFlags(false, true, true, 16);


} // /namespace sp


#endif



// ================================================================================
