/*
 * SoftPixel Device Windows header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SOFTPIXELDEVICE_WINDOWS_H__
#define __SP_SOFTPIXELDEVICE_WINDOWS_H__


#include "Base/spStandard.hpp"

#if defined(SP_PLATFORM_WINDOWS)


#include "Platform/spSoftPixelDevice.hpp"


namespace sp
{


/**
SoftPixelDevice class for the Microsoft(c) Windows(c) platform.
\todo Rename to "SoftPixelDeviceWindows"
*/
class SP_EXPORT SoftPixelDeviceWin32 : public SoftPixelDevice
{
    
    public:
        
        SoftPixelDeviceWin32(
            const video::ERenderSystems RendererType, const dim::size2di &Resolution, s32 ColorDepth,
            const io::stringc &Title, bool isFullscreen, const SDeviceFlags &Flags, void* ParentWindow
        );
        ~SoftPixelDeviceWin32();
        
        /* === Functions === */
        
        bool updateEvents();
        
        void beep(u32 Milliseconds = 100, u32 Frequency = 1000);
        
        s32 registerFontResource(const io::stringc &Filename);
        void unregisterFontResource(const io::stringc &Filename);
        
        io::stringc getWorkingDir() const;
        
    private:
        
        friend SP_EXPORT LRESULT CALLBACK SpWin32Callback(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
        
        /* === Members === */
        
        MSG Message_;
        
        bool isWindowOpened_;
        
        std::list<io::stringc> FontResources_;
        
};


SP_EXPORT LRESULT CALLBACK SpWin32Callback(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);


} // /namespace sp


#endif

#endif



// ================================================================================
