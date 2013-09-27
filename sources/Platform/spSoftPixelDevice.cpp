/*
 * SoftPixelDevice file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Base/spSharedObjects.hpp"
#include "Base/spTimer.hpp"
#include "GUI/spGUIManager.hpp"

#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/OpenGL/spOpenGLRenderSystem.hpp"
#include "RenderSystem/OpenGL/spOpenGLRenderContext.hpp"
#include "RenderSystem/OpenGLES/spOpenGLES1RenderSystem.hpp"
#include "RenderSystem/OpenGLES/spOpenGLES2RenderSystem.hpp"
#include "RenderSystem/OpenGLES/spOpenGLESRenderContext.hpp"
#include "RenderSystem/Direct3D9/spDirect3D9RenderSystem.hpp"
#include "RenderSystem/Direct3D9/spDirect3D9RenderContext.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11RenderContext.hpp"
#include "RenderSystem/spDummyRenderSystem.hpp"
#include "RenderSystem/spDesktopRenderContext.hpp"

#include "SceneGraph/spSceneBillboard.hpp"
#include "SceneGraph/spSceneGraphPortalBased.hpp"
#include "SceneGraph/Collision/spCollisionGraph.hpp"

#include "SoundSystem/OpenAL/spOpenALSoundDevice.hpp"
#include "SoundSystem/WinMM/spWinMMSoundDevice.hpp"
#include "SoundSystem/XAudio2/spXAudio2SoundDevice.hpp"
#include "SoundSystem/OpenSLES/spOpenSLESSoundDevice.hpp"
#include "SoundSystem/spDummySoundDevice.hpp"

#include "Framework/Physics/Newton/spNewtonSimulator.hpp"
#include "Framework/Physics/PhysX/spPhysXSimulator.hpp"
#include "Framework/Physics/Bullet/spBulletSimulator.hpp"

#include "Framework/Network/spNetworkSystemUDP.hpp"
#include "Framework/Network/spNetworkSystemTCP.hpp"

#include "Framework/Cg/spCgShaderContext.hpp"


namespace sp
{

namespace scene
{
    extern bool __spLightIDList[MAX_COUNT_OF_SCENELIGHTS];
}


/*
 * Global members
 */

SoftPixelDevice*            GlbEngineDev    = 0;
video::RenderSystem*        GlbRenderSys    = 0;
video::RenderContext*       GlbRenderCtx    = 0;
scene::SceneGraph*          GlbSceneGraph   = 0;
io::InputControl*           GlbInputCtrl    = 0;
io::OSInformator*           GlbPlatformInfo = 0;
audio::SoundDevice*         GlbSoundSys     = 0;

#ifdef SP_COMPILE_WITH_GUI
gui::GUIManager*            GlbGUIMngr      = 0;
#endif


/*
 * SoftPixelDevice class
 */

SoftPixelDevice::SoftPixelDevice(
    const video::ERenderSystems RendererType, const dim::size2di &Resolution,
    s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags) :
    RendererType_       (RendererType   ),
    Resolution_         (Resolution     ),
    ColorDepth_         (ColorDepth     ),
    isFullscreen_       (isFullscreen   ),
    Flags_              (Flags          ),
    FrameRate_          (0              ),
    DefaultSceneManager_(0              )
{
    /* Initialize light ID list */
    memset(scene::__spLightIDList, 0, sizeof(scene::__spLightIDList));
    
    /* Create main input controller */
    GlbInputCtrl                = MemoryManager::createMemory<io::InputControl>("io::InputControl");
    GlbPlatformInfo             = MemoryManager::createMemory<io::OSInformator>("io::OSInformator");
    gSharedObjects.SceneMngr    = MemoryManager::createMemory<scene::SceneManager>("scene::SceneManager");
}
SoftPixelDevice::~SoftPixelDevice()
{
}

video::RenderSystem* SoftPixelDevice::getRenderSystem() const
{
    return GlbRenderSys;
}
video::RenderContext* SoftPixelDevice::getRenderContext() const
{
    return GlbRenderCtx;
}
scene::SceneManager* SoftPixelDevice::getSceneManager() const
{
    return gSharedObjects.SceneMngr;
}

io::InputControl* SoftPixelDevice::getInputControl() const
{
    return GlbInputCtrl;
}
io::OSInformator* SoftPixelDevice::getOSInformator() const
{
    return GlbPlatformInfo;
}

