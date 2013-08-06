/*
 * Audio PCM plotter file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spUtilityAudioPCMPlotter.hpp"
#include "RenderSystem/spRenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace tool
{


namespace AudioPCMPlotter
{

template <typename T, s32 Channels, s32 AmlpMin, s32 AmlpMax, s32 Offset> static void plotAudioBufferGeneric(
    const audio::SAudioBuffer &AudioBuffer, const dim::rect2di &Rect, const video::color &Color, f32 RangeFrom, f32 RangeTo)
{
    /* Get raw buffer, indices and range */
    const T* Buf            = reinterpret_cast<const T*>(AudioBuffer.BufferPCM);
    const T* BufEnd         = reinterpret_cast<const T*>(AudioBuffer.BufferPCM + AudioBuffer.BufferSize);
    
    const s32 y             = (Rect.Bottom + Rect.Top) / 2;
    const s32 Width         = Rect.getWidth();
    
    const f32 Scale         = static_cast<f32>(Rect.getHeight()/2);
    const s32 Range         = (AmlpMax - AmlpMin) / 2;
    const f32 RangeScale    = Scale / static_cast<f32>(Range);
    
    const f32 RangeLen      = RangeTo - RangeFrom;
    const u32 Len           = AudioBuffer.BufferSize / Channels / sizeof(T);
    const u32 BlockLen      = static_cast<u32>(RangeLen * static_cast<f32>(Len / Width));
    
    Buf += static_cast<u32>(RangeFrom * static_cast<f32>(AudioBuffer.BufferSize / sizeof(T)));
    
    for (s32 x = Rect.Left; x < Rect.Right; ++x)
    {
        /* Get amplitude min and max value */
        s32 Min = AmlpMax, Max = AmlpMin;
        
        for (u32 i = 0; i <= BlockLen && Buf < BufEnd; ++i)
        {
            for (u32 j = 0; j < Channels; ++j)
            {
                math::decrease(Min, static_cast<s32>(*Buf));
                math::increase(Max, static_cast<s32>(*Buf));
                ++Buf;
            }
        }
        
        if (Max > Min)
        {
            /* Scale amplitude */
            Min = static_cast<s32>(static_cast<f32>(Min) * RangeScale);
            Max = static_cast<s32>(static_cast<f32>(Max) * RangeScale);
            
            if (Min == Max)
                ++Max;
            
            /* Draw amplitude line */
            GlbRenderSys->draw2DLine(
                dim::point2di(x, y + Min + Offset),
                dim::point2di(x, y + Max + Offset),
                Color
            );
        }
    }
}

SP_EXPORT bool plotAudioBuffer(
    const audio::SAudioBuffer &AudioBuffer, const dim::rect2di &Rect, const video::color &Color, f32 RangeFrom, f32 RangeTo)
{
    /* Check for valid parameters */
    if (!AudioBuffer.BufferPCM || !AudioBuffer.BufferSize || !Rect.valid() || RangeFrom >= RangeTo)
        return false;
    
    RangeFrom   = math::Max(0.0f, RangeFrom );
    RangeTo     = math::Max(0.0f, RangeTo   );
    
    /* Choose channel format */
    switch (AudioBuffer.FormatFlags.ChannelFormat)
    {
        case audio::WAVECHANNEL_MONO8:
            plotAudioBufferGeneric<u8, 1, 0, 255, 127>(AudioBuffer, Rect, Color, RangeFrom, RangeTo);
            break;
        case audio::WAVECHANNEL_MONO16:
            plotAudioBufferGeneric<u8, 2, 0, 255, 127>(AudioBuffer, Rect, Color, RangeFrom, RangeTo);
            break;
        case audio::WAVECHANNEL_STEREO8:
            plotAudioBufferGeneric<s16, 1, SHRT_MIN, SHRT_MAX, 0>(AudioBuffer, Rect, Color, RangeFrom, RangeTo);
            break;
        case audio::WAVECHANNEL_STEREO16:
            plotAudioBufferGeneric<s16, 2, SHRT_MIN, SHRT_MAX, 0>(AudioBuffer, Rect, Color, RangeFrom, RangeTo);
            break;
    }
    
    return true;
}

} // /namespace AudioPCMPlotter


} // /namespace tool

} // /namespace sp



// ================================================================================
