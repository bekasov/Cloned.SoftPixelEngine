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

#include <boost/function.hpp>


namespace sp
{
namespace video
{


class RenderContext;

/**
Render context resize callback.
\param[in] Context Pointer to the context which has been resized.
\see RenderContext::setResizeCallback
\since Version 3.3
*/
typedef boost::function<void (RenderContext* Context)> RenderContextResizeCallback;

/**
Window drop file callback.
\param[in] Context Pointer to the context whose window has received the drop file feedback.
\param[in] Filename Specifies the filename of the file which was droped over the window.
\param[in] Index Specifies the index of the current droped file.
\param[in] NumFiles Specifies the number of files which have been droped.
\see video::RenderContext
\since Version 3.3
*/
typedef boost::function<void (RenderContext* Context, const io::stringc &Filename, u32 Index, u32 NumFiles)> DropFileCallback;


//! Render context class which holds the graphics context objects and the window objects.
class SP_EXPORT RenderContext
{
    
    public:
        
        virtual ~RenderContext();
        
        /* === Functions === */
        
        /**
        Opens the graphics screen. This function will be called automatically
        once when the graphics device will be created.
        \param[in] ParentWindow Pointer to the window object. For MS/Windows this must be a pointer to a HWND instance
        or null if no parent window is to be used.
        \param[in] Resolution Specifies the screen resolution for this context.
        \param[in] Title Specifies the window title.
        \param[in] ColorDepth Specifies the color bit depth. Valid values are 16, 24 and 32.
        \param[in] isFullscreen Specifies whether fullscreen is to be enabled or disabled.
        \param[in] Flags Specifies the device flags. This contains information about anti-aliasing and some other options.
        \return True if the graphics screen has been successfully created. Otherwise false.
        */
        virtual bool openGraphicsScreen(
            void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title,
            s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags
        ) = 0;
        //! Closes the graphics screen and releases all hardware specific resources.
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
        
        /**
        Returns the window object. For MS/Windows it's a HWND* and on GNU/Linux it's a Window*.
        \code
        #if defined(SP_PLATFORM_WINDOWS)
        HWND hWnd = *((HWND*)Context->getWindowObject());
        #elif defined(SP_PLATFORM_LINUX)
        Window XWin = *((Window*)Context->getWindowObject());
        #endif
        \endcode
        */
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
        
        //! Enables or disabels vertical synchronisation.
        virtual void setVsync(bool isVsync);
        
        /**
        Changes the screen resolution. This requires for some render sytems to re-create
        all graphics resources such as textures, mesh buffers, shaders etc.
        \return True if the screen resolution could be changed. Otherwise the render system
        does not support dynamic resolution change or an error occured.
        */
        virtual bool setResolution(const dim::size2di &Resolution);
        
        /**
        Returns true if this is the active render context.
        \since Version 3.3
        */
        bool activated() const;
        
        /**
        Calls the resize callback function if used.
        \see setResizeCallback
        \since Version 3.3
        */
        void registerResize();

        /**
        Calls the drop file callback function if used.
        \param[in] Filename Specifies the filename of the file which was droped over the window.
        \param[in] Index Specifies the index of the current droped file.
        \param[in] NumFiles Specifies the number of files which have been droped.
        \see setDropFileCallback
        \since Version 3.3
        */
        void registerDropedFile(const io::stringc &Filename, u32 Index, u32 NumFiles);
        
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
        
        /**
        Returns true if vertical synchronisation is enabled.
        This is equivalent to the following code snippet:
        \code
        spContext->getFlags().isVsync;
        \endcode
        */
        inline bool getVsync() const
        {
            return Flags_.VSync.Enabled;
        }
        
        /**
        Sets the new context resize callback function. Use this to register whenever the window size will changed by the user.
        \see registerResize
        \see RenderContextResizeCallback
        \since Version 3.3
        */
        inline void setResizeCallback(const RenderContextResizeCallback &Callback)
        {
            ResizeCallback_ = Callback;
        }
        
        /**
        Sets the drop file callback.
        \note Drop file feedback is currently only supported for MS/Windows.
        \see DropFileCallback
        \since Version 3.3
        */
        inline void setDropFileCallback(const DropFileCallback &Callback)
        {
            DropFileCallback_ = Callback;
        }

    protected:
        
        /* === Functions === */
        
        RenderContext();
        
        void resetConfig();
        
        void applyResolution() const;
        
        /* === Members === */
        
        void* ParentWindow_;
        
        dim::size2di Resolution_;
        s32 ColorDepth_;
        bool isFullscreen_;
        io::stringc Title_;
        SDeviceFlags Flags_;
        
        std::list<SharedRenderContext*> SharedContextList_;
        RenderContextResizeCallback ResizeCallback_;
        
        static RenderContext* ActiveRenderContext_;
        
    private:

        /* === Members === */

        DropFileCallback DropFileCallback_;

};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