gui::GUIManager* SoftPixelDevice::getGUIManager() const
{
    #ifdef SP_COMPILE_WITH_GUI
    if (!GlbGUIMngr)
        GlbGUIMngr = MemoryManager::createMemory<gui::GUIManager>("gui::GUIManager");
    return GlbGUIMngr;
    #else
    io::Log::error("This engine was not compiled with the GUI");
    return 0;
    #endif
}

audio::SoundDevice* SoftPixelDevice::createSoundDevice(const audio::ESoundDevices Type)
{
    #ifdef SP_COMPILE_WITH_SOUNDSYSTEM
    audio::SoundDevice* NewSoundDevice = allocSoundDevice(Type);
    
    NewSoundDevice->printConsoleHeader();
    
    #if 1//!!!
    GlbSoundSys = NewSoundDevice;
    #endif
    
    SoundDeviceList_.push_back(NewSoundDevice);
    return NewSoundDevice;
    #else
    io::Log::error("This engine was not compiled with the sound system");
    return 0;
    #endif
}

void SoftPixelDevice::deleteSoundDevice(audio::SoundDevice* SoundDevice)
{
    MemoryManager::removeElement(SoundDeviceList_, SoundDevice, true);
}

scene::SceneGraph* SoftPixelDevice::createSceneGraph(const scene::ESceneGraphs Type)
{
    /* Create new scene graph object */
    scene::SceneGraph* NewSceneGraph = 0;
    
    switch (Type)
    {
        #ifdef SP_COMPILE_WITH_SCENEGRAPH_SIMPLE
        case scene::SCENEGRAPH_SIMPLE:
            NewSceneGraph = new scene::SceneGraphSimple();
            break;
        #endif
        
        #ifdef SP_COMPILE_WITH_SCENEGRAPH_SIMPLE_STREAM
        case scene::SCENEGRAPH_SIMPLE_STREAM:
            NewSceneGraph = new scene::SceneGraphSimpleStream();
            break;
        #endif
        
        #ifdef SP_COMPILE_WITH_SCENEGRAPH_FAMILY_TREE
        case scene::SCENEGRAPH_FAMILY_TREE:
            NewSceneGraph = new scene::SceneGraphFamilyTree();
            break;
        #endif
        
        #ifdef SP_COMPILE_WITH_SCENEGRAPH_PORTAL_BASED
        case scene::SCENEGRAPH_PORTAL_BASED:
            NewSceneGraph = new scene::SceneGraphPortalBased();
            break;
        #endif
        
        default:
            io::Log::error("Specified scene graph is not supported or the engine was not compiled with it");
            return 0;
    }
    
    /* Add scene graph to the list */
    SceneGraphList_.push_back(NewSceneGraph);
    setActiveSceneGraph(NewSceneGraph);
    
    return NewSceneGraph;
}

void SoftPixelDevice::deleteSceneGraph(scene::SceneGraph* SceneGraph)
{
    MemoryManager::removeElement(SceneGraphList_, SceneGraph, true);
}

scene::CollisionGraph* SoftPixelDevice::createCollisionGraph()
{
    scene::CollisionGraph* NewCollGraph = MemoryManager::createMemory<scene::CollisionGraph>("scene::CollisionGraph");
    CollGraphList_.push_back(NewCollGraph);
    return NewCollGraph;
}

void SoftPixelDevice::deleteCollisionGraph(scene::CollisionGraph* CollGraph)
{
    MemoryManager::removeElement(CollGraphList_, CollGraph, true);
}

#ifdef SP_COMPILE_WITH_PHYSICS

physics::PhysicsSimulator* SoftPixelDevice::createPhysicsSimulator(const physics::EPhysicsSimulators Type)
{
    physics::PhysicsSimulator* NewSimulator = 0;
    
    try
    {
        switch (Type)
        {
            #ifdef SP_COMPILE_WITH_NEWTON
            case physics::SIMULATOR_NEWTON:
                NewSimulator = MemoryManager::createMemory<physics::NewtonSimulator>("physics::NewtonSimulator");
                break;
            #endif
            
            #ifdef SP_COMPILE_WITH_PHYSX
            case physics::SIMULATOR_PHYSX:
                NewSimulator = MemoryManager::createMemory<physics::PhysXSimulator>("physics::PhysXSimulator");
                break;
            #endif
            
            #ifdef SP_COMPILE_WITH_BULLET
            case physics::SIMULATOR_BULLET:
                NewSimulator = MemoryManager::createMemory<physics::BulletSimulator>("physics::BulletSimulator");
                break;
            #endif
            
            default:
                throw io::stringc("This engine was not compiled with the specified physics simulator");
        }
    }
    catch (const io::stringc &ErrorStr)
    {
        io::Log::error(ErrorStr);
        return 0;
    }
    
    /* Add new physics simulator */
    PhysicsSimulatorList_.push_back(NewSimulator);
    return NewSimulator;
}

