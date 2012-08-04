/*
 * GUI scroll view based header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_SCROLLVIEW_BASED_H__
#define __SP_GUI_SCROLLVIEW_BASED_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIScrollbarGadget.hpp"


namespace sp
{
namespace gui
{


class SP_EXPORT GUIScrollViewBased
{
    
    public:
        
        virtual ~GUIScrollViewBased()
        {
        }
        
        /* Inline functions */
        
        inline GUIScrollbarGadget* getHorzScrollBar()
        {
            return &HorzScroll_;
        }
        inline const GUIScrollbarGadget* getHorzScrollBar() const
        {
            return &HorzScroll_;
        }
        
        inline GUIScrollbarGadget* getVertScrollBar()
        {
            return &VertScroll_;
        }
        inline const GUIScrollbarGadget* getVertScrollBar() const
        {
            return &VertScroll_;
        }
        
    protected:
        
        /* Functions */
        
        GUIScrollViewBased()
        {
            HorzScroll_.setFlags(GUIFLAG_NOSCROLL);
            VertScroll_.setFlags(GUIFLAG_NOSCROLL | GUIFLAG_VERTICAL);
        }
        
        /* Members */
        
        GUIScrollbarGadget HorzScroll_;
        GUIScrollbarGadget VertScroll_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
