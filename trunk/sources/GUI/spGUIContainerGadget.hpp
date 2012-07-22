/*
 * GUI container gadget header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_CONTAINERGADGET_H__
#define __SP_GUI_CONTAINERGADGET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIGadget.hpp"


namespace sp
{
namespace gui
{


typedef void (*PFNGUIOWNERDRAWPROC)(GUIController* Controller);


class SP_EXPORT GUIContainerGadget : public GUIGadget
{
    
    public:
        
        GUIContainerGadget();
        ~GUIContainerGadget();
        
        bool update();
        void draw();
        
        /* Inline functions */
        
        inline void setOwnerDrawCallback(PFNGUIOWNERDRAWPROC Callback)
        {
            OwnerDrawProc_ = Callback;
        }
        
    private:
        
        /* Members */
        
        PFNGUIOWNERDRAWPROC OwnerDrawProc_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
