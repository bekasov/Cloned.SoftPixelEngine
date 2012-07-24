/*
 * GUI list gadget file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUIListGadget.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "RenderSystem/spRenderSystem.hpp"
#include "GUI/spGUIManager.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern gui::GUIManager* __spGUIManager;

namespace gui
{


/*
 * GUIListRootEntry class
 */

GUIListRootEntry::GUIListRootEntry() : Icon_(0)
{
    isVisible_  = true;
    isPicked_   = false;
    Color_      = 0;
}
GUIListRootEntry::~GUIListRootEntry()
{
}

void GUIListRootEntry::setText(const io::stringc &Text)
{
    Text_ = Text;
}


/*
 * GUIListColumn class
 */

GUIListColumn::GUIListColumn() : GUIListRootEntry()
{
    ColumnSize_ = 100;
}
GUIListColumn::~GUIListColumn()
{
}


/*
 * GUIListItem class
 */

GUIListItem::GUIListItem()
    : GUIListRootEntry(), GroupParent_(0), StateGadget_(0), StateGadgetSub_(0)
{
    isGroup_    = false;
    ItemSize_   = 20;
}
GUIListItem::~GUIListItem()
{
}

void GUIListItem::setText(const io::stringc &Text)
{
    Text_ = Text;
    
    /* Store the sub texts in an extra list */
    s32 pos = -1, last = 0;
    
    do
    {
        pos = Text_.find("\n", pos + 1);
        
        if (pos == -1)
            pos = Text_.size();
        
        SubTextList_.push_back(Text_.section(last, pos));
        
        last = pos;
    }
    while (pos < static_cast<s32>(Text_.size()));
}


/*
 * GUIListGadget class
 */

const video::color GUIListGadget::ITEMPICK_COLOR_A = video::color(200, 200, 255);
const video::color GUIListGadget::ITEMPICK_COLOR_B = video::color(120, 120, 170);

GUIListGadget::GUIListGadget()
    : GUIGadget(GADGET_LIST), HorzScroll_(0), VertScroll_(0), FocusedColumn_(0), SelectedItem_(0)
{
    init();
}
GUIListGadget::~GUIListGadget()
{
    clear();
}

bool GUIListGadget::update()
{
    updateScrollBars(HorzScroll_, VertScroll_);
    
    if (hasFocus() && __spGUIManager->MouseWheel_)
        VertScroll_->scroll(-__spGUIManager->MouseWheel_ * 30);
    
    if (!checkDefaultUpdate())
        return false;
    
    if (isEnabled_ && mouseOver(Rect_) && !foreignUsage())
    {
        if (mouseLeft())
        {
            focus();
            
            GUIListColumn* Column;
            bool isEdgePick;
            
            if (pickColumn(Column, isEdgePick))
            {
                FocusedColumn_ = Column;
                
                if (isEdgePick)
                    useFocus(USAGE_RESIZE_COLUMN);
                else
                    ; // !TODO!
            }
        }
    }
    
    dim::point2di Pos(getItemsStartPos());
    
    for (std::list<GUIListItem*>::iterator it = ItemList_.begin(); it != ItemList_.end(); ++it)
        updateItem(*it, Pos);
    
    if (usage(USAGE_RESIZE_COLUMN) && FocusedColumn_)
        FocusedColumn_->setColumnSize(__spGUIManager->CursorPos_.X - FocusedColumnPosHorz_);
    
    updateChildren();
    
    return true;
}

void GUIListGadget::draw()
{
    if (!isVisible_ || isValidated_ || !setupClipping())
        return;
    
    __spVideoDriver->draw2DRectangle(Rect_, Color_);
    
    /* Draw all item entries */
    s32 ItemPos = 0;
    for (std::list<GUIListItem*>::iterator it = ItemList_.begin(); it != ItemList_.end(); ++it)
    {
        drawItem(*it, ItemPos);
        ItemPos += (*it)->getItemSize();
    }
    
    /* Draw all column entries */
    s32 ColumnPos = 0;
    for (std::list<GUIListColumn*>::iterator it = ColumnList_.begin(); it != ColumnList_.end(); ++it)
    {
        drawColumn(*it, ColumnPos);
        ColumnPos += (*it)->getColumnSize();
    }
    
    /* Update scrollbar ranges */
    HorzScroll_->setRange(ColumnPos);
    VertScroll_->setRange(ItemPos + COLUMN_HEIGHT);
    
    drawChildren();
    
    __spVideoDriver->setClipping(true, dim::point2di(VisRect_.Left, VisRect_.Top), VisRect_.getSize());
    
    drawFrame(Rect_, 0, false);
}

