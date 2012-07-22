/*
 * GUI window file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIWindow.hpp"
#include "GUI/spGUIManager.hpp"
#include "Base/spSharedObjects.hpp"
#include "RenderSystem/spRenderSystem.hpp"


namespace sp
{

extern gui::GUIManager* __spGUIManager;
extern video::RenderSystem* __spVideoDriver;

namespace gui
{


extern video::Font* __spGUIFont;

video::color GUIWindow::CaptionColorA[2] = { video::color(64, 64, 255), video::color(160, 190, 255) };
video::color GUIWindow::CaptionColorB[2] = { video::color(128), video::color(255) };

GUIWindow::GUIWindow()
    : GUIController(CONTROLLER_WINDOW), MenuRoot_(0), HorzScroll_(0), VertScroll_(0)
{
    init();
}
GUIWindow::~GUIWindow()
{
    clear();
}

bool GUIWindow::update()
{
    updateScrollBars(HorzScroll_, VertScroll_);
    updateMenu();
    
    if (!checkDefaultUpdate())
        return false;
    
    /* Temporary position members */
    s32 CurPosHorz = Rect_.Right;
    
    if (Flags_ & GUIFLAG_CLOSEBUTTON) CurPosHorz -= 22;
    const s32 PosHorzBtnClose = CurPosHorz;
    
    if (Flags_ & GUIFLAG_MAXBUTTON) CurPosHorz -= 22;
    const s32 PosHorzBtnMax = CurPosHorz;
    
    if (Flags_ & GUIFLAG_MINBUTTON) CurPosHorz -= 22;
    const s32 PosHorzBtnMin = CurPosHorz;
    
    if (mouseLeft())
    {
        /* Put window to foreground */
        foreground();
        
        /* Check which part has been clicked */
        if ((Flags_ & GUIFLAG_MAXBUTTON) &&
            mouseOver(dim::rect2di(PosHorzBtnMax - 1, Rect_.Top + 3, PosHorzBtnMax + 19, Rect_.Top + 23)))
        {
            useFocus(USAGE_MAXBUTTON);
        }
        else if ((Flags_ & GUIFLAG_MINBUTTON) &&
            mouseOver(dim::rect2di(PosHorzBtnMin - 1, Rect_.Top + 3, PosHorzBtnMin + 19, Rect_.Top + 23)))
        {
            useFocus(USAGE_MINBUTTON);
        }
        else if ((Flags_ & GUIFLAG_SIZEBUTTON) && VisState_ != VISSTATE_MAXIMIZED &&
            mouseOver(dim::rect2di(Rect_.Right - 20, Rect_.Bottom - 20, Rect_.Right, Rect_.Bottom)))
        {
            useFocus(USAGE_RESIZE);
        }
        else if (VisState_ != VISSTATE_MAXIMIZED && !(Flags_ & GUIFLAG_BORDERLESS) &&
                 mouseOver(dim::rect2di(Rect_.Left, Rect_.Top, Rect_.Right, Rect_.Top + 25)))
        {
            useFocus(USAGE_MOVE);
        }
    }
    else if (mouseLeftUp())
    {
        /* Check which part was clicked */
        if (usage(USAGE_MAXBUTTON) &&
            mouseOver(dim::rect2di(PosHorzBtnMax - 1, Rect_.Top + 3, PosHorzBtnMax + 19, Rect_.Top + 23)))
        {
            if (VisState_ != VISSTATE_MAXIMIZED)
                maximize();
            else
                normalize();
        }
        else if (usage(USAGE_MINBUTTON) &&
            mouseOver(dim::rect2di(PosHorzBtnMin - 1, Rect_.Top + 3, PosHorzBtnMin + 19, Rect_.Top + 23)))
        {
            if (VisState_ != VISSTATE_MINIMIZED)
                minimize();
            else
                normalize();
        }
    }
    
    if (usage(USAGE_RESIZE))
        transform(dim::size2di(__spGUIManager->CursorSpeed_.X, __spGUIManager->CursorSpeed_.Y));
    else if (usage(USAGE_MOVE))
    {
        translate(__spGUIManager->CursorSpeed_);
        updateMenu();
        
        if (mouseLeftUp())
            clampWindowLocation();
    }
    
    if (isEnabled_)
        updateChildren();
    
    return true;
}

