/*
 * GUI scrollbar gadget header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_SCROLLBARGADGET_H__
#define __SP_GUI_SCROLLBARGADGET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIGadget.hpp"


namespace sp
{
namespace gui
{


enum EScrollbarFlags
{
    GUIFLAG_VERTICAL    = 0x00000001,
    GUIFLAG_HATCHEDFACE = 0x00000002,
};


class SP_EXPORT GUIScrollbarGadget : public GUIGadget
{
    
    public:
        
        GUIScrollbarGadget();
        ~GUIScrollbarGadget();
        
        bool update();
        void draw();
        
        void setSize(const dim::size2di &Size);
        
        void setState(s32 State);
        void setRange(s32 Range);
        
        /* Inline functions */
        
        inline s32 getState() const
        {
            return State_;
        }
        inline s32 getRange() const
        {
            return Range_;
        }
        
        inline void scroll(s32 Speed)
        {
            setState(getState() + Speed);
        }
        
    private:
        
        /* Enumerations */
        
        enum EFocusUsages
        {
            USAGE_DRAG,
            USAGE_FORWARDS,
            USAGE_BACKWARDS,
        };
        
        /* Functions */
        
        void drawScrollbar(const dim::rect2di &Rect);
        void drawScrollbarButton(const dim::rect2di &Rect, const dim::rect2df &Mapping, s32 UsageType);
        
        void updateBarLocation();
        void updateDrag();
        void updateState();
        
        void getButtonRects(
            dim::rect2di &BarRect,
            dim::rect2di &ButtonARect, dim::rect2di &ButtonBRect,
            dim::rect2di &PageARect, dim::rect2di &PageBRect
        ) const;
        
        /* Members */
        
        s32 State_, Range_, MaxState_, PageSize_;
        s32 BarPos_, BarLen_;
        
        dim::point2di CursorPosBias_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
