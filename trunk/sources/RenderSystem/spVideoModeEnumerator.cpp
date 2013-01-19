/*
 * Video mode enumerator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spVideoModeEnumerator.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#endif

#if 1//!!!
#   include "Base/spInputOutputLog.hpp"
#endif

#include <boost/foreach.hpp>


namespace sp
{
namespace video
{


#if defined(SP_PLATFORM_WINDOWS)

VideoModeEnumerator::VideoModeEnumerator()
{
    /* Get deksotp video mode */
    Desktop_.Resolution.Width   = GetSystemMetrics(SM_CXSCREEN);
    Desktop_.Resolution.Height  = GetSystemMetrics(SM_CYSCREEN);
    Desktop_.ColorDepth         = GetDeviceCaps(GetDC(GetDesktopWindow()), BITSPIXEL);
    
    /* Enumerate supported display settings */
    DEVMODE DevMode;
    s32 i = 0;
    
    std::list<SVideoMode> VideoModes;
    
    while (EnumDisplaySettings(0, i++, &DevMode))
    {
        SVideoMode Mode;
        {
            Mode.Resolution.Width   = DevMode.dmPelsWidth;
            Mode.Resolution.Height  = DevMode.dmPelsHeight;
            Mode.ColorDepth         = DevMode.dmBitsPerPel;
        }
        VideoModes.push_back(Mode);
    }
    
    VideoModes.unique();
    
    /* Copy video modes into the vector for faster access */
    VideoModes_.resize(VideoModes.size());
    
    i = 0;
    foreach (const SVideoMode &Mode, VideoModes)
        VideoModes_[i++] = Mode;
    
    /* Enumerate connected display devices */
    DISPLAY_DEVICE Device, Monitor;
    
    ZeroMemory(&Device, sizeof(DISPLAY_DEVICE));
    Device.cb = sizeof(DISPLAY_DEVICE);
    
    ZeroMemory(&Monitor, sizeof(DISPLAY_DEVICE));
    Monitor.cb = sizeof(DISPLAY_DEVICE);
    
    i = 0;
    
    while (EnumDisplayDevices(0, i++, &Device, 0))
    {
        for (s32 j = 0; EnumDisplayDevices(Device.DeviceName, j, &Monitor, 0); ++j)
        {
            SDisplayDevice Display;
            {
                Display.VideoController = io::stringc(Device.DeviceString);
                Display.Monitor         = io::stringc(Monitor.DeviceString);
            }
            DisplayDevices_.push_back(Display);
        }
    }
}
VideoModeEnumerator::~VideoModeEnumerator()
{
}

#else

//!TODO!

VideoModeEnumerator::VideoModeEnumerator()
{
}
VideoModeEnumerator::~VideoModeEnumerator()
{
}

#endif


} // /namespace video

} // /namespace sp



// ================================================================================