void GUIWindow::draw()
{
    if (!isVisible_ || isValidated_ || !setupClipping())
        return;
    
    drawWindowBackground();
    drawWindowFrame();
    
    s32 PosHorz = 0;
    
    if (!(Flags_ & GUIFLAG_BORDERLESS))
    {
        if (Flags_ & GUIFLAG_CLOSEBUTTON)
            drawWindowButtonClose(PosHorz);
        if (Flags_ & GUIFLAG_MAXBUTTON)
            drawWindowButtonMax(PosHorz);
        if (Flags_ & GUIFLAG_MINBUTTON)
            drawWindowButtonMin(PosHorz);
    }
    
    drawChildren();
    
    /* Draw menu items */
    if (MenuRoot_)
    {
        __spVideoDriver->setClipping(false, 0, 0);
        MenuRoot_->draw();
    }
    
    //isValidated_ = true;
}

dim::rect2di GUIWindow::getLocalViewArea(const GUIController* Obj) const
{
    dim::rect2di Rect(Rect_);
    
    if (Flags_ & GUIFLAG_BORDERLESS)
        Rect += dim::rect2di(2, 2, -2, -2);
    else
        Rect += dim::rect2di(2, 26, -2, -2);
    
    if (Obj != HorzScroll_ && Obj != VertScroll_)
    {
        if (HorzScroll_ && HorzScroll_->getVisible())
            Rect.Bottom -= SCROLLBAR_SIZE;
        if (VertScroll_ && VertScroll_->getVisible())
            Rect.Right -= SCROLLBAR_SIZE;
    }
    
    return Rect; 
}
dim::point2di GUIWindow::getViewOrigin() const
{
    dim::point2di Origin(2, (Flags_ & GUIFLAG_BORDERLESS) ? 2 : 26);
    
    if (Parent_)
        Origin += Parent_->getViewOrigin();
    
    return Origin;
}


void GUIWindow::setSize(const dim::size2di &Size)
{
    s32 Width   = Size.Width;
    s32 Height  = Size.Height;
    
    const dim::size2di ParentViewAreaSize(getParentViewArea().getSize());
    const s32 MaxWidth  = math::Min(MaxSize_.Width, ParentViewAreaSize.Width);
    const s32 MaxHeight = math::Min(MaxSize_.Height, ParentViewAreaSize.Height);
    
    if (Width > MaxWidth) Width = MaxWidth;
    if (Height > MaxHeight) Height = MaxHeight;
    
    if (Width < MinSize_.Width) Width = MinSize_.Width;
    if (Height < MinSize_.Height) Height = MinSize_.Height;
    
    if (Width < 0) Width = 0;
    if (Height < 0) Height = 0;
    
    RootRect_.Right     = RootRect_.Left    + Width;
    RootRect_.Bottom    = RootRect_.Top     + Height;
    
    updateRect();
    updateClipping();
    
    sendEvent(EVENT_WINDOW, EVENT_RESIZE);
}

void GUIWindow::close()
{
    // !TODO!
}
void GUIWindow::maximize()
{
    if (VisState_ == VISSTATE_NORMAL)
    {
        VisNormalPos_   = getPosition();
        VisNormalSize_  = getSize();
        
        setPosition(0);
        setSize(dim::size2di(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight));
        
        VisState_ = VISSTATE_MAXIMIZED;
    }
}
void GUIWindow::minimize()
{
    if (VisState_ == VISSTATE_NORMAL)
    {
        VisNormalPos_   = getPosition();
        VisNormalSize_  = getSize();
        
        setSize(0);
        
        VisState_ = VISSTATE_MINIMIZED;
    }
}
void GUIWindow::normalize()
{
    if (VisState_ != VISSTATE_NORMAL)
    {
        setPosition(VisNormalPos_);
        setSize(VisNormalSize_);
        
        VisState_ = VISSTATE_NORMAL;
    }
}

void GUIWindow::setMenuRoot(GUIMenuItem* MenuRoot)
{
    MenuRoot_ = MenuRoot;
    
    if (MenuRoot_)
    {
        const dim::rect2di Viewarea(getViewArea());
        MenuRoot_->update(dim::point2di(Viewarea.Left, Viewarea.Top));
    }
}

