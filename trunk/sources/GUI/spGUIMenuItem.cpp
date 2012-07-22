/*
 * GUI menu item file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUIMenuItem.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "GUI/spGUIManager.hpp"
#include "RenderSystem/spRenderSystem.hpp"


namespace sp
{

extern gui::GUIManager* __spGUIManager;
extern video::RenderSystem* __spVideoDriver;

namespace gui
{


const video::color GUIMenuItem::ITEMPICK_COLOR_A = video::color(200, 200, 255);
const video::color GUIMenuItem::ITEMPICK_COLOR_B = video::color(120, 120, 170);

GUIMenuItem::GUIMenuItem(const EMenuItemTypes Type, const io::stringc &Text)
    : Type_(Type), Parent_(0)
{
    init();
    setText(Text);
}
GUIMenuItem::~GUIMenuItem()
{
    clear();
}

bool GUIMenuItem::update(const dim::point2di &Position)
{
    Position_ = Position;
    
    if (Type_ != MENUITEM_ROOT && Type_ != MENUITEM_SEPARATOR)
    {
        /* Update user input */
        const dim::rect2di Rect(
            Position_.X, Position_.Y, Position_.X + Size_.Width, Position_.Y + Size_.Height
        );
        
        isMouseOver_ = mouseOver(Rect);
        
        if ( isMouseOver_ && isEnabled_ && ( mouseLeftUp() ||
             ( Type_ == MENUITEM_TITLE && !isExpand_ && Parent_ && Parent_->anyChildExpand() ) ) )
        {
            /* Update menu expansion */
            if (!Children_.empty())
            {
                if (!isExpand_ && Parent_)
                    Parent_->closeExpansionChildren();
                isExpand_ = !isExpand_;
            }
            else
                closeExpansionParent();
            
            sendEvent(EVENT_MENUITEM, EVENT_ACTIVATE);
        }
    }
    
    if ( Type_ != MENUITEM_SEPARATOR && ( isExpand_ || Type_ == MENUITEM_ROOT ) )
    {
        /* Set the children start position */
        dim::point2di ChildPos;
        
        if (Type_ == MENUITEM_TITLE)
            ChildPos = dim::point2di(Position_.X, Position_.Y + MENUITEM_HEIGHT);
        else if (Type_ == MENUITEM_ROOT)
            ChildPos = dim::point2di(Position_.X, Position_.Y);
        else
            ChildPos = dim::point2di(Position_.X + Size_.Width, Position_.Y);
        
        /* Draw the children */
        for (std::list<GUIMenuItem*>::iterator it = Children_.begin(); it != Children_.end(); ++it)
        {
            (*it)->update(ChildPos);
            
            if (Type_ == MENUITEM_ROOT)
                ChildPos.X += (*it)->Size_.Width;
            else
                ChildPos.Y += (*it)->Size_.Height;
        }
    }
    
    return false;
}

