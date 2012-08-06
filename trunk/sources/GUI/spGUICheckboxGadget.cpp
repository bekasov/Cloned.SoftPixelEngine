/*
 * GUI checkbox gadget file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUICheckboxGadget.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "RenderSystem/spRenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace gui
{


GUICheckboxGadget::GUICheckboxGadget() :
    GUIGadget   (GADGET_CHECKBOX),
    isChecked_  (false          ),
    isAutoState_(false          )
{
}
GUICheckboxGadget::~GUICheckboxGadget()
{
}

bool GUICheckboxGadget::update()
{
    if (!checkDefaultUpdate())
        return false;
    
    if (isEnabled_ && mouseOver(Rect_) && !foreignUsage())
    {
        if (mouseLeftDown())
            useFocus();
        else if (mouseLeftUp())
        {
            sendEvent(EVENT_GADGET, EVENT_ACTIVATE);
            setState(!getState());
        }
    }
    
    return true;
}

void GUICheckboxGadget::draw()
{
    if (!isVisible_ || isValidated_ || !setupClipping())
        return;
    
    const s32 BoxSize = Rect_.Bottom - Rect_.Top;
    const dim::rect2di Rect(
        Rect_.Left, (Rect_.Top + Rect_.Bottom)/2 - BoxSize/2,
        Rect_.Left + BoxSize, (Rect_.Top + Rect_.Bottom)/2 + BoxSize/2
    );
    
    drawBgRect(Rect, Rect_);
    
    if (isChecked_)
    {
        const dim::point2di BoxCenter(Rect.Left + BoxSize/2, Rect.Top + BoxSize/2);
        
        __spVideoDriver->draw2DLine(
            dim::point2di(BoxCenter.X - 5, BoxCenter.Y), dim::point2di(BoxCenter.X - 1, BoxCenter.Y + 4), 0
        );
        __spVideoDriver->draw2DLine(
            dim::point2di(BoxCenter.X - 2, BoxCenter.Y + 4), dim::point2di(BoxCenter.X + 4, BoxCenter.Y - 2), 0
        );
    }
    
    drawFrame(Rect, 0, true);
    
    drawText(
        dim::point2di(Rect_.Left + BoxSize + 5, (Rect_.Top + Rect_.Bottom)/2 - 10), Text_, 0
    );
    
    if (!isEnabled_)
        drawHatchedFace(Rect_);
}

void GUICheckboxGadget::setState(bool isChecked)
{
    isChecked_ = isChecked;
    updateAutoState();
}
void GUICheckboxGadget::setAutoState(bool isAutoState)
{
    isAutoState_ = isAutoState;
    updateAutoState();
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
