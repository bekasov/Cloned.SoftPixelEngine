/*
 * Font header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_VIDEODRIVER_FONT_H__
#define __SP_VIDEODRIVER_FONT_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spDimension.hpp"
#include "Base/spMaterialColor.hpp"
#include "RenderSystem/spTextureBase.hpp"


namespace sp
{
namespace video
{


//! Font creation flags
enum EFontFlags
{
    FONT_BOLD       = 0x01, //!< Bold type (text looks fetter).
    FONT_ITALIC     = 0x02, //!< Italic type (text looks cursive).
    FONT_UNDERLINED = 0x04, //!< Text is underlined.
    FONT_STRIKEOUT  = 0x08, //!< Text is striked out.
    FONT_SYMBOLS    = 0x10, //!< Text may contain special symbols.
    FONT_BITMAP     = 0x20, //!< Uses bitmap font instead of textured font. This is slower and looks worse.
};

struct SFontGlyph
{
    SFontGlyph() :
        StartOffset (0),
        DrawnWidth  (0),
        WhiteSpace  (0)
    {
    }
    SFontGlyph(const dim::rect2di &GlyphRect) :
        Rect        (GlyphRect  ),
        StartOffset (0          ),
        DrawnWidth  (0          ),
        WhiteSpace  (0          )
    {
    }
    SFontGlyph(
        const dim::rect2di &GlyphRect, s32 GlyphStartOffset, s32 GlyphDrawnWidth, s32 GlyphWhiteSpace) :
        Rect        (GlyphRect          ),
        StartOffset (GlyphStartOffset   ),
        DrawnWidth  (GlyphDrawnWidth    ),
        WhiteSpace  (GlyphWhiteSpace    )
    {
    }
    SFontGlyph(const SFontGlyph &Other) :
        Rect        (Other.Rect         ),
        StartOffset (Other.StartOffset  ),
        DrawnWidth  (Other.DrawnWidth   ),
        WhiteSpace  (Other.WhiteSpace   )
    {
    }
    virtual ~SFontGlyph()
    {
    }
    
    /* Functions */
    
    //! Returns the complete glyph width.
    inline s32 getWidth() const
    {
        return DrawnWidth + StartOffset + WhiteSpace;
    }
    
    /* Members */
    dim::rect2di Rect;
    s32 StartOffset;
    s32 DrawnWidth;
    s32 WhiteSpace;
};


class SP_EXPORT Font
{
    
    public:
        
        Font();
        Font(
            void* BufferRawData, const io::stringc &FontName,
            const dim::size2di &Size, const std::vector<SFontGlyph> &GlyphList,
            video::Texture* FontTexture = 0
        );
        ~Font();
        
        /* === Functions === */
        
        //! Returns the width of the specified text.
        s32 getStringWidth(const io::stringc &Text) const;
        
        //! Returns the height of the specified text with regard to the line breaks.
        s32 getStringHeight(const io::stringc &Text) const;
        
        /* === Inline functions === */
        
        /**
        Returns a pointer to the renderer specific buffer raw data. This is only used internally.
        Only use this when you have created the font manually.
        */
        inline void* getBufferRawData() const
        {
            return BufferRawData_;
        }
        
        //! Returns the font's size.
        inline dim::size2di getSize() const
        {
            return Size_;
        }
        //! Returns the font's name (e.g. "Times New Roman").
        inline io::stringc getName() const
        {
            return FontName_;
        };
        
        /**
        Returns the font's height. This is equivalent to "getSize().Height".
        \note To get the font's width use the "getSize" function. This is an exeptional case
        because normally only the font's height is important.
        \see getSize
        */
        inline s32 getHeight() const
        {
            return Size_.Height;
        }
        
        //! Returns the size of the specified text.
        inline dim::size2di getStringSize(const io::stringc &Text) const
        {
            return dim::size2di(getStringWidth(Text), Size_.Height);
        }
        
        //! Returns the texture if this font is a texture font.
        inline video::Texture* getTexture() const
        {
            return Texture_;
        }
        
        inline const std::vector<SFontGlyph>& getGlyphList() const
        {
            return GlyphList_;
        }
        
    private:
        
        #ifdef SP_COMPILE_WITH_DIRECT3D11
        friend class Direct3D11Driver;
        #endif
        #if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES1)
        friend class GLFixedFunctionPipeline;
        #endif
        
        /* Members */
        
        void* BufferRawData_;
        
        io::stringc FontName_;
        dim::size2di Size_;
        
        std::vector<SFontGlyph> GlyphList_;
        
        video::Texture* Texture_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
