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
    BufferRawData_  (0  ),
    GlyphList_      (256),
    Texture_        (0  )
{
}
Font::Font(
    void* BufferRawData, const io::stringc &FontName, const dim::size2di &Size,
    const std::vector<SFontGlyph> &GlyphList, video::Texture* FontTexture) :
    BufferRawData_  (BufferRawData  ),
    FontName_       (FontName       ),
    Size_           (Size           ),
    GlyphList_      (GlyphList      ),
    Texture_        (FontTexture    )
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
        Width += Glyph->getWidth();
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


/*
 * SFontGlyph structure
 */

SFontGlyph::SFontGlyph() :
    StartOffset (0),
    DrawnWidth  (0),
    WhiteSpace  (0)
{
}
SFontGlyph::SFontGlyph(const dim::rect2di &GlyphRect) :
    Rect        (GlyphRect  ),
    StartOffset (0          ),
    DrawnWidth  (0          ),
    WhiteSpace  (0          )
{
}
SFontGlyph::SFontGlyph(
    const dim::rect2di &GlyphRect, s32 GlyphStartOffset, s32 GlyphDrawnWidth, s32 GlyphWhiteSpace) :
    Rect        (GlyphRect          ),
    StartOffset (GlyphStartOffset   ),
    DrawnWidth  (GlyphDrawnWidth    ),
    WhiteSpace  (GlyphWhiteSpace    )
{
}
SFontGlyph::SFontGlyph(const SFontGlyph &Other) :
    Rect        (Other.Rect         ),
    StartOffset (Other.StartOffset  ),
    DrawnWidth  (Other.DrawnWidth   ),
    WhiteSpace  (Other.WhiteSpace   )
{
}
SFontGlyph::~SFontGlyph()
{
}


} // /namespace video

} // /namespace sp



// ================================================================================
