/*
 * GUI basic object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_BASICOBJECT_H__
#define __SP_GUI_BASICOBJECT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "RenderSystem/spRenderSystem.hpp"

#include <boost/function.hpp>


namespace sp
{
namespace gui
{


class GUIWindow;
class GUIMenuItem;
class GUIController;
class GUIGadget;

enum EGUIEventObjects
{
    EVENT_WINDOW,
    EVENT_GADGET,
    EVENT_MENUITEM,
};

enum EGUIEventTypes
{
    /* General events */
    EVENT_ACTIVATE,
    EVENT_FOCUS,
    EVENT_LOSTFOCUS,
    EVENT_LEFTCLICK,
    EVENT_LEFTDOUBLECLICK,
    EVENT_RIGHTCLICK,
    EVENT_RIGHTDOUBLECLICK,
    EVENT_KEYPRESS,
    EVENT_MOUSEWHEEL,
    
    /* Window events */
    EVENT_MINIMIZE,
    EVENT_MAXIMIZE,
    EVENT_MOVE,
    EVENT_RESIZE,
    EVENT_CLOSE,
};

struct SGUIEvent
{
    SGUIEvent() :
        Object  (EVENT_WINDOW   ),
        Type    (EVENT_ACTIVATE ),
        Window  (0              ),
        MenuItem(0              ),
        SubData (0              ),
        KeyCode (io::KEY_RETURN )
    {
    }
    ~SGUIEvent()
    {
    }
    
    /* Members */
    
    EGUIEventObjects Object;
    EGUIEventTypes Type;
    
    GUIWindow* Window;
    GUIGadget* Gadget;
    GUIMenuItem* MenuItem;
    
    void* SubData;
    
    io::EKeyCodes KeyCode;
};

typedef boost::function<void (const SGUIEvent &Event)> GUIEventCallback;


class SP_EXPORT GUIBaseObject
{
    
    public:
        
        virtual ~GUIBaseObject();
        
        /* Functions */
        
        virtual void setFont(video::Font* TextFont);
        
        /* Inline functions */
        
        inline void setID(u32 ID)
        {
            ID_ = ID;
        }
        inline u32 getID() const
        {
            return ID_;
        }
        
        inline void setText(const io::stringc &Text)
        {
            Text_ = Text;
        }
        inline io::stringc getText() const
        {
            return Text_;
        }
        
        inline void setColor(const video::color &Color)
        {
            Color_ = Color;
        }
        inline video::color getColor() const
        {
            return Color_;
        }
        
        inline void setEnable(bool isEnabled)
        {
            isEnabled_ = isEnabled;
        }
        inline bool getEnable() const
        {
            return isEnabled_;
        }
        
        inline video::Font* getFont() const
        {
            return Font_;
        }
        
    protected:
        
        friend class GUIManager;
        
        /* === Enumerations === */
        
        enum EDrawTextFlags
        {
            DRAWTEXT_CENTER     = 0x01,
            DRAWTEXT_VCENTER    = 0x02,
            DRAWTEXT_RIGHT      = 0x04,
            DRAWTEXT_3D         = 0x08,
        };
        
        /* === Functions === */
        
        GUIBaseObject();
        
        void drawFrame(
            const dim::rect2di &Rect, const video::color &Color, bool isFrame3D
        );
        void drawFrame(
            const dim::rect2di &Rect,
            const video::color &ColorA, const video::color &ColorB,
            const video::color &ColorC, const video::color &ColorD
        );
        void drawText(
            dim::point2di Pos, const io::stringc &Text, const video::color &Color = 255, s32 Flags = DRAWTEXT_3D
        );
        void drawButton(
            const dim::rect2di &Rect, const video::color &Color = 255, bool isMouseOver = false
        );
        
        void drawHatchedFace(const dim::rect2di &Rect);
        
        bool mouseOver(const dim::rect2di &Rect) const;
        bool mouseLeft() const;
        bool mouseRight() const;
        bool mouseLeftUp() const;
        bool mouseRightUp() const;
        bool mouseLeftDown() const;
        bool mouseRightDown() const;
        
        void sendEvent(const EGUIEventObjects ObjectType, const EGUIEventTypes EventType);
        void sendEvent(const SGUIEvent &Event);
        
        /* Members */
        
        u32 ID_;
        
        io::stringc Text_;
        video::color Color_;
        video::Font* Font_;
        
        bool isEnabled_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