GUIButtonGadget* GUIWindow::addButtonGadget(
    const dim::point2di &Position, const dim::size2di &Size, const io::stringc &Text, s32 Flags)
{
    GUIButtonGadget* NewGadget = new GUIButtonGadget();
    addController(NewGadget, Position, Size, Text, Flags);
    return NewGadget;
}
GUICheckboxGadget* GUIWindow::addCheckboxGadget(
    const dim::point2di &Position, const dim::size2di &Size, const io::stringc &Text, s32 Flags)
{
    GUICheckboxGadget* NewGadget = new GUICheckboxGadget();
    addController(NewGadget, Position, Size, Text, Flags);
    return NewGadget;
}
GUIContainerGadget* GUIWindow::addContainerGadget(
    const dim::point2di &Position, const dim::size2di &Size, s32 Flags)
{
    GUIContainerGadget* NewGadget = new GUIContainerGadget();
    addController(NewGadget, Position, Size, "", Flags);
    return NewGadget;
}
GUIListGadget* GUIWindow::addListGadget(
    const dim::point2di &Position, const dim::size2di &Size, s32 Flags)
{
    GUIListGadget* NewGadget = new GUIListGadget();
    addController(NewGadget, Position, Size, "", Flags);
    return NewGadget;
}
GUIProgressbarGadget* GUIWindow::addProgressbarGadget(
    const dim::point2di &Position, const dim::size2di &Size, s32 Range, s32 Flags)
{
    GUIProgressbarGadget* NewGadget = new GUIProgressbarGadget();
    addController(NewGadget, Position, Size, "", Flags);
    NewGadget->setRange(Range);
    return NewGadget;
}
GUIScrollbarGadget* GUIWindow::addScrollbarGadget(
    const dim::point2di &Position, const dim::size2di &Size, s32 Range, s32 Flags)
{
    GUIScrollbarGadget* NewGadget = new GUIScrollbarGadget();
    addController(NewGadget, Position, Size, "", Flags);
    NewGadget->setRange(Range);
    return NewGadget;
}
GUIStringGadget* GUIWindow::addStringGadget(
    const dim::point2di &Position, const dim::size2di &Size, const io::stringc &Text, s32 Flags)
{
    GUIStringGadget* NewGadget = new GUIStringGadget();
    addController(NewGadget, Position, Size, Text, Flags);
    return NewGadget;
}
GUITextGadget* GUIWindow::addTextGadget(
    const dim::point2di &Position, const dim::size2di &Size, const io::stringc &Text, s32 Flags)
{
    GUITextGadget* NewGadget = new GUITextGadget();
    addController(NewGadget, Position, Size, Text, Flags);
    return NewGadget;
}
GUITrackbarGadget* GUIWindow::addTrackbarGadget(
    const dim::point2di &Position, const dim::size2di &Size, s32 MinRange, s32 MaxRange, s32 Flags)
{
    GUITrackbarGadget* NewGadget = new GUITrackbarGadget();
    addController(NewGadget, Position, Size, "", Flags);
    NewGadget->setRange(MinRange, MaxRange);
    return NewGadget;
}
GUITreeGadget* GUIWindow::addTreeGadget(
    const dim::point2di &Position, const dim::size2di &Size, s32 Flags)
{
    GUITreeGadget* NewGadget = new GUITreeGadget();
    addController(NewGadget, Position, Size, "", Flags);
    return NewGadget;
}
GUIWebGadget* GUIWindow::addWebGadget(
    const dim::point2di &Position, const dim::size2di &Size, s32 Flags)
{
    GUIWebGadget* NewGadget = new GUIWebGadget();
    addController(NewGadget, Position, Size, "", Flags);
    return NewGadget;
}


/*
 * ======= Private: =======
 */

void GUIWindow::init()
{
    MinSize_    = dim::size2di(100, 45);
    VisState_   = VISSTATE_NORMAL;
}
void GUIWindow::clear()
{
    MemoryManager::deleteMemory(MenuRoot_);
    MemoryManager::deleteMemory(HorzScroll_);
    MemoryManager::deleteMemory(VertScroll_);
}

