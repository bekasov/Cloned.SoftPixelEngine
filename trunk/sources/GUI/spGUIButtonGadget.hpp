/*
 * GUI button gadget header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_BUTTONGADGET_H__
#define __SP_GUI_BUTTONGADGET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIGadget.hpp"


namespace sp
{
namespace gui
{


enum EButtonFlags
{
    GUIFLAG_TOGGLE = 0x00000001,
};


/**
Button controller gadget.
\ingroup group_gui
*/
class SP_EXPORT GUIButtonGadget : public GUIGadget
{
    
    public:
        
        GUIButtonGadget();
        ~GUIButtonGadget();
        
        bool update();
        void draw();
        
        /* Inline functions */
        
        inline void setState(bool isToggled)
        {
            isToggled_ = isToggled;
        }
        inline bool getState() const
        {
            return isToggled_;
        }
        
        inline void setImage(video::Texture* Tex)
        {
            Image_ = Tex;
        }
        inline video::Texture* getImage() const
        {
            return Image_;
        }
        
    private:
        
        /* Members */
        
        bool isToggled_;
        video::Texture* Image_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