void SoftPixelDevice::deletePhysicsSimulator(physics::PhysicsSimulator* Simulator)
{
    MemoryManager::removeElement(PhysicsSimulatorList_, Simulator, true);
}

#endif

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM

network::NetworkSystem* SoftPixelDevice::createNetworkSystem(const network::ENetworkSystems Type)
{
    network::NetworkSystem* NetSys = 0;
    
    switch (Type)
    {
        case network::NETWORK_UDP:
            NetSys = MemoryManager::createMemory<network::NetworkSystemUDP>("network::NetworkSystemUDP");
            break;
            
        case network::NETWORK_TCP:
            NetSys = MemoryManager::createMemory<network::NetworkSystemTCP>("network::NetworkSystemTCP");
            break;
            
        default:
            io::Log::error("This engine was not compiled with the specified network system");
            return 0;
    }
    
    NetworkSystemList_.push_back(NetSys);
    return NetSys;
}

void SoftPixelDevice::deleteNetworkSystem(network::NetworkSystem* NetSys)
{
    MemoryManager::removeElement(NetworkSystemList_, NetSys, true);
}

#endif

#ifdef SP_COMPILE_WITH_CG

video::CgShaderContext* SoftPixelDevice::createCgShaderContext()
{
    if (gSharedObjects.CgContext)
    {
        io::Log::error("Only a single Cg shader context can be created");
        return 0;
    }
    
    gSharedObjects.CgContext = new video::CgShaderContext();
    
    return gSharedObjects.CgContext;
}

void SoftPixelDevice::deleteCgShaderContext()
{
    MemoryManager::deleteMemory(gSharedObjects.CgContext);
}

#endif

video::RenderContext* SoftPixelDevice::createRenderContext(
    void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title)
{
    video::RenderContext* NewRenderContext = allocRenderContext();
    RenderContextList_.push_back(NewRenderContext);
    
    GlbRenderSys->ContextList_.push_back(NewRenderContext);
    
    NewRenderContext->openGraphicsScreen(ParentWindow, Resolution, Title, ColorDepth_, isFullscreen_, Flags_);
    
    return NewRenderContext;
}
void SoftPixelDevice::deleteRenderContext(video::RenderContext* Context)
{
    if (Context && Context != GlbRenderCtx)
    {
        Context->closeGraphicsScreen();
        
        MemoryManager::removeElement(GlbRenderSys->ContextList_, Context);
        MemoryManager::removeElement(RenderContextList_, Context, true);
        
        GlbRenderCtx->activate();
    }
}

bool SoftPixelDevice::updateDeviceSettings(
    const dim::size2di &Resolution, s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags, void* ParentWindow)
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("SoftPixelDevice::updateDeviceSettings", "Not implemented yet (and marked as deprecated)");
    #endif
    return true; // do nothing
}

void SoftPixelDevice::updateBaseEvents()
{
    /* Reset keyboard and mouse events */
    io::InputControl::resetInput();
    
    /* Update global FPS counter */
    io::Timer::updateGlobalFPSCounter();
    
    /* Update base input events */
    GlbInputCtrl->updateBaseEvents();
    
    #ifdef SP_COMPILE_WITH_RENDERSYS_QUERIES
    /* Reset draw call counter */
    video::RenderSystem::resetQueryCounters();
    #endif
}

void SoftPixelDevice::setActiveSceneGraph(scene::SceneGraph* ActiveSceneGraph)
{
    if (ActiveSceneGraph)
    {
        GlbSceneGraph = ActiveSceneGraph;
        if (!DefaultSceneManager_)
            DefaultSceneManager_ = GlbSceneGraph;
    }
    else
        GlbSceneGraph = DefaultSceneManager_;
}
scene::SceneGraph* SoftPixelDevice::getActiveSceneGraph() const
{
    return GlbSceneGraph;
}

