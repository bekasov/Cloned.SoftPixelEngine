/*
 * Movie sequence packet header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MOVIESEQUENCEPACKET_H__
#define __SP_MOVIESEQUENCEPACKET_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionSize2D.hpp"
#include "Base/spInputOutputString.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#   include <vfw.h>
#endif


namespace sp
{
namespace video
{


#if defined(SP_PLATFORM_WINDOWS)

struct SMovieSequencePacket
{
    /* Basics */
    dim::size2di        Size;
    s32                 VideoMPF,           AudioMPF;
    s32                 VideoLastFrame,     AudioLastFrame;
    s8*                 VideoData;
    u8*                 RawData;
    
    /* Movie status */
    s32                 Next, Frame;
    s8                  State;
    u32                 Time, LastTime;
    s32                 Resolution;
    bool                Looped;
    
    /* Buffer data */
    AVISTREAMINFO       VideoStreamInfo,    AudioStreamInfo;
    PAVISTREAM          pVideoStream,       pAudioStream;
    PGETFRAME           pGetFrame;
    BITMAPINFOHEADER    BitmapInfoHeader;
    HDRAWDIB            hDrawDIB;
    HBITMAP             hBitmap;
    HDC                 hDeviceContext;
};

#endif


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
