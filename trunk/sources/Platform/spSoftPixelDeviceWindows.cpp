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
        ShowCursor(FALSE);
    
    /* Create render context and renderer system */
    createRenderSystemAndContext();
    
    static_cast<video::DesktopRenderContext*>(__spRenderContext)->registerWindowClass();
    
    if (!__spRenderContext->openGraphicsScreen(ParentWindow, Resolution, Title, ColorDepth, isFullscreen, Flags))
        throw DEVICE_ERROR_OPENSCREEN;
    
    /* Setup render system */
    __spVideoDriver->DeviceContext_ = static_cast<video::DesktopRenderContext*>(__spRenderContext)->DeviceContext_;
    
    if (Flags.isAntiAlias)
        __spVideoDriver->setAntiAlias(true);
    
    __spVideoDriver->setupConfiguration();
    __spRenderContext->setVsync(Flags_.isVsync);
    
    video::RenderContext::setActiveRenderContext(__spRenderContext);
    
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
    
    /* Close all graphics windows, delete contexts and unregister the window class */
    foreach (video::RenderContext* Context, RenderContextList_)
        Context->closeGraphicsScreen();
    
    __spRenderContext->closeGraphicsScreen();
    
    static_cast<video::DesktopRenderContext*>(__spRenderContext)->unregisterWindowClass();
    
    deleteResourceDevices();
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
    /* Reset keyboard and mouse events */
    io::InputControl::resetInput();
    
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


/*
 * Global functions
 */

static void RecordKeyEvent(u32 KeyCode, bool IsDown)
{
    if (IsDown)
        io::InputControl::keyEventDown(KeyCode);
    else
        io::InputControl::keyEventUp(KeyCode);
}

static void RecordKey(WPARAM wParam, LPARAM lParam, bool IsDown)
{
    /* Extract key code */
    const u32 KeyCode = static_cast<s32>(wParam);
    const u32 OEMCode = static_cast<s32>(lParam & (0xFF << 16)) >> 16;
    
    /* Check for extended keys */
    switch (KeyCode)
    {
        case io::KEY_SHIFT:
        {
            if (OEMCode == 0x36)
                RecordKeyEvent(io::KEY_RSHIFT, IsDown);
            else if (OEMCode == 0x2A)
                RecordKeyEvent(io::KEY_LSHIFT, IsDown);
        }
        break;
        
        case io::KEY_CONTROL:
        {
            if ( ( ( static_cast<u32>(lParam) >> 24 ) & 0x1 ) != 0 )
                RecordKeyEvent(io::KEY_RCONTROL, IsDown);
            else
                RecordKeyEvent(io::KEY_LCONTROL, IsDown);
        }
        break;
    }
    
    /* Record base key event */
    RecordKeyEvent(KeyCode, IsDown);
}

static s32 MouseCaptureCount = 0;

static void CaptureMouseButton(s32 MouseButton, HWND hWnd)
{
    if (!__isMouseKey[MouseButton])
        __hitMouseKey[MouseButton] = true;
    __isMouseKey[MouseButton] = true;
    
    if (++MouseCaptureCount == 1)
        SetCapture(hWnd);
}

