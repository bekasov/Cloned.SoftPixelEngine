/*
 * Desktop render context file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spDesktopRenderContext.hpp"

#if defined(SP_PLATFORM_WINDOWS) || defined(SP_PLATFORM_LINUX)


#include "Base/spInputOutputLog.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Base/spSharedObjects.hpp"


namespace sp
{

#if defined(SP_PLATFORM_WINDOWS)
SP_EXPORT LRESULT CALLBACK spWindowCallback(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
#endif

namespace video
{


#if defined(SP_PLATFORM_WINDOWS)

const c8* DesktopRenderContext::WINDOW_CLASSNAME = "SoftPixelWindowClass";

DesktopRenderContext::DesktopRenderContext() :
    RenderContext   (   ),
    Window_         (0  ),
    DeviceContext_  (0  )
{
}
DesktopRenderContext::~DesktopRenderContext()
{
}

void DesktopRenderContext::setWindowTitle(const io::stringc &Title)
{
    SetWindowText(Window_, Title.c_str());
    RenderContext::setWindowTitle(Title);
}
io::stringc DesktopRenderContext::getWindowTitle() const
{
    c8 Title[MAX_PATH];
    GetWindowText(Window_, Title, MAX_PATH);
    return io::stringc(Title);
}

void DesktopRenderContext::setWindowPosition(const dim::point2di &Position)
{
    SetWindowPos(Window_, 0, Position.X, Position.Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}
dim::point2di DesktopRenderContext::getWindowPosition() const
{
    RECT Rect;
    GetWindowRect(Window_, &Rect);
    return dim::point2di(Rect.left, Rect.top);
}

dim::size2di DesktopRenderContext::getWindowSize() const
{
    RECT rc;
    GetWindowRect(Window_, &rc);
    return dim::size2di(rc.right - rc.left, rc.bottom - rc.top);
}
dim::size2di DesktopRenderContext::getWindowBorder() const
{
    if (!isFullscreen_)
    {
        return dim::size2di(
            GetSystemMetrics(SM_CXFIXEDFRAME),
            GetSystemMetrics(SM_CYFIXEDFRAME)
        );
    }
    return 0;
}

bool DesktopRenderContext::isWindowActive() const
{
    return GetForegroundWindow() == Window_;
}

void* DesktopRenderContext::getWindowObject()
{
    return &Window_;
}


/*
 * ======= Protected: =======
 */

void DesktopRenderContext::registerWindowClass()
{
    WNDCLASS WinClass;
    
    /* Fill the window class */
    WinClass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    WinClass.hInstance      = GetModuleHandle(0);
    WinClass.lpfnWndProc    = (WNDPROC)sp::spWindowCallback;
    WinClass.hIcon          = LoadIcon(0, IDI_APPLICATION);
    WinClass.hCursor        = LoadCursor(0, IDC_ARROW);
    WinClass.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WinClass.cbClsExtra     = 0;
    WinClass.cbWndExtra     = 0;
    WinClass.lpszMenuName   = 0;
    WinClass.lpszClassName  = DesktopRenderContext::WINDOW_CLASSNAME;
    
    /* Register window class */
    if (!RegisterClass(&WinClass))
        io::Log::error("Could not register window class");
}

void DesktopRenderContext::unregisterWindowClass()
{
    if (!UnregisterClass(DesktopRenderContext::WINDOW_CLASSNAME, GetModuleHandle(0)))
        io::Log::error("Could not unregister window class");
}

DWORD DesktopRenderContext::getWindowStyle() const
{
    /* Get window style */
    DWORD Style = (isFullscreen_ ? WS_POPUP : WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION);
    
    if (!isFullscreen_ && Flags_.isDropFileAccept)
        Style |= WM_DROPFILES;
    
    return Style;
}

void DesktopRenderContext::getWindowDimension(dim::point2di &Position, dim::size2di &Size) const
{
    /* Get window border size */
    s32 WinBorderWidth, WinBorderHeight, WinCaptionHeight;
    
    if (isFullscreen_)
    {
        WinBorderWidth      = 0;
        WinBorderHeight     = 0;
        WinCaptionHeight    = 0;
    }
    else
    {
        WinBorderWidth      = GetSystemMetrics(SM_CXFIXEDFRAME);
        WinBorderHeight     = GetSystemMetrics(SM_CYFIXEDFRAME);
        WinCaptionHeight    = GetSystemMetrics(SM_CYCAPTION);
    }
    
    /* Get window size */
    Size.Width  = Resolution_.Width + WinBorderWidth*2,
    Size.Height = Resolution_.Height + WinBorderHeight*2 + WinCaptionHeight;
    
    /* Get window position */
    if (!isFullscreen_)
    {
        Position.X = GetSystemMetrics(SM_CXSCREEN)/2 - Size.Width/2;
        Position.Y = GetSystemMetrics(SM_CYSCREEN)/2 - Size.Height/2;
    }
    else
        Position = dim::point2di(0);
}

