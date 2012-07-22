/*
 * GUI gadget header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_GADGET_H__
#define __SP_GUI_GADGET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIController.hpp"


namespace sp
{
namespace gui
{


enum EGUIGadgets
{
    GADGET_BUTTON,      //!< Button gadget (text or image).
    GADGET_CHECKBOX,    //!< Checkbox gadget (only two states: checked or unchecked).
    GADGET_CONTAINER,   //!< Container gadget (to hold other gadgets or to have an owner-draw gadget).
    GADGET_LIST,        //!< List gadget.
    GADGET_PROGRESSBAR, //!< Progressbar gadget.
    GADGET_SCROLLBAR,   //!< Scrollbar gadget (particular for windows and lists).
    GADGET_STRING,      //!< String gadget (for text or number input).
    GADGET_TEXT,        //!< Text gadget (text view only).
    GADGET_TRACKBAR,    //!< Trackbar gadget.
    GADGET_TREE,        //!< Tree gadget (listed items in a tree hierarchy).
    GADGET_WEB,         //!< XML based web view gadget.
    
    /* !PLANED!
    GADGET_COMBOBOX,
    GADGET_CONSOLE,     // <- "courier new" font text based area + vertical scroll bar
    GADGET_FILE,        // <- string + image button ( [file.name][...] )
    GADGET_FRAME,
    GADGET_IMAGE,
    GADGET_SCROLLAREA,
    GADGET_SPLITTER
    GADGET_TAB,
    GADGET_VECTOR,      // <- 3 float number string ( [x][y][z] )
    */
};


/**
In this GUI gadget is the object class for buttons, lists, trackbars etc. all things the user
can comunicate with the program. Its another type of controller.
*/
class SP_EXPORT GUIGadget : public GUIController
{
    
    public:
        
        virtual ~GUIGadget();
        
        virtual bool update() = 0;
        virtual void draw() = 0;
        
        /* Inline functions */
        
        //! Returns the type of the gadget.
        inline EGUIGadgets getGadgetType() const
        {
            return GadType_;
        }
        
    protected:
        
        /* Functions */
        
        GUIGadget(const EGUIGadgets Type);
        
        void drawBgRect(
            const dim::rect2di &DrawRect, const dim::rect2di &MouseRect,
            bool isSwaped = false, bool isAlwaysUsing = false, s32 UsageType = -1
        );
        
        /* Members */
        
        EGUIGadgets GadType_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
