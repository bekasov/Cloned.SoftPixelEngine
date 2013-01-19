/*
 * Video mode enumerator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_VIDEOMODE_RECEIVER_H__
#define __SP_VIDEOMODE_RECEIVER_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionSize2D.hpp"
#include "Base/spInputOutputString.hpp"


namespace sp
{
namespace video
{


//! This structure stores the resolution and color depth for a supported video mode.
struct SVideoMode
{
    SVideoMode(const dim::size2di &InitResolution = 0, s32 InitColorDepth = 0) :
        Resolution(InitResolution),
        ColorDepth(InitColorDepth)
    {
    }
    SVideoMode(const SVideoMode &Other) :
        Resolution(Other.Resolution),
        ColorDepth(Other.ColorDepth)
    {
    }
    ~SVideoMode()
    {
    }
    
    /* Operators */
    bool operator == (const SVideoMode &Other) const
    {
        return Resolution == Other.Resolution && ColorDepth == Other.ColorDepth;
    }
    
    /* Members */
    dim::size2di Resolution;
    s32 ColorDepth;
};

//! This structure stores the name and description of a connected display device.
struct SDisplayDevice
{
    SDisplayDevice()
    {
    }
    SDisplayDevice(const io::stringc &InitVideoController, const io::stringc &InitMonitor) :
        VideoController (InitVideoController),
        Monitor         (InitMonitor        )
    {
    }
    SDisplayDevice(const SDisplayDevice &Other) :
        VideoController (Other.VideoController  ),
        Monitor         (Other.Monitor          )
    {
    }
    ~SDisplayDevice()
    {
    }
    
    /* Members */
    io::stringc VideoController;    //!< Name of the video controller (graphics card or on-board graphics).
    io::stringc Monitor;            //!< Name of the monitor.
};


/**
The video mode enumerator class is used to detect (or rather enumerate) all video modes
(including screen resolution and color depth) which are supported by the display.
It can also enumerate all connected display devices (or rather the monitors).
\since Version 3.2
*/
class SP_EXPORT VideoModeEnumerator
{
    
    public:
        
        VideoModeEnumerator();
        ~VideoModeEnumerator();
        
        /* Inline functions */
        
        //! Returns the desktop video mode.
        inline SVideoMode getDesktop() const
        {
            return Desktop_;
        }
        
        //! Returns the count of supported video modes.
        inline u32 getVideoModeCount() const
        {
            return VideoModes_.size();
        }
        //! Returns the specified video mode or an invalid video mode if the index is invalid.
        inline SVideoMode getVideoMode(u32 Index) const
        {
            return Index < VideoModes_.size() ? VideoModes_[Index] : SVideoMode();
        }
        inline const std::vector<SVideoMode>& getVideoModeList() const
        {
            return VideoModes_;
        }
        
        //! Returns the count of connected display devices.
        inline u32 getDisplayDeviceCount() const
        {
            return DisplayDevices_.size();
        }
        //! Returns the specified display device or an invalid display device if the index is invalid.
        inline SDisplayDevice getDisplayDevice(u32 Index) const
        {
            return Index < DisplayDevices_.size() ? DisplayDevices_[Index] : SDisplayDevice();
        }
        inline const std::vector<SDisplayDevice>& getDisplayDeviceList() const
        {
            return DisplayDevices_;
        }
        
    private:
        
        /* Members */
        
        SVideoMode Desktop_;
        std::vector<SVideoMode> VideoModes_;
        std::vector<SDisplayDevice> DisplayDevices_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