void DesktopRenderContext::updateWindowStyleAndDimension()
{
    /* Update window style */
    ShowWindow(Window_, SW_HIDE);
    SetWindowLong(Window_, GWL_STYLE, getWindowStyle());
    SetWindowPos(Window_, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    UpdateWindow(Window_);
    
    /* Update window dimension */
    dim::size2di WindowSize;
    dim::point2di WindowPosition;
    
    getWindowDimension(WindowPosition, WindowSize);
    
    SetWindowPos(
        Window_, 0,
        WindowPosition.X, WindowPosition.Y,
        WindowSize.Width, WindowSize.Height,
        SWP_NOZORDER
    );
}

void DesktopRenderContext::updateScreenOffset(bool isFullscreen)
{
    if (!isFullscreen)
    {
        gSharedObjects.ScreenOffsetX = GetSystemMetrics(SM_CXFIXEDFRAME);
        gSharedObjects.ScreenOffsetY = GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
    }
    else
    {
        gSharedObjects.ScreenOffsetX = 0;
        gSharedObjects.ScreenOffsetY = 0;
    }
}

bool DesktopRenderContext::createWindow(const io::stringc &Title)
{
    gSharedObjects.ScreenWidth  = Resolution_.Width;
    gSharedObjects.ScreenHeight = Resolution_.Height;
    
    if (ParentWindow_)
    {
        Window_ = *(HWND*)ParentWindow_;
        
        /* Get device context from window */
        DeviceContext_ = GetDC(Window_);
        
        return true;
    }
    
    updateScreenOffset(isFullscreen_);
    
    /* Get window dimension */
    dim::size2di WindowSize;
    dim::point2di WindowPosition;
    
    getWindowDimension(WindowPosition, WindowSize);
    
    /* Create main window */
    Window_ = CreateWindowEx(
        WS_EX_APPWINDOW, DesktopRenderContext::WINDOW_CLASSNAME,
        TEXT(Title.c_str()), getWindowStyle(),
        WindowPosition.X, WindowPosition.Y,
        WindowSize.Width, WindowSize.Height,
        0, 0, GetModuleHandle(0), 0
    );
    
    if (!Window_)
    {
        io::Log::error("Could not create main window");
        return false;
    }
    
    if (Flags_.isDropFileAccept)
        DragAcceptFiles(Window_, TRUE);
    
    /* Get device context from window */
    DeviceContext_ = GetDC(Window_);
    
    return true;
}

void DesktopRenderContext::deleteWindow()
{
    if (!ParentWindow_ && Window_ && !DestroyWindow(Window_))
        io::Log::error("Could not destroy main window");
    Window_ = 0;
    DeviceContext_ = 0;
}

void DesktopRenderContext::showWindow()
{
    ShowWindow(Window_, SW_NORMAL);
    SetForegroundWindow(Window_);
}

#elif defined(SP_PLATFORM_LINUX)

DesktopRenderContext::DesktopRenderContext() :
    RenderContext   (   ),
    Display_        (0  ),
    Screen_         (0  ),
    Visual_         (0  )
{
}
DesktopRenderContext::~DesktopRenderContext()
{
}

void DesktopRenderContext::setWindowTitle(const io::stringc &Title)
{
    XStoreName(Display_, Window_, Title.c_str());
}
io::stringc DesktopRenderContext::getWindowTitle() const
{
    return ""; //todo
}

void DesktopRenderContext::setWindowPosition(const dim::point2di &Position)
{
    XWindowChanges Changes;
    {
        Changes.x = Position.X;
        Changes.y = Position.Y;
    }
    XConfigureWindow(Display_, Window_, CWX | CWY, &Changes);
}
dim::point2di DesktopRenderContext::getWindowPosition() const
{
    return 0; // todo
}

bool DesktopRenderContext::isWindowActive() const
{
    Window FocusWin;
    s32 State;
    
    XGetInputFocus(Display_, &FocusWin, &State);
    
    return FocusWin == Window_;
}

void* DesktopRenderContext::getWindowObject()
{
    return 0;
}


/*
 * ======= Protected: =======
 */

bool DesktopRenderContext::openDisplay()
{
    /* Open an X-server display */
    Display_ = XOpenDisplay(0);
    
    if (!Display_)
    {
        io::Log::error("Could not open display, need running X Server");
        return false;
    }
    
    /* Get defautl screen and root window */
    Screen_         = DefaultScreen(Display_);
    DefaultWindow_  = DefaultRootWindow(Display_);
    
    return true;
}

bool DesktopRenderContext::chooseVisual()
{
    /* Choose the visual mode */
    s32 Major, Minor;
    
    if (!glXQueryExtension(Display_, &Major, &Minor))
    {
        io::Log::error("GLX is not supported, OpenGL will not work");
        return false;
    }
    
    s32 MultiSamples = (Flags_.isAntiAlias ? Flags_.MultiSamples : 0);
    
    while (!Visual_)
    {
        /* Setup visual mode attributes */
        s32 Attributes[] = {
            GLX_RGBA,           GL_TRUE,
            GLX_RED_SIZE,       4,
            GLX_GREEN_SIZE,     4,
            GLX_BLUE_SIZE,      4,
            GLX_ALPHA_SIZE,     1,
            GLX_DEPTH_SIZE,     24,
            GLX_DOUBLEBUFFER,   GL_TRUE,
            GLX_STENCIL_SIZE,   1,
            GLX_SAMPLES,        MultiSamples,
            None
        };
        
        Visual_ = glXChooseVisual(Display_, 0, Attributes);
        
        if (!Visual_)
        {
            if (Flags_.isAntiAlias && MultiSamples > 0)
            {
                /* Try lower anti-alias mode */
                io::Log::error("Could not choose visual mode with " + io::stringc(MultiSamples) + " samples; trying lower count");
                MultiSamples /= 2;
            }
            else
            {
                io::Log::error("Could not choose visual mode");
                return false;
            }
        }
    }
    
    return true;
}

bool DesktopRenderContext::createWindow(const io::stringc &Title)
{
    /* Create the colormap */
    ColorMap_ = XCreateColormap(Display_, DefaultWindow_, Visual_->visual, AllocNone);
    
    /* Get all video modes */
    s32 ModesCount, BestMode = 0;
    XF86VidModeModeInfo** modes;
    
    XF86VidModeGetAllModeLines(Display_, Screen_, &ModesCount, &modes);
    
    DesktopVideoMode_ = *modes[0];
    
    WinAttribs_.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
    
    /* Create the window */
    if (isFullscreen_)
    {
        /* Find suitable video mode */
        for (s32 i = 0; i < ModesCount; ++i)
        {
            if (modes[i]->hdisplay == Resolution_.Width && modes[i]->vdisplay == Resolution_.Height)
                BestMode = i;
        }
        
        /* Switch to fullscreen mode */
        XF86VidModeSwitchToMode(Display_, Screen_, modes[BestMode]);
        XF86VidModeSetViewPort(Display_, Screen_, 0, 0);
        
        XFree(modes);
        
        /* Set window attributes */
        WinAttribs_.colormap            = ColorMap_;
        WinAttribs_.event_mask          |= StructureNotifyMask;
        WinAttribs_.override_redirect   = true;
        
        /* Create the window */
        Window_ = XCreateWindow(
            Display_,
            DefaultWindow_,
            0,
            0,
            Resolution_.Width,
            Resolution_.Height,
            0,
            Visual_->depth,
            InputOutput,
            Visual_->visual,
            CWColormap | CWEventMask | CWOverrideRedirect,
            &WinAttribs_
        );
        
        /* Grab keyboard and mouse */
        XMapRaised(Display_, Window_);
        XGrabKeyboard(Display_, Window_, true, GrabModeAsync, GrabModeAsync, CurrentTime);
        
        XGrabPointer(
            Display_, Window_, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, Window_, None, CurrentTime
        );
    }
    else
    {
        /* Set window attributes */
        WinAttribs_.colormap   = ColorMap_;
        
        /* Create the window */
        Window_ = XCreateWindow(
            Display_,
            DefaultWindow_,
            (DesktopVideoMode_.hdisplay/2 - Resolution_.Width/2),
            (DesktopVideoMode_.vdisplay/2 - Resolution_.Height/2),
            Resolution_.Width,
            Resolution_.Height,
            0,
            Visual_->depth,
            InputOutput,
            Visual_->visual,
            CWColormap | CWEventMask,
            &WinAttribs_
        );
    }
    
    gSharedObjects.ScreenWidth  = Resolution_.Width;
    gSharedObjects.ScreenHeight = Resolution_.Height;
    
    /* Set resizablility */
    if (!Flags_.isResizAble)
    {
        XSizeHints* Hints = XAllocSizeHints();
        
        Hints->flags        = PSize | PMinSize | PMaxSize;
        Hints->min_width    = Hints->max_width  = Hints->base_width     = Resolution_.Width;
        Hints->min_height   = Hints->max_height = Hints->base_height    = Resolution_.Height;
        
        XSetWMNormalHints(Display_, Window_, Hints);
        XFree(Hints);
    }
    
    /* Show the window and set its title */
    XMapWindow(Display_, Window_);
    XStoreName(Display_, Window_, Title.c_str());
    
    /* Position the window */
    if (!isFullscreen_)
    {
        XWindowChanges ChangeAttributes;
        {
            ChangeAttributes.x = (DesktopVideoMode_.hdisplay/2 - Resolution_.Width/2);
            ChangeAttributes.y = (DesktopVideoMode_.vdisplay/2 - Resolution_.Height/2);
        }
        XConfigureWindow(Display_, Window_, CWX | CWY, &ChangeAttributes);
    }
    
    return true;
}

#else

DesktopRenderContext::DesktopRenderContext() : RenderContext()
{
}
DesktopRenderContext::~DesktopRenderContext()
{
}

#endif

bool DesktopRenderContext::openGraphicsScreen(
    void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title,
    s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags)
{
    return true;
}

void DesktopRenderContext::closeGraphicsScreen()
{
    // do nothing
}

void DesktopRenderContext::flipBuffers()
{
    // do nothing
}
bool DesktopRenderContext::activate()
{
    return true;
}
bool DesktopRenderContext::deactivate()
{
    return true;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