void SoftPixelDevice::setFrameRate(u32 FrameRate)
{
    if (FrameRate > 0)
        FrameRate_ = static_cast<u32>(1000.0f / static_cast<f32>(FrameRate + 0.5f));
    else
        FrameRate_ = 0;
}

io::stringc SoftPixelDevice::getVersion() const
{
    return
        io::stringc("SoftPixel Engine")
        + " - v."   + io::stringc(SOFTPIXEL_VERSION_MAJOR)
        + "."       + io::stringc(SOFTPIXEL_VERSION_MINOR)
    #if SOFTPIXEL_VERSION_REVISION
        + "."       + io::stringc(SOFTPIXEL_VERSION_REVISION)
    #endif
    #ifdef SOFTPIXEL_VERSION_STATUS
        + " "       + io::stringc(SOFTPIXEL_VERSION_STATUS)
    #endif
    ;
}

void SoftPixelDevice::beep(u32 Milliseconds, u32 Frequency)
{
    // do nothing
}

void SoftPixelDevice::manipulateScreenSize(const dim::size2di &ScreenSize)
{
    /* Manipulate internal screen size values */
    Resolution_                 = ScreenSize;
    gSharedObjects.ScreenWidth  = ScreenSize.Width;
    gSharedObjects.ScreenHeight = ScreenSize.Height;
    
    GlbRenderSys->setRenderMode(video::RENDERMODE_NONE);
}

s32 SoftPixelDevice::registerFontResource(const io::stringc &Filename)
{
    return 0; // do nothing
}
void SoftPixelDevice::unregisterFontResource(const io::stringc &Filename)
{
    // do nothing
}

io::stringc SoftPixelDevice::getWorkingDir() const
{
    return "";
}


/*
 * ======= Protected: =======
 */

void SoftPixelDevice::autoDetectRenderSystem()
{
    if (RendererType_ == video::RENDERER_AUTODETECT)
    {
        #if defined(SP_PLATFORM_WINDOWS)
        RendererType_ = video::RENDERER_DIRECT3D9;
        #elif defined(SP_PLATFORM_ANDROID) || defined(SP_PLATFORM_IOS)
        RendererType_ = video::RENDERER_OPENGLES1;
        #else
        RendererType_ = video::RENDERER_OPENGL;
        #endif
    }
    
    io::stringc Name;
    
    switch (RendererType_)
    {
        case video::RENDERER_OPENGL:
            Name = "OpenGL"; break;
        case video::RENDERER_OPENGLES1:
            Name = "OpenGL|ES 1"; break;
        case video::RENDERER_OPENGLES2:
            Name = "OpenGL|ES 2"; break;
        case video::RENDERER_DIRECT3D9:
            Name = "Direct3D9"; break;
        case video::RENDERER_DIRECT3D11:
            Name = "Direct3D11"; break;
        default:
            break;
    }
    
    if (!checkRenderSystem(RendererType_))
    {
        if (RendererType_ != video::RENDERER_DIRECT3D11 && checkRenderSystem(video::RENDERER_DIRECT3D11))
        {
            RendererType_ = video::RENDERER_DIRECT3D11;
            io::Log::error(Name + " is not supported; using Direct3D11");
        }
        else if (RendererType_ != video::RENDERER_DIRECT3D9 && checkRenderSystem(video::RENDERER_DIRECT3D9))
        {
            RendererType_ = video::RENDERER_DIRECT3D9;
            io::Log::error(Name + " is not supported; using Direct3D9");
        }
        else if (RendererType_ != video::RENDERER_OPENGL && checkRenderSystem(video::RENDERER_OPENGL))
        {
            RendererType_ = video::RENDERER_OPENGL;
            io::Log::error(Name + " is not supported; using OpenGL");
        }
        else if (RendererType_ != video::RENDERER_OPENGLES2 && checkRenderSystem(video::RENDERER_OPENGLES2))
        {
            RendererType_ = video::RENDERER_OPENGLES2;
            io::Log::error(Name + " is not supported; using OpenGL|ES 2");
        }
        else if (RendererType_ != video::RENDERER_OPENGLES1 && checkRenderSystem(video::RENDERER_OPENGLES1))
        {
            RendererType_ = video::RENDERER_OPENGLES1;
            io::Log::error(Name + " is not supported; using OpenGL|ES 1");
        }
        else
        {
            RendererType_ = video::RENDERER_DUMMY;
            io::Log::error(Name + " is not supported; using Dummy");
        }
    }
}

