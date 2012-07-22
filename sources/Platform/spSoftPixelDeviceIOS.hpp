/*
 * SoftPixel Device iOS header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SOFTPIXELDEVICE_IOS_H__
#define __SP_SOFTPIXELDEVICE_IOS_H__


#include "Base/spStandard.hpp"

#if defined(SP_PLATFORM_IOS)


#include "Platform/spSoftPixelDevice.hpp"


namespace sp
{


//! SoftPixelDevice class for the Apple iOS platform.
class SP_EXPORT SoftPixelDeviceIOS : public SoftPixelDevice
{
    
    public:
        
        SoftPixelDeviceIOS(
            const video::ERenderSystems RendererType, const io::stringc &Title, bool isFullscreen
        );
        ~SoftPixelDeviceIOS();
        
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
