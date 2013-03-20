/*
 * Storyboard trigger header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_STORYBOARD_EVENT_H__
#define __SP_STORYBOARD_EVENT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include "Framework/Tools/spStoryboardTrigger.hpp"
#include "Base/spTimer.hpp"

#include <vector>


namespace sp
{
namespace tool
{


/**
A storyboard operator connects events and facts with a consequence.
\see StoryboardEvent, StoryboardFact, StoryboardConsequence
\since Version 3.2
*/
class SP_EXPORT Event : public Trigger
{
    
    public:
        
        virtual ~Event();
        
        /* === Functions === */
        
        virtual void update() = 0;
        
    protected:
        
        Event();
        
};

class SP_EXPORT EventTimer : public Event
{
    
    public:
        
        EventTimer(u64 Duration);
        ~EventTimer();
        
        /* === Functions === */
        
        void update();
        
        void onTriggered();
        
    private:
        
        io::Timer Timer_;
        
};

class SP_EXPORT TriggerCounter : public Trigger
{
    
    public:
        
        TriggerCounter(u32 Counter = 1);
        ~TriggerCounter();
        
        /* === Functions === */
        
        bool canTrigger() const;
        
        void onTriggered();
        
        void reset();
        void reset(u32 Counter);
        
    private:
        
        /* === Members === */
        
        u32 OrigCounter_;
        u32 Counter_;
        
};


class SP_EXPORT TriggerSwitch : public Trigger
{
    
    public:
        
        TriggerSwitch(u32 Selection = 0);
        ~TriggerSwitch();
        
        /* === Functions === */
        
        void onTriggered();
        void onUntriggered();
        
    private:
        
        /* === Members === */
        
        u32 Selection_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
