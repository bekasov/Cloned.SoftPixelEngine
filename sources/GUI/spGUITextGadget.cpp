/*
 * GUI text gadget file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUITextGadget.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "RenderSystem/spRenderSystem.hpp"
#include "GUI/spGUIManager.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern gui::GUIManager* __spGUIManager;

namespace gui
{


GUITextGadget::GUITextGadget() : GUIGadget(GADGET_TEXT)
{
}
GUITextGadget::~GUITextGadget()
{
}

bool GUITextGadget::update()
{
    if (!checkDefaultUpdate())
        return false;
    
    if (isEnabled_ && mouseOver(Rect_) && !foreignUsage())
    {
        if (mouseLeftDown())
            useFocus();
        else if (mouseLeftUp())
            sendEvent(EVENT_GADGET, EVENT_ACTIVATE);
    }
    
    return true;
}

void GUITextGadget::draw()
{
    if (!isVisible_ || isValidated_ || !setupClipping())
        return;
    
    s32 Flags = DRAWTEXT_3D | DRAWTEXT_VCENTER;
    dim::point2di TextPos(0, (Rect_.Top + Rect_.Bottom)/2);
    
    if (Flags_ & GUIFLAG_TEXTRIGHT)
    {
        Flags |= DRAWTEXT_RIGHT;
        TextPos.X = Rect_.Right - 5;
    }
    else if (Flags_ & GUIFLAG_TEXTCENTER)
    {
        Flags |= DRAWTEXT_CENTER;
        TextPos.X = (Rect_.Left + Rect_.Right)/2;
    }
    else
        TextPos.X = Rect_.Left + 5;
    
    drawText(TextPos, Text_, (isEnabled_ ? 0 : 100), Flags);
}


} // /namespace gui

} // /namespace sp


#endif



// ================================================================================
