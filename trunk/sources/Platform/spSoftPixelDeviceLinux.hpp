/*
 * SoftPixel Device Linux header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SOFTPIXELDEVICE_LINUX_H__
#define __SP_SOFTPIXELDEVICE_LINUX_H__


#include "Base/spStandard.hpp"

#if defined(SP_PLATFORM_LINUX)


#include "Platform/spSoftPixelDevice.hpp"

#include <sys/utsname.h>
#include <sys/time.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>
#include <GL/gl.h>
#include <GL/glx.h>


namespace sp
{

namespace video { class OpenGLRenderSystem; }


//! SoftPixelDevice class for the GNU/Linux platform.
class SP_EXPORT SoftPixelDeviceLinux : public SoftPixelDevice
{
    
    public:
        
        SoftPixelDeviceLinux(
            const video::ERenderSystems RendererType, const dim::size2di &Resolution, s32 ColorDepth,
            const io::stringc &Title, bool isFullScreen, const SDeviceFlags &Flags
        );
        ~SoftPixelDeviceLinux();
        
        /* === Functions === */
        
        bool updateEvent();
        void deleteDevice();
        
        void beep(u32 Milliseconds = 100, u32 Frequency = 1000);
        
    private:
        
        friend class video::OpenGLRenderSystem;
        friend class SoftwareDriver;
        friend class io::InputControl;
        
        /* === Functions === */
        
        void setupKeyCodeTable();
        void createCursor();
        
        /* === Members === */
        
        Display* Display_;
        Window Window_;
        
        Cursor Cursor_;
        XEvent Event_;
        
        static timeval TimeStart_;
        
};


} // /namespace sp


#endif

#endif



// ================================================================================
