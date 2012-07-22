/*
 * GUI scrollbar gadget file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUIScrollbarGadget.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "RenderSystem/spRenderSystem.hpp"
#include "GUI/spGUIManager.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern gui::GUIManager* __spGUIManager;

namespace gui
{


extern video::Font* __spGUIFont;

GUIScrollbarGadget::GUIScrollbarGadget() : GUIGadget(GADGET_SCROLLBAR)
{
    State_      = 0;
    BarPos_     = 0;
    BarLen_     = 0;
    Range_      = 100;
    MaxState_   = 10;
    PageSize_   = 10;
}
GUIScrollbarGadget::~GUIScrollbarGadget()
{
}

bool GUIScrollbarGadget::update()
{
    if (hasFocus() && __spGUIManager->MouseWheel_)
        scroll(-__spGUIManager->MouseWheel_ * 30);
    
    if (!checkDefaultUpdate())
        return false;
    
    if (isEnabled_ && mouseOver(Rect_) && !foreignUsage())
    {
        if (mouseLeft())
        {
            focus();
            
            CursorPosBias_ = __spGUIManager->CursorPos_ - dim::point2di(Rect_.Left, Rect_.Top + 1) - dim::point2di(BarPos_);
            
            dim::rect2di BarRect, ButtonARect, ButtonBRect, PageARect, PageBRect;
            getButtonRects(BarRect, ButtonARect, ButtonBRect, PageARect, PageBRect);
            
            if (mouseOver(BarRect))
                useFocus(USAGE_DRAG);
            else if (mouseOver(ButtonARect))
                useFocus(USAGE_FORWARDS);
            else if (mouseOver(ButtonBRect))
                useFocus(USAGE_BACKWARDS);
            else if (mouseOver(PageARect))
                scroll(-PageSize_);
            else if (mouseOver(PageBRect))
                scroll(PageSize_);
        }
    }
    
    if (usage(USAGE_DRAG))
        updateDrag();
    else if (usage(USAGE_FORWARDS))
        scroll(-3);
    else if (usage(USAGE_BACKWARDS))
        scroll(3);
    
    return true;
}

void GUIScrollbarGadget::draw()
{
    if (!isVisible_ || isValidated_ || !setupClipping())
        return;
    
    #if 1 // !TODO! -> call in virtual setPosition/ setRect/ setSize functino
    updateBarLocation();
    #endif
    
    __spVideoDriver->draw2DRectangle(Rect_, Color_);
    
    if (Flags_ & GUIFLAG_HATCHEDFACE)
        drawHatchedFace(Rect_);
    
    if (!(Flags_ & GUIFLAG_BORDERLESS))
        drawFrame(Rect_, 0, false);
    
    dim::rect2di BarRect, ButtonARect, ButtonBRect, PageARect, PageBRect;
    getButtonRects(BarRect, ButtonARect, ButtonBRect, PageARect, PageBRect);
    
    drawScrollbar(BarRect);
    drawScrollbarButton(ButtonARect, dim::rect2df(0, 0, 1, 1), USAGE_FORWARDS);
    drawScrollbarButton(ButtonBRect, dim::rect2df(0, 1, 1, 0), USAGE_BACKWARDS);
    
    if (!isEnabled_)
        drawHatchedFace(Rect_);
}

void GUIScrollbarGadget::setSize(const dim::size2di &Size)
{
    GUIController::setSize(Size);
    updateState();
}

void GUIScrollbarGadget::setState(s32 State)
{
    State_ = State;
    math::Clamp(State_, 0, MaxState_);
    updateBarLocation();
}
void GUIScrollbarGadget::setRange(s32 Range)
{
    Range_ = math::Max(0, Range);
    updateBarLocation();
}


/*
 * ======= Private: =======
 */

void GUIScrollbarGadget::drawScrollbar(const dim::rect2di &Rect)
{
    drawBgRect(Rect, Rect);
    drawFrame(Rect, 0, true);
}

void GUIScrollbarGadget::drawScrollbarButton(const dim::rect2di &Rect, const dim::rect2df &Mapping, s32 UsageType)
{
    drawBgRect(Rect, Rect, false, false, UsageType);
    
    if (Flags_ & GUIFLAG_VERTICAL)
    {
        __spVideoDriver->draw2DImage(
            __spGUIManager->ArrowTex_,
            dim::point2di(Rect.Left, Rect.Top), dim::point2di(Rect.Right, Rect.Top),
            dim::point2di(Rect.Right, Rect.Bottom), dim::point2di(Rect.Left, Rect.Bottom),
            dim::point2df(Mapping.Left, Mapping.Top), dim::point2df(Mapping.Right, Mapping.Top),
            dim::point2df(Mapping.Right, Mapping.Bottom), dim::point2df(Mapping.Left, Mapping.Bottom)
        );
    }
    else
    {
        __spVideoDriver->draw2DImage(
            __spGUIManager->ArrowTex_,
            dim::point2di(Rect.Left, Rect.Bottom), dim::point2di(Rect.Left, Rect.Top),
            dim::point2di(Rect.Right, Rect.Top), dim::point2di(Rect.Right, Rect.Bottom),
            dim::point2df(Mapping.Left, Mapping.Top), dim::point2df(Mapping.Right, Mapping.Top),
            dim::point2df(Mapping.Right, Mapping.Bottom), dim::point2df(Mapping.Left, Mapping.Bottom)
        );
    }
    
    drawFrame(Rect, 0, true);
}

