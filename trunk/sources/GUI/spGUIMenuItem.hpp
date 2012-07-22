/*
 * GUI menu item header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_MENUITEM_H__
#define __SP_GUI_MENUITEM_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIBasicObject.hpp"


namespace sp
{
namespace gui
{


enum EMenuItemTypes
{
    MENUITEM_ROOT,      //!< Menu holder.
    MENUITEM_TITLE,     //!< Menu title.
    MENUITEM_ENTRY,     //!< Menu item entry.
    MENUITEM_SEPARATOR, //!< Menu item separator.
};


class SP_EXPORT GUIMenuItem : public GUIBasicObject
{
    
    public:
        
        GUIMenuItem(const EMenuItemTypes Type, const io::stringc &Text = "");
        ~GUIMenuItem();
        
        /* Functions */
        
        bool update(const dim::point2di &Position);
        void draw();
        
        void setText(const io::stringc &Text);
        void setParent(GUIMenuItem* Parent);
        
        void addChild(GUIMenuItem* Child);
        void removeChild(GUIMenuItem* Child);
        
        void setColor(const video::color &Color);
        
        /* Inline functions */
        
        inline EMenuItemTypes getType() const
        {
            return Type_;
        }
        inline io::stringc getText() const
        {
            return Text_;
        }
        inline io::stringc getExText() const
        {
            return ExText_;
        }
        inline GUIMenuItem* getParent() const
        {
            return Parent_;
        }
        
        inline void setExpand(bool isExpand)
        {
            isExpand_ = isExpand;
        }
        inline bool getExpand() const
        {
            return isExpand_;
        }
        
        inline void setState(bool isChecked)
        {
            isChecked_ = isChecked;
        }
        inline bool getState() const
        {
            return isChecked_;
        }
        
    private:
        
        /* Macros */
        
        static const s32 MENUITEM_HEIGHT    = 20;
        static const s32 SEPARATOR_HEIGHT   = 10;
        
        static const video::color ITEMPICK_COLOR_A;
        static const video::color ITEMPICK_COLOR_B;
        
        /* Enumerations */
        
        
        /* Functions */
        
        void init();
        void clear();
        
        void updateChildrenSize();
        
        void closeExpansionChildren();
        void closeExpansionParent();
        
        void setColorRecursive(const video::color &Color);
        
        s32 getTextWidth() const;
        s32 getExTextWidth() const;
        
        bool anyChildExpand() const;
        
        /* Members */
        
        EMenuItemTypes Type_;
        io::stringc Text_, ExText_;
        
        dim::point2di Position_;
        dim::size2di Size_;
        s32 Space_, ExPos_;
        
        GUIMenuItem* Parent_;
        std::list<GUIMenuItem*> Children_;
        
        bool isExpand_;
        bool isMouseOver_;
        bool isChecked_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