void GUIWindow::drawWindowBackground()
{
    const dim::rect2di Rect = (
        Flags_ & GUIFLAG_BORDERLESS ? Rect_ : dim::rect2di(Rect_.Left, Rect_.Top + CAPTION_HEIGHT, Rect_.Right, Rect_.Bottom)
    );
    
    __spVideoDriver->draw2DRectangle(
        Rect, Color_ * 0.75f, Color_ / 2, Color_ * 0.75f, Color_
    );
    
    if (!(Flags_ & GUIFLAG_BORDERLESS))
    {
        video::color* CaptionColor = (isForeground_ ? CaptionColorA : CaptionColorB);
        
        __spVideoDriver->draw2DRectangle(
            dim::rect2di(Rect_.Left, Rect_.Top, Rect_.Right, Rect_.Top + CAPTION_HEIGHT),
            CaptionColor[0], CaptionColor[1], CaptionColor[1], CaptionColor[0]
        );
        
        if (Flags_ & GUIFLAG_SIZEBUTTON)
        {
            /* Draw resize button */
            for (s32 i = 0; i < 4; ++i)
            {
                __spVideoDriver->draw2DLine(
                    dim::point2di(Rect_.Right - 7 - i*3, Rect_.Bottom - 2),
                    dim::point2di(Rect_.Right - 2, Rect_.Bottom - 7 - i*3),
                    0
                );
            }
        }
    }
}

void GUIWindow::drawWindowFrame()
{
    /* Draw bounding box frame */
    drawFrame(Rect_, video::color(0), true);
    
    if (!(Flags_ & GUIFLAG_BORDERLESS))
    {
        /* Draw caption frame */
        __spVideoDriver->draw2DLine(
            dim::point2di(Rect_.Left, Rect_.Top + CAPTION_HEIGHT),
            dim::point2di(Rect_.Right, Rect_.Top + CAPTION_HEIGHT),
            0
        );
        
        /* Draw window title */
        io::stringc Title   = Text_;
        s32 MaxWidth        = Rect_.Right - Rect_.Left - 16;
        
        if (Flags_ & GUIFLAG_CLOSEBUTTON)
            MaxWidth -= 22;
        if (Flags_ & GUIFLAG_MAXBUTTON)
            MaxWidth -= 22;
        if (Flags_ & GUIFLAG_MINBUTTON)
            MaxWidth -= 22;
        
        if (__spGUIFont->getStringWidth(Title + "...") > MaxWidth)
        {
            while (__spGUIFont->getStringWidth(Title + "...") > MaxWidth && Title.size())
                Title = Title.left(Title.size() - 1);
            Title += "...";
        }
        
        drawText(dim::point2di(Rect_.Left + 8, Rect_.Top + 3), Title);
    }
}

void GUIWindow::drawWindowButtonMin(s32 &PosHorz)
{
    drawWindowButton(PosHorz, isForeground_ ? video::color(230, 230, 255) : 255);
    
    if (VisState_ == VISSTATE_MINIMIZED)
        drawWindowButtonFrameNormalize(PosHorz);
    else
    {
        __spVideoDriver->draw2DRectangle(
            dim::rect2di(Rect_.Right + 2 + PosHorz, Rect_.Top + 18, Rect_.Right + 10 + PosHorz, Rect_.Top + 20), 0
        );
    }
}

void GUIWindow::drawWindowButtonMax(s32 &PosHorz)
{
    drawWindowButton(PosHorz, isForeground_ ? video::color(230, 230, 255) : 255);
    
    if (VisState_ == VISSTATE_MAXIMIZED)
        drawWindowButtonFrameNormalize(PosHorz);
    else
    {
        drawFrame(
            dim::rect2di(Rect_.Right + 2 + PosHorz, Rect_.Top + 6, Rect_.Right + 16 + PosHorz, Rect_.Top + 20), 0, false
        );
        __spVideoDriver->draw2DLine(
            dim::point2di(Rect_.Right + 3 + PosHorz, Rect_.Top + 7), dim::point2di(Rect_.Right + 15 + PosHorz, Rect_.Top + 7), 0
        );
    }
}

void GUIWindow::drawWindowButtonFrameNormalize(s32 &PosHorz)
{
    drawFrame(
        dim::rect2di(Rect_.Right + 2 + PosHorz, Rect_.Top + 9, Rect_.Right + 13 + PosHorz, Rect_.Top + 20), 0, false
    );
    drawFrame(
        dim::rect2di(Rect_.Right + 5 + PosHorz, Rect_.Top + 6, Rect_.Right + 16 + PosHorz, Rect_.Top + 17), 0, false
    );
}

