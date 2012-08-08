/*
 * GUI progressbar gadget header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_PROGRESSBARGADGET_H__
#define __SP_GUI_PROGRESSBARGADGET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIGadget.hpp"


namespace sp
{
namespace gui
{


enum EProgressbarFlags
{
    GUIFLAG_NOTEXT = 0x00000001,
};


/**
Progressbar controller gadget.
\ingroup group_gui
*/
class SP_EXPORT GUIProgressbarGadget : public GUIGadget
{
    
    public:
        
        GUIProgressbarGadget();
        ~GUIProgressbarGadget();
        
        bool update();
        void draw();
        
        /* Inline functions */
        
        inline void setState(s32 State)
        {
            State_ = math::MinMax(State, 0, Range_);
        }
        inline s32 getState() const
        {
            return State_;
        }
        
        inline void setRange(s32 Range)
        {
            Range_ = math::Max(1, Range);
        }
        inline s32 getRange() const
        {
            return Range_;
        }
        
    private:
        
        /* Members */
        
        s32 State_, Range_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
