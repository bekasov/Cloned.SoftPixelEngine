/*
 * GUI controller header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_CONTROLLER_H__
#define __SP_GUI_CONTROLLER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "RenderSystem/spRenderSystem.hpp"
#include "GUI/spGUIBasicObject.hpp"


namespace sp
{
namespace gui
{


class GUIScrollbarGadget;

enum EGUIControllerTypes
{
    CONTROLLER_WINDOW,
    CONTROLLER_GADGET,
};

enum EControllerFlags
{
    GUIFLAG_TEXTLEFT        = 0x01000000,
    GUIFLAG_TEXTRIGHT       = 0x02000000,
    GUIFLAG_TEXTCENTER      = 0x04000000,
    GUIFLAG_BORDERLESS      = 0x08000000,
    GUIFLAG_NOSCROLL        = 0x10000000,
    GUIFLAG_HOLDSELECTION   = 0x20000000,
};


class SP_EXPORT GUIController : public GUIBasicObject
{
    
    public:
        
        virtual ~GUIController();
        
        /* === Functions === */
        
        virtual bool update() = 0;
        virtual void draw() = 0;
        virtual void updateAlways();
        
        //! Sets the rectangular area of the GUI object object.
        virtual void setRect(const dim::rect2di &Rect);
        
        //! Sets the position of the GUI object. Setting the position is always local!
        virtual void setPosition(const dim::point2di &Position);
        
        //! Sets the size of the GUI object.
        virtual void setSize(const dim::size2di &Size);
        
        /**
        Returns the position of the GUI object.
        \param isGlobal: If true the returned position is global i.e. dependent of the
        GUI object's parents. Otherwise local.
        */
        virtual dim::point2di getPosition(bool isGlobal) const;
        
        //! Returns the rectangular area of the GUI object (local or global).
        virtual dim::rect2di getRect(bool isGlobal) const;
        
        //! Returns the local view area. This is the visible area where you can see the child objects.
        virtual dim::rect2di getLocalViewArea(const GUIController* Obj = 0) const;
        
        //! Returns the global view area.
        virtual dim::rect2di getViewArea(const GUIController* Obj = 0) const;
        
        //! Returns the local view area origin.
        virtual dim::point2di getViewOrigin() const;
        
        //! Returns the scroll position (local or global).
        virtual dim::point2di getScrollPosition(bool isGlobal) const;
        
        /**
        Sets the parent object of the GUI object. Several objects support a children hierarchy
        e.h. windows (GUIWindow objects).
        \param Parent: Parent GUI object.
        */
        virtual void setParent(GUIController* Parent);
        
        //! Sets the focus to this GUI object.
        virtual void focus();
        
        //! Returns true if this GUI object has the global focus.
        virtual bool hasFocus() const;
        
        //! Sets the GUI object in the foreground. This is particular used for windows.
        virtual void foreground();
        
        /**
        Sets the flags (or rather options).
        \param Flags: Flags which are to be set. There are several flags for several GUI object types.
        The basic flags are defined in the EControllerFlags enumeration. For windows there are additional
        falgs in EWindowFlags etc.
        */
        virtual void setFlags(s32 Flags);
        
        /* === Inline functions === */
        
        //! Returns the GUI object's type.
        inline EGUIControllerTypes getType() const
        {
            return Type_;
        }
        
        //! Returns the GUI object's parent or 0 if not set.
        inline GUIController* getParent() const
        {
            return Parent_;
        }
        
        //! Returns the global GUI object rectangular area.
        inline dim::rect2di getRect() const
        {
            return Rect_;
        }
        
        //! Returns the local GUI object position.
        inline dim::point2di getPosition() const
        {
            return dim::point2di(RootRect_.Left, RootRect_.Top);
        }
        
        //! Returns the GUI object size.
        inline dim::size2di getSize() const
        {
            return dim::size2di(RootRect_.Right - RootRect_.Left, RootRect_.Bottom - RootRect_.Top);
        }
        
        //! Sets the scroll position. This is particular used for windows with scroll bars.
        inline void setScrollPosition(const dim::point2di &Position)
        {
            ScrollPos_ = Position;
        }
        
        //! Returns the scroll position.
        inline dim::point2di getScrollPosition() const
        {
            return ScrollPos_;
        }
        
        //! Translates the GUI object.
        inline void translate(const dim::point2di &Direction)
        {
            setPosition(getPosition() + Direction);
        }
        
        //! Transforms the GUI object.
        inline void transform(const dim::size2di &Size)
        {
            setSize(getSize() + Size);
        }
        
        //! Scrolls the GUI object (for more information see setScrollPosition).
        inline void scroll(const dim::point2di &Direction)
        {
            setScrollPosition(getScrollPosition() + Direction);
        }
        
        //! Sets the minimal GUI object's size.
        inline void setMinSize(const dim::size2di &Size)
        {
            MinSize_ = Size;
        }
        inline dim::size2di getMinSize() const
        {
            return MinSize_;
        }
        
        //! Sets the maximal GUI object's size.
        inline void setMaxSize(const dim::size2di &Size)
        {
            MaxSize_ = Size;
        }
        inline dim::size2di getMaxSize() const
        {
            return MaxSize_;
        }
        
        //! Sets the GUI object's order for the depth sort process.
        inline void setOrder(s32 Order)
        {
            Order_ = Order;
        }
        inline s32 getOrder() const
        {
            return Order_;
        }
        
        //! Sets the visibility of the GUI object.
        inline void setVisible(bool isVisible)
        {
            isVisible_ = isVisible;
        }
        inline bool getVisible() const
        {
            return isVisible_;
        }
        
        //! Returns true if this GUI object is in the foreground.
        inline bool hasForeground() const
        {
            return isForeground_;
        }
        
        //! Returns the GUI object's flags.
        inline s32 getFlags() const
        {
            return Flags_;
        }
        
    protected:
        
        friend class GUIManager;
        
        /* === Macros === */
        
        static const s32 SCROLLBAR_SIZE = 15;
        
        /* === Functions === */
        
        GUIController(const EGUIControllerTypes Type);
        
        void updateChildren();
        void drawChildren();
        
        void updateClipping();
        bool setupClipping();
        bool checkClipping() const;
        void updateRect();
        bool checkDefaultUpdate();
        
        virtual void addChild(GUIController* Child);
        virtual void removeChild(GUIController* Child);
        
        void useFocus(s32 Usage = 0);
        bool usage(s32 Usage) const;
        bool usage() const;
        bool foreignUsage() const;
        
        dim::rect2di getParentViewArea() const;
        
        void updateScrollBars(GUIScrollbarGadget* HorzScroll, GUIScrollbarGadget* VertScroll);
        
        virtual void checkFlags();
        
        /* === Members === */
        
        EGUIControllerTypes Type_;
        
        GUIController* Parent_;
        std::list<GUIController*> Children_;
        
        s32 Order_;
        dim::rect2di RootRect_, Rect_, VisRect_;
        dim::size2di MinSize_, MaxSize_;
        dim::point2di ScrollPos_;
        
        bool isVisible_;
        bool isValidated_;
        bool isForeground_;
        
        s32 Flags_;
        
        s32 FocusUsage_;
        bool Usable_;
        
    private:
        
        /* Functions */
        
        void init();
        void clear();
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
