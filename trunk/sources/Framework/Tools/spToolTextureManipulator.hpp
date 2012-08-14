/*
 * Image modifier header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILITY_IMAGEMODIFIER_H__
#define __SP_UTILITY_IMAGEMODIFIER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_IMAGEMODIFIER


#include "Base/spDimensionRect2D.hpp"


namespace sp
{

namespace video
{
    class ImageBuffer;
}

namespace tool
{


//! Image modifier namespace to bake normal maps, blur images etc.
namespace ImageModifier
{

static const dim::rect2di DEF_TEXMANIP_RECT = dim::rect2di(-1, -1);
static const f32 DEF_NORMALMAP_AMPLITUDE = 5.0f;

SP_EXPORT void drawMosaic(video::ImageBuffer* ImgBuffer, s32 PixelSize, dim::rect2di Rect = DEF_TEXMANIP_RECT);
SP_EXPORT void drawBlur(video::ImageBuffer* ImgBuffer, s32 PixelSize, dim::rect2di Rect = DEF_TEXMANIP_RECT);

/**
Generates a normal map out of the given height map.
\param HeightMap: Specifies the height map from which the normal map is to be generated.
\param Amplitude: Specifies the factor (or rather amplitude) for the normal map generation.
*/
SP_EXPORT void bakeNormalMap(video::ImageBuffer* ImgBuffer, f32 Amplitude = DEF_NORMALMAP_AMPLITUDE);

} // /namespace TextureModifier


} // /namespace tool
    
} // /namespace sp


#endif

#endif



// ================================================================================
