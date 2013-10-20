//
// SoftPixel Engine - Storyboard Tests
//

#include <SoftPixelEngine.hpp>
#include <boost/foreach.hpp>

using namespace sp;

#ifdef SP_COMPILE_WITH_STORYBOARD

#include "../common.hpp"

SP_TESTS_DECLARE

#define CONVERSATION_TEST


/*
 * Global members
 */

tool::Storyboard* Story = 0;

const s32 BLOCK_RADIUS = 35;
dim::point2df CharPos(400, 300);
video::color CharColor(255);


/*
 * Classes
 */

template <typename T> T Val(const std::string &Str)
{
    return io::stringc(Str).val<T>();
}

#ifndef CONVERSATION_TEST

class TriggerMoveChar : public tool::Trigger
{
    
    public:
        
        TriggerMoveChar(const std::vector<std::string> &Params) :
            tool::Trigger   (       ),
            DebTimer_       (250ul  )
        {
            if (Params.size() == 2)
            {
                Dir_.X = Val<f32>(Params[0]);
                Dir_.Y = Val<f32>(Params[1]);
            }
            trigger();
        }
        ~TriggerMoveChar()
        {
        }
        
        void onTriggered()
        {
            io::Log::message("TriggerMoveChar Triggered");
        }
        void onUntriggered()
        {
            io::Log::message("TriggerMoveChar Un-Triggered");
        }
        
        void onRunning()
        {
            CharPos += Dir_;
            
            if (DebTimer_.finish())
            {
                //io::Log::message("Move Speed = " + io::stringc(math::Max(math::Abs(Dir_.X), math::Abs(Dir_.Y))));
                io::Log::message("X = " + io::stringc(Dir_.X) + ", Y = " + io::stringc(Dir_.Y));
                DebTimer_.reset();
            }
        }
        
        bool needLoopUpdate() const
        {
            return true;
        }
        
    private:
        
        dim::point2df Dir_;
        
        io::Timer DebTimer_;
        
};

class EventKeyboard : public tool::Event
{
    
    public:
        
        EventKeyboard(const std::vector<std::string> &Params) :
            tool::Event (           ),
            KeyCode_    (io::KEY_0  )
        {
            if (Params.size())
            {
                switch (Val<s32>(Params[0]))
                {
                    case 0: KeyCode_ = io::KEY_UP;      break;
                    case 1: KeyCode_ = io::KEY_DOWN;    break;
                    case 2: KeyCode_ = io::KEY_LEFT;    break;
                    case 3: KeyCode_ = io::KEY_RIGHT;   break;
                    
                    case 7:
                    case 8: KeyCode_ = io::KEY_SHIFT;   break;
                }
            }
        }
        ~EventKeyboard()
        {
        }
        
        void update()
        {
            if (spControl->keyDown(KeyCode_))
                trigger();
            else
                untrigger();
        }
        
    private:
        
        io::EKeyCodes KeyCode_;
        
};

class EventCharInBox : public tool::Event
{
    
    public:
        
        EventCharInBox(const std::vector<std::string> &Params) :
            tool::Event()
        {
            if (Params.size() == 4)
            {
                Rect_.Left      = Val<f32>(Params[0]);
                Rect_.Right     = Val<f32>(Params[1]);
                Rect_.Top       = Val<f32>(Params[2]);
                Rect_.Bottom    = Val<f32>(Params[3]);
            }
        }
        ~EventCharInBox()
        {
        }
        
        void onTriggered()
        {
            CharColor = video::color(255, 0, 0);
        }
        void onUntriggered()
        {
            CharColor = 255;
        }
        
        void update()
        {
            if (Rect_.overlap(CharPos))
                trigger();
            else
                untrigger();
        }
        
    private:
        
        dim::rect2df Rect_;
        
};

class CustomSceneLoader : public scene::SceneLoaderSPSB
{
    
    public:
        
        CustomSceneLoader() : scene::SceneLoaderSPSB()
        {
        }
        ~CustomSceneLoader()
        {
        }
        
    private:
        
        /* === Functions === */
        
        tool::Trigger* createStoryboardItem(const SpStoryboardItem &Object)
        {
            const io::stringc& TemplateName = Object.ScriptTemplate.TemplateName;
            const std::vector<std::string>& Params = Object.ScriptTemplate.Parameters;
            
            switch (Object.Type)
            {
                case 0:
                    return Story->createTrigger<tool::LogicGate>(
                        static_cast<tool::ELogicGates>(Object.LogicGateType + tool::LOGICGATE_AND)
                    );
                    
                case 1:
                    if (TemplateName == "Keyboard")
                        return Story->createEvent<EventKeyboard>(Params);
                    else if (TemplateName == "Timer")
                    {
                        if (Params.size() == 1)
                            return Story->createEvent<tool::EventTimer>(io::stringc(Params[0]).val<u64>());
                    }
                    else if (TemplateName == "Character In Box")
                        return Story->createEvent<EventCharInBox>(Params);
                    break;
                    
                case 2:
                    if (TemplateName == "Move Character")
                        return Story->createTrigger<TriggerMoveChar>(Params);
                    break;
            }
            
            return 0;
        }
        
};

