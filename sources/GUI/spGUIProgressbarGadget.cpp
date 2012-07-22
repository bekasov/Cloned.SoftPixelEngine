/*
 * GUI progressbar gadget file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUIProgressbarGadget.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "RenderSystem/spRenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace gui
{


GUIProgressbarGadget::GUIProgressbarGadget() : GUIGadget(GADGET_PROGRESSBAR)
{
    State_ = 0;
    Range_ = 100;
}
GUIProgressbarGadget::~GUIProgressbarGadget()
{
}

bool GUIProgressbarGadget::update()
{
    if (!checkDefaultUpdate())
        return false;
    
    if (isEnabled_ && mouseOver(Rect_) && !foreignUsage() && mouseLeftDown())
        sendEvent(EVENT_GADGET, EVENT_ACTIVATE);
    
    return true;
}

void GUIProgressbarGadget::draw()
{
    if (!isVisible_ || isValidated_ || !setupClipping())
        return;
    
    const s32 BarWidth = (Rect_.Right - Rect_.Left) * State_ / Range_;
    
    const dim::rect2di RectLeft(Rect_.Left, Rect_.Top, Rect_.Left + BarWidth, Rect_.Bottom);
    const dim::rect2di RectRight(RectLeft.Right, Rect_.Top, Rect_.Right, Rect_.Bottom);
    
    __spVideoDriver->draw2DRectangle(RectRight, 255);
    
    __spVideoDriver->draw2DRectangle(
        RectLeft,
        video::color(80, 255, 80), video::color(80, 255, 80),
        video::color(0, 180, 0), video::color(0, 180, 0)
    );
    
    if (!(Flags_ & GUIFLAG_NOTEXT))
    {
        drawText(
            dim::point2di((Rect_.Right + Rect_.Left)/2, (Rect_.Top + Rect_.Bottom)/2 - 10),
            io::stringc(State_ * 100 / Range_) + "%", 0, DRAWTEXT_CENTER
        );
    }
    
    drawFrame(Rect_, 0, false);
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
