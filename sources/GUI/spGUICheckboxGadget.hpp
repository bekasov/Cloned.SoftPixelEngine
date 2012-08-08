/*
 * GUI checkbox gadget header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_CHECKBOXGADGET_H__
#define __SP_GUI_CHECKBOXGADGET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIGadget.hpp"


namespace sp
{
namespace gui
{


/**
Checkbox controller gadget.
\ingroup group_gui
*/
class SP_EXPORT GUICheckboxGadget : public GUIGadget
{
    
    public:
        
        GUICheckboxGadget();
        ~GUICheckboxGadget();
        
        bool update();
        void draw();
        
        void setState(bool isChecked);
        void setAutoState(bool isAutoState);
        
        /* Inline functions */
        
        inline bool getState() const
        {
            return isChecked_;
        }
        inline bool getAutoState() const
        {
            return isAutoState_;
        }
        
    private:
        
        /* Inline functions */
        
        inline void updateAutoState()
        {
            if (isAutoState_)
                Text_ = (isChecked_ ? "Enabled" : "Disabled");
        }
        
        /* Members */
        
        bool isChecked_;
        bool isAutoState_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