bool SoftPixelDevice::checkRenderSystem(const video::ERenderSystems Type) const
{
    switch (Type)
    {
        #ifdef SP_COMPILE_WITH_OPENGL
        case video::RENDERER_OPENGL:
            return true;
        #endif
        
        #ifdef SP_COMPILE_WITH_OPENGLES1
        case video::RENDERER_OPENGLES1:
            return true;
        #endif
        
        #ifdef SP_COMPILE_WITH_OPENGLES2
        case video::RENDERER_OPENGLES2:
            return true;
        #endif
        
        #ifdef SP_COMPILE_WITH_DIRECT3D9
        case video::RENDERER_DIRECT3D9:
        {
            c8 SysDir[256];
            GetSystemDirectory(SysDir, 256);
            
            if (io::FileSystem().findFile(io::stringc(SysDir) + "\\d3d9.dll"))
                return true;
        }
        break;
        #endif
        
        #ifdef SP_COMPILE_WITH_DIRECT3D11
        case video::RENDERER_DIRECT3D11:
        {
            c8 SysDir[256];
            GetSystemDirectory(SysDir, 256);
            
            if (io::FileSystem().findFile(io::stringc(SysDir) + "\\d3d11.dll"))
                return true;
        }
        break;
        #endif
        
        case video::RENDERER_DUMMY:
            return true;
        
        default:
            break;
    }
    
    return false;
}

bool SoftPixelDevice::createRenderSystemAndContext()
{
    try
    {
        autoDetectRenderSystem();
        
        GlbRenderSys = allocRenderSystem();
        GlbRenderCtx = allocRenderContext();
        
        GlbRenderSys->ContextList_.push_back(GlbRenderCtx);
        
        return true;
    }
    catch (const std::exception &Err)
    {
        io::Log::error(Err.what());
    }
    return false;
}

video::RenderSystem* SoftPixelDevice::allocRenderSystem()
{
    switch (RendererType_)
    {
        #ifdef SP_COMPILE_WITH_OPENGL
        case video::RENDERER_OPENGL:
            return new video::OpenGLRenderSystem();
        #endif
        #ifdef SP_COMPILE_WITH_OPENGLES1
        case video::RENDERER_OPENGLES1:
            return new video::OpenGLES1RenderSystem();
        #endif
        #ifdef SP_COMPILE_WITH_OPENGLES2
        case video::RENDERER_OPENGLES2:
            return new video::OpenGLES2RenderSystem();
        #endif
        #ifdef SP_COMPILE_WITH_DIRECT3D9
        case video::RENDERER_DIRECT3D9:
            return new video::Direct3D9RenderSystem();
        #endif
        #ifdef SP_COMPILE_WITH_DIRECT3D11
        case video::RENDERER_DIRECT3D11:
            return new video::Direct3D11RenderSystem(Flags_.RendererProfile);
        #endif
        case video::RENDERER_DUMMY:
        default:
            return new video::DummyRenderSystem();
    }
    return 0;
}

video::RenderContext* SoftPixelDevice::allocRenderContext()
{
    switch (RendererType_)
    {
        #ifdef SP_COMPILE_WITH_OPENGL
        case video::RENDERER_OPENGL:
            return new video::OpenGLRenderContext();
        #endif
        #ifdef SP_COMPILE_WITH_OPENGLES1
        case video::RENDERER_OPENGLES1:
            return new video::OpenGLESRenderContext(false);
        #endif
        #ifdef SP_COMPILE_WITH_OPENGLES2
        case video::RENDERER_OPENGLES2:
            return new video::OpenGLESRenderContext(true);
        #endif
        #ifdef SP_COMPILE_WITH_DIRECT3D9
        case video::RENDERER_DIRECT3D9:
            return new video::Direct3D9RenderContext();
        #endif
        #ifdef SP_COMPILE_WITH_DIRECT3D11
        case video::RENDERER_DIRECT3D11:
            return new video::Direct3D11RenderContext();
        #endif
        case video::RENDERER_DUMMY:
        default:
            #if defined(SP_PLATFORM_WINDOWS) || defined(SP_PLATFORM_LINUX)
            return new video::DesktopRenderContext();
            #else
            break;
            #endif
    }
    return 0;
}

