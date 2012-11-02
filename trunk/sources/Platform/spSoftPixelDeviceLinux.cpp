/*
 * SoftPixel Device Windows file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Platform/spSoftPixelDeviceLinux.hpp"

#if defined(SP_PLATFORM_LINUX)


#include "RenderSystem/OpenGL/spOpenGLRenderSystem.hpp"
#include "RenderSystem/spDesktopRenderContext.hpp"
#include "RenderSystem/spDummyRenderSystem.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "GUI/spGUIManager.hpp"


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

u8 x11KeyCodes[65469] = { 0 };

timeval SoftPixelDeviceLinux::TimeStart_;


/*
 * SoftPixel Engine device interface class
 */

SoftPixelDeviceLinux::SoftPixelDeviceLinux(
    const video::ERenderSystems RendererType, const dim::size2di &Resolution, s32 ColorDepth,
    const io::stringc &Title, bool isFullscreen, const SDeviceFlags &Flags) :
    SoftPixelDevice(
        RendererType, Resolution, ColorDepth, Title, isFullscreen, Flags
    ),
    Display_(0)
{
    /* Setup keycode table to handle X11 lib key events */
    setupKeyCodeTable();
    
    /* Create render system */
    createRenderSystemAndContext();
    
    /* Create window, renderer context and open the screen */
    if (!__spRenderContext->openGraphicsScreen(0, Resolution, Title, ColorDepth, isFullscreen, Flags))
        throw io::stringc("Could not open graphics screen");
    
    /* Setup render system */
    Display_    = static_cast<video::DesktopRenderContext*>(__spRenderContext)->Display_;
    Window_     = static_cast<video::DesktopRenderContext*>(__spRenderContext)->Window_;
    
    __spVideoDriver->setupConfiguration();
    __spRenderContext->setVsync(Flags_.isVsync);
    
    /* Create cursor handler */
    createCursor();
    
    /* Print console header */
    printConsoleHeader();
}

SoftPixelDeviceLinux::~SoftPixelDeviceLinux()
{
    deleteDevice();
}

