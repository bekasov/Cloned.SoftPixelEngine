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
extern scene::CollisionDetector*    __spCollisionDetector;
extern io::InputControl*            __spInputControl;
extern io::OSInformator*            __spOSInformator;
extern gui::GUIManager*             __spGUIManager;

io::stringc spUserCharList;

static const c8* DEVICE_ERROR_OPENSCREEN = "Could not open graphics screen";


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
        ShowCursor(FALSE);
    
    /* Create render context and renderer system */
    createRenderSystemAndContext();
    
    static_cast<video::DesktopRenderContext*>(__spRenderContext)->registerWindowClass();
    
    if (!__spRenderContext->openGraphicsScreen(ParentWindow, Resolution, Title, ColorDepth, isFullscreen, Flags))
    {
        io::Log::error(DEVICE_ERROR_OPENSCREEN);
        return;
    }
    
    /* Setup render system */
    __spVideoDriver->DeviceContext_ = static_cast<video::DesktopRenderContext*>(__spRenderContext)->DeviceContext_;
    
    __spVideoDriver->setupConfiguration();
    __spVideoDriver->setVsync(Flags_.isVsync);
    
    /* Print console header */
    printConsoleHeader();
}
SoftPixelDeviceWin32::~SoftPixelDeviceWin32()
{
    SoftPixelDeviceWin32::deleteDevice();
}

void SoftPixelDeviceWin32::deleteDevice()
{
    if (!isWindowOpened_)
        return;
    
    isWindowOpened_ = false;
    
    /* Delete all textures before deleting the render context */
    __spVideoDriver->clearTextureList();
    __spVideoDriver->clearBuffers();
    
    /* Close all graphics windows, delete contexts and unregister the window class */
    foreach (video::RenderContext* Context, RenderContextList_)
        Context->closeGraphicsScreen();
    
    __spRenderContext->closeGraphicsScreen();
    
    static_cast<video::DesktopRenderContext*>(__spRenderContext)->unregisterWindowClass();
    
    deleteResourceDevices();
    
    /* Close the possible debug log file */
    io::Log::close();
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
    
    return false;
}

bool SoftPixelDeviceWin32::updateEvent()
{
    /* Reset keyboard and mouse information */
    memset(__wasKey, 0, sizeof(__wasKey));
    memset(__hitKey, 0, sizeof(__hitKey));
    memset(__wasMouseKey, 0, sizeof(__wasMouseKey));
    memset(__hitMouseKey, 0, sizeof(__hitMouseKey));
    
    gSharedObjects.MouseWheel = 0;
    
    resetCursorSpeedLock();
    
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

io::stringc SoftPixelDeviceWin32::getUserCharList() const
{
    io::stringc Result = spUserCharList;
    spUserCharList = "";
    return Result;
}

void SoftPixelDeviceWin32::beep(u32 Milliseconds, u32 Frequency)
{
    Beep(Frequency, Milliseconds);
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


/*
 * Global functions
 */

SP_EXPORT LRESULT CALLBACK spWindowCallback(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    #ifndef WM_MOUSEWHEEL
    #   define WM_MOUSEWHEEL    0x020A
    #endif
    #ifndef WHEEL_DELTA
    #   define WHEEL_DELTA      120
    #endif
    
    static s32 LastCursorX, LastCursorY;
    
    switch (Message)
    {
        case WM_CLOSE:
        {
            static_cast<SoftPixelDeviceWin32*>(__spDevice)->isWindowOpened_ = false;
            PostQuitMessage(0);
        }
        return 0;
        
        case WM_KEYDOWN:
        {
            const s32 KeyIndex = static_cast<s32>(wParam);
            
            if (!__isKey[KeyIndex])
                __hitKey[KeyIndex] = true;
            __isKey[KeyIndex] = true;
        }
        return 0;
        
        case WM_KEYUP:
        {
            const s32 KeyIndex = static_cast<s32>(wParam);
            
            __wasKey[KeyIndex] = true;
            __isKey[KeyIndex] = false;
        }
        return 0;
        
        case WM_CHAR:
        {
            if (static_cast<u8>(wParam) <= 126)
                spUserCharList += io::stringc((TCHAR)wParam);
        }
        return 0;
        
        case WM_LBUTTONDOWN:
        {
            if (!__isMouseKey[io::MOUSE_LEFT])
                __hitMouseKey[io::MOUSE_LEFT] = true;
            __isMouseKey[io::MOUSE_LEFT] = true;
            
            SetCapture(hWnd);
        }
        return 0;
        
        case WM_LBUTTONUP:
        {
            __wasMouseKey[io::MOUSE_LEFT] = true;
            __isMouseKey[io::MOUSE_LEFT] = false;
            
            ReleaseCapture();
        }
        return 0;
        
        case WM_RBUTTONDOWN:
        {
            if (!__isMouseKey[io::MOUSE_RIGHT])
                __hitMouseKey[io::MOUSE_RIGHT] = true;
            __isMouseKey[io::MOUSE_RIGHT] = true;
            
            SetCapture(hWnd);
        }
        return 0;
        
        case WM_RBUTTONUP:
        {
            __wasMouseKey[io::MOUSE_RIGHT] = true;
            __isMouseKey[io::MOUSE_RIGHT] = false;
            
            ReleaseCapture();
        }
        return 0;
        
        case WM_MBUTTONDOWN:
        {
            if (!__isMouseKey[io::MOUSE_MIDDLE])
                __hitMouseKey[io::MOUSE_MIDDLE] = true;
            __isMouseKey[io::MOUSE_MIDDLE] = true;
            
            SetCapture(hWnd);
        }
        return 0;
        
        case WM_MBUTTONUP:
        {
            __wasMouseKey[io::MOUSE_MIDDLE] = true;
            __isMouseKey[io::MOUSE_MIDDLE] = false;
            
            ReleaseCapture();
        }
        return 0;
        
        case WM_MOUSEMOVE:
        {
            gSharedObjects.CursorSpeedX = LOWORD(lParam) - LastCursorX;
            gSharedObjects.CursorSpeedY = HIWORD(lParam) - LastCursorY;
            LastCursorX = LOWORD(lParam);
            LastCursorY = HIWORD(lParam);
        }
        return 0;
        
        case WM_MOUSEWHEEL:
        {
            gSharedObjects.MouseWheel = static_cast<s16>(
                static_cast<f32>(static_cast<s16>(HIWORD(wParam))) / WHEEL_DELTA
            );
        }
        return 0;
        
        case WM_KILLFOCUS:
        {
            memset(__isKey, 0, sizeof(__isKey));
            
            __wasMouseKey[io::MOUSE_LEFT] = true;
            __isMouseKey[io::MOUSE_LEFT] = false;
            
            ReleaseCapture();
        }
        return 0;
        
        case WM_DROPFILES:
        {
            HDROP hDrop = (HDROP)wParam;
            
            c8 DropFileBuffer[2048];
            
            DragQueryFile(hDrop, 0, DropFileBuffer, sizeof(DropFileBuffer));
            static_cast<SoftPixelDeviceWin32*>(__spDevice)->DropFilename_ = io::stringc(DropFileBuffer);
            DragFinish(hDrop);
        }
        return 0;
        
        case WM_SYSCOMMAND:
        {
            // Prevent for a powersave mode of monitor or the screensaver
            if ((wParam & 0xFFF0) == SC_SCREENSAVE || (wParam & 0xFFF0) == SC_MONITORPOWER)
                return 0;
        }
        break;
    }
    
    return DefWindowProc(hWnd, Message, wParam, lParam);
}


} // /namespace sp


#endif



// ================================================================================
