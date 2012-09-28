//
// SoftPixel Engine - Storyboard Tests
//

#include <SoftPixelEngine.hpp>
#include <boost/foreach.hpp>

using namespace sp;

#ifdef SP_COMPILE_WITH_STORYBOARD

#include "../common.hpp"

SP_TESTS_DECLARE

const s32 BLOCK_RADIUS = 35;
dim::point2di Pos(400, 300);
std::vector<dim::point2di> BlockList;

class MyConsequence : public tool::StoryboardConsequence
{
    public:
        MyConsequence()
        {
        }
        ~MyConsequence()
        {
        }
        
        void run()
        {
            io::Log::message("Activated Consequence");
        }
        
    //private:
        
};

class MyTrigger : public tool::StoryboardTrigger
{
    public:
        MyTrigger() :
            hasCollided_(false)
        {
        }
        ~MyTrigger()
        {
        }
        
        bool isActive() const
        {
            return false;
        }
        
        void updateCollisions()
        {
            bool Collided = false;
            
            const dim::rect2di Rect(
                Pos.X - BLOCK_RADIUS, Pos.Y - BLOCK_RADIUS,
                Pos.X + BLOCK_RADIUS, Pos.Y + BLOCK_RADIUS
            );
            
            foreach (const dim::point2di &Pnt, BlockList)
            {
                const dim::rect2di BlockRect(
                    Pnt.X - BLOCK_RADIUS, Pnt.Y - BLOCK_RADIUS,
                    Pnt.X + BLOCK_RADIUS, Pnt.Y + BLOCK_RADIUS
                );
                
                if (BlockRect.overlap(Rect))
                {
                    Collided = true;
                    break;
                }
            }
            
            if (Collided)
                activate();
            else
                deactivate();
        }
        
        void activate()
        {
            if (!hasCollided_)
            {
                hasCollided_ = true;
                trigger();
            }
        }
        void deactivate()
        {
            hasCollided_ = false;
        }
        
    private:
        bool hasCollided_;
};

void DrawBlock(s32 X, s32 Y, s32 Radius = BLOCK_RADIUS, const video::color &Color = 255)
{
    spRenderer->draw2DRectangle(dim::rect2di(X - Radius, Y - Radius, X + Radius, Y + Radius), Color);
}

int main()
{
    SP_TESTS_INIT("Storyboard")
    
    MyTrigger trigger;
    MyConsequence consequence;
    
    trigger.addConsequence(&consequence);
    
    //tool::Storyboard story;
    
    
    const s32 Speed = 5;
    
    BlockList.push_back(dim::point2di(100, 50));
    
    
    SP_TESTS_MAIN_BEGIN
    {
        spRenderer->beginDrawing2D();
        
        if (spControl->keyDown(io::KEY_LEFT )) Pos.X -= Speed;
        if (spControl->keyDown(io::KEY_RIGHT)) Pos.X += Speed;
        if (spControl->keyDown(io::KEY_UP   )) Pos.Y -= Speed;
        if (spControl->keyDown(io::KEY_DOWN )) Pos.Y += Speed;
        
        foreach (const dim::point2di &Pnt, BlockList)
            DrawBlock(Pnt.X, Pnt.Y);
        
        trigger.updateCollisions();
        
        DrawBlock(Pos.X, Pos.Y, 35, video::color(0, 255, 0));
        
        spRenderer->endDrawing2D();
    }
    SP_TESTS_MAIN_END
}

#else

int main()
{
    io::Log::error("This engine was not compiled with storyboard utility");
    return 0;
}

#endif