static void ReleaseMouseButton(s32 MouseButton)
{
    __wasMouseKey[MouseButton] = true;
    __isMouseKey[MouseButton] = false;
    
    if (--MouseCaptureCount == 0)
        ReleaseCapture();
    
    #ifdef SP_DEBUGMODE
    if (MouseCaptureCount < 0)
        io::Log::debug("ReleaseMouseButton", "\"MouseCaptureCount\" should never be less zero");
    #endif
}

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
        /* === Keyboard events === */
        
        case WM_KEYDOWN:
        {
            RecordKey(wParam, lParam, true);
        }
        return 0;
        
        case WM_KEYUP:
        {
            RecordKey(wParam, lParam, false);
        }
        return 0;
        
        case WM_CHAR:
        {
            if (__spInputControl->getWordInput())
            {
                switch (wParam)
                {
                    case 0x08: // Backspace
                        if (__spInputControl->getEnteredWord().size() > 0)
                            __spInputControl->getEnteredWord().resize(__spInputControl->getEnteredWord().size() - 1);
                        break;
                        
                    case 0x09: // Tabulator
                        __spInputControl->getEnteredWord() += io::stringc('\t');
                        break;
                        
                    default:
                        if (wParam >= 32 && wParam < 256)
                            __spInputControl->getEnteredWord() += io::stringc((TCHAR)wParam);
                        break;
                }
            }
        }
        return 0;
        
        /* === Left mouse button events === */
        
        case WM_LBUTTONDOWN:
        {
            CaptureMouseButton(io::MOUSE_LEFT, hWnd);
        }
        return 0;
        
        case WM_LBUTTONUP:
        {
            ReleaseMouseButton(io::MOUSE_LEFT);
        }
        return 0;
        
        case WM_LBUTTONDBLCLK:
        {
            CaptureMouseButton(io::MOUSE_LEFT, hWnd);
            __dbclkMouseKey[io::MOUSE_LEFT] = true;
        }
        return 0;
        
        /* === Right mouse button events === */
        
        case WM_RBUTTONDOWN:
        {
            CaptureMouseButton(io::MOUSE_RIGHT, hWnd);
        }
        return 0;
        
        case WM_RBUTTONUP:
        {
            ReleaseMouseButton(io::MOUSE_RIGHT);
        }
        return 0;
        
        case WM_RBUTTONDBLCLK:
        {
            CaptureMouseButton(io::MOUSE_RIGHT, hWnd);
            __dbclkMouseKey[io::MOUSE_RIGHT] = true;
        }
        return 0;
        
        /* === Middle mouse button events === */
        
        case WM_MBUTTONDOWN:
        {
            CaptureMouseButton(io::MOUSE_MIDDLE, hWnd);
        }
        return 0;
        
        case WM_MBUTTONUP:
        {
            ReleaseMouseButton(io::MOUSE_MIDDLE);
        }
        return 0;
        
        case WM_MBUTTONDBLCLK:
        {
            CaptureMouseButton(io::MOUSE_MIDDLE, hWnd);
            __dbclkMouseKey[io::MOUSE_MIDDLE] = true;
        }
        return 0;
        
        /* === X mouse button events === */
        
        case WM_XBUTTONDOWN:
        {
            const s16 XButton = HIWORD(wParam);
            
            if (XButton == 0x0001)
                CaptureMouseButton(io::MOUSE_XBUTTON1, hWnd);
            else if (XButton == 0x0002)
                CaptureMouseButton(io::MOUSE_XBUTTON2, hWnd);
        }
        break;
        
        case WM_XBUTTONUP:
        {
            const s16 XButton = HIWORD(wParam);
            
            if (XButton == 0x0001)
                ReleaseMouseButton(io::MOUSE_XBUTTON1);
            else if (XButton == 0x0002)
                ReleaseMouseButton(io::MOUSE_XBUTTON2);
        }
        break;
        
        case WM_XBUTTONDBLCLK:
        {
            const s16 XButton = HIWORD(wParam);
            
            if (XButton == 0x0001)
            {
                CaptureMouseButton(io::MOUSE_XBUTTON1, hWnd);
                __dbclkMouseKey[io::MOUSE_XBUTTON1] = true;
            }
            else if (XButton == 0x0002)
            {
                CaptureMouseButton(io::MOUSE_XBUTTON2, hWnd);
                __dbclkMouseKey[io::MOUSE_XBUTTON2] = true;
            }
        }
        break;
        
        /* === Mouse motion events === */
        
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
        
        /* === General window events === */
        
        case WM_CLOSE:
        {
            static_cast<SoftPixelDeviceWin32*>(__spDevice)->isWindowOpened_ = false;
            PostQuitMessage(0);
        }
        return 0;
        
        case WM_KILLFOCUS:
        {
            io::InputControl::clearInput();
            ReleaseCapture();
        }
        return 0;
        
        /*case WM_ACTIVATEAPP:
        {
            if ((BOOL)lParam != FALSE)
            {
                if (__spRenderContext->getFullscreen())
                    __spRenderContext->setFullscreen(false);
            }
        }
        return 0;*/
        
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
