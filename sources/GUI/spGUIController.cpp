/*
 * GUI controller file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUIController.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "GUI/spGUIManager.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Base/spSharedObjects.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern gui::GUIManager* __spGUIManager;
extern video::RenderSystem* __spVideoDriver;

namespace gui
{


/*
 * Internal members
 */

bool cmpGUIController(GUIController* &obj1, GUIController* &obj2)
{
    return obj1->getOrder() > obj2->getOrder();
}


/*
 * GUIController class
 */

GUIController::GUIController(const EGUIControllerTypes Type) :
    GUIBaseObject   (                                                       ),
    Type_           (Type                                                   ),
    Parent_         (0                                                      ),
    Order_          (-1                                                     ),
    MaxSize_        (gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight),
    isVisible_      (true                                                   ),
    isValidated_    (false                                                  ),
    isForeground_   (false                                                  ),
    Flags_          (0                                                      ),
    FocusUsage_     (-1                                                     ),
    Usable_         (false                                                  )
{
    foreground();
}
GUIController::~GUIController()
{
    foreach (GUIController* Child, Children_)
        __spGUIManager->removeController(Child, false);
}

void GUIController::updateAlways()
{
}

void GUIController::setRect(const dim::rect2di &Rect)
{
    setPosition(dim::point2di(Rect.Left, Rect.Top));
    setSize(Rect.getSize());
    
    updateRect();
    updateClipping();
}

void GUIController::setPosition(const dim::point2di &Position)
{
    s32 PosX = Position.X;
    s32 PosY = Position.Y;
    
    const dim::rect2di ParentViewArea(getParentViewArea());
    
    const s32 Horz = PosX - RootRect_.Left;
    const s32 Vert = PosY - RootRect_.Top;
    
    RootRect_.Left      += Horz;
    RootRect_.Top       += Vert;
    RootRect_.Right     += Horz;
    RootRect_.Bottom    += Vert;
    
    updateRect();
    updateClipping();
}

dim::rect2di GUIController::getLocalViewArea(const GUIController* Obj) const
{
    return Rect_;
}
dim::rect2di GUIController::getViewArea(const GUIController* Obj) const
{
    dim::rect2di Rect(getLocalViewArea(Obj));
    
    if (Parent_)
    {
        const dim::rect2di ParentRect(Parent_->getViewArea(this));
        
        math::clamp(Rect.Left   , ParentRect.Left, ParentRect.Right);
        math::clamp(Rect.Right  , ParentRect.Left, ParentRect.Right);
        
        math::clamp(Rect.Top    , ParentRect.Top, ParentRect.Bottom);
        math::clamp(Rect.Bottom , ParentRect.Top, ParentRect.Bottom);
    }
    
    return Rect;
}
dim::point2di GUIController::getViewOrigin() const
{
    dim::point2di Origin;
    
    if (Parent_)
        Origin += Parent_->getViewOrigin();
    
    return Origin;
}

dim::point2di GUIController::getScrollPosition(bool isGlobal) const
{
    return (Parent_ && isGlobal) ? ScrollPos_ + Parent_->getScrollPosition(true) : ScrollPos_;
}

void GUIController::setSize(const dim::size2di &Size)
{
    s32 Width   = Size.Width;
    s32 Height  = Size.Height;
    
    if (Width < 0) Width = 0;
    if (Height < 0) Height = 0;
    
    RootRect_.Right     = RootRect_.Left    + Width;
    RootRect_.Bottom    = RootRect_.Top     + Height;
    
    updateRect();
    updateClipping();
}

dim::point2di GUIController::getPosition(bool isGlobal) const
{
    if (isGlobal && Parent_)
        return dim::point2di(RootRect_.Left, RootRect_.Top) + Parent_->getPosition(true);
    return dim::point2di(RootRect_.Left, RootRect_.Top);
}

dim::rect2di GUIController::getRect(bool isGlobal) const
{
    if (isGlobal && Parent_)
        return RootRect_ + Parent_->getRect(true);
    return RootRect_;
}

void GUIController::setParent(GUIController* Parent)
{
    if (Parent)
    {
        /* Check if this object is already inside the parent tree */
        GUIController* CurCtrl = Parent, * LastCtrl = 0;
        
        do
        {
            if (CurCtrl == this)
                return;
            LastCtrl = CurCtrl;
        }
        while (CurCtrl = LastCtrl->getParent());
        
        Parent->addChild(this);
    }
    else if (Parent_)
        Parent_->removeChild(this);
    
    if (Parent)
        __spGUIManager->removeParentController(this);
    else
        __spGUIManager->ParentControllerList_.push_back(this);
    
    Parent_ = Parent;
}

void GUIController::focus()
{
    __spGUIManager->FocusedController_ = this;
}
bool GUIController::hasFocus() const
{
    return __spGUIManager->FocusedController_ == this;
}

void GUIController::foreground()
{
    /* Get the correct GUI controller list */
    std::list<GUIController*>* ControllerList = 0;
    
    if (Parent_)
        ControllerList = &Parent_->Children_;
    else
        ControllerList = &__spGUIManager->ParentControllerList_;
    
    /* Change the order of each GUI controller */
    Order_ = 0;
    
    foreach (GUIController* Obj, *ControllerList)
    {
        if (Obj != this)
            ++Obj->Order_;
    }
    
    /* Sort the GUI controller list */
    ControllerList->sort(cmpGUIController);
    
    /* Fit the order numbers */
    s32 i = 0;
    foreach_reverse (GUIController* Obj, *ControllerList)
        Obj->Order_ = i++;
}

