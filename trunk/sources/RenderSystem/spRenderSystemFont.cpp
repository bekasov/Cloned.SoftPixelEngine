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


Font::Font() :
    ID_         (0  ),
    GlyphList_  (256),
    Texture_    (0  )
{
}
Font::Font(
    void* ID, const io::stringc &FontName, const dim::size2di &Size,
    const std::vector<SFontGlyph> &GlyphList, video::Texture* FontTexture) :
    ID_         (ID         ),
    FontName_   (FontName   ),
    Size_       (Size       ),
    GlyphList_  (GlyphList  ),
    Texture_    (FontTexture)
{
    if (GlyphList_.size() < 256)
        GlyphList_.resize(256);
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
    
    for (u32 i = 0, c = Text.size(); i < c; ++i)
    {
        const SFontGlyph* Glyph = &(GlyphList_[static_cast<u8>(Text[i])]);
        Width += Glyph->DrawnWidth + Glyph->StartOffset + Glyph->WhiteSpace;
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
        
        for (u32 i = 0, c = Text.size(); i < c; ++i)
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
