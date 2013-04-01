/*
 * OpenGL context base file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLRenderContext.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"
#include "RenderSystem/OpenGL/spOpenGLSharedRenderContext.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Base/spSharedObjects.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderContext* __spRenderContext;
extern io::InputControl* __spInputControl;

namespace video
{


/*
 * Internal macros
 */

const c8* GLCONTEXT_ERROR_CREATE        = "Could not create OpenGL render context";
const c8* GLCONTEXT_ERROR_ACTIVATE      = "Could not activate OpenGL render context";
const c8* GLCONTEXT_ERROR_DEACTIVATE    = "Could not release OpenGL render context";
const c8* GLCONTEXT_ERROR_DESTROY       = "Could not delete OpenGL render context";
const c8* GLCONTEXT_ERROR_SHARELISTS    = "Could not share lists for OpenGL render context";

/*
 * OpenGLRenderContext class
 */

#if defined(SP_PLATFORM_WINDOWS)

OpenGLRenderContext::OpenGLRenderContext() :
    DesktopRenderContext    (   ),
    RenderContext_          (0  ),
    PixelFormat_            (0  ),
    NumPixelFormatAA_       (0  )
{
    clearPixelFormatAA();
}
OpenGLRenderContext::~OpenGLRenderContext()
{
}

bool OpenGLRenderContext::openGraphicsScreen(
    void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title, s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags)
{
    const bool HasFSChanged = (isFullscreen_ != isFullscreen);
    
    /* Update settings */
    ParentWindow_   = ParentWindow;
    Resolution_     = Resolution;
    ColorDepth_     = ColorDepth;
    isFullscreen_   = isFullscreen;
    Flags_          = Flags;
    
    /* Create main window */
    if (!createWindow(Title))
        return false;
    
    /* Create render context */
    if (!createRenderContext())
    {
        io::Log::error("Could not create render context");
        return false;
    }
    
    /*
    Setup anti aliasing after creating a standard render context.
    This is required because the wglChoosePixelFormatARB function can not be loaded,
    before a valid render context is available!
    */
    if (Flags_.isAntiAlias && !NumPixelFormatAA_)
    {
        /* Setup anti-aliasing */
        setupAntiAliasing();
        
        /* ReOpen screen */
        deleteContextAndWindow();
        return openGraphicsScreen(ParentWindow, Resolution_, Title, ColorDepth_, isFullscreen_, Flags_);
    }
    
    /* Show window */
    if (!ParentWindow_)
    {
        /* Change display settings */
        if (HasFSChanged)
            switchFullscreenMode(isFullscreen_);
		if (Flags.isWindowVisible)
			showWindow();
    }
    
    return true;
}

void OpenGLRenderContext::closeGraphicsScreen()
{
    if (!Window_)
        return;
    
    deleteContextAndWindow();
    
    /* Close fullscreen mode */
    if (isFullscreen_)
        switchFullscreenMode(false);
    
    /* Reset configuration */
    isFullscreen_       = false;
    RenderContext_      = 0;
    PixelFormat_        = 0;
    
    clearPixelFormatAA();
    
    RenderContext::resetConfig();
}

void OpenGLRenderContext::flipBuffers()
{
#ifdef SP_DEBUGMODE
    if (!SwapBuffers(DeviceContext_))
        io::Log::debug("OpenGLRenderContext::flipBuffers", "Flip buffers failed");
#else
    SwapBuffers(DeviceContext_);
#endif
}

bool OpenGLRenderContext::activate()
{
    if (RenderContext::ActiveRenderContext_ != this)
    {
        RenderContext::setActiveRenderContext(this);
        return wglMakeCurrent(DeviceContext_, RenderContext_) == TRUE;
    }
    return true;
}
bool OpenGLRenderContext::deactivate()
{
    RenderContext::ActiveRenderContext_ = 0;
    return wglMakeCurrent(0, 0) == TRUE;
}

void OpenGLRenderContext::setFullscreen(bool Enable)
{
    if (isFullscreen_ != Enable)
    {
        isFullscreen_ = Enable;
        
        /* Update display mode and window dimension */
        switchFullscreenMode(Enable);
        updateWindowStyleAndDimension();
    }
}

void OpenGLRenderContext::setResolution(const dim::size2di &Resolution)
{
    if (Resolution_ != Resolution)
    {
        /* Setup new resolution value */
        Resolution_ = Resolution;
        
        gSharedObjects.ScreenWidth  = Resolution.Width;
        gSharedObjects.ScreenHeight = Resolution.Height;
        
        /* Update display mode and window dimension */
        switchFullscreenMode(isFullscreen_);
        updateWindowStyleAndDimension();
    }
}