void GUIController::setFlags(s32 Flags)
{
    Flags_ = Flags;
    checkFlags();
}


/*
 * ======= Protected: =======
 */

void GUIController::updateChildren()
{
    for (std::list<GUIController*>::reverse_iterator it = Children_.rbegin(); it != Children_.rend(); ++it)
    {
        if ((*it)->update())
            break;
    }
}
void GUIController::drawChildren()
{
    for (std::list<GUIController*>::iterator it = Children_.begin(); it != Children_.end(); ++it)
        (*it)->draw();
}

void GUIController::updateClipping()
{
    Usable_ = false;
    
    /* Update clipping area */
    dim::rect2di Rect(Rect_);
    
    if (Parent_)
    {
        const dim::rect2di ParentRect(Parent_->getViewArea(this));
        
        math::clamp(Rect.Left   , ParentRect.Left, ParentRect.Right);
        math::clamp(Rect.Right  , ParentRect.Left, ParentRect.Right);
        
        math::clamp(Rect.Top    , ParentRect.Top, ParentRect.Bottom);
        math::clamp(Rect.Bottom , ParentRect.Top, ParentRect.Bottom);
    }
    
    VisRect_ = Rect;
    
    /* Update clipping for all children */
    u32 i = 0;
    for (std::list<GUIController*>::reverse_iterator it = Children_.rbegin(); it != Children_.rend(); ++it, ++i)
    {
        (*it)->isForeground_ = (i == 0) && isForeground_;
        (*it)->updateRect();
        (*it)->updateClipping();
    }
}

bool GUIController::setupClipping()
{
    __spVideoDriver->setClipping(true, dim::point2di(VisRect_.Left, VisRect_.Top), VisRect_.getSize());
    return checkClipping();
}

bool GUIController::checkClipping() const
{
    return VisRect_.Right > VisRect_.Left && VisRect_.Bottom > VisRect_.Top;
}

void GUIController::updateRect()
{
    Rect_ = RootRect_;
    
    if (Parent_)
    {
        dim::point2di Translation(Parent_->getPosition(true) + Parent_->getViewOrigin());
        
        if (Flags_ & GUIFLAG_NOSCROLL)
        {
            if (Parent_->Parent_)
                Translation += Parent_->Parent_->getScrollPosition(true);
        }
        else
            Translation += Parent_->getScrollPosition(true);
        
        Rect_.Left      += Translation.X;
        Rect_.Top       += Translation.Y;
        Rect_.Right     += Translation.X;
        Rect_.Bottom    += Translation.Y;
    }
}

bool GUIController::checkDefaultUpdate()
{
    if (foreignUsage())
    {
        updateChildren();
        return false;
    }
    
    if ( !isVisible_ || ( !usage() && ( !checkClipping() || !mouseOver(VisRect_) ) ) )
        return false;
    
    return (Usable_ = true);
}

void GUIController::addChild(GUIController* Child)
{
    Children_.push_back(Child);
}
void GUIController::removeChild(GUIController* Child)
{
    for (std::list<GUIController*>::iterator it = Children_.begin(); it != Children_.end(); ++it)
    {
        if (Child == *it)
        {
            Children_.erase(it);
            break;
        }
    }
}

void GUIController::useFocus(s32 Usage)
{
    __spGUIManager->FocusUsing_ = true;
    FocusUsage_ = Usage;
    focus();
}
bool GUIController::usage(s32 Usage) const
{
    return __spGUIManager->FocusUsing_ && __spGUIManager->FocusedController_ == this && FocusUsage_ == Usage;
}
bool GUIController::usage() const
{
    return __spGUIManager->FocusUsing_ && __spGUIManager->FocusedController_ == this;
}
bool GUIController::foreignUsage() const
{
    return __spGUIManager->FocusUsing_ && __spGUIManager->FocusedController_ != this;
}

dim::rect2di GUIController::getParentViewArea() const
{
    if (Parent_)
    {
        const dim::rect2di ParentViewArea(Parent_->getViewArea(this));
        return dim::rect2di(0, 0, ParentViewArea.Right - ParentViewArea.Left, ParentViewArea.Bottom - ParentViewArea.Top);
    }
    
    return dim::rect2di(0, 0, gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight);
}

void GUIController::updateScrollBars(GUIScrollbarGadget* HorzScroll, GUIScrollbarGadget* VertScroll)
{
    if (!HorzScroll && !VertScroll)
        return;
    
    const dim::rect2di Rect(
        getLocalViewArea(HorzScroll ? HorzScroll : VertScroll) - dim::rect2di(Rect_.Left, Rect_.Top, Rect_.Left, Rect_.Top)
    );
    
    if (HorzScroll)
    {
        HorzScroll->setPosition(dim::point2di(0, Rect.Bottom - Rect.Top - SCROLLBAR_SIZE));
        HorzScroll->setSize(dim::size2di(Rect.Right - Rect.Left - SCROLLBAR_SIZE, SCROLLBAR_SIZE));
        ScrollPos_.X = -HorzScroll->getState();
        HorzScroll->setVisible(HorzScroll->getSize().Width < HorzScroll->getRange());
    }
    if (VertScroll)
    {
        VertScroll->setPosition(dim::point2di(Rect.Right - Rect.Left - SCROLLBAR_SIZE, 0));
        VertScroll->setSize(dim::size2di(SCROLLBAR_SIZE, Rect.Bottom - Rect.Top - SCROLLBAR_SIZE));
        ScrollPos_.Y = -VertScroll->getState();
        VertScroll->setVisible(VertScroll->getSize().Height < VertScroll->getRange());
    }
}

void GUIController::checkFlags()
{
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