bool SoftPixelDeviceLinux::updateEvent()
{
    /* Reset keyboard and mouse events */
    io::InputControl::resetInput();
    
    resetCursorSpeedLock();
    
    /* Framerate delay */
    if (FrameRate_ > 0)
        usleep(FrameRate_ * 1000);
    
    /* Update window event */
    while (XPending(Display_) > 0)
    {
        XNextEvent(Display_, &Event_);
        
        switch (Event_.type)
        {
            case KeyPress:
            {
                const s32 KeyCode   = x11KeyCodes[ static_cast<s32>((XLookupKeysym(&Event_.xkey, 0)) ];
                
                __isKey[KeyCode]    = true;
                __hitKey[KeyCode]   = true;
                
                io::InputControl::recordKey(KeyCode);
            }
            break;
            
            case KeyRelease:
            {
                const s32 KeyCode   = x11KeyCodes[ static_cast<s32>(XLookupKeysym(&Event_.xkey, 0)) ];
                
                __isKey[KeyCode]    = false;
                __wasKey[KeyCode]   = true;
                
                io::InputControl::recordKey(KeyCode);
            }
            break;
            
            case ButtonPress:
            {
                const s32 KeyCode = Event_.xbutton.button - 1;
                
                if (KeyCode >= 0 && KeyCode <= 2)
                {
                    __isMouseKey[KeyCode]   = true;
                    __hitMouseKey[KeyCode]  = true;
                }
                else if (KeyCode == 3)
                    ++gSharedObjects.MouseWheel;
                else if (KeyCode == 4)
                    --gSharedObjects.MouseWheel;
            }
            break;
            
            case ButtonRelease:
            {
                const s32 KeyCode = Event_.xbutton.button - 1;
                
                if (KeyCode >= 0 && KeyCode <= 2)
                {
                    __isMouseKey[KeyCode]   = false;
                    __wasMouseKey[KeyCode]  = true;
                }
            }
            break;
            
            case DestroyNotify:
                return false;
        }
    }
    
    return true;
}

void SoftPixelDeviceLinux::deleteDevice()
{
    /* Delete all textures before deleting the render context */
    __spVideoDriver->clearTextureList();
    __spVideoDriver->clearBuffers();
    
    /* Close screen, delete resource devices and unregister the window class */
    __spRenderContext->closeGraphicsScreen();
    deleteResourceDevices();
}

void SoftPixelDeviceLinux::beep(u32 Milliseconds, u32 Frequency)
{
    // todo
}


/*
 * ======= Private: =======
 */

void SoftPixelDeviceLinux::setupKeyCodeTable()
{
    gettimeofday(&TimeStart_, 0);
    
    /* === Fill each keycode of the X11 lib == */
    
    x11KeyCodes[XK_BackSpace    ] = io::KEY_BACK;
    x11KeyCodes[XK_Tab          ] = io::KEY_TAB;
    x11KeyCodes[XK_Clear        ] = io::KEY_CLEAR;
    x11KeyCodes[XK_Return       ] = io::KEY_RETURN;
    x11KeyCodes[XK_Menu         ] = io::KEY_MENU;
    x11KeyCodes[XK_Pause        ] = io::KEY_PAUSE;
    x11KeyCodes[XK_Caps_Lock    ] = io::KEY_CAPITAL;
    
    x11KeyCodes[XK_Escape       ] = io::KEY_ESCAPE;
    x11KeyCodes[XK_space        ] = io::KEY_SPACE;
    x11KeyCodes[XK_Page_Up      ] = io::KEY_PAGEUP;
    x11KeyCodes[XK_Page_Down    ] = io::KEY_PAGEDOWN;
    x11KeyCodes[XK_End          ] = io::KEY_END;
    x11KeyCodes[XK_Home         ] = io::KEY_HOME;
    x11KeyCodes[XK_Left         ] = io::KEY_LEFT;
    x11KeyCodes[XK_Up           ] = io::KEY_UP;
    x11KeyCodes[XK_Right        ] = io::KEY_RIGHT;
    x11KeyCodes[XK_Down         ] = io::KEY_DOWN;
    x11KeyCodes[XK_Select       ] = io::KEY_SELECT;
    x11KeyCodes[XK_Execute      ] = io::KEY_EXE;
    x11KeyCodes[XK_Print        ] = io::KEY_SNAPSHOT;
    x11KeyCodes[XK_Insert       ] = io::KEY_INSERT;
    x11KeyCodes[XK_Delete       ] = io::KEY_DELETE;
    x11KeyCodes[XK_Help         ] = io::KEY_HELP;
    
    x11KeyCodes[XK_0            ] = io::KEY_0;
    x11KeyCodes[XK_1            ] = io::KEY_1;
    x11KeyCodes[XK_2            ] = io::KEY_2;
    x11KeyCodes[XK_3            ] = io::KEY_3;
    x11KeyCodes[XK_4            ] = io::KEY_4;
    x11KeyCodes[XK_5            ] = io::KEY_5;
    x11KeyCodes[XK_6            ] = io::KEY_6;
    x11KeyCodes[XK_7            ] = io::KEY_7;
    x11KeyCodes[XK_8            ] = io::KEY_8;
    x11KeyCodes[XK_9            ] = io::KEY_9;
    
    x11KeyCodes[XK_a            ] = io::KEY_A;
    x11KeyCodes[XK_b            ] = io::KEY_B;
    x11KeyCodes[XK_c            ] = io::KEY_C;
    x11KeyCodes[XK_d            ] = io::KEY_D;
    x11KeyCodes[XK_e            ] = io::KEY_E;
    x11KeyCodes[XK_f            ] = io::KEY_F;
    x11KeyCodes[XK_g            ] = io::KEY_G;
    x11KeyCodes[XK_h            ] = io::KEY_H;
    x11KeyCodes[XK_i            ] = io::KEY_I;
    x11KeyCodes[XK_j            ] = io::KEY_J;
    x11KeyCodes[XK_k            ] = io::KEY_K;
    x11KeyCodes[XK_l            ] = io::KEY_L;
    x11KeyCodes[XK_m            ] = io::KEY_M;
    x11KeyCodes[XK_n            ] = io::KEY_N;
    x11KeyCodes[XK_o            ] = io::KEY_O;
    x11KeyCodes[XK_p            ] = io::KEY_P;
    x11KeyCodes[XK_q            ] = io::KEY_Q;
    x11KeyCodes[XK_r            ] = io::KEY_R;
    x11KeyCodes[XK_s            ] = io::KEY_S;
    x11KeyCodes[XK_t            ] = io::KEY_T;
    x11KeyCodes[XK_u            ] = io::KEY_U;
    x11KeyCodes[XK_v            ] = io::KEY_V;
    x11KeyCodes[XK_w            ] = io::KEY_W;
    x11KeyCodes[XK_x            ] = io::KEY_X;
    x11KeyCodes[XK_y            ] = io::KEY_Y;
    x11KeyCodes[XK_z            ] = io::KEY_Z;
    
    x11KeyCodes[XK_Meta_L       ] = io::KEY_WINLEFT;
    x11KeyCodes[XK_Meta_R       ] = io::KEY_WINRIGHT;
    
    x11KeyCodes[65438           ] = io::KEY_NUMPAD0;
    x11KeyCodes[65436           ] = io::KEY_NUMPAD1;
    x11KeyCodes[65433           ] = io::KEY_NUMPAD2;
    x11KeyCodes[65435           ] = io::KEY_NUMPAD3;
    x11KeyCodes[65430           ] = io::KEY_NUMPAD4;
    x11KeyCodes[65437           ] = io::KEY_NUMPAD5;
    x11KeyCodes[65432           ] = io::KEY_NUMPAD6;
    x11KeyCodes[65429           ] = io::KEY_NUMPAD7;
    x11KeyCodes[65431           ] = io::KEY_NUMPAD8;
    x11KeyCodes[65434           ] = io::KEY_NUMPAD9;
    
    x11KeyCodes[XK_KP_Multiply  ] = io::KEY_MULTIPLY;
    x11KeyCodes[XK_KP_Add       ] = io::KEY_ADD;
    x11KeyCodes[XK_KP_Separator ] = io::KEY_SEPARATOR;
    x11KeyCodes[XK_KP_Subtract  ] = io::KEY_SUBTRACT;
    x11KeyCodes[XK_KP_Decimal   ] = io::KEY_DECIMAL;
    x11KeyCodes[XK_KP_Divide    ] = io::KEY_DIVIDE;
    
    x11KeyCodes[XK_F1           ] = io::KEY_F1;
    x11KeyCodes[XK_F2           ] = io::KEY_F2;
    x11KeyCodes[XK_F3           ] = io::KEY_F3;
    x11KeyCodes[XK_F4           ] = io::KEY_F4;
    x11KeyCodes[XK_F5           ] = io::KEY_F5;
    x11KeyCodes[XK_F6           ] = io::KEY_F6;
    x11KeyCodes[XK_F7           ] = io::KEY_F7;
    x11KeyCodes[XK_F8           ] = io::KEY_F8;
    x11KeyCodes[XK_F9           ] = io::KEY_F9;
    x11KeyCodes[XK_F10          ] = io::KEY_F10;
    x11KeyCodes[XK_F11          ] = io::KEY_F11;
    x11KeyCodes[XK_F12          ] = io::KEY_F12;
    x11KeyCodes[XK_F13          ] = io::KEY_F13;
    x11KeyCodes[XK_F14          ] = io::KEY_F14;
    x11KeyCodes[XK_F15          ] = io::KEY_F15;
    x11KeyCodes[XK_F16          ] = io::KEY_F16;
    x11KeyCodes[XK_F17          ] = io::KEY_F17;
    x11KeyCodes[XK_F18          ] = io::KEY_F18;
    x11KeyCodes[XK_F19          ] = io::KEY_F19;
    x11KeyCodes[XK_F20          ] = io::KEY_F20;
    x11KeyCodes[XK_F21          ] = io::KEY_F21;
    x11KeyCodes[XK_F22          ] = io::KEY_F22;
    x11KeyCodes[XK_F23          ] = io::KEY_F23;
    x11KeyCodes[XK_F24          ] = io::KEY_F24;
    
    x11KeyCodes[XK_Scroll_Lock  ] = io::KEY_SCROLL;
    
    x11KeyCodes[XK_Shift_L      ] = io::KEY_LSHIFT;
    x11KeyCodes[XK_Shift_R      ] = io::KEY_RSHIFT;
    x11KeyCodes[XK_Control_L    ] = io::KEY_LCONTROL;
    x11KeyCodes[XK_Control_R    ] = io::KEY_RCONTROL;
    
    x11KeyCodes[XK_plus         ] = io::KEY_PLUS;
    x11KeyCodes[XK_comma        ] = io::KEY_COMMA;
    x11KeyCodes[XK_minus        ] = io::KEY_MINUS;
    x11KeyCodes[XK_period       ] = io::KEY_PERIOD;
    
    x11KeyCodes[94              ] = io::KEY_EXPONENT;
}

void SoftPixelDeviceLinux::createCursor()
{
    /* Temporary variables */
    XGCValues Values;
    XColor ClrForeground, ClrBackground;
    
    /* Create pixmaps */
    Pixmap CrsPixmap        = XCreatePixmap(Display_, Window_, 32, 32, 1);
    Pixmap MaskPixmap       = XCreatePixmap(Display_, Window_, 32, 32, 1);
    
    Colormap ScrColormap    = DefaultColormap(Display_, DefaultScreen(Display_));
    
    XAllocNamedColor(Display_, ScrColormap, "black", &ClrForeground, &ClrForeground);
    XAllocNamedColor(Display_, ScrColormap, "white", &ClrBackground, &ClrBackground);
    
    /* Create graphics context */
    GC gfxContext = XCreateGC(Display_, CrsPixmap, 0, &Values);
    
    XSetForeground(Display_, gfxContext, BlackPixel(Display_, DefaultScreen(Display_)));
    XFillRectangle(Display_, CrsPixmap, gfxContext, 0, 0, 32, 32);
    XFillRectangle(Display_, MaskPixmap, gfxContext, 0, 0, 32, 32);
    
    /* Create cursor */
    Cursor_ = XCreatePixmapCursor(Display_, CrsPixmap, MaskPixmap, &ClrForeground, &ClrBackground, 1, 1);
    
    XFreeGC(Display_, gfxContext);
    XFreePixmap(Display_, CrsPixmap);
    XFreePixmap(Display_, MaskPixmap);
    
    /* Check if the cursor shall be hidden to the beginning */
    if (isFullscreen_)
        XDefineCursor(Display_, Window_, Cursor_);
}


} // /namespace sp


#endif



// ================================================================================
