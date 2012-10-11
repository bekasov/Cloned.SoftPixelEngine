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
    
    bool State1 = false;
    
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
        
        if (spControl->mouseDoubleClicked(io::MOUSE_XBUTTON1))
            State1 = !State1;
        
        const dim::rect2di Rect(
            Pos.X - Radius, Pos.Y - Radius, Pos.X + Radius, Pos.Y + Radius
        );
        
        spRenderer->draw2DRectangle(Rect, Color);
        
        if (State1)
            spRenderer->draw2DRectangle(Rect + dim::rect2di(-2, -2, 2, 2), Color, false);
        
        spRenderer->endDrawing2D();
    }
    SP_TESTS_MAIN_END
}
