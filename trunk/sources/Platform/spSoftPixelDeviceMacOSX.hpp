/*
 * SoftPixel Device Mac OS X header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SOFTPIXELDEVICE_MACOSX_H__
#define __SP_SOFTPIXELDEVICE_MACOSX_H__


#include "Base/spStandard.hpp"

#if defined(SP_PLATFORM_MACOSX)


#include "Platform/spSoftPixelDevice.hpp"


namespace sp
{


//! SoftPixelDevice class for the Apple Mac OS X platform.
class SP_EXPORT SoftPixelDeviceMacOSX : public SoftPixelDevice
{
    
    public:
        
        SoftPixelDeviceMacOSX(
            const video::ERenderSystems RendererType, const dim::size2di &Resolution, s32 ColorDepth,
            const io::stringc &Title, bool isFullScreen, const SDeviceFlags &Flags
        );
        SoftPixelDeviceMacOSX();
        
        /* Functions */
        
        bool updateEvent();
        void deleteDevice();
        
    private:
        
        /* Functions */
        
        bool openGraphicsScreen();
        void closeGraphicsScreen();
        
        /* Members */
        
        //todo
        
};


} // /namespace sp


#endif

#endif



// ================================================================================
