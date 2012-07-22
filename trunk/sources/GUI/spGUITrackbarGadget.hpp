/*
 * GUI trackbar gadget header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_TRACKBARGADGET_H__
#define __SP_GUI_TRACKBARGADGET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIGadget.hpp"


namespace sp
{
namespace gui
{


enum ETrackbarFlags
{
    GUIFLAG_GRID = 0x00000001,
};


class SP_EXPORT GUITrackbarGadget : public GUIGadget
{
    
    public:
        
        GUITrackbarGadget();
        ~GUITrackbarGadget();
        
        bool update();
        void draw();
        
        void setState(s32 State);
        void setRange(s32 Min, s32 Max);
        void setBarSize(s32 Size);
        
        /* Inline functions */
        
        inline s32 getState() const
        {
            return State_;
        }
        inline s32 getRangeMin() const
        {
            return RangeMin_;
        }
        inline s32 getRangeMax() const
        {
            return RangeMax_;
        }
        inline s32 getBarSize() const
        {
            return BarSize_;
        }
        
    private:
        
        /* Enumerations */
        
        enum EFocusUsages
        {
            USAGE_DRAG,
        };
        
        /* Functions */
        
        void setStatePos(s32 PosHorz, bool isMoving);
        
        dim::rect2di getTrackBarRect() const;
        
        /* Members */
        
        s32 State_, RangeMin_, RangeMax_;
        s32 CursorPosBias_, BarSize_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
