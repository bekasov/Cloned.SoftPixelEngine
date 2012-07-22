/*
 * GUI gadget file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUIGadget.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "RenderSystem/spRenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace gui
{


GUIGadget::GUIGadget(const EGUIGadgets Type)
    : GUIController(CONTROLLER_GADGET), GadType_(Type)
{
}
GUIGadget::~GUIGadget()
{
}


/*
 * ======= Protected: =======
 */

void GUIGadget::drawBgRect(
    const dim::rect2di &DrawRect, const dim::rect2di &MouseRect,
    bool isSwaped, bool isAlwaysUsing, s32 UsageType)
{
    const f32 Factor = ( isEnabled_ && Usable_ && ( mouseOver(MouseRect) || ( isAlwaysUsing && usage() ) ) ? 0.8f : 0.6f );
    video::color ColorA(Color_), ColorB(Color_ * Factor);
    
    if ( ( UsageType == -1 && usage() ) || usage(UsageType) )
        math::Swap(ColorA, ColorB);
    if (isSwaped)
        math::Swap(ColorA, ColorB);
    
    __spVideoDriver->draw2DRectangle(DrawRect, ColorA, ColorA, ColorB, ColorB);
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