#else

struct SConversation;

std::map<u32, SConversation> Conversations;

bool cmpConv(SConversation* a, SConversation* b);

struct SConversation
{
    SConversation() :
        Index(0)
    {
    }
    ~SConversation()
    {
    }

    /* Functions */
    std::vector<SConversation*> getAnswers() const
    {
        std::vector<SConversation*> List;

        foreach (u32 Id, Answers)
            List.push_back(&Conversations[Id]);

        std::sort(List.begin(), List.end(), cmpConv);

        return List;
    }

    /* Members */
    io::stringc Text;
    s32 Index;
    std::vector<u32> Answers;
};

bool cmpConv(SConversation* a, SConversation* b)
{
    return a->Index < b->Index;
}

class ConversationSceneLoader : public scene::SceneLoaderSPSB
{
    
    public:
        
        ConversationSceneLoader() : scene::SceneLoaderSPSB()
        {
        }
        ~ConversationSceneLoader()
        {
        }
        
    private:
        
        /* === Functions === */
        
        tool::Trigger* createStoryboardItem(const SpStoryboardItem &Object)
        {
            SConversation Conv;
            {
                Conv.Text       = Object.ScriptTemplate.Parameters[1];
                Conv.Index      = Val<s32>(Object.ScriptTemplate.Parameters[0]);
                Conv.Answers    = Object.LinkIds;
            }
            Conversations[Object.Id] = Conv;

            return 0;
        }

};

#endif


/*
 * Global functions
 */

#ifndef CONVERSATION_TEST

void DrawBlock(const dim::point2df &Pos, s32 Radius = BLOCK_RADIUS, const video::color &Color = 255)
{
    const s32 X = static_cast<s32>(Pos.X);
    const s32 Y = static_cast<s32>(Pos.Y);
    spRenderer->draw2DRectangle(dim::rect2di(X - Radius, Y - Radius, X + Radius, Y + Radius), Color);
}

#else

SConversation* getConversationByIndex(s32 Index)
{
    for (std::map<u32, SConversation>::iterator it = Conversations.begin(); it != Conversations.end(); ++it)
    {
        if (it->second.Index == Index)
            return &it->second;
    }
    return 0;
}

#endif

int main()
{
    SP_TESTS_INIT("Storyboard")
    
    #ifndef CONVERSATION_TEST

    /* Create storyboard */
    Story = new tool::Storyboard();
    
    /* Load scene */
    CustomSceneLoader loader;
    loader.loadScene("TestScene.spsb", "", scene::SCENEFLAG_ALL);
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        {
            /* Update storyboard */
            Story->update();
            
            /* Draw scene */
            DrawBlock(CharPos, BLOCK_RADIUS, CharColor);
        }
        spContext->flipBuffers();
    }
    
    delete Story;

    #else
    
    /* Load scene */
    ConversationSceneLoader loader;
    loader.loadScene("ConversationEventHandling.spsb", "", scene::SCENEFLAG_ALL);

    /* Initialize conversation */
    SConversation* Conv = getConversationByIndex(-1);

    size_t Selection = 0;

    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();

        if (Conv)
        {
            size_t NumAnswers = Conv->Answers.size();

            /* Draw question */
            Draw2DText(dim::point2di(15, 15), Conv->Text);

            if (NumAnswers >= 2)
            {
                /* Draw answers */
                s32 PosY = 50;
                size_t i = 0;

                foreach (SConversation* SubConv, Conv->getAnswers())
                {
                    Draw2DText(
                        dim::point2di(15, PosY), SubConv->Text,
                        (Selection == i ? video::color(255, 255, 0) : video::color(255))
                    );
                    PosY += 20;
                    ++i;
                }

                /* Update user input */
                if (spControl->keyHit(io::KEY_UP))
                {
                    if (Selection == 0)
                        Selection = NumAnswers - 1;
                    else
                        --Selection;
                }
                if (spControl->keyHit(io::KEY_DOWN))
                {
                    if (Selection == NumAnswers - 1)
                        Selection = 0;
                    else
                        ++Selection;
                }
            }

            if (spControl->keyHit(io::KEY_RETURN))
            {
                if (NumAnswers >= 2)
                    Conv = Conv->getAnswers()[Selection]->getAnswers().front();
                else if (NumAnswers >= 1)
                    Conv = Conv->getAnswers().front();
                else
                    Conv = getConversationByIndex(-1);
                Selection = 0;
            }
        }

        spContext->flipBuffers();
    }
    
    #endif
    
    deleteDevice();
    
    return 0;
}

#else

int main()
{
    io::Log::error("This engine was not compiled with storyboard utility");
    return 0;
}

#endif
