/*
 * SoftPixelDevice header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SOFTPIXELDEVICE_H__
#define __SP_SOFTPIXELDEVICE_H__


#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"
#include "Base/spInputOutput.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spRenderContext.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "SceneGraph/spSceneGraphSimple.hpp"
#include "SceneGraph/spSceneGraphSimpleStream.hpp"
#include "SceneGraph/spSceneGraphFamilyTree.hpp"
#include "SceneGraph/Collision/spCollisionDetector.hpp"
#include "SoundSystem/spSoundDevice.hpp"
#include "Platform/spSoftPixelDeviceFlags.hpp"
#include "Framework/Physics/spPhysicsSimulator.hpp"
#include "Framework/Network/spNetworkStructures.hpp"

#if defined(SP_PLATFORM_ANDROID)
#   include "Platform/Android/android_native_app_glue.h"
#endif


namespace sp
{


/*
 * Declarations
 */

namespace gui
{
    class GUIManager;
}

namespace scene
{
    class CollisionGraph;
};

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM
namespace network
{
    class NetworkSystem;
}
#endif

#ifdef SP_COMPILE_WITH_CG
namespace video
{
    class CgShaderContext;
}
#endif


//! SoftPixel Engine device interface.
class SP_EXPORT SoftPixelDevice
{
    
    public:
        
        virtual ~SoftPixelDevice();
        
        /* === Functions === */
        
        //! Returns the render system. There is only one instance.
        video::RenderSystem* getRenderSystem() const;
        /**
        Returns the main render context. There is at least one instance.
        \see createRenderContext
        */
        video::RenderContext* getRenderContext() const;
        
        //! Returns the input controller. There is only one instance.
        io::InputControl* getInputControl() const;
        //! Returns the OS informator. There is only one instance.
        io::OSInformator* getOSInformator() const;
        
        //! Creates a new GUI manager.
        gui::GUIManager* getGUIManager() const;
        
        //! Creates a new sound device.
        audio::SoundDevice* createSoundDevice(const audio::ESoundDevices Type = audio::SOUNDDEVICE_AUTODETECT);
        void deleteSoundDevice(audio::SoundDevice* SoundDevice);
        
        //! \deprecated Since 3.2
        scene::CollisionDetector* getCollisionDetector() const;
        
        #ifdef SP_COMPILE_WITH_PHYSICS
        /**
        Creates a physics simulator.
        \note This is only available if the engine has been compiled while
        the SP_COMPILE_WITH_PHYSICS was defined. Look at "sources/Base/spCompilationOptions.hpp".
        */
        physics::PhysicsSimulator* createPhysicsSimulator(const physics::EPhysicsSimulators Type);
        void deletePhysicsSimulator(physics::PhysicsSimulator* Simulator);
        #endif
        
        #ifdef SP_COMPILE_WITH_NETWORKSYSTEM
        /**
        Creates a network system.
        \note This is only available if the engine has been compiled while
        the SP_COMPILE_WITH_NETWORKSYSTEM was defined. Look at "sources/Base/spCompilationOptions.hpp".
        */
        network::NetworkSystem* createNetworkSystem(const network::ENetworkSystems Type);
        void deleteNetworkSystem(network::NetworkSystem* NetSys);
        #endif
        
        #ifdef SP_COMPILE_WITH_CG
        //! Creates a Cg shader context. Only one instance of this class can be created.
        video::CgShaderContext* createCgShaderContext();
        void deleteCgShaderContext();
        #endif
        
        /**
        Creates a new scene graph. You can also create several scene graphs (using "new" and "delete" manual).
        \param Type: Type of the scene graph. Since version 2.1 you can choose the type of the scene graph.
        By default the straight forwared scene graph is used where each object will be passed through. e.g. for extremely
        large (or rather expansive) scenes you can use a tree hierarchy (e.g. scene::SCENEGRAPH_TREE).
        */
        scene::SceneGraph* createSceneGraph(const scene::ESceneGraphs Type = scene::SCENEGRAPH_SIMPLE);
        void deleteSceneGraph(scene::SceneGraph* SceneGraph);
        