#endif

SharedRenderContext* OpenGLRenderContext::createSharedContext()
{
    SharedRenderContext* NewSharedContext = new OpenGLSharedRenderContext(this);
    SharedContextList_.push_back(NewSharedContext);
    return NewSharedContext;
}

void OpenGLRenderContext::setVsync(bool Enable)
{
    #if defined(SP_PLATFORM_WINDOWS)
    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(Enable ? 1 : 0);
    #elif defined(SP_PLATFORM_LINUX)
    if (glXSwapIntervalSGI)
        glXSwapIntervalSGI(Enable ? 1 : 0);
    #endif
    
    Flags_.isVsync = Enable;
}


/*
 * ======= Private: =======
 */

#ifdef SP_PLATFORM_WINDOWS

bool OpenGLRenderContext::createRenderContext()
{
    if (!DeviceContext_)
    {
        io::Log::error("Cannot create render context without device context");
        return false;
    }
    
    /* Select pixel format (standard or anti-aliased) */
    if (!selectPixelFormat())
        return false;
    
    /* Create OpenGL render context */
    RenderContext_ = wglCreateContext(DeviceContext_);
    
    if (!RenderContext_)
    {
        io::Log::error(GLCONTEXT_ERROR_CREATE);
        return false;
    }
    
    /* Activate OpenGL context */
    if (!activate())
    {
        io::Log::error(GLCONTEXT_ERROR_ACTIVATE);
        return false;
    }
    
    /* Share OpenGL lists if this is not the first render context */
    if (__spRenderContext && __spRenderContext != this)
    {
        OpenGLRenderContext* RootRenderContext = static_cast<OpenGLRenderContext*>(__spRenderContext);
        
        if (!wglShareLists(RootRenderContext->RenderContext_, RenderContext_))
        {
            io::Log::error(GLCONTEXT_ERROR_SHARELISTS);
            return false;
        }
    }
    
    /* Initialize OpenGL states */
    initRenderStates();
    
    return true;
}

void OpenGLRenderContext::releaseRenderContext()
{
    if (!RenderContext_ || !DeviceContext_)
        return;
    
    /* Deactivate render context */
    if (!wglMakeCurrent(DeviceContext_, 0))
        io::Log::error(GLCONTEXT_ERROR_DEACTIVATE);
    
    /* Delete render context */
    if (!wglDeleteContext(RenderContext_))
        io::Log::error(GLCONTEXT_ERROR_DESTROY);
    
    /* Reset active render context */
    RenderContext_ = 0;
    RenderContext::ActiveRenderContext_ = 0;
    
    /* Release device context */
    if (!wglMakeCurrent(DeviceContext_, 0))
        io::Log::error(GLCONTEXT_ERROR_DEACTIVATE);
    
    if (!ParentWindow_ && !ReleaseDC(Window_, DeviceContext_))
        io::Log::error("Could not release device context");
    
    DeviceContext_ = 0;
}

bool OpenGLRenderContext::switchFullscreenMode(bool isFullscreen)
{
    if (ParentWindow_)
        return false;
    
    /* Switch fullscreen mode */
    LONG Result = 0;
    
    if (isFullscreen)
    {
        DEVMODE Config;
        memset(&Config, 0, sizeof(DEVMODE));
        {
            Config.dmSize       = sizeof(DEVMODE);
            Config.dmPelsWidth  = Resolution_.Width;
            Config.dmPelsHeight = Resolution_.Height;
            Config.dmBitsPerPel = ColorDepth_;
            Config.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        }
        Result = ChangeDisplaySettings(&Config, CDS_FULLSCREEN);
    }
    else
        Result = ChangeDisplaySettings(0, 0);
    
    /* Check for errors */
    if (Result != DISP_CHANGE_SUCCESSFUL)
    {
        io::Log::error("Switching fullscreen mode failed");
        return false;
    }
    
    updateScreenOffset(isFullscreen);
    
    return true;
}

bool OpenGLRenderContext::getGLPixelFormatExt() const
{
    /* Load OpenGL extension for anti-aliasing earlie than the others */
    if (!wglChoosePixelFormatARB)
    {
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        if (!wglChoosePixelFormatARB)
        {
            io::Log::error("Could not load OpenGL function 'wglChoosePixelFormatARB'");
            return false;
        }
    }
    return true;
}

