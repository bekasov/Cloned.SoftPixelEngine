/*
 * GUI manager header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_MANAGER_H__
#define __SP_GUI_MANAGER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "RenderSystem/spRenderSystem.hpp"
#include "GUI/spGUIWindow.hpp"


namespace sp
{
namespace gui
{


/**
Main class for managing the whole GUI.
\ingroup group_gui
*/
class SP_EXPORT GUIManager
{
    
    public:
        
        GUIManager();
        ~GUIManager();
        
        //! Returns the GUI version (e.g. "SoftGUI 2.0 alpha").
        io::stringc getVersion() const;
        
        //! Updates the whole GUI events. Use the event callback (setEventCallback) to catch all events.
        void update();
        
        GUIWindow* addWindow(const dim::point2di &Position, const dim::size2di &Size, const io::stringc &Title, s32 Flags = 0);
        void removeWindow(GUIWindow* Window);
        
        /* Inline functions */
        
        inline void setEventCallback(const GUIEventCallback &Callback)
        {
            EventCallback_ = Callback;
        }
        
        //! Returns the rendered interface texture. Use this texture to draw the GUI in 2D or 3D onto the screen.
        inline video::Texture* getInterfaceTexture() const
        {
            return TexInterface_;
        }
        
    private:
        
        friend class GUIBaseObject;
        friend class GUIController;
        friend class GUIWindow;
        friend class GUIMenuItem;
        friend class GUIScrollbarGadget;
        friend class GUIStringGadget;
        friend class GUITrackbarGadget;
        friend class GUITreeGadget;
        friend class GUIListGadget;
        friend class GUIWebGadget;
        
        /* === Functions === */
        
        void createHatchedFace();
        void createArrowTex();
        
        void addController(GUIController* Controller, bool isParent);
        void removeController(GUIController* Controller, bool isParent);
        void removeParentController(GUIController* Controller);
        
        /* === Members === */
        
        std::list<GUIController*> ControllerList_, ParentControllerList_;
        
        video::Texture* TexInterface_;
        GUIEventCallback EventCallback_;
        
        GUIController* FocusedController_;
        bool FocusUsing_;
        
        dim::point2di CursorPos_, CursorSpeed_;
        s32 MouseWheel_;
        u64 Time_;
        io::stringc InputStr_;
        
        video::Texture* HatchedFace_;
        video::Texture* ArrowTex_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