void GUIMenuItem::draw()
{
    if (Type_ != MENUITEM_ROOT)
    {
        /* Draw item background */
        __spVideoDriver->draw2DRectangle(
            dim::rect2di(Position_.X, Position_.Y, Position_.X + Size_.Width, Position_.Y + Size_.Height), Color_
        );
        
        if (Type_ == MENUITEM_SEPARATOR)
        {
            /* Draw separator */
            __spVideoDriver->draw2DLine(
                dim::point2di(Position_.X + 25, Position_.Y + 5), dim::point2di(Position_.X + Size_.Width - 5, Position_.Y + 5), 160
            );
        }
        else
        {
            const video::color TextColor(
                (isEnabled_ ? (isMouseOver_ || isExpand_ ? 255 : 0) : 128)
            );
            
            /* Draw selection */
            if ( isEnabled_ && ( isMouseOver_ || isExpand_ ) )
            {
                const dim::rect2di Rect(
                    Position_.X + 3, Position_.Y + 1, Position_.X + Size_.Width - 3, Position_.Y + Size_.Height - 1
                );
                
                __spVideoDriver->draw2DRectangle(
                    Rect, ITEMPICK_COLOR_A, ITEMPICK_COLOR_A, ITEMPICK_COLOR_B, ITEMPICK_COLOR_B
                );
            }
            
            /* Draw item text */
            const s32 StartPos = (Type_ == MENUITEM_ENTRY ? Space_ + 20 : Space_);
            
            __spVideoDriver->draw2DText(
                Font_, dim::point2di(Position_.X + StartPos, Position_.Y), Text_, TextColor
            );
            
            if (ExText_.size())
            {
                __spVideoDriver->draw2DText(
                    Font_, dim::point2di(Position_.X + Space_ + ExPos_, Position_.Y), ExText_, TextColor
                );
            }
            
            /* Draw state icon */
            if (isChecked_ && Type_ == MENUITEM_ENTRY)
            {
                __spVideoDriver->draw2DLine(
                    dim::point2di(Position_.X + 8, Position_.Y + 11), dim::point2di(Position_.X + 12, Position_.Y + 15), TextColor
                );
                __spVideoDriver->draw2DLine(
                    dim::point2di(Position_.X + 11, Position_.Y + 15), dim::point2di(Position_.X + 18, Position_.Y + 8), TextColor
                );
            }
            
            /* Draw sub menu icon */
            if (!Children_.empty() && Type_ != MENUITEM_TITLE)
            {
                for (s32 i = 0; i < 5; ++i)
                {
                    __spVideoDriver->draw2DLine(
                        dim::point2di(Position_.X + Size_.Width - Space_ - i, Position_.Y + 9 - i),
                        dim::point2di(Position_.X + Size_.Width - Space_ - i, Position_.Y + 10 + i),
                        TextColor
                    );
                }
            }
        }
    }
    
    if (isExpand_ || Type_ == MENUITEM_ROOT)
    {
        dim::size2di Size;
        
        /* Draw the children */
        for (std::list<GUIMenuItem*>::iterator it = Children_.begin(); it != Children_.end(); ++it)
        {
            (*it)->draw();
            
            Size.Width = (*it)->Size_.Width;
            Size.Height += (*it)->Size_.Height;
        }
        
        /* Draw box frame */
        if (Type_ != MENUITEM_ROOT && !Children_.empty())
        {
            dim::point2di Pos;
            
            if (Type_ == MENUITEM_TITLE)
                Pos = dim::point2di(Position_.X, Position_.Y + Size_.Height);
            else
                Pos = dim::point2di(Position_.X + Size_.Width, Position_.Y);
            
            drawFrame(
                dim::rect2di(Pos.X, Pos.Y, Pos.X + Size.Width, Pos.Y + Size.Height), 0, true
            );
            
            __spVideoDriver->draw2DLine(
                dim::point2di(Pos.X + 23, Pos.Y + 5), dim::point2di(Pos.X + 23, Pos.Y + Size.Height - 5), 128
            );
        }
    }
}

void GUIMenuItem::setText(const io::stringc &Text)
{
    s32 pos;
    
    if ( ( pos = Text.find("\t") ) != -1 )
    {
        Text_   = Text.left(pos);
        ExText_ = Text.right(Text.size() - pos - 1);
    }
    else
    {
        Text_   = Text;
        ExText_ = "";
    }
    
    if (Parent_)
        Parent_->updateChildrenSize();
}
void GUIMenuItem::setParent(GUIMenuItem* Parent)
{
    if ( Parent != Parent_ && ( !Parent_ || Parent_->Type_ != MENUITEM_SEPARATOR ) )
    {
        if (Parent_)
            Parent_->removeChild(this);
        if (Parent)
            Parent->addChild(this);
        Parent_ = Parent;
    }
}

void GUIMenuItem::addChild(GUIMenuItem* Child)
{
    if ( Child && Type_ != MENUITEM_SEPARATOR &&
         ( ( Type_ == MENUITEM_ROOT && Child->Type_ == MENUITEM_TITLE ) ||
           ( Type_ != MENUITEM_ROOT && ( Child->Type_ == MENUITEM_ENTRY || Child->Type_ == MENUITEM_SEPARATOR ) ) ) )
    {
        if (Child->Parent_)
            Child->Parent_->removeChild(Child);
        
        Child->Parent_ = this;
        Children_.push_back(Child);
        
        if (Parent_)
            Parent_->updateChildrenSize();
        
        updateChildrenSize();
    }
}
void GUIMenuItem::removeChild(GUIMenuItem* Child)
{
    for (std::list<GUIMenuItem*>::iterator it = Children_.begin(); it != Children_.end(); ++it)
    {
        if (*it == Child)
        {
            Children_.erase(it);
            Child->Parent_ = 0;
            
            if (Parent_)
                Parent_->updateChildrenSize();
            
            updateChildrenSize();
            break;
        }
    }
}