bool OpenGLRenderContext::selectPixelFormat()
{
    /* Setup pixel format attributes */
    static PIXELFORMATDESCRIPTOR FormatDesc =
    {
        sizeof(PIXELFORMATDESCRIPTOR),  // Structure size
        1,                              // Version number
        PFD_DRAW_TO_WINDOW |            // Format must support window
        PFD_SUPPORT_OPENGL |            // Format must support OpenGL
        PFD_DOUBLEBUFFER,               // Must support double buffering
        PFD_TYPE_RGBA,                  // Request an RGBA format
        ColorDepth_,                    // Select our color depth
        0, 0, 0, 0, 0, 0,               // Color bits ignored
        0,                              // No alpha buffer
        0,                              // Shift bit ignored
        0,                              // No accumulation buffer
        0, 0, 0, 0,                     // Accumulation bits ignored
        24,                             // Z-Buffer bits (Depth Buffer)
        1,                              // Stencil buffer
        0,                              // No auxiliary buffer
        PFD_MAIN_PLANE,                 // Main drawing layer
        0,                              // Reserved
        0, 0, 0                         // Layer masks ignored
    };
    
    u32 PixelFormatIndexAA = 0;
    BOOL FormatSelected = FALSE;
    bool StandardFormatUsed = false;
    
    while (1)
    {
        if (Flags_.isAntiAlias && NumPixelFormatAA_ > 0 && PixelFormatIndexAA < OpenGLRenderContext::PIXELFORMATAA_COUNT)
        {
            /* Choose anti-aliasing pixel format */
            PixelFormat_ = MultiSamplePixelFormats_[PixelFormatIndexAA++];
        }
        
        if (!PixelFormat_)
        {
            /* Choose standard pixel format */
            PixelFormat_ = ChoosePixelFormat(DeviceContext_, &FormatDesc);
            
            if (Flags_.isAntiAlias && NumPixelFormatAA_ > 0)
                io::Log::error("Anti-aliasing is not supported");
            
            StandardFormatUsed = true;
        }
        
        /* Check for errors */
        if (!PixelFormat_)
        {
            io::Log::error("Could not find suitable pixelformat");
            return false;
        }
        
        /* Set pixel format */
        FormatSelected = SetPixelFormat(DeviceContext_, PixelFormat_, &FormatDesc);
        
        if (!FormatSelected)
        {
            if (StandardFormatUsed)
            {
                io::Log::error("Could not setup pixel format");
                return false;
            }
        }
        else
            break;
    }
    
    return true;
}

void OpenGLRenderContext::clearPixelFormatAA()
{
    NumPixelFormatAA_ = 0;
    memset(MultiSamplePixelFormats_, 0, sizeof(MultiSamplePixelFormats_));
}

bool OpenGLRenderContext::setupAntiAliasing()
{
    /* Load OpenGL extension for anti-aliasing earlie than the others */
    if (!getGLPixelFormatExt())
    {
        Flags_.isAntiAlias  = false;
        Flags_.MultiSamples = 0;
        clearPixelFormatAA();
        return false;
    }
    
    /* Setup pixel format for anti-aliasing */
    NumPixelFormatAA_ = 0;
    f32 AttribsFlt[] = { 0.0f, 0.0f };
    
    s32 AttribsInt[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB,     ColorDepth_,
        WGL_ALPHA_BITS_ARB,     8,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   1,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_SAMPLE_BUFFERS_ARB, Flags_.isAntiAlias ? GL_TRUE : GL_FALSE,
        WGL_SAMPLES_ARB,        Flags_.MultiSamples,
        0, 0
    };
    
    /* Choose new pixel format with anti-aliasing */
    s32 Result = wglChoosePixelFormatARB(
        DeviceContext_, AttribsInt, AttribsFlt,
        OpenGLRenderContext::PIXELFORMATAA_COUNT, MultiSamplePixelFormats_, &NumPixelFormatAA_
    );
    
    if (!Result || NumPixelFormatAA_ < 1)
    {
        io::Log::warning(
            io::stringc(Flags_.MultiSamples) + " mutlisamples for AntiAliasing are not supported; trying lower count"
        );
        --Flags_.MultiSamples;
    }
    
    /* Enable anti-aliasing */
    if (Flags_.isAntiAlias)
        glEnable(GL_MULTISAMPLE_ARB);
    else
        glDisable(GL_MULTISAMPLE_ARB);
    
    return true;
}

