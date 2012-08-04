/*
 * Direct3D9 render context file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D9/spDirect3D9RenderContext.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "Base/spInternalDeclarations.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "RenderSystem/Direct3D9/spDirect3D9RenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


Direct3D9RenderContext::Direct3D9RenderContext() :
    DesktopRenderContext(),
    D3DInstance_        (static_cast<Direct3D9RenderSystem*>(__spVideoDriver)->D3DInstance_),
    D3DDevice_          (0)
{
    ZeroMemory(&Presenter_, sizeof(Presenter_));
}
Direct3D9RenderContext::~Direct3D9RenderContext()
{
}

bool Direct3D9RenderContext::openGraphicsScreen(
    void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title,
    s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags)
{
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
    return createRenderContext();
}

void Direct3D9RenderContext::closeGraphicsScreen()
{
    if (!Window_)
        return;
    
    /* Delete window and release render context */
    deleteWindow();
    releaseRenderContext();
    
    /* Close fullscreen mode */
    switchFullscreenMode(false);
    
    /* Reset configuration */
    RenderContext::resetConfig();
}

void Direct3D9RenderContext::flipBuffers()
{
    D3DDevice_->EndScene();
    D3DDevice_->Present(0, 0, 0, 0);
}


/*
 * ======= Private: =======
 */

bool Direct3D9RenderContext::createRenderContext()
{
    /* Get the display mode */
    D3DDISPLAYMODE DisplayMode;
    D3DInstance_->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &DisplayMode);
    
    /* Fill the Direct3D present parameter structure */
    if (isFullscreen_)
    {
        Presenter_.Windowed                     = false;
        Presenter_.SwapEffect                   = D3DSWAPEFFECT_FLIP;
        Presenter_.FullScreen_RefreshRateInHz   = D3DPRESENT_RATE_DEFAULT;
        Presenter_.BackBufferFormat             = (ColorDepth_ == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8);
    }
    else
    {
        Presenter_.Windowed         = true;
        Presenter_.SwapEffect       = D3DSWAPEFFECT_COPY;
        Presenter_.BackBufferFormat = DisplayMode.Format;
    }
    
    Presenter_.BackBufferWidth          = Resolution_.Width;
    Presenter_.BackBufferHeight         = Resolution_.Height;
    Presenter_.BackBufferCount          = 1;
    Presenter_.EnableAutoDepthStencil   = true;
    Presenter_.hDeviceWindow            = Window_;
    Presenter_.PresentationInterval     = (Flags_.isVsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE);
    
    /* Choose depth/stencil format */
    Presenter_.AutoDepthStencilFormat   = D3DFMT_D24S8;
    if (!checkDepthStencilFormat())
    {
        Presenter_.AutoDepthStencilFormat = D3DFMT_D24X4S4;
        if (!checkDepthStencilFormat())
        {
            Presenter_.AutoDepthStencilFormat = D3DFMT_D15S1;
            if (!checkDepthStencilFormat())
            {
                io::Log::error("Could not found a suitable depth-stencil-format");
                return false;
            }
        }
    }
    
    /* Setup anti-aliasing */
    if (Flags_.isAntiAlias)
    {
        Presenter_.MultiSampleType = (D3DMULTISAMPLE_TYPE)Flags_.MultiSamples;
        
        while (Flags_.MultiSamples > 0 && !checkAntiAliasFormat())
        {
            --Flags_.MultiSamples;
            Presenter_.MultiSampleType = (D3DMULTISAMPLE_TYPE)Flags_.MultiSamples;
        }
        
        if (Flags_.MultiSamples > 0)
            Presenter_.SwapEffect = D3DSWAPEFFECT_DISCARD;
    }
    
    /*
     * Create a device class using this information and
     * the info from the d3dpp stucture
     */
    HRESULT Result = D3DInstance_->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Window_, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Presenter_, &D3DDevice_
    );
    
    if (Result != D3D_OK)
    {
        Result = D3DInstance_->CreateDevice(
            D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Window_, D3DCREATE_MIXED_VERTEXPROCESSING, &Presenter_, &D3DDevice_
        );
    }
    
    if (Result != D3D_OK)
    {
        Result = D3DInstance_->CreateDevice(
            D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Window_, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &Presenter_, &D3DDevice_
        );
    }
    
    if (!D3DDevice_)
    {
        io::Log::error("Could not create Direct3D9 device");
        return false;
    }
    
    static_cast<Direct3D9RenderSystem*>(__spVideoDriver)->D3DDevice_ = D3DDevice_;
    
    showWindow();
    
    return true;
}

void Direct3D9RenderContext::releaseRenderContext()
{
    if (D3DDevice_)
    {
        D3DDevice_->Release();
        D3DDevice_ = 0;
    }
}

void Direct3D9RenderContext::switchFullscreenMode(bool isFullscreen)
{
    //todo
}

bool Direct3D9RenderContext::checkDepthStencilFormat()
{
    return !D3DInstance_->CheckDeviceFormat(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Presenter_.BackBufferFormat,
        D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, Presenter_.AutoDepthStencilFormat
    );
}

bool Direct3D9RenderContext::checkAntiAliasFormat()
{
    DWORD QualityLevels = 0;
    
    if (!D3DInstance_->CheckDeviceMultiSampleType(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Presenter_.BackBufferFormat,
        Presenter_.Windowed, Presenter_.MultiSampleType, &QualityLevels))
    {
        Presenter_.MultiSampleQuality = QualityLevels - 1;
        return true;
    }
    
    io::Log::warning(
        io::stringc(Flags_.MultiSamples) + " mutlisamples for anti-aliasing are not supported, try lower count"
    );
    
    return false;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