        //! Creates an individual scene graph object.
        template <class T> T* createSceneGraph()
        {
            T* NewSceneGraph = new T();
            
            SceneGraphList_.push_back(NewSceneGraph);
            setActiveSceneGraph(NewSceneGraph);
            
            return NewSceneGraph;
        }
        
        //! Creates a new collision graph for collision detection and resolving.
        scene::CollisionGraph* createCollisionGraph();
        void deleteCollisionGraph(scene::CollisionGraph* CollGraph);
        
        /**
        Creates a new render context which will autimatically share the resources with the main render context
        which can be get by "getRenderContext".
        \param ParentWindow: Specifies a pointer to the parent window (e.g. HWND for Windows) which is to be used
        for the new render context. If this pointer is null a default graphics window will be created.
        \param Resolution: Specifies the graphics resolution and window size if no parent window is specified.
        \param Title: Specifies the title for the graphics window if no parent window is specified.
        \see getRenderContext
        \since Version 3.2
        */
        video::RenderContext* createRenderContext(
            void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title = ""
        );
        
        /**
        Deletes the specified render context.
        \note The main render context can not be deleted with this function.
        It will only be deleted with the whole engine device.
        */
        void deleteRenderContext(video::RenderContext* Context);
        
        /**
        Updates the device settings. This recreated the main window, device- and render context.
        If any settings has changed all resources will automatically updated. If you want to change
        the render system you need to delete the device, create a new one and load all resources again.
        This feature is only supported on desktop platforms like Windows and Linux.
        \param Resolution: Specifies the new device resolution.
        \param ColorDepth: Specifies the new color depth (in bits). By default 32.
        \param isFullscreen: Specifies whether the new device is to be in window mode or fullscreen.
        \param Flags: Specifies the new devices flags.
        \param ParentWindow: Specifies the new parent window. On Windows this must be a pointer to a HWND instance.
        \return True on success otherwise false.
        */
        virtual bool updateDeviceSettings(
            const dim::size2di &Resolution, s32 ColorDepth = DEF_COLORDEPTH, bool isFullscreen = false,
            const SDeviceFlags &Flags = SDeviceFlags(), void* ParentWindow = 0
        );
        
        /**
        Updates the window event. This functions needs to be called in each program frame. Typically in the
        main "while" loop. It will update the key states and window callback function on a Windows(c) platform.
        \return False if the user clicked the close button in the window title bar. Otherwise true.
        */
        virtual bool updateEvent() = 0;
        
        /**
        Sets the active scene graph. Functions like "draw3DLine" are using the active scene manager.
        \param ActiveSceneGraph: SceneGraph which is to be set to the new active one.
        If this parameter is 0 the default SceneGraph is used.
        */
        void setActiveSceneGraph(scene::SceneGraph* ActiveSceneGraph);
        scene::SceneGraph* getActiveSceneGraph() const;
        
        /**
        Sets the maximal frame rate. Use this feature if vertical-synchronisation does not effect the program speed
        and your application runs too fast (in the case of 500 FPS and higher).
        \param FrameRate: Count of frames per second. If you want to disable maxial frame rate to let the program
        run with full speed set it to 0.
        */
        virtual void setFrameRate(u32 FrameRate = DEF_FRAMERATE);
        
        //! \return SoftPixel Engine version (e.g. "SoftPixel Engine - v.2.1").
        virtual io::stringc getVersion() const;
        
        //! Occurs a beep sound with the specified frequency and for the spcified duration.
        virtual void beep(u32 Milliseconds = 100, u32 Frequency = 1000);
        
        //! Returns string of the last entered characters. Use this as a direct text input control.
        virtual io::stringc getUserCharList() const;
        
        /**
        Manipulates the internal screen size (or rather resolution) values.
        \warning Should only be used by an experienced "SoftPixel Engine" developer!
        */
        virtual void manipulateScreenSize(const dim::size2di &ScreenSize);
        
        /* === Inline functions === */
        
        //! Returns the screen resolution. Only the screen from the main context is taken into account.
        inline dim::size2di getResolution() const
        {
            return Resolution_;
        }
        
