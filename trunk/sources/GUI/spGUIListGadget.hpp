/*
 * GUI list gadget header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_LISTGADGET_H__
#define __SP_GUI_LISTGADGET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIGadget.hpp"


namespace sp
{
namespace gui
{


static const s32 DEF_GUILIST_COLUMNSIZE = -1;

enum EListFlags
{
    GUIFLAG_NOCOLUMNSCROLL = 0x00000001,
};


/*
 * GUIListRootEntry class
 */

class SP_EXPORT GUIListRootEntry
{
    
    public:
        
        virtual ~GUIListRootEntry();
        
        virtual void setText(const io::stringc &Text);
        
        /* Inline functions */
        
        inline io::stringc getText() const
        {
            return Text_;
        }
        
        inline void setVisible(bool isVisible)
        {
            isVisible_ = isVisible;
        }
        inline bool getVisible() const
        {
            return isVisible_;
        }
        
        inline void setIcon(video::Texture* Icon)
        {
            Icon_ = Icon;
        }
        inline video::Texture* getIcon() const
        {
            return Icon_;
        }
        
        inline void setColor(const video::color &Color)
        {
            Color_ = Color;
        }
        inline video::color getColor() const
        {
            return Color_;
        }
        
    protected:
        
        friend class GUIListGadget;
        
        /* Functions */
        
        GUIListRootEntry();
        
        /* Members */
        
        io::stringc Text_;
        video::Texture* Icon_;
        video::color Color_;
        
        bool isVisible_;
        bool isPicked_;
        
};


/*
 * GUIListColumn class
 */

class SP_EXPORT GUIListColumn : public GUIListRootEntry
{
    
    public:
        
        GUIListColumn();
        ~GUIListColumn();
        
        /* Inline functions */
        
        inline void setColumnSize(s32 Size)
        {
            ColumnSize_ = math::Max(0, Size);
        }
        inline s32 getColumnSize() const
        {
            return ColumnSize_;
        }
        
    private:
        
        /* Members */
        
        s32 ColumnSize_;
        
};


/*
 * GUIListItem class
 */

class SP_EXPORT GUIListItem : public GUIListRootEntry
{
    
    public:
        
        GUIListItem();
        ~GUIListItem();
        
        void setText(const io::stringc &Text);
        
        /* Inline functions */
        
        inline void setGroup(bool isGroup)
        {
            isGroup_ = isGroup;
        }
        inline bool getGroup() const
        {
            return isGroup_;
        }
        
        inline void setGroupParent(GUIListItem* Parent)
        {
            GroupParent_ = Parent;
        }
        inline GUIListItem* getGroupParent() const
        {
            return GroupParent_;
        }
        
        inline void setItemSize(s32 Size)
        {
            ItemSize_ = math::Max(0, Size);
        }
        inline s32 getItemSize() const
        {
            return ItemSize_;
        }
        
    private:
        
        /* Members */
        
        std::vector<io::stringc> SubTextList_;
        
        bool isGroup_;
        GUIListItem* GroupParent_;
        s32 ItemSize_;
        
        GUIGadget* StateGadget_;
        u32 StateGadgetSub_;
        
};


/*
 * GUIListGadget class
 */

class SP_EXPORT GUIListGadget : public GUIGadget
{
    
    public:
        
        GUIListGadget();
        ~GUIListGadget();
        
        /* Functions */
        
        bool update();
        void draw();
        
        dim::rect2di getLocalViewArea(const GUIController* Obj = 0) const;
        
        GUIListColumn* addColumn(
            const io::stringc &Text, s32 ColumnSize = DEF_GUILIST_COLUMNSIZE, video::Texture* Icon = 0
        );
        void removeColumn(GUIListColumn* Column);
        
        //! Removes and deletes all columns;
        void clearColumns();
        
        GUIListItem* addItem(
            const io::stringc &Text, GUIListItem* GroupParent = 0, bool isGroup = false, video::Texture* Icon = 0
        );
        void removeItem(GUIListItem* Item);
        
        //! Removes and deletes all items.
        void clearItems();
        
        /**
        Adds all items of the specified disk directory. The path can be in the following forms:
        "C:/Windows/" or "C:\\Windows\\" or "C:\\Windows\\*.exe" or "C:\\Windows\\*".
        */
        void addDirectoryItems(io::stringc DirPath);//, s32 Flags = 0);
        
        /* Inline functions */
        
        inline GUIScrollbarGadget* getHorzScrollBar() const
        {
            return HorzScroll_;
        }
        inline GUIScrollbarGadget* getVertScrollBar() const
        {
            return VertScroll_;
        }
        
        inline GUIListItem* getSelectedItem() const
        {
            return SelectedItem_;
        }
        
    private:
        
        /* === Macros === */
        
        static const s32 COLUMN_HEIGHT      = 25;
        
        static const video::color ITEMPICK_COLOR_A;
        static const video::color ITEMPICK_COLOR_B;
        
        /* === Enumerations === */
        
        enum EFocusUsages
        {
            USAGE_RESIZE_COLUMN,
        };
        
        /* === Functions === */
        
        void init();
        void clear();
        
        void drawColumn(GUIListColumn* Column, s32 EntryPos);
        void drawItem(GUIListItem* Item, s32 EntryPos);
        
        bool pickColumn(GUIListColumn* &Column, bool &isEdgePick);
        void updateItem(GUIListItem* Item, dim::point2di &Pos);
        
        dim::point2di getItemsStartPos() const;
        
        /* === Template === */
        
        template <class T> bool deleteListObject(T* Object, std::list<T*> &List)
        {
            if (Object)
            {
                for (typename std::list< T*, std::allocator<T*> >::iterator it = List.begin(); it != List.end(); ++it)
                {
                    if (*it == Object)
                    {
                        MemoryManager::deleteMemory(*it);
                        List.erase(it);
                        return true;
                    }
                }
            }
            return false;
        }
        
        /* === Members === */
        
        std::list<GUIListColumn*> ColumnList_;
        std::list<GUIListItem*> ItemList_;
        
        GUIScrollbarGadget* HorzScroll_;
        GUIScrollbarGadget* VertScroll_;
        
        GUIListColumn* FocusedColumn_;
        s32 FocusedColumnPosHorz_;
        
        GUIListItem* SelectedItem_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
