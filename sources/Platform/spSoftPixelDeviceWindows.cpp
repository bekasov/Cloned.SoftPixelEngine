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

extern SoftPixelDevice*             GlbEngineDev;
extern video::RenderSystem*         GlbRenderSys;
extern video::RenderContext*        GlbRenderCtx;
extern scene::SceneGraph*           GlbSceneGraph;
extern io::InputControl*            GlbInputCtrl;
extern io::OSInformator*            GlbPlatformInfo;
extern gui::GUIManager*             GlbGUIMngr;

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
        GlbInputCtrl->setCursorVisible(false);
    
    /* Create render context and renderer system */
    if (!createRenderSystemAndContext())
        throw io::stringc("Creating render-system failed");
    
    static_cast<video::DesktopRenderContext*>(GlbRenderCtx)->registerWindowClass();
    
    if (!GlbRenderCtx->openGraphicsScreen(ParentWindow, Resolution, Title, ColorDepth, isFullscreen, Flags))
        throw DEVICE_ERROR_OPENSCREEN;
    
    /* Setup render system */
    GlbRenderSys->DeviceContext_ = static_cast<video::DesktopRenderContext*>(GlbRenderCtx)->DeviceContext_;
    
    if (Flags.AntiAliasing.Enabled)
        GlbRenderSys->setAntiAlias(true);
    
    GlbRenderSys->setupConfiguration();
    GlbRenderSys->createDefaultResources();
    
    GlbRenderCtx->setVsync(Flags_.VSync.Enabled);
    
    video::RenderContext::setActiveRenderContext(GlbRenderCtx);
    
    /* Setup initial cursor position */
    GlbInputCtrl->setupInitialCursorPosition();
    
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
    GlbRenderSys->clearTextureList();
    GlbRenderSys->clearBuffers();
    GlbRenderSys->deleteDefaultResources();
    
    /* Delete all resources devices: scene graphs, sub-systems etc. */
    deleteResourceDevices();

    /* Close all graphics windows, delete contexts and unregister the window class */
    foreach (video::RenderContext* Context, RenderContextList_)
        Context->closeGraphicsScreen();
    
    GlbRenderCtx->closeGraphicsScreen();
    
    static_cast<video::DesktopRenderContext*>(GlbRenderCtx)->unregisterWindowClass();
    
    /* Release final graphics context */
    releaseGraphicsContext();
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

io::stringc SoftPixelDeviceWin32::getWorkingDir() const
{
    /* Get current working directory */
    TCHAR Buffer[MAX_PATH];
    DWORD Len = GetCurrentDirectory(MAX_PATH, Buffer);
    return io::stringc(Buffer);
}


} // /namespace sp


#endif



// ================================================================================
