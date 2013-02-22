/*
 * SoftPixel Device Windows file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Platform/spSoftPixelDeviceWindows.hpp"

#if defined(SP_PLATFORM_WINDOWS)


#include "RenderSystem/OpenGL/spOpenGLRenderSystem.hpp"
#include "RenderSystem/Direct3D9/spDirect3D9RenderSystem.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"
#include "RenderSystem/spDummyRenderSystem.hpp"
#include "RenderSystem/spDesktopRenderContext.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Base/spSharedObjects.hpp"
#include "GUI/spGUIManager.hpp"

#include <boost/foreach.hpp>

#ifdef SP_COMPILER_GCC
#   include <unistd.h>
#endif


namespace sp
{


/*
 * Internal members
 */

extern SoftPixelDevice*             __spDevice;
extern video::RenderSystem*         __spVideoDriver;
extern video::RenderContext*        __spRenderContext;
extern scene::SceneGraph*           __spSceneManager;
extern io::InputControl*            __spInputControl;
extern io::OSInformator*            __spOSInformator;
extern gui::GUIManager*             __spGUIManager;

static const io::stringc DEVICE_ERROR_OPENSCREEN = "Could not open graphics screen";


/*
 * SoftPixel Engine device class
 */

SoftPixelDeviceWin32::SoftPixelDeviceWin32(
    const video::ERenderSystems RendererType, const dim::size2di &Resolution, s32 ColorDepth,
    const io::stringc &Title, bool isFullscreen, const SDeviceFlags &Flags, void* ParentWindow) :
    SoftPixelDevice(
        RendererType, Resolution, ColorDepth, isFullscreen, Flags
    ),
    isWindowOpened_(true)
{
    if (isFullscreen)
        __spInputControl->setCursorVisible(false);
    
    /* Create render context and renderer system */
    if (!createRenderSystemAndContext())
        throw io::stringc("Creating render-system failed");
    
    static_cast<video::DesktopRenderContext*>(__spRenderContext)->registerWindowClass();
    
    if (!__spRenderContext->openGraphicsScreen(ParentWindow, Resolution, Title, ColorDepth, isFullscreen, Flags))
        throw DEVICE_ERROR_OPENSCREEN;
    
    /* Setup render system */
    __spVideoDriver->DeviceContext_ = static_cast<video::DesktopRenderContext*>(__spRenderContext)->DeviceContext_;
    
    if (Flags.isAntiAlias)
        __spVideoDriver->setAntiAlias(true);
    
    __spVideoDriver->setupConfiguration();
    __spVideoDriver->createDefaultResources();
    
    __spRenderContext->setVsync(Flags_.isVsync);
    
    video::RenderContext::setActiveRenderContext(__spRenderContext);
    
    /* Setup initial cursor position */
    __spInputControl->setupInitialCursorPosition();
    
    /* Print console header */
    printConsoleHeader();
}
SoftPixelDeviceWin32::~SoftPixelDeviceWin32()
{
    if (!isWindowOpened_)
        return;
    
    isWindowOpened_ = false;
    
    /* Remove all previously added font resources */
    foreach (const io::stringc &Filename, FontResources_)
        RemoveFontResource(Filename.c_str());
    
    /* Delete all textures before deleting the render context */
    __spVideoDriver->clearTextureList();
    __spVideoDriver->clearBuffers();
    __spVideoDriver->deleteDefaultResources();
    
    /* Delete all resources devices: scene graphs, sub-systems etc. */
    deleteResourceDevices();

    /* Close all graphics windows, delete contexts and unregister the window class */
    foreach (video::RenderContext* Context, RenderContextList_)
        Context->closeGraphicsScreen();
    
    __spRenderContext->closeGraphicsScreen();
    
    static_cast<video::DesktopRenderContext*>(__spRenderContext)->unregisterWindowClass();
    
    /* Release final graphics context */
    releaseGraphicsContext();
}

bool SoftPixelDeviceWin32::updateDeviceSettings(
    const dim::size2di &Resolution, s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags, void* ParentWindow)
{
    #if 0
    //!TODO!
    
    /* Clear old device */
    __spRenderContext->closeGraphicsScreen();
    
    //todo
    
    /* Create new device */
    if (!__spRenderContext->openGraphicsScreen(ParentWindow, Resolution, Title_, ColorDepth, isFullscreen, Flags))
    {
        io::Log::error(DEVICE_ERROR_OPENSCREEN);
        return false;
    }
    
    //todo
    
    #endif
    
    #ifdef SP_DEBUGMODE
    io::Log::debug("SoftPixelDeviceWin32::updateDeviceSettings", "Not yet implemented and deprecated");
    #endif
    
    return false;
}

bool SoftPixelDeviceWin32::updateEvents()
{
    /* Update base events */
    updateBaseEvents();
    
    /* Framerate delay */
    if (FrameRate_ > 0)
        Sleep(static_cast<DWORD>(FrameRate_));
    
    /* Update window event */
    while (PeekMessage(&Message_, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Message_);
        DispatchMessage(&Message_);
    }
    
    return isWindowOpened_;
}

void SoftPixelDeviceWin32::beep(u32 Milliseconds, u32 Frequency)
{
    Beep(Frequency, Milliseconds);
}

s32 SoftPixelDeviceWin32::registerFontResource(const io::stringc &Filename)
{
    if (!MemoryManager::hasElement(FontResources_, Filename))
    {
        FontResources_.push_back(Filename);
        
        const s32 Count = AddFontResource(Filename.c_str());
        
        if (!Count)
            io::Log::error("Could not register font resource: \"" + Filename + "\"");
        
        return Count;
    }
    return 0;
}

void SoftPixelDeviceWin32::unregisterFontResource(const io::stringc &Filename)
{
    std::list<io::stringc>::iterator it = std::find(FontResources_.begin(), FontResources_.end(), Filename);
    
    if (it != FontResources_.end())
    {
        if (!RemoveFontResource(it->c_str()))
            io::Log::error("Could not unregister font resource: \"" + *it + "\"");
        FontResources_.erase(it);
    }
}

io::stringc SoftPixelDeviceWin32::getDropFilename()
{
    if (DropFilename_.size())
    {
        const io::stringc Str(DropFilename_);
        DropFilename_ = "";
        return Str;
    }
    return "";
}


} // /namespace sp


#endif



// ================================================================================