void GUIScrollbarGadget::updateBarLocation()
{
    s32 FullSize, ButtonSize, Size;
    
    if (Flags_ & GUIFLAG_VERTICAL)
    {
        FullSize    = Rect_.Bottom - Rect_.Top;
        ButtonSize  = (Rect_.Right - Rect_.Left)*2;
        Size        = FullSize - ButtonSize;
    }
    else
    {
        FullSize    = Rect_.Right - Rect_.Left;
        ButtonSize  = (Rect_.Bottom - Rect_.Top)*2;
        Size        = FullSize - ButtonSize;
    }
    
    if (FullSize <= 0)
        return;
    
    const s32 Overage       = math::Max(0, Range_ - FullSize);
    const f32 Percentage    = 1.0f - (f32)Overage / Range_;
    
    BarLen_ = math::MinMax((s32)(Percentage * Size), 10, Size);
    
    MaxState_ = math::Max(0, Range_ - FullSize);
    PageSize_ = FullSize;
    
    if (MaxState_ > 0)
        BarPos_ = State_ * (Size - BarLen_) / MaxState_;
}

void GUIScrollbarGadget::updateDrag()
{
    s32 FullSize, ButtonSize, CursorPos;
    
    if (Flags_ & GUIFLAG_VERTICAL)
    {
        FullSize    = Rect_.Bottom - Rect_.Top;
        ButtonSize  = (Rect_.Right - Rect_.Left)*2;
        CursorPos   = __spGUIManager->CursorPos_.Y - Rect_.Top - CursorPosBias_.Y;
    }
    else
    {
        FullSize    = Rect_.Right - Rect_.Left;
        ButtonSize  = (Rect_.Bottom - Rect_.Top)*2;
        CursorPos   = __spGUIManager->CursorPos_.X - Rect_.Left - CursorPosBias_.X;
    }
    
    const s32 DragArea = FullSize - BarLen_ - ButtonSize;
    
    if (DragArea > 0)
        setState(CursorPos * MaxState_ / DragArea);
}

void GUIScrollbarGadget::updateState()
{
    updateBarLocation();
    setState(State_);
}

void GUIScrollbarGadget::getButtonRects(
    dim::rect2di &BarRect, dim::rect2di &ButtonARect, dim::rect2di &ButtonBRect,
    dim::rect2di &PageARect, dim::rect2di &PageBRect) const
{
    if (Flags_ & GUIFLAG_VERTICAL)
    {
        const s32 Size = (Rect_.Right - Rect_.Left);
        
        BarRect = dim::rect2di(
            Rect_.Left, Rect_.Top + Size + BarPos_,
            Rect_.Right, Rect_.Top + Size + BarPos_ + BarLen_
        );
        
        ButtonARect = dim::rect2di(Rect_.Left, Rect_.Top, Rect_.Right, Rect_.Top + Size);
        ButtonBRect = dim::rect2di(Rect_.Left, Rect_.Bottom - Size, Rect_.Right, Rect_.Bottom);
        
        PageARect = dim::rect2di(Rect_.Left, ButtonARect.Bottom, Rect_.Right, BarRect.Top);
        PageBRect = dim::rect2di(Rect_.Left, BarRect.Bottom, Rect_.Right, ButtonBRect.Top);
    }
    else
    {
        const s32 Size = (Rect_.Bottom - Rect_.Top);
        
        BarRect = dim::rect2di(
            Rect_.Left + Size + BarPos_, Rect_.Top,
            Rect_.Left + Size + BarPos_ + BarLen_, Rect_.Bottom
        );
        
        ButtonARect = dim::rect2di(Rect_.Left, Rect_.Top, Rect_.Left + Size, Rect_.Bottom);
        ButtonBRect = dim::rect2di(Rect_.Right - Size, Rect_.Top, Rect_.Right, Rect_.Bottom);
        
        PageARect = dim::rect2di(ButtonARect.Right, Rect_.Top, BarRect.Left, Rect_.Bottom);
        PageBRect = dim::rect2di(BarRect.Right, Rect_.Top, ButtonBRect.Left, Rect_.Bottom);
    }
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
