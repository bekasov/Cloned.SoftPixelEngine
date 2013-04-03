/*
 * Storyboard header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_STORYBOARD_H__
#define __SP_STORYBOARD_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include "Framework/Tools/spStoryboardTrigger.hpp"
#include "Framework/Tools/spStoryboardEvent.hpp"
#include "Framework/Tools/spStoryboardLogicGate.hpp"

#include <list>
#include <vector>


namespace sp
{
namespace tool
{


class SP_EXPORT Storyboard
{
    
    public:
        
        Storyboard();
        ~Storyboard();
        
        /* === Functions === */
        
        //! Updates the whole storyboard.
        void update();
        
        void deleteTrigger(Trigger* Obj);
        void clearTriggers();
        
        void deleteEvent(Event* Obj);
        void clearEvents();
        
        /* === Template functions === */
        
        template <typename T>
            T* createTrigger()
        {
            return addTrigger(new T());
        }
        template <typename T, typename Arg0>
            T* createTrigger(const Arg0 &arg0)
        {
            return addTrigger(new T(arg0));
        }
        template <typename T, typename Arg0, typename Arg1>
            T* createTrigger(const Arg0 &arg0, const Arg1 &arg1)
        {
            return addTrigger(new T(arg0, arg1));
        }
        template <typename T, typename Arg0, typename Arg1, typename Arg2>
            T* createTrigger(const Arg0 &arg0, const Arg1 &arg1, const Arg2 &arg2)
        {
            return addTrigger(new T(arg0, arg1, arg2));
        }
        template <typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
            T* createTrigger(const Arg0 &arg0, const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3)
        {
            return addTrigger(new T(arg0, arg1, arg2, arg3));
        }
        template <typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
            T* createTrigger(const Arg0 &arg0, const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3, const Arg4 &arg4)
        {
            return addTrigger(new T(arg0, arg1, arg2, arg3, arg4));
        }
        
        template <typename T>
            T* createEvent()
        {
            return addEvent(new T());
        }
        template <typename T, typename Arg0>
            T* createEvent(const Arg0 &arg0)
        {
            return addEvent(new T(arg0));
        }
        template <typename T, typename Arg0, typename Arg1>
            T* createEvent(const Arg0 &arg0, const Arg1 &arg1)
        {
            return addEvent(new T(arg0, arg1));
        }
        template <typename T, typename Arg0, typename Arg1, typename Arg2>
            T* createEvent(const Arg0 &arg0, const Arg1 &arg1, const Arg2 &arg2)
        {
            return addEvent(new T(arg0, arg1, arg2));
        }
        template <typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
            T* createEvent(const Arg0 &arg0, const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3)
        {
            return addEvent(new T(arg0, arg1, arg2, arg3));
        }
        template <typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
            T* createEvent(const Arg0 &arg0, const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3, const Arg4 &arg4)
        {
            return addEvent(new T(arg0, arg1, arg2, arg3, arg4));
        }
        
        /* === Static functions === */
        
        static Storyboard* getActive();
        static void setActive(Storyboard* ActiveStoryboard);
        
        /* === Inline functions === */
        
        inline const std::list<Trigger*>& getTriggerList() const
        {
            return Triggers_;
        }
        inline const std::vector<Trigger*>& getActiveTriggerList() const
        {
            return ActiveTriggers_;
        }
        
        inline const std::vector<Event*>& getEventList() const
        {
            return Events_;
        }
        
    private:
        
        friend class Trigger;
        
        /* === Functions === */
        
        void addLoopUpdate(Trigger* Obj);
        void removeLoopUpdate(Trigger* Obj);
        
        /* === Template functions === */
        
        template <typename T> T* addTrigger(T* NewTrigger)
        {
            Triggers_.push_back(NewTrigger);
            return NewTrigger;
        }
        
        template <typename T> T* addEvent(T* NewEvent)
        {
            Events_.push_back(NewEvent);
            return NewEvent;
        }
        
        /* === Members === */
        
        std::list<Trigger*> Triggers_;
        std::vector<Trigger*> ActiveTriggers_;
        std::vector<Event*> Events_;
        
        static Storyboard* Active_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
