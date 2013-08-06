/*
 * GUI trackbar gadget file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUITrackbarGadget.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "RenderSystem/spRenderSystem.hpp"
#include "GUI/spGUIManager.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;
extern gui::GUIManager* GlbGUIMngr;

namespace gui
{


extern video::Font* __spGUIFont;

GUITrackbarGadget::GUITrackbarGadget() :
    GUIGadget       (GADGET_TRACKBAR),
    State_          (0              ),
    RangeMin_       (0              ),
    RangeMax_       (100            ),
    CursorPosBias_  (0              ),
    BarSize_        (10             )
{
}
GUITrackbarGadget::~GUITrackbarGadget()
{
}

bool GUITrackbarGadget::update()
{
    if (hasFocus() && GlbGUIMngr->MouseWheel_)
        setStatePos(GlbGUIMngr->MouseWheel_, true);
    
    if (!checkDefaultUpdate())
        return false;
    
    if (isEnabled_ && mouseOver(Rect_) && !foreignUsage() && mouseLeft())
    {
        focus();
        
        const dim::rect2di BarRect(getTrackBarRect());
        
        CursorPosBias_ = GlbGUIMngr->CursorPos_.X - getTrackBarRect().Left - 1;
        
        if (mouseOver(BarRect))
            useFocus(USAGE_DRAG);
    }
    
    if (usage(USAGE_DRAG))
    {
        setStatePos(GlbGUIMngr->CursorPos_.X, false);
        sendEvent(EVENT_GADGET, EVENT_ACTIVATE);
    }
    
    return true;
}

void GUITrackbarGadget::draw()
{
    if (!isVisible_ || isValidated_ || !setupClipping())
        return;
    
    const dim::rect2di TrackRect(
        Rect_.Left, (Rect_.Top + Rect_.Bottom)/2 - 3,
        Rect_.Right, (Rect_.Top + Rect_.Bottom)/2 + 3
    );
    const dim::rect2di BarRect(getTrackBarRect());
    
    if (Flags_ & GUIFLAG_GRID)
    {
        for (s32 i = 0, PosHorz; i <= RangeMax_ - RangeMin_; ++i)
        {
            PosHorz = Rect_.Left + BarSize_/2 + i * (Rect_.Right - Rect_.Left - BarSize_) / (RangeMax_ - RangeMin_);
            
            GlbRenderSys->draw2DLine(
                dim::point2di(PosHorz, Rect_.Top), dim::point2di(PosHorz, Rect_.Bottom), 0
            );
        }
    }
    
    GlbRenderSys->draw2DRectangle(TrackRect, Color_, Color_ / 2, Color_ / 2, Color_);
    drawFrame(TrackRect, 0, true);
    
    drawBgRect(BarRect, BarRect, false, true);
    drawFrame(BarRect, 0, true);
    
    if (!isEnabled_)
        drawHatchedFace(Rect_);
}

void GUITrackbarGadget::setState(s32 State)
{
    State_ = State;
    math::clamp(State_, RangeMin_, RangeMax_);
}
void GUITrackbarGadget::setRange(s32 Min, s32 Max)
{
    if (Min != Max)
    {
        if (Max > Min)
        {
            RangeMin_ = Min;
            RangeMax_ = Max;
        }
        else
        {
            RangeMin_ = Max;
            RangeMax_ = Min;
        }
        
        math::clamp(State_, RangeMin_, RangeMax_);
    }
}
void GUITrackbarGadget::setBarSize(s32 Size)
{
    BarSize_ = math::MinMax(Size, 1, (Rect_.Right - Rect_.Left) - 1);
}


/*
 * ======= Private: =======
 */

void GUITrackbarGadget::setStatePos(s32 PosHorz, bool isMoving)
{
    if (isMoving)
        setState(getState() + PosHorz);
    else
    {
        PosHorz -= Rect_.Left + CursorPosBias_;
        PosHorz = (s32)((f32)(PosHorz * (RangeMax_ - RangeMin_)) / (Rect_.Right - Rect_.Left - BarSize_) + 0.5f);
        setState(PosHorz + RangeMin_);
    }
}

dim::rect2di GUITrackbarGadget::getTrackBarRect() const
{
    const s32 StatePos = (Rect_.Right - Rect_.Left - BarSize_) * (State_ - RangeMin_) / (RangeMax_ - RangeMin_);
    
    return dim::rect2di(
        Rect_.Left + StatePos, Rect_.Top, Rect_.Left + StatePos + BarSize_, Rect_.Bottom
    );
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
