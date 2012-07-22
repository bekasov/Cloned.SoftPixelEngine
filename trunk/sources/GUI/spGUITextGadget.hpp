/*
 * GUI text gadget header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_TEXTGADGET_H__
#define __SP_GUI_TEXTGADGET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIGadget.hpp"


namespace sp
{
namespace gui
{


class SP_EXPORT GUITextGadget : public GUIGadget
{
    
    public:
        
        GUITextGadget();
        ~GUITextGadget();
        
        bool update();
        void draw();
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
