/*
 * Storyboard trigger header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_STORYBOARD_TRIGGER_H__
#define __SP_STORYBOARD_TRIGGER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include <vector>


namespace sp
{
namespace tool
{


class SP_EXPORT Trigger
{
    
    public:
        
        virtual ~Trigger();
        
        /* === Functions === */
        
        //! Returns true if any of the trigger's parents is already being triggered or this trigger has no parents.
        virtual bool triggeredParents() const;
        
        /**
        Returns true if this trigger class needs to be updated every frame when it's active.
        By default false. Overwrite this function if you need a loop update.
        */
        virtual bool needLoopUpdate() const;
        
        //! Returns true if this trigger can be triggered. By default true.
        virtual bool canTrigger() const;
        
        virtual void trigger();
        virtual void untrigger();
        
        virtual void connect(Trigger* ChildTrigger);
        virtual void disconnect(Trigger* ChildTrigger);
        
        virtual void clearTriggers();
        
        //! Event callback function when the trigger will be triggerd.
        virtual void onTriggered();
        //! Event callback function when the trigger will be untriggerd.
        virtual void onUntriggered();
        //! Event callback function during the trigger is running.
        virtual void onRunning();
        
        /* === Inline functions === */
        
        inline const std::vector<Trigger*>& getChildList() const
        {
            return Children_;
        }
        inline const std::vector<Trigger*>& getParentList() const
        {
            return Parents_;
        }
        
        //! Returns true if this trigger is currently being triggered.
        inline bool triggered() const
        {
            return IsTriggered_;
        }
        
    protected:
        
        Trigger();
        
        /* === Functions === */
        
        void triggerChildren();
        void untriggerChildren();
        
        void addToLoopUpdate();
        void removeFromLoopUpdate();
        
        /* === Members === */
        
        bool IsTriggered_;
        
    private:
        
        /* === Members === */
        
        std::vector<Trigger*> Children_;
        std::vector<Trigger*> Parents_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
