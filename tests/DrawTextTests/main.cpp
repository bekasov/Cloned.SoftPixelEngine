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
    
    video::Texture* Tex = spRenderer->loadTexture("FontComicSansMS.png");
    video::Font* Fnt = spRenderer->createFont(Tex);
    
    std::vector<dim::rect2di> ClipList;
    //spRenderer->createFontTexture(ClipList, dim::size2di(512, 256), "Comic Sans MS", 15);
    spRenderer->createFontTexture(ClipList, dim::size2di(512, 256), "Angry Blue", 15);
    
    SP_TESTS_MAIN_BEGIN
    {
        spRenderer->beginDrawing2D();
        
        spRenderer->draw2DText(Fnt, dim::point2di(15, 15), "This is a test string for 2D textured font drawing.", video::color(255));
        
        spRenderer->endDrawing2D();
    }
    SP_TESTS_MAIN_END
}
