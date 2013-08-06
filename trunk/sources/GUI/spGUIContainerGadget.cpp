/*
 * GUI container gadget file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUIContainerGadget.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "RenderSystem/spRenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace gui
{


GUIContainerGadget::GUIContainerGadget() :
    GUIGadget       (GADGET_CONTAINER   ),
    OwnerDrawProc_  (0                  )
{
}
GUIContainerGadget::~GUIContainerGadget()
{
}

bool GUIContainerGadget::update()
{
    if (!checkDefaultUpdate())
        return false;
    
    if (isEnabled_ && mouseOver(Rect_) && !foreignUsage() && mouseLeft())
        useFocus();
    
    return true;
}

void GUIContainerGadget::draw()
{
    if (!isVisible_ || isValidated_ || !setupClipping())
        return;
    
    if (OwnerDrawProc_)
    {
        GlbRenderSys->endDrawing2D();
        
        OwnerDrawProc_(this);
        
        GlbRenderSys->beginDrawing2D();
    }
    else
    {
        GlbRenderSys->draw2DRectangle(Rect_, Color_);
        
        if (!(Flags_ & GUIFLAG_BORDERLESS))
            drawFrame(Rect_, 0, true);
    }
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