dim::rect2di GUIListGadget::getLocalViewArea(const GUIController* Obj) const
{
    dim::rect2di Rect(Rect_);
    
    if (Obj != HorzScroll_ && Obj != VertScroll_)
    {
        if (HorzScroll_ && HorzScroll_->getVisible())
            Rect.Bottom -= SCROLLBAR_SIZE;
        if (VertScroll_ && VertScroll_->getVisible())
            Rect.Right -= SCROLLBAR_SIZE;
    }
    
    return Rect; 
}

GUIListColumn* GUIListGadget::addColumn(
    const io::stringc &Text, s32 ColumnSize, video::Texture* Icon)
{
    GUIListColumn* NewColumn = new GUIListColumn();
    {
        NewColumn->setText(Text);
        NewColumn->setColumnSize(ColumnSize);
        NewColumn->setIcon(Icon);
    }
    ColumnList_.push_back(NewColumn);
    
    return NewColumn;
}
void GUIListGadget::removeColumn(GUIListColumn* Column)
{
    deleteListObject<GUIListColumn>(Column, ColumnList_);
}

void GUIListGadget::clearColumns()
{
    for (std::list<GUIListColumn*>::iterator it = ColumnList_.begin(); it != ColumnList_.end(); ++it)
        MemoryManager::deleteMemory(*it);
    ColumnList_.clear();
}

GUIListItem* GUIListGadget::addItem(
    const io::stringc &Text, GUIListItem* GroupParent, bool isGroup, video::Texture* Icon)
{
    GUIListItem* NewItem = new GUIListItem();
    {
        NewItem->setText(Text);
        NewItem->setGroupParent(GroupParent);
        NewItem->setGroup(isGroup);
        NewItem->setIcon(Icon);
    }
    ItemList_.push_back(NewItem);
    
    return NewItem;
}
void GUIListGadget::removeItem(GUIListItem* Item)
{
    if (SelectedItem_ == Item)
        SelectedItem_ = 0;
    
    deleteListObject<GUIListItem>(Item, ItemList_);
}

void GUIListGadget::clearItems()
{
    for (std::list<GUIListItem*>::iterator it = ItemList_.begin(); it != ItemList_.end(); ++it)
        MemoryManager::deleteMemory(*it);
    ItemList_.clear();
    SelectedItem_ = 0;
}

void GUIListGadget::addDirectoryItems(io::stringc DirPath)
{
    #if defined(SP_PLATFORM_WINDOWS)
    
    if (DirPath.right(1) == "/" || DirPath.right(1) == "\\")
        DirPath += "*";
    
    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile = FindFirstFile(DirPath.c_str(), &FindFileData);
    
    io::stringc EntryName;
    
    if (hFindFile)
    {
        do
        {
            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
            {
                EntryName = FindFileData.cFileName;
                
                if (EntryName != "." && EntryName != "..")
                    addItem(EntryName);
            }
        }
        while (FindNextFile(hFindFile, &FindFileData));
        
        FindClose(hFindFile);
    }
    
    #endif
}


/*
 * ======= Private: =======
 */

void GUIListGadget::init()
{
    /* Create scrollbar gadgets */
    HorzScroll_ = new GUIScrollbarGadget();
    HorzScroll_->setFlags(GUIFLAG_NOSCROLL);
    HorzScroll_->setParent(this);
    
    VertScroll_ = new GUIScrollbarGadget();
    VertScroll_->setFlags(GUIFLAG_NOSCROLL | GUIFLAG_VERTICAL);
    VertScroll_->setParent(this);
}
void GUIListGadget::clear()
{
    MemoryManager::deleteMemory(HorzScroll_);
    MemoryManager::deleteMemory(VertScroll_);
    
    clearItems();
    clearColumns();
}

void GUIListGadget::drawColumn(GUIListColumn* Column, s32 EntryPos)
{
    const s32 PosHorz = Rect_.Left + ScrollPos_.X + EntryPos;
    
    dim::rect2di Rect(
        PosHorz, Rect_.Top, PosHorz + Column->getColumnSize(), Rect_.Top + COLUMN_HEIGHT
    );
    
    if (!(Flags_ & GUIFLAG_NOCOLUMNSCROLL))
    {
        Rect.Top    += ScrollPos_.Y;
        Rect.Bottom += ScrollPos_.Y;
    }
    
    Color_ = video::color(255, 220, 50); // !!!
    drawBgRect(Rect, Rect);
    Color_ = 255;
    
    drawText(
        dim::point2di(Rect.Left + 5, Rect.Top + 2), Column->getText(), Column->getColor(), 0
    );
    
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect.Right, Rect_.Top), dim::point2di(Rect.Right, Rect_.Bottom), 0
    );
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect.Left, Rect.Bottom), dim::point2di(Rect.Right, Rect.Bottom), 0
    );
}

