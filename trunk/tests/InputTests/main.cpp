//
// SoftPixel Engine - Input Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

int main()
{
    SP_TESTS_INIT("Input")
    
    dim::point2di Pos(400, 300);
    
    spRenderer->setClearColor(255);
    
    SP_TESTS_MAIN_BEGIN
    {
        spRenderer->beginDrawing2D();
        
        const s32 Radius = 35;
        
        bool SpeedUp = spControl->keyDown(io::KEY_SHIFT);
        
        const s32 Speed = (SpeedUp ? 6 : 3);
        
        if (spControl->keyDown(io::KEY_UP))
            Pos.Y -= Speed;
        if (spControl->keyDown(io::KEY_DOWN))
            Pos.Y += Speed;
        if (spControl->keyDown(io::KEY_LEFT))
            Pos.X -= Speed;
        if (spControl->keyDown(io::KEY_RIGHT))
            Pos.X += Speed;
        
        video::color Color(0);
        
        if (spControl->keyDown(io::KEY_LCONTROL))
            Color.Red = 255;
        if (spControl->keyDown(io::KEY_RCONTROL))
            Color.Green = 255;
        
        if (spControl->keyHit(io::KEY_RCONTROL))
            io::Log::message("Right Control");
        if (spControl->keyHit(io::KEY_LCONTROL))
            io::Log::message("Left Control");
        if (spControl->keyHit(io::KEY_RSHIFT))
            io::Log::message("Right Shift");
        if (spControl->keyHit(io::KEY_LSHIFT))
            io::Log::message("Left Shift");
        
        spRenderer->draw2DRectangle(
            dim::rect2di(Pos.X - Radius, Pos.Y - Radius, Pos.X + Radius, Pos.Y + Radius), Color
        );
        
        spRenderer->endDrawing2D();
    }
    SP_TESTS_MAIN_END
}
