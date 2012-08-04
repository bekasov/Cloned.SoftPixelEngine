/*
 * GUI basic object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUIBaseObject.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "GUI/spGUIManager.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "Base/spInternalDeclarations.hpp"


namespace sp
{

extern gui::GUIManager* __spGUIManager;
extern video::RenderSystem* __spVideoDriver;

namespace gui
{


extern video::Font* __spGUIFont;

GUIBaseObject::GUIBaseObject() :
    ID_         (0          ),
    Font_       (__spGUIFont),
    isEnabled_  (true       )
{
}
GUIBaseObject::~GUIBaseObject()
{
}

void GUIBaseObject::setFont(video::Font* TextFont)
{
    Font_ = (TextFont ? TextFont : __spGUIFont);
}


/*
 * ======= Protected: =======
 */

void GUIBaseObject::drawFrame(
    const dim::rect2di &Rect, const video::color &Color, bool isFrame3D)
{
    drawFrame(Rect, Color, Color, Color, Color);
    
    if (isFrame3D)
    {
        drawFrame(
            dim::rect2di(Rect.Left + 1, Rect.Top + 1, Rect.Right - 1, Rect.Bottom - 1),
            255, 90, 90, 255
        );
    }
}
void GUIBaseObject::drawFrame(
    const dim::rect2di &Rect,
    const video::color &ColorA, const video::color &ColorB,
    const video::color &ColorC, const video::color &ColorD)
{
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect.Left, Rect.Top), dim::point2di(Rect.Right, Rect.Top), ColorA
    );
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect.Right, Rect.Top + 1), dim::point2di(Rect.Right, Rect.Bottom), ColorB
    );
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect.Left, Rect.Bottom - 1), dim::point2di(Rect.Right, Rect.Bottom - 1), ColorC
    );
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect.Left + 1, Rect.Top + 1), dim::point2di(Rect.Left + 1, Rect.Bottom), ColorD
    );
}

void GUIBaseObject::drawText(dim::point2di Pos, const io::stringc &Text, const video::color &Color, s32 Flags)
{
    if (Flags & DRAWTEXT_CENTER)
        Pos.X -= Font_->getStringWidth(Text) / 2;
    else if (Flags & DRAWTEXT_RIGHT)
        Pos.X -= Font_->getStringWidth(Text);
    
    if (Flags & DRAWTEXT_VCENTER)
        Pos.Y -= 10;
    
    if (Flags & DRAWTEXT_3D)
    {
        video::color BkColor(video::color(255) - Color);
        BkColor.Alpha = 255;
        __spVideoDriver->draw2DText(Font_, Pos + 1, Text, BkColor);
    }
    
    __spVideoDriver->draw2DText(Font_, Pos, Text, Color);
}

void GUIBaseObject::drawButton(const dim::rect2di &Rect, const video::color &Color, bool isMouseOver)
{
    if (isMouseOver)
        __spVideoDriver->draw2DRectangle(Rect, Color);
    else
    {
        __spVideoDriver->draw2DRectangle(
            Rect, Color * 0.75f, Color / 2, Color * 0.75f, Color
        );
    }
    
    drawFrame(Rect, 0, true);
}

void GUIBaseObject::drawHatchedFace(const dim::rect2di &Rect)
{
    dim::rect2di Viewarea(Rect);
    dim::size2df Clip(
        static_cast<f32>(Rect.Right - Rect.Left),
        static_cast<f32>(Rect.Bottom - Rect.Top)
    );
    
    Clip *= 0.5f;
    
    Viewarea.Right  -= Viewarea.Left;
    Viewarea.Bottom -= Viewarea.Top;
    
    __spVideoDriver->draw2DImage(__spGUIManager->HatchedFace_, Viewarea, dim::rect2df(0, 0, Clip.Width, Clip.Height));
}

bool GUIBaseObject::mouseOver(const dim::rect2di &Rect) const
{
    return Rect.isPointCollided(__spGUIManager->CursorPos_);
}
bool GUIBaseObject::mouseLeft() const
{
    return __hitMouseKey[io::MOUSE_LEFT];
}
bool GUIBaseObject::mouseRight() const
{
    return __hitMouseKey[io::MOUSE_RIGHT];
}
bool GUIBaseObject::mouseLeftUp() const
{
    return __wasMouseKey[io::MOUSE_LEFT];
}
bool GUIBaseObject::mouseRightUp() const
{
    return __wasMouseKey[io::MOUSE_RIGHT];
}
bool GUIBaseObject::mouseLeftDown() const
{
    return __isMouseKey[io::MOUSE_LEFT];
}
bool GUIBaseObject::mouseRightDown() const
{
    return __isMouseKey[io::MOUSE_RIGHT];
}

void GUIBaseObject::sendEvent(const EGUIEventObjects ObjectType, const EGUIEventTypes EventType)
{
    SGUIEvent Event;
    {
        Event.Object    = ObjectType;
        Event.Type      = EventType;
        
        switch (ObjectType)
        {
            case EVENT_WINDOW:
                Event.Window = (GUIWindow*)this; break;
            case EVENT_GADGET:
                Event.Gadget = (GUIGadget*)this; break;
            case EVENT_MENUITEM:
                Event.MenuItem = (GUIMenuItem*)this; break;
        }
    }
    sendEvent(Event);
}
void GUIBaseObject::sendEvent(const SGUIEvent &Event)
{
    if (__spGUIManager->EventCallback_)
        __spGUIManager->EventCallback_(Event);
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
