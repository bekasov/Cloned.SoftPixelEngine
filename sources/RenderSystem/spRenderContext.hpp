/*
 * Render context header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERCONTEXT_H__
#define __SP_RENDERCONTEXT_H__


#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"
#include "Base/spInputOutputString.hpp"
#include "Platform/spSoftPixelDeviceFlags.hpp"
#include "RenderSystem/spSharedRenderContext.hpp"


namespace sp
{
namespace video
{


//! Render context class which holds the graphics context objects and the window objects.
class SP_EXPORT RenderContext
{
    
    public:
        
        virtual ~RenderContext();
        
        /* === Functions === */
        
        virtual bool openGraphicsScreen(
            void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title,
            s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags
        ) = 0;
        virtual void closeGraphicsScreen() = 0;
        
        /**
        Flips the buffers to see the final image onto the monitor.
        If vsync (vertical synchronisation) is enabled the program waits at this point until the graphics
        card releases the synchronisation.
        */
        virtual void flipBuffers() = 0;
        
        //! Activates the render context by making it to the current.
        virtual bool activate() = 0;
        
        //! Deactivates the render context.
        virtual bool deactivate() = 0;
        
        //! Sets the window title.
        virtual void setWindowTitle(const io::stringc &Title);
        
        //! Returns the window title.
        virtual io::stringc getWindowTitle() const;
        
        //! Sets the window position. By default the window is screen centered.
        virtual void setWindowPosition(const dim::point2di &Position);
        
        //! Returns the window position.
        virtual dim::point2di getWindowPosition() const;
        
        //! Returns the window size.
        virtual dim::size2di getWindowSize() const;
        
        //! Returns the window border size.
        virtual dim::size2di getWindowBorder() const;
        
        //! Returns true if the window is the active one.
        virtual bool isWindowActive() const;
        
        //! Returns the window object. For Windows it's a HWND* and on Linux it's a Window*.
        virtual void* getWindowObject() = 0;
        
        /**
        Creates a shared render context. Use shared render contexts when creating renderer resources
        in multi-threading. e.g. when you want to load textures on the fly in another thread.
        \param Window: Specifies a pointer to the window (e.g. HWND for Windows) from which
        the device-context is to be used. By default 0 which means that no separate window is used.
        \return Pointer to the new SharedRenderContext object.
        */
        virtual SharedRenderContext* createSharedContext();
        
        //! Deletes the specified shared render context.
        virtual void deleteSharedContext(SharedRenderContext* SharedContext);
        
        //! Enables or disables fullscreen mode.
        virtual void setFullscreen(bool Enable);
        
        /* === Static functiosn === */
        
        //! Returns a pointer to the the active render context.
        static RenderContext* getActiveRenderContext();
        
        /**
        Sets the new render context manually. This is used internally for state changes only.
        The render system's state will not change.
        */
        static void setActiveRenderContext(RenderContext* Context);
        
        /* === Inline functions === */
        
        /**
        Returns true if fullscreen mode is enabled. Otherwise false.
        \see setFullscreen
        */
        inline bool getFullscreen() const
        {
            return isFullscreen_;
        }
        //! Returns the color depth. This can not be changed.
        inline s32 getColorDepth() const
        {
            return ColorDepth_;
        }
        //! Returns the screen resolution. This can not be changed.
        inline dim::size2di getResolution() const
        {
            return Resolution_;
        }
        //! Returns the device flags. This can not be changed.
        inline SDeviceFlags getFlags() const
        {
            return Flags_;
        }
        
    protected:
        
        /* === Functions === */
        
        RenderContext();
        
        void resetConfig();
        
        /* === Members === */
        
        void* ParentWindow_;
        
        dim::size2di Resolution_;
        s32 ColorDepth_;
        bool isFullscreen_;
        io::stringc Title_;
        SDeviceFlags Flags_;
        
        std::list<SharedRenderContext*> SharedContextList_;
        
        static RenderContext* ActiveRenderContext_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
