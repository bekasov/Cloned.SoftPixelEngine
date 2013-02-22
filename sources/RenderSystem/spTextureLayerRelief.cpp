/*
 * Relief texture layer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spTextureLayerRelief.hpp"


namespace sp
{
namespace video
{


TextureLayerRelief::TextureLayerRelief() :
    TextureLayer    (TEXLAYER_RELIEF),
    ReliefEnabled_  (false          ),
    MinSamples_     (0              ),
    MaxSamples_     (50             ),
    HeightMapScale_ (0.015f         ),
    ViewRange_      (2.0f           )
{
}
TextureLayerRelief::~TextureLayerRelief()
{
}


} // /namespace video

} // /namespace sp



// ================================================================================
