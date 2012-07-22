/*
 * Font file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spRenderSystemFont.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#endif


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


Font::Font() : ID_(0), Texture_(0)
{
}
Font::Font(
    void* ID, const io::stringc &FontName, const dim::size2di &Size,
    const std::vector<s32> &CharWidthList, video::Texture* TextureMap)
    : ID_(ID), FontName_(FontName), Size_(Size), CharWidthList_(CharWidthList), Texture_(TextureMap)
{
}
Font::~Font()
{
}

s32 Font::getStringWidth(const io::stringc &Text) const
{
    #if defined(SP_PLATFORM_LINUX)
    
    return Text.size() * Size_.Width;
    
    #else
    
    s32 Width = 0;
    
    if (CharWidthList_.size() >= 256)
    {
        for (s32 i = 0, c = Text.size(); i < c; ++i)
            Width += CharWidthList_[Text[i]];
    }
    
    return Width;
    
    #endif
}

s32 Font::getStringHeight(const io::stringc &Text) const
{
    s32 Lines = 0;
    
    if (Text.size())
    {
        Lines = 1;
        
        for (s32 i = 0, c = Text.size(); i < c; ++i)
        {
            if (Text[i] == '\n')
                ++Lines;
        }
    }
    
    return Size_.Height * Lines;
}


} // /namespace video

} // /namespace sp



// ================================================================================