void SoftPixelDevice::deleteResourceDevices()
{
    /* Release static resources */
    scene::Billboard::deleteDefaultMeshBuffer();
    
    /* Delete global objects */
    #ifdef SP_COMPILE_WITH_PHYSICS
    MemoryManager::deleteList(PhysicsSimulatorList_);
    #endif
    #ifdef SP_COMPILE_WITH_NETWORKSYSTEM
    MemoryManager::deleteList(NetworkSystemList_);
    #endif
    
    MemoryManager::deleteMemory(GlbInputCtrl);
    MemoryManager::deleteMemory(GlbPlatformInfo);
    
    #ifdef SP_COMPILE_WITH_CG
    MemoryManager::deleteMemory(gSharedObjects.CgContext);
    #endif
    #ifdef SP_COMPILE_WITH_GUI
    MemoryManager::deleteMemory(GlbGUIMngr);
    #endif
    
    MemoryManager::deleteMemory(gSharedObjects.SceneMngr);
    MemoryManager::deleteList(SceneGraphList_);
    MemoryManager::deleteList(CollGraphList_);
    
    #ifdef SP_COMPILE_WITH_SOUNDSYSTEM
    MemoryManager::deleteList(SoundDeviceList_);
    #endif

    MemoryManager::deleteMemory(GlbRenderSys);
}

void SoftPixelDevice::releaseGraphicsContext()
{
    MemoryManager::deleteList(RenderContextList_);
    MemoryManager::deleteMemory(GlbRenderCtx);
}

void SoftPixelDevice::printConsoleHeader()
{
    io::Log::message(getVersion(), 0);                                                          // Engine version
    io::Log::message("Copyright (c) 2008 - Lukas Hermanns", 0);                                 // Copyright
    io::Log::message(GlbPlatformInfo->getOSVersion(), 0);                                      // OS version
    io::Log::message(
        "Compiler: " + GlbPlatformInfo->getCompilerVersion() +                                 // Compiler information
        " on " + io::stringc(__DATE__) + " at " + io::stringc(__TIME__), 0                      // Compilation date and time
    );
    io::Log::message("Renderer: " + GlbRenderSys->getVersion(), 0);                          // Renderer version
    
    if (GlbRenderSys->queryVideoSupport(video::QUERY_SHADER))
        io::Log::message("Shader version: " + GlbRenderSys->getShaderVersion(), 0);          // Shader version
    
    io::Log::message(GlbRenderSys->getRenderer() + ": " + GlbRenderSys->getVendor(), 0);  // Renderer information
    
    GlbRenderSys->printWarning();
    
    io::Log::message("", 0);
}

#ifdef SP_COMPILE_WITH_SOUNDSYSTEM