void OpenGLRenderContext::deleteContextAndWindow()
{
    /* Release render context and destroy main window */
    releaseRenderContext();
    
    /* Destroy window object */
    deleteWindow();
}

#elif defined(SP_PLATFORM_LINUX)

OpenGLRenderContext::OpenGLRenderContext() :
    DesktopRenderContext(   ),
    RenderContext_      (0  )
{
}
OpenGLRenderContext::~OpenGLRenderContext()
{
}

bool OpenGLRenderContext::openGraphicsScreen(
    void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title, s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags)
{
    /* Update settings */
    ParentWindow_   = ParentWindow;
    Resolution_     = Resolution;
    ColorDepth_     = ColorDepth;
    isFullscreen_   = isFullscreen;
    Flags_          = Flags;
    
    /* Create window */
    if (!openDisplay() || !chooseVisual() || !createWindow(Title))
        return false;
    
    /* Create render context */
    return createRenderContext();
}

void OpenGLRenderContext::closeGraphicsScreen()
{
    if (!Display_)
        return;
    
    /* Release render context and destroy main window */
    releaseRenderContext();
    
    /* Destroy window object */
    XDestroyWindow(Display_, Window_);
    
    /* Close fullscreen mode */
    switchFullscreenMode(false);
    
    /* Close the X-server display */
    XCloseDisplay(Display_);
    
    /* Reset configuration */
    RenderContext_ = 0;
    
    RenderContext::resetConfig();
}

bool OpenGLRenderContext::switchFullscreenMode(bool isFullscreen)
{
    if (!Display_ || isFullscreen_ == isFullscreen)
        return false;
    
    isFullscreen_ = isFullscreen;
    
    if (isFullscreen)
    {
        //todo
    }
    else
    {
        XF86VidModeSwitchToMode(Display_, Screen_, &DesktopVideoMode_);
        XF86VidModeSetViewPort(Display_, Screen_, 0, 0);
    }
    
    return true;
}

void OpenGLRenderContext::flipBuffers()
{
    glXSwapBuffers(Display_, Window_);
}

bool OpenGLRenderContext::activate()
{
    if (RenderContext::ActiveRenderContext_ != this)
    {
        RenderContext::setActiveRenderContext(this);
        return glXMakeCurrent(Display_, Window_, RenderContext_) == GL_TRUE;
    }
    return true;
}
bool OpenGLRenderContext::deactivate()
{
    RenderContext::ActiveRenderContext_ = 0;
    return glXMakeCurrent(Display_, None, 0) == GL_TRUE;
}


/*
 * ======= Private: =======
 */

bool OpenGLRenderContext::createRenderContext()
{
    if (Display_ && Visual_)
    {
        /* Create render context */
        if ( !( RenderContext_ = glXCreateContext(Display_, Visual_, 0, GL_TRUE) ) )
        {
            io::Log::error(GLCONTEXT_ERROR_CREATE);
            return false;
        }
        
        if (!activate())
        {
            io::Log::error(GLCONTEXT_ERROR_ACTIVATE);
            return false;
        }
        
        return true;
    }
    
    return false;
}

void OpenGLRenderContext::releaseRenderContext()
{
    if (Display_)
    {
        /* Release render context */
        if (!glXMakeCurrent(Display_, None, 0))
        {
            io::Log::error(GLCONTEXT_ERROR_DEACTIVATE);
            return;
        }
        glXDestroyContext(Display_, RenderContext_);
    }
}

#endif


/*
 * ======= Private: =======
 */

void OpenGLRenderContext::initRenderStates()
{
    /* Default settings */
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_NORMALIZE);
    
    /* Hints for the renderer */
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);//(only if GL >= v.1.4)
    
    /* Set the coloring mode with lighting */
    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
    
    /*
    This OpenGL render system is using CW (clockwise) triangulation by default since version 3.2.
    Use "GLBasePipeline::setFrontFace" function to change the direction to CCW (counter clockwise).
    This was changed, since a bug was fixed in which the front and back faces are reversed.
    */
    glFrontFace(GL_CW);
    
    /* Tessellation shader configuration */
    #ifdef GL_ARB_tessellation_shader
    // !!!! PROBLEMS WITH TEXT DRAWING !!!
    /*if (glPatchParameteriARB)
    {
        glPatchParameteriARB(GL_PATCH_VERTICES, 3);
        glPatchParameteriARB(GL_PATCH_DEFAULT_OUTER_LEVEL, 1);
        glPatchParameteriARB(GL_PATCH_DEFAULT_INNER_LEVEL, 1);
    }*/
    #endif
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
