/*
 * GUI button gadget file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUIButtonGadget.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "RenderSystem/spRenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace gui
{


GUIButtonGadget::GUIButtonGadget() : GUIGadget(GADGET_BUTTON), Image_(0)
{
    isToggled_ = false;
}
GUIButtonGadget::~GUIButtonGadget()
{
}

bool GUIButtonGadget::update()
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
            
            if (Flags_ & GUIFLAG_TOGGLE)
                isToggled_ = !isToggled_;
        }
    }
    
    return true;
}

void GUIButtonGadget::draw()
{
    if (!isVisible_ || isValidated_ || !setupClipping())
        return;
    
    drawBgRect(Rect_, Rect_, isToggled_);
    
    if (Image_)
    {
        GlbRenderSys->draw2DImage(
            Image_,
            dim::point2di(
                (Rect_.Left + Rect_.Right)/2 - Image_->getSize().Width/2,
                (Rect_.Top + Rect_.Bottom)/2 - Image_->getSize().Height/2
            )
        );
    }
    else
    {
        s32 Flags = DRAWTEXT_3D | DRAWTEXT_VCENTER;
        dim::point2di TextPos(0, (Rect_.Top + Rect_.Bottom)/2);
        
        if (Flags_ & GUIFLAG_TEXTRIGHT)
        {
            Flags |= DRAWTEXT_RIGHT;
            TextPos.X = Rect_.Right - 5;
        }
        else if (!(Flags_ & GUIFLAG_TEXTLEFT))
        {
            Flags |= DRAWTEXT_CENTER;
            TextPos.X = (Rect_.Left + Rect_.Right)/2;
        }
        else
            TextPos.X = Rect_.Left + 5;
        
        drawText(TextPos, Text_, 0, Flags);
    }
    
    if (!isEnabled_)
        drawHatchedFace(Rect_);
    
    drawFrame(Rect_, 0, true);
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