audio::SoundDevice* SoftPixelDevice::allocSoundDevice(audio::ESoundDevices DeviceType) const
{
    if (DeviceType == audio::SOUNDDEVICE_AUTODETECT)
    {
        #if defined(SP_COMPILE_WITH_OPENAL)
        DeviceType = audio::SOUNDDEVICE_OPENAL;
        #elif defined(SP_COMPILE_WITH_XAUDIO2)
        DeviceType = audio::SOUNDDEVICE_XAUDIO2;
        #elif defined(SP_COMPILE_WITH_WINMM)
        DeviceType = audio::SOUNDDEVICE_WINMM;
        #elif defined(SP_COMPILE_WITH_OPENSLES)
        DeviceType = audio::SOUNDDEVICE_OPENSLES;
        #else
        DeviceType = audio::SOUNDDEVICE_DUMMY;
        #endif
    }
    else
    {
        #ifndef SP_COMPILE_WITH_OPENAL
        if (DeviceType == audio::SOUNDDEVICE_OPENAL)
        {
            io::Log::warning("OpenAL is not supported; using XAudio2");
            DeviceType = audio::SOUNDDEVICE_XAUDIO2;
        }
        #endif
        #ifndef SP_COMPILE_WITH_XAUDIO2
        if (DeviceType == audio::SOUNDDEVICE_XAUDIO2)
        {
            io::Log::warning("XAudio2 is not supported; using WinMM");
            DeviceType = audio::SOUNDDEVICE_WINMM;
        }
        #endif
        #ifndef SP_COMPILE_WITH_WINMM
        if (DeviceType == audio::SOUNDDEVICE_WINMM)
        {
            io::Log::warning("WinMM is not supported; using Dummy");
            DeviceType = audio::SOUNDDEVICE_DUMMY;
        }
        #endif
        #ifndef SP_COMPILE_WITH_OPENSLES
        if (DeviceType == audio::SOUNDDEVICE_OPENSLES)
        {
            io::Log::warning("OpenSL|ES is not supported; using Dummy");
            DeviceType = audio::SOUNDDEVICE_DUMMY;
        }
        #endif
    }
    
    switch (DeviceType)
    {
        #ifdef SP_COMPILE_WITH_OPENAL
        case audio::SOUNDDEVICE_OPENAL:
            return new audio::OpenALSoundDevice();
        #endif
        
        #ifdef SP_COMPILE_WITH_XAUDIO2
        case audio::SOUNDDEVICE_XAUDIO2:
            return new audio::XAudio2SoundDevice();
        #endif
        
        #ifdef SP_COMPILE_WITH_OPENSLES
        case audio::SOUNDDEVICE_OPENSLES:
            return new audio::OpenSLESSoundDevice();
        #endif
        
        #ifdef SP_COMPILE_WITH_WINMM
        case audio::SOUNDDEVICE_WINMM:
            return new audio::WinMMSoundDevice();
        #endif
        
        default:
            break;
    }
    
    return new audio::DummySoundDevice();
}

#endif


/*
 * Global functions
 */

#if defined(SP_PLATFORM_ANDROID)

SP_EXPORT SoftPixelDevice* createGraphicsDevice(
    android_app* App, const video::ERenderSystems RendererType,
    const io::stringc &Title, const bool isFullscreen, const u32 SDKVersion)
{
    if (SDKVersion != SP_SDK_VERSION)
    {
        io::Log::error("Wrong SDK version");
        return 0;
    }
    
    GlbEngineDev = new SoftPixelDeviceAndroid(App, RendererType, Title, isFullscreen);
    
    return GlbEngineDev;
}

#elif defined(SP_PLATFORM_IOS)

SP_EXPORT SoftPixelDevice* createGraphicsDevice(
    const video::ERenderSystems RendererType, const io::stringc &Title,
    const bool isFullscreen, const u32 SDKVersion)
{
    if (SDKVersion != SP_SDK_VERSION)
    {
        io::Log::error("Wrong SDK version");
        return 0;
    }
    
    GlbEngineDev = new SoftPixelDeviceIOS(RendererType, Title, isFullscreen);
    
    return GlbEngineDev;
}

#else

SP_EXPORT SoftPixelDevice* createGraphicsDevice(
    const video::ERenderSystems RendererType, const dim::size2di &Resolution,
    const s32 ColorDepth, const io::stringc &Title, const bool isFullscreen,
    const SDeviceFlags &Flags, void* ParentWindow, const u32 SDKVersion)
{
    if (SDKVersion != SP_SDK_VERSION)
    {
        io::Log::error("Wrong SDK version");
        return 0;
    }
    
    try
    {
        #if defined(SP_PLATFORM_WINDOWS)
        return GlbEngineDev = new SoftPixelDeviceWin32(
            RendererType, Resolution, ColorDepth, Title, isFullscreen, Flags, ParentWindow
        );
        #elif defined(SP_PLATFORM_MACOSX)
        return GlbEngineDev = new SoftPixelDeviceMacOSX(
            RendererType, Resolution, ColorDepth, Title, isFullscreen, Flags
        );
        #elif defined(SP_PLATFORM_LINUX)
        return GlbEngineDev = new SoftPixelDeviceLinux(
            RendererType, Resolution, ColorDepth, Title, isFullscreen, Flags
        );
        #endif
    }
    catch (const io::stringc &ErrorStr)
    {
        io::Log::error(ErrorStr);
        return 0;
    }
}

#endif

SP_EXPORT void deleteDevice()
{
    MemoryManager::deleteMemory(GlbEngineDev);
    
    /* Close the possible debug log file */
    io::Log::close();
}


} // /namespace sp



// ================================================================================