        //! Returns the device flags.
        inline SDeviceFlags getFlags() const
        {
            return Flags_;
        }
        
    protected:
        
        /* === Functions === */
        
        SoftPixelDevice(
            const video::ERenderSystems RendererType, const dim::size2di &Resolution,
            s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags
        );
        
        virtual void autoDetectRenderSystem();
        virtual bool checkRenderSystem(const video::ERenderSystems Type) const;
        
        virtual void createRenderSystemAndContext();
        virtual video::RenderSystem* allocRenderSystem();
        virtual video::RenderContext* allocRenderContext();
        
        virtual void deleteResourceDevices();
        
        virtual void printConsoleHeader();
        
        virtual void resetCursorSpeedLock();
        
        #ifdef SP_COMPILE_WITH_SOUNDSYSTEM
        audio::SoundDevice* allocSoundDevice(audio::ESoundDevices DeviceType) const;
        #endif
        
        /* === Members === */
        
        video::ERenderSystems RendererType_;
        dim::size2di Resolution_;
        s32 ColorDepth_;
        bool isFullscreen_;
        SDeviceFlags Flags_;
        
        u32 FrameRate_;
        
        scene::SceneGraph* DefaultSceneManager_;
        
        std::list<video::RenderContext*> RenderContextList_;
        std::list<audio::SoundDevice*> SoundDeviceList_;
        std::list<scene::SceneGraph*> SceneGraphList_;
        std::list<scene::CollisionGraph*> CollGraphList_;
        
        #ifdef SP_COMPILE_WITH_PHYSICS
        std::list<physics::PhysicsSimulator*> PhysicsSimulatorList_;
        #endif
        #ifdef SP_COMPILE_WITH_NETWORKSYSTEM
        std::list<network::NetworkSystem*> NetworkSystemList_;
        #endif
        
};


/*
 * Global functions
 */

/**
Creates the SoftPixelDevice. This is always the first function you need to call when you are creating
a graphics application with this 3D engine.
\param RendererType: Specifies the type for renderer (e.g. OpenGL, Direct3D9 etc.).
You can also let the engine detect the best renderer for the running system (RENDERER_AUTODETECT).
\param Resolution: Specifies the screen resolution. When you use fullscreen mode you have to use
a conventional screen resolution such as 640x480, 800x600, ..., 1920x1080 etc.
\param ColorDepth: Specifies the color depth in bits. Valid values are 16, 24 or 32.
This option is only used for fullscreen mode. By default 32.
\param Title: Specifies the window title.
\param isFullscreen: Specifies whether fullscreen mode shall be used or not.
\param Flags: Specifies additional flags (or rather options) such as if vsync (vertical-synchronization) shall be
activated, anti-aliasing settings etc.
\param ParentWindow: This optional parameter can be used for GUI applications such as a 3D world editor
where you need a 3D graphics context in your window program.
The given type is OS dependent. For windows you need to pass a HWND pointer (e.g. "HWND hWnd ...; -> &hWnd").
*/
#if defined(SP_PLATFORM_ANDROID)
SP_EXPORT SoftPixelDevice* createGraphicsDevice(
    android_app* App, const video::ERenderSystems RendererType, io::stringc Title = "", const bool isFullscreen = false
);
#elif defined(SP_PLATFORM_IOS)
SP_EXPORT SoftPixelDevice* createGraphicsDevice(
    const video::ERenderSystems RendererType, io::stringc Title = "", const bool isFullscreen = false
);
#else
SP_EXPORT SoftPixelDevice* createGraphicsDevice(
    const video::ERenderSystems RendererType, const dim::size2di &Resolution,
    const s32 ColorDepth = DEF_COLORDEPTH, io::stringc Title = "", const bool isFullscreen = false,
    const SDeviceFlags &Flags = SDeviceFlags(), void* ParentWindow = 0
);
#endif

//! Delete the SoftPixelDevice.
SP_EXPORT void deleteDevice();


} // /namespace sp


#endif



// ================================================================================
