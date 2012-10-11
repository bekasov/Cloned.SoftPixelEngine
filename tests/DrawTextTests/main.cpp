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
    SP_TESTS_INIT_EX(
        video::RENDERER_DIRECT3D9,
        //video::RENDERER_OPENGL,
        dim::size2di(1024, 768),
        "Draw Text", false
    )
    
    spRenderer->setVsync(false);
    
    // Create textured-font and bitmap-font
    std::vector<video::SFontGlyph> GlyphList;
    
    s32 Flags = (
        0
        #if 0
        //| video::FONT_ITALIC
        //| video::FONT_UNDERLINED
        | video::FONT_BOLD
        //| video::FONT_SYMBOLS
        #endif
    );
    
    spDevice->registerFontResource("One Starry Night.ttf");
    
    const io::stringc FontName(
        //"Arial"
        //"Courier New"
        //"Comic Sans MS"
        //"Times New Roman"
        //"Brush Script MT"
        "One Starry Night"
    );
    
    s32 FontSize = 60;//35;
    
    video::Font* Fnt = spRenderer->createFont(FontName, FontSize, Flags);
    video::Font* BmpFnt = spRenderer->createFont(FontName, FontSize, Flags | video::FONT_BITMAP);
    
    video::Texture* Tex = Fnt->getTexture();
    
    //if (Tex)
    //    Tex->setFilter(video::FILTER_LINEAR);
    
    const io::stringc TestString(
        "This is a test string for 2D textured font drawing. { [x] }"
    );
    
    const video::color FontColor(255, 0, 0);
    
    if (Tex)
        io::Log::message("Width = " + io::stringc(Tex->getSize().Width) + ", Height = " + io::stringc(Tex->getSize().Height));
    
    // Create timer
    io::Timer timer(true);
    
    SP_TESTS_MAIN_BEGIN
    {
        spRenderer->beginDrawing2D();
        
        #if 0
        
        for (s32 i = 0; i < 100; ++i)
        {
            #if 1
            dim::matrix4f Mat;
            Mat.rotateZ(math::Randomizer::randFloat()*360);
            spRenderer->setFontTransformation(Mat);
            #endif
            
            spRenderer->draw2DText(
                Fnt,
                dim::point2di(
                    math::Randomizer::randInt(0, 1024),
                    math::Randomizer::randInt(0, 768)
                ),
                TestString,
                math::Randomizer::randColor()
            );
        }
        
        #   if 1
        spRenderer->setFontTransformation(dim::matrix4f());
        #   endif
        
        spRenderer->draw2DRectangle(dim::rect2di(0, 0, 300, 60), video::color(0, 0, 255));
        spRenderer->draw2DText(
            Fnt,
            dim::point2di(15, 15),
            "FPS: " + io::stringc(timer.getFPS()),
            FontColor
        );
        
        #else
        
        const s32 TextSize = BmpFnt->getStringWidth(TestString);
        
        #   if 0
        static f32 x = 0.0f, s = 1.5f;
        x += static_cast<f32>(spControl->getMouseWheel()) * 2.5f;
        
        dim::matrix4f Mat;
        Mat.translate(dim::vector3df(150, 150, 0));
        Mat.rotateZ(x);
        Mat.translate(dim::vector3df(-s * (TextSize/2), 0, 0));
        Mat.scale(s);
        
        spRenderer->setFontTransformation(Mat);
        #   endif
        
        spRenderer->draw2DRectangle(
            dim::rect2di(10, 10, TextSize + 20, 55),
            video::color(255, 0, 0),
            false
        );
        
        spRenderer->draw2DText(
            Fnt, dim::point2di(15), TestString, FontColor
        );
        
        spRenderer->draw2DText(
            BmpFnt, dim::point2di(15, 60), TestString, FontColor
        );
        
        #endif
        
        #if 1
        if (Tex)
            spRenderer->draw2DImage(Tex, dim::point2di(15, spContext->getResolution().Height - Tex->getSize().Height - 15));
        #endif
        
        spRenderer->endDrawing2D();
    }
    SP_TESTS_MAIN_END
}