void GUIMenuItem::setColor(const video::color &Color)
{
    if (Type_ == MENUITEM_ROOT)
        setColorRecursive(Color);
    else
        Color_ = Color;
}


/*
 * ======= Private: =======
 */

void GUIMenuItem::init()
{
    /* Default settings */
    isExpand_       = false;
    isMouseOver_    = false;
    isChecked_      = false;
    
    Space_          = 10;
    Color_          = 235;
    ExPos_          = 0;
    
    Size_.Height    = (Type_ == MENUITEM_SEPARATOR ? SEPARATOR_HEIGHT : MENUITEM_HEIGHT);
}
void GUIMenuItem::clear()
{
    for (std::list<GUIMenuItem*>::iterator it = Children_.begin(); it != Children_.end(); ++it)
        MemoryManager::deleteMemory(*it);
}

void GUIMenuItem::updateChildrenSize()
{
    if (Type_ == MENUITEM_ROOT)
    {
        /* Update children width */
        for (std::list<GUIMenuItem*>::iterator it = Children_.begin(); it != Children_.end(); ++it)
            (*it)->Size_.Width = (*it)->getTextWidth();
    }
    else
    {
        /* Get the longest item text */
        s32 MaxWidth = 0, ExPos = 0, TmpWidth;
        bool AnySubMenu = false;
        
        for (std::list<GUIMenuItem*>::iterator it = Children_.begin(); it != Children_.end(); ++it)
        {
            TmpWidth = (*it)->getTextWidth();
            if (ExPos < TmpWidth)
                ExPos = TmpWidth;
            
            TmpWidth = (*it)->getExTextWidth();
            if (MaxWidth < TmpWidth)
                MaxWidth = TmpWidth;
            
            if (!(*it)->Children_.empty() && (*it)->Type_ != MENUITEM_TITLE)
                AnySubMenu = true;
        }
        
        MaxWidth += ExPos;
        
        if (AnySubMenu)
            MaxWidth += 15;
        
        /* Update children width */
        for (std::list<GUIMenuItem*>::iterator it = Children_.begin(); it != Children_.end(); ++it)
        {
            (*it)->Size_.Width  = MaxWidth;
            (*it)->ExPos_       = ExPos;
        }
    }
}

void GUIMenuItem::closeExpansionChildren()
{
    for (std::list<GUIMenuItem*>::iterator it = Children_.begin(); it != Children_.end(); ++it)
    {
        (*it)->isExpand_ = false;
        (*it)->closeExpansionChildren();
    }
}
void GUIMenuItem::closeExpansionParent()
{
    if (Parent_)
    {
        Parent_->isExpand_ = false;
        Parent_->closeExpansionParent();
    }
}

void GUIMenuItem::setColorRecursive(const video::color &Color)
{
    Color_ = Color;
    for (std::list<GUIMenuItem*>::iterator it = Children_.begin(); it != Children_.end(); ++it)
        (*it)->setColorRecursive(Color);
}

s32 GUIMenuItem::getTextWidth() const
{
    s32 Width = Font_->getStringWidth(Text_) + Space_*2;
    
    if (Type_ == MENUITEM_ENTRY || Type_ == MENUITEM_SEPARATOR)
        Width += 20;
    
    return Width;
}
s32 GUIMenuItem::getExTextWidth() const
{
    return ExText_.size() ? Font_->getStringWidth(ExText_) + Space_*2 : 0;
}

bool GUIMenuItem::anyChildExpand() const
{
    for (std::list<GUIMenuItem*>::const_iterator it = Children_.begin(); it != Children_.end(); ++it)
    {
        if ((*it)->isExpand_)
            return true;
    }
    
    return false;
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
