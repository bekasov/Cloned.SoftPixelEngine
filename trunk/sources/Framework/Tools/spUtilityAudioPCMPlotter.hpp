/*
 * Audio PCM plotter header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILTIY_AUDIO_PCM_PLOTTER_H__
#define __SP_UTILTIY_AUDIO_PCM_PLOTTER_H__


#include "Base/spStandard.hpp"
#include "FileFormats/Sound/spSoundLoader.hpp"
#include "RenderSystem/spTextureBase.hpp"


namespace sp
{
namespace tool
{


//! Namespace for audio PCM raw data plotter.
namespace AudioPCMPlotter
{

/**
Plots the given audio PCM raw buffer into the frame buffer.
If you want to render it into a texture, just set a render target.
\param[in] AudioBuffer Specifies the raw audio buffer. You can load these buffers with the
"loadAudioPCMBuffer" function of the "audio::SoundDevice" class.
\param[in] Rect Specifies the rectangle where the output is to be plot.
\param[in] Color Specifies the drawing color.
\param[in] RangeFrom Specifies the plotting range start. By default 0.0.
\param[in] RangeTo Specifies the plotting range end. By default 1.0.
\note 'RangeFrom' must be greater than 'RangeTo'.
\return True on success. Otherwise the audio buffer is corrupted or the rectangle has a negative size.
\see audio::SoundDevice::loadAudioPCMBuffer
\see audio::SAudioBuffer
*/
SP_EXPORT bool plotAudioBuffer(
    const audio::SAudioBuffer &AudioBuffer, const dim::rect2di &Rect, const video::color &Color,
    f32 RangeFrom = 0.0f, f32 RangeTo = 1.0f
);

} // /namespace AudioPCMPlotter


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
