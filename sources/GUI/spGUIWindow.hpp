/*
 * GUI window header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_WINDOW_H__
#define __SP_GUI_WINDOW_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIController.hpp"
#include "GUI/spGUIMenuItem.hpp"
#include "GUI/spGUIButtonGadget.hpp"
#include "GUI/spGUICheckboxGadget.hpp"
#include "GUI/spGUIContainerGadget.hpp"
#include "GUI/spGUIListGadget.hpp"
#include "GUI/spGUIProgressbarGadget.hpp"
#include "GUI/spGUIScrollbarGadget.hpp"
#include "GUI/spGUIStringGadget.hpp"
#include "GUI/spGUITrackbarGadget.hpp"
#include "GUI/spGUITreeGadget.hpp"
#include "GUI/spGUITextGadget.hpp"
#include "GUI/spGUIWebGadget.hpp"
#include "GUI/spGUIScrollViewBased.hpp"


namespace sp
{
namespace gui
{


enum EWindowFlags
{
    GUIFLAG_CLOSEBUTTON = 0x00000001, //!< Creates a close button.
    GUIFLAG_MINBUTTON   = 0x00000002, //!< Creates a minimize button.
    GUIFLAG_MAXBUTTON   = 0x00000004, //!< Creates a maximize button.
    GUIFLAG_SIZEBUTTON  = 0x00000008, //!< Creates a size (or rather resize) button.
    GUIFLAG_VSCROLL     = 0x00000010, //!< Creates a vertical scroll bar.
    GUIFLAG_HSCROLL     = 0x00000020, //!< Creates a horizontal scroll bar.
};


/**
Main GUI window class.
\ingroup group_gui
*/
class SP_EXPORT GUIWindow : public GUIController, public GUIScrollViewBased
{
    
    public:
        
        GUIWindow();
        ~GUIWindow();
        
        /* === Functions === */
        
        bool update();
        void draw();
        
        dim::rect2di getLocalViewArea(const GUIController* Obj = 0) const;
        dim::point2di getViewOrigin() const;
        
        void setSize(const dim::size2di &Size);
        
        //! Closes the window.
        void close();
        
        //! Maximizes the window.
        void maximize();
        
        //! Minimizes the window.
        void minimize();
        
        //! Normalizes the window (when it was minimized or maximized before).
        void normalize();
        
        //! Sets the main menu of the window. If 0 the last menu will be removed.
        void setMenuRoot(GUIMenuItem* MenuRoot);
        
        GUIButtonGadget* addButtonGadget(
            const dim::point2di &Position, const dim::size2di &Size, const io::stringc &Text = "", s32 Flags = 0
        );
        GUICheckboxGadget* addCheckboxGadget(
            const dim::point2di &Position, const dim::size2di &Size, const io::stringc &Text = "", s32 Flags = 0
        );
        GUIContainerGadget* addContainerGadget(
            const dim::point2di &Position, const dim::size2di &Size, s32 Flags = 0
        );
        GUIListGadget* addListGadget(
            const dim::point2di &Position, const dim::size2di &Size, s32 Flags = 0
        );
        GUIProgressbarGadget* addProgressbarGadget(
            const dim::point2di &Position, const dim::size2di &Size, s32 Range = 100, s32 Flags = 0
        );
        GUIScrollbarGadget* addScrollbarGadget(
            const dim::point2di &Position, const dim::size2di &Size, s32 Range = 100, s32 Flags = 0
        );
        GUIStringGadget* addStringGadget(
            const dim::point2di &Position, const dim::size2di &Size, const io::stringc &Text = "", s32 Flags = 0
        );
        GUITextGadget* addTextGadget(
            const dim::point2di &Position, const dim::size2di &Size, const io::stringc &Text = "", s32 Flags = 0
        );
        GUITrackbarGadget* addTrackbarGadget(
            const dim::point2di &Position, const dim::size2di &Size, s32 MinRange = 0, s32 MaxRange = 100, s32 Flags = 0
        );
        GUITreeGadget* addTreeGadget(
            const dim::point2di &Position, const dim::size2di &Size, s32 Flags = 0
        );
        GUIWebGadget* addWebGadget(
            const dim::point2di &Position, const dim::size2di &Size, s32 Flags = 0
        );
        
        /* === Inline functions === */
        
        //! Returns pointer to a GUIMenuItem object which is currently used as main menu entry point.
        inline GUIMenuItem* getMenuRoot() const
        {
            return MenuRoot_;
        }
        
        //! Sets the origin position for each new gadget.
        inline void setGadgetOrigin(const dim::point2di &Position)
        {
            GadgetOrigin_ = Position;
        }
        inline dim::point2di getGadgetOrigin() const
        {
            return GadgetOrigin_;
        }
        
    private:
        
        /* Macros */
        
        static const s32 CAPTION_HEIGHT = 25;
        
        /* Enumerations */
        
        enum EFocusUsages
        {
            USAGE_MOVE,
            USAGE_RESIZE,
            USAGE_MAXBUTTON,
            USAGE_MINBUTTON,
            USAGE_CLOSEBUTTON,
        };
        
        enum EWindowVisStates
        {
            VISSTATE_NORMAL,
            VISSTATE_MINIMIZED,
            VISSTATE_MAXIMIZED,
        };
        
        /* === Functions === */
        
        void drawWindowBackground();
        void drawWindowFrame();
        void drawWindowButtonMin(s32 &PosHorz);
        void drawWindowButtonMax(s32 &PosHorz);
        void drawWindowButtonFrameNormalize(s32 &PosHorz);
        void drawWindowButtonClose(s32 &PosHorz);
        bool drawWindowButton(s32 &PosHorz, const video::color &Color);
        
        void checkFlags();
        
        void updateMenu();
        void clampWindowLocation();
        
        void addController(
            GUIController* NewController, const dim::point2di &Position,
            const dim::size2di &Size, const io::stringc &Text, s32 Flags
        );
        
        /* === Members === */
        
        static video::color CaptionColorA[2];
        static video::color CaptionColorB[2];
        
        EWindowVisStates VisState_;
        dim::point2di VisNormalPos_;
        dim::size2di VisNormalSize_;
        
        GUIMenuItem* MenuRoot_;
        
        dim::point2di GadgetOrigin_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
