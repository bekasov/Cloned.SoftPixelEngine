//
// SoftPixel Engine - Draw Text Tests
//

#include <SoftPixelEngine.hpp>
#include <boost/foreach.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

int main()
{
    SP_TESTS_INIT("Draw Text")
    
    //spRenderer->setVsync(false);
    
    std::vector<video::SFontGlyph> GlyphList;
    
    s32 Flags = (
        #if 0
        //video::FONT_ITALIC |
        //video::FONT_UNDERLINED
        video::FONT_BOLD
        #else
        0
        #endif
    );
    
    const io::stringc FontName(
        "Arial"
        //"Courier New"
        //"Comic Sans MS"
        //"Brush Script MT"
    );
    
    s32 FontSize = 35;
    
    video::Texture* Tex = spRenderer->createFontTexture(
        GlyphList, dim::size2di(512, 512), 
        FontName,
        FontSize,
        Flags
    );
    
    const io::stringc TestString(
        "This is a test string for 2D textured font drawing. [x]"
    );
    
    video::Font* Fnt = spRenderer->createFont(Tex, GlyphList, FontSize);
    
    video::Font* BmpFnt = spRenderer->createFont(FontName, FontSize, Flags);
    
    io::Timer timer(true);
    
    SP_TESTS_MAIN_BEGIN
    {
        spRenderer->beginDrawing2D();
        
        #if 0
        
        for (s32 i = 0; i < 100; ++i)
        {
            spRenderer->draw2DText(
                Fnt,
                dim::point2di(
                    math::Randomizer::randInt(0, 800),
                    math::Randomizer::randInt(0, 600)
                ),
                TestString,
                math::Randomizer::randColor()
            );
        }
        
        spRenderer->draw2DRectangle(dim::rect2di(0, 0, 300, 60), video::color(0, 0, 255));
        spRenderer->draw2DText(
            Fnt,
            dim::point2di(15, 15),
            "FPS: " + io::stringc(timer.getFPS()),
            video::color(255)
        );
        
        #else
        
        const s32 TextSize = BmpFnt->getStringWidth(TestString);
        
        spRenderer->draw2DRectangle(
            dim::rect2di(10, 10, TextSize + 20, 55),
            video::color(255, 0, 0),
            false
        );
        
        spRenderer->draw2DText(
            Fnt, dim::point2di(15, 15), TestString, video::color(255)
        );
        
        spRenderer->draw2DText(
            BmpFnt, dim::point2di(15, 45), TestString, video::color(255)
        );
        
        #endif
        
        #if 0
        if (Tex)
            spRenderer->draw2DImage(Tex, dim::point2di(15, 100));
        #endif
        
        spRenderer->endDrawing2D();
    }
    SP_TESTS_MAIN_END
}