void GUIListGadget::drawItem(GUIListItem* Item, s32 EntryPos)
{
    const s32 PosVert = Rect_.Top + ScrollPos_.Y + EntryPos + COLUMN_HEIGHT;
    
    const dim::rect2di Rect(
        Rect_.Left + ScrollPos_.X, PosVert + 1, Rect_.Right, PosVert + Item->getItemSize()
    );
    
    /* Update item picking */
    if ( Item->isPicked_ || ( (Flags_ & GUIFLAG_HOLDSELECTION) && Item == SelectedItem_ ))
    {
        __spVideoDriver->draw2DRectangle(
            Rect, ITEMPICK_COLOR_A, ITEMPICK_COLOR_A, ITEMPICK_COLOR_B, ITEMPICK_COLOR_B
        );
        
        Item->isPicked_ = false;
    }
    
    /* Draw the item text */
    drawText(
        dim::point2di(Rect.Left + 5, Rect.Top), Item->getText(), Item->getColor(), 0
    );
    
    __spVideoDriver->draw2DLine(
        dim::point2di(Rect.Left, Rect.Bottom), dim::point2di(Rect.Right, Rect.Bottom), 0
    );
}

bool GUIListGadget::pickColumn(GUIListColumn* &Column, bool &isEdgePick)
{
    if (!mouseOver(dim::rect2di(Rect_.Left, Rect_.Top, Rect_.Right, Rect_.Top + COLUMN_HEIGHT)))
        return false;
    
    s32 ColumnPosHorz = Rect_.Left + ScrollPos_.X;
    s32 ColumnPosVert = Rect_.Top;
    
    if (!(Flags_ & GUIFLAG_NOCOLUMNSCROLL))
        ColumnPosVert += ScrollPos_.Y;
    
    for (std::list<GUIListColumn*>::iterator it = ColumnList_.begin(); it != ColumnList_.end(); ++it)
    {
        FocusedColumnPosHorz_ = ColumnPosHorz;
        ColumnPosHorz += (*it)->getColumnSize();
        
        if (mouseOver(dim::rect2di(ColumnPosHorz - 5, ColumnPosVert, ColumnPosHorz + 5, ColumnPosVert + COLUMN_HEIGHT)))
        {
            Column      = *it;
            isEdgePick  = true;
            return true;
        }
        else if (mouseOver(dim::rect2di(ColumnPosHorz - (*it)->getColumnSize(), ColumnPosVert,
                                        ColumnPosHorz, ColumnPosVert + COLUMN_HEIGHT)))
        {
            Column      = *it;
            isEdgePick  = false;
            return true;
        }
    }
    
    return false;
}

void GUIListGadget::updateItem(GUIListItem* Item, dim::point2di &Pos)
{
    if (!Item || !Item->getVisible())
        return;
    
    dim::rect2di Rect(
        Rect_.Left, Pos.Y, Rect_.Right, Pos.Y + Item->getItemSize()
    );
    
    if (VertScroll_ && VertScroll_->getVisible())
        Rect.Right -= SCROLLBAR_SIZE;
    if (HorzScroll_ && HorzScroll_->getVisible() && Rect.Bottom > Rect_.Bottom - SCROLLBAR_SIZE)
        Rect.Bottom = Rect_.Bottom - SCROLLBAR_SIZE;
    
    Pos.Y += Item->getItemSize();
    
    /* Update current item */
    if (mouseOver(Rect))
    {
        Item->isPicked_ = true;
        
        if (mouseLeft())
        {
            SelectedItem_ = Item;
            
            SGUIEvent Event;
            {
                Event.Object    = EVENT_GADGET;
                Event.Type      = EVENT_ACTIVATE;
                Event.Gadget    = this;
                Event.SubData   = Item;
            }
            sendEvent(Event);
        }
    }
}

dim::point2di GUIListGadget::getItemsStartPos() const
{
    dim::point2di ScrollPos(getScrollPosition());
    return dim::point2di(Rect_.Left + 2 + ScrollPos.X, Rect_.Top + 2 + COLUMN_HEIGHT + ScrollPos.Y);
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