void GUIWindow::drawWindowButtonClose(s32 &PosHorz)
{
    drawWindowButton(PosHorz, isForeground_ ? video::color(255, 0, 0) : 255);
    
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect_.Right + 3 + PosHorz, Rect_.Top + 19), dim::point2di(Rect_.Right + 15 + PosHorz, Rect_.Top + 7), 0
    );
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect_.Right + 4 + PosHorz, Rect_.Top + 19), dim::point2di(Rect_.Right + 15 + PosHorz, Rect_.Top + 8), 0
    );
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect_.Right + 3 + PosHorz, Rect_.Top + 18), dim::point2di(Rect_.Right + 14 + PosHorz, Rect_.Top + 7), 0
    );
    
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect_.Right + 3 + PosHorz, Rect_.Top + 7), dim::point2di(Rect_.Right + 15 + PosHorz, Rect_.Top + 19), 0
    );
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect_.Right + 4 + PosHorz, Rect_.Top + 7), dim::point2di(Rect_.Right + 15 + PosHorz, Rect_.Top + 18), 0
    );
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect_.Right + 3 + PosHorz, Rect_.Top + 8), dim::point2di(Rect_.Right + 14 + PosHorz, Rect_.Top + 19), 0
    );
}
bool GUIWindow::drawWindowButton(s32 &PosHorz, const video::color &Color)
{
    PosHorz -= 22;
    
    const dim::rect2di Rect(Rect_.Right - 1 + PosHorz, Rect_.Top + 3, Rect_.Right + 19 + PosHorz, Rect_.Top + 23);
    const bool isMouseOver = (Usable_ && mouseOver(Rect));
    
    drawButton(Rect, Color, isMouseOver);
    
    return isMouseOver;
}

void GUIWindow::checkFlags()
{
    /* Update basic flags */
    if (Flags_ & GUIFLAG_BORDERLESS)
    {
        Flags_ ^= GUIFLAG_CLOSEBUTTON;
        Flags_ ^= GUIFLAG_MINBUTTON;
        Flags_ ^= GUIFLAG_MAXBUTTON;
    }
    
    /* Update scrollbar flags */
    const dim::size2di ViewSize(
        (getLocalViewArea() - dim::rect2di(Rect_.Left, Rect_.Top, Rect_.Left, Rect_.Top)).getSize()
    );
    
    if (Flags_ & GUIFLAG_HSCROLL)
    {
        if (!HorzScroll_)
        {
            HorzScroll_ = new GUIScrollbarGadget();
            HorzScroll_->setFlags(GUIFLAG_NOSCROLL);
            HorzScroll_->setParent(this);
            HorzScroll_->setRange(ViewSize.Width - SCROLLBAR_SIZE);
        }
    }
    else
    {
        removeChild(HorzScroll_);
        MemoryManager::deleteMemory(HorzScroll_);
    }
    
    if (Flags_ & GUIFLAG_VSCROLL)
    {
        if (!VertScroll_)
        {
            VertScroll_ = new GUIScrollbarGadget();
            VertScroll_->setFlags(GUIFLAG_NOSCROLL | GUIFLAG_VERTICAL);
            VertScroll_->setParent(this);
            VertScroll_->setRange(ViewSize.Height - SCROLLBAR_SIZE);
        }
    }
    else
    {
        removeChild(VertScroll_);
        MemoryManager::deleteMemory(VertScroll_);
    }
    
    updateScrollBars(HorzScroll_, VertScroll_);
}

void GUIWindow::updateMenu()
{
    /* Update menu items */
    if (MenuRoot_)
    {
        const dim::rect2di Viewarea(getViewArea());
        MenuRoot_->update(dim::point2di(Viewarea.Left, Viewarea.Top));
    }
}

void GUIWindow::clampWindowLocation()
{
    const dim::rect2di Rect(getParentViewArea());
    dim::point2di Pos(getPosition());
    
    math::Clamp(Pos.Y, Rect.Top, Rect.Bottom - CAPTION_HEIGHT);
    
    setPosition(Pos);
}

void GUIWindow::addController(
    GUIController* NewController, const dim::point2di &Position,
    const dim::size2di &Size, const io::stringc &Text, s32 Flags)
{
    NewController->setPosition(Position + GadgetOrigin_);
    NewController->setSize(Size);
    NewController->setFlags(Flags);
    NewController->setText(Text);
    NewController->setParent(this);
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
