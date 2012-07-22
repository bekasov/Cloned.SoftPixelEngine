/*
 * GUI tree gadget header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_TREEGADGET_H__
#define __SP_GUI_TREEGADGET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIGadget.hpp"


namespace sp
{
namespace gui
{


class GUITreeGadget;

enum ETreeFlags
{
    GUIFLAG_LINES = 0x00000001,
};


/*
 * GUITreeItem class
 */

class SP_EXPORT GUITreeItem
{
    
    public:
        
        GUITreeItem();
        ~GUITreeItem();
        
        /* Functions */
        
        void setParent(GUITreeItem* Parent);
        
        /* Inline functions */
        
        inline void setText(const io::stringc &Text)
        {
            Text_ = Text;
        }
        inline io::stringc getText() const
        {
            return Text_;
        }
        
        //! Expands/ reduces (or rather opens/ closes) the chidren list items.
        inline void setExpand(bool isExpand)
        {
            isExpand_ = isExpand;
        }
        inline bool getExpand() const
        {
            return isExpand_;
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
        
        inline GUITreeItem* getParent() const
        {
            return Parent_;
        }
        
    private:
        
        friend class GUITreeGadget;
        
        /* Functions */
        
        void addChild(GUITreeItem* Child);
        void removeChild(GUITreeItem* Child);
        
        /* Members */
        
        io::stringc Text_;
        video::Texture* Icon_;
        
        GUITreeItem* Parent_;
        std::list<GUITreeItem*> Children_;
        
        bool isExpand_;
        bool isVisible_;
        bool isPicked_;
        bool hasExplorerSubDir_;
        
};


/*
 * GUITreeGadget class
 */

class SP_EXPORT GUITreeGadget : public GUIGadget
{
    
    public:
        
        GUITreeGadget();
        ~GUITreeGadget();
        
        /* Functions */
        
        bool update();
        void draw();
        
        /**
        Adds a new item to the tree list.
        \param Text: Item text (or rather title).
        \param Parent: Optional parent item.
        \param Icon: Optional item icon (or rather image/ texture etc.).
        */
        GUITreeItem* addItem(const io::stringc &Text, GUITreeItem* Parent = 0, video::Texture* Icon = 0);
        
        //! Removes the specified item object from the tree.
        void removeItem(GUITreeItem* Item);
        
        //! Removes and deletes all items.
        void clearItems();
        
        /**
        Sets the explorer mode. If a tree gadget is in the explorer mode the whole disk will be
        represented in the tree gadget. When a sub directory will be opened new items will be created.
        It is similiar to the Windows(c) explorer(c) on the left side.
        */
        void setExplorer(bool isExplorer);//, s32 Flags = 0);
        
        io::stringc getExplorerFullPath(const GUITreeItem* Item) const;
        
        /* Inline functions */
        
        inline GUIScrollbarGadget* getHorzScrollBar() const
        {
            return HorzScroll_;
        }
        inline GUIScrollbarGadget* getVertScrollBar() const
        {
            return VertScroll_;
        }
        
        //! \return Pointer to the currently selected item object. If no item is selected the return value is 0 (null).
        inline GUITreeItem* getSelectedItem() const
        {
            return SelectedItem_;
        }
        
        inline bool getExplorer() const
        {
            return isExplorer_;
        }
        
    private:
        
        /* Macros */
        
        static const s32 TREEITEM_EXPAND_SIZE   = 15;
        static const s32 TREEITEM_HEIGHT        = 17;
        static const s32 EXPANDICON_HALFSIZE    = 5;
        
        static const video::color ITEMPICK_COLOR_A;
        static const video::color ITEMPICK_COLOR_B;
        
        /* Functions */
        
        void init();
        void clear();
        
        void drawItem(GUITreeItem* Item, dim::point2di &Pos);
        void drawExpandIcon(const dim::point2di &Pos, bool isExpand);
        
        void updateItem(GUITreeItem* Item, dim::point2di &Pos);
        bool updateExpandIcon(const dim::point2di &Pos, GUITreeItem* Item);
        
        void createExplorerDirs(GUITreeItem* Item);
        void addExplorerSubItem(GUITreeItem* Item, const void* FindFileDataRaw);
        
        //void updateMaxItemWidth(); // !!!
        
        dim::point2di getItemsStartPos() const;
        
        /* Members */
        
        std::list<GUITreeItem*> ItemList_;
        
        GUIScrollbarGadget* HorzScroll_;
        GUIScrollbarGadget* VertScroll_;
        
        GUITreeItem* SelectedItem_;
        
        s32 MaxItemWidth_;
        
        bool isExplorer_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
