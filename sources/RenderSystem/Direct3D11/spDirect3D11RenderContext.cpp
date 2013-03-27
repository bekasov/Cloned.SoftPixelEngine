/*
 * Direct3D11 render context file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11RenderContext.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "Base/spInternalDeclarations.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


Direct3D11RenderContext::Direct3D11RenderContext() :
    DesktopRenderContext(   ),
    SwapChain_          (0  ),
    BackBuffer_         (0  ),
    D3DDevice_          (0  ),
    D3DDeviceContext_   (0  ),
    RenderTargetView_   (0  ),
    DepthStencil_       (0  ),
    DepthStencilView_   (0  ),
    SyncInterval_       (0  )
{
}
Direct3D11RenderContext::~Direct3D11RenderContext()
{
}

bool Direct3D11RenderContext::openGraphicsScreen(
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

void Direct3D11RenderContext::closeGraphicsScreen()
{
    if (!Window_)
        return;
    
    /* Delete window and release render context */
    deleteWindow();
    releaseRenderContext();
    
    /* Close fullscreen mode */
    //switchFullscreenMode(false);
    
    /* Reset configuration */
    RenderContext::resetConfig();
}

void Direct3D11RenderContext::flipBuffers()
{
    SwapChain_->Present(SyncInterval_, 0);
}

void Direct3D11RenderContext::setVsync(bool Enable)
{
    Flags_.isVsync = Enable;
    SyncInterval_ = (Flags_.isVsync ? 1 : 0);
}


/*
 * ======= Private: =======
 */

bool Direct3D11RenderContext::createRenderContext()
{
    Direct3D11RenderSystem* D3DRenderer = static_cast<Direct3D11RenderSystem*>(__spVideoDriver);
    
    /* Temporary variables */
    HRESULT Result = 0;
    
    /* Create the Direct3D11 device, swap-chain and device-context */
    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    ZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));
    
    SwapChainDesc.BufferCount                           = 1;
    SwapChainDesc.BufferDesc.Width                      = Resolution_.Width;
    SwapChainDesc.BufferDesc.Height                     = Resolution_.Height;
    SwapChainDesc.BufferDesc.Format                     = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator      = 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator    = 1;
    SwapChainDesc.BufferUsage                           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.OutputWindow                          = Window_;
    SwapChainDesc.SampleDesc.Count                      = (Flags_.isAntiAlias ? Flags_.MultiSamples : 1);
    SwapChainDesc.SampleDesc.Quality                    = 0;
    SwapChainDesc.Windowed                              = !isFullscreen_;
    
    /* List for each possible Direct3D11 device type */
    struct
    {
        D3D_DRIVER_TYPE Type;
        io::stringc Name;
    }
    DriverTypes[] =
    {
        { D3D_DRIVER_TYPE_HARDWARE, "hardware" },
        { D3D_DRIVER_TYPE_WARP, "emulator" },
        { D3D_DRIVER_TYPE_REFERENCE, "software" }
    };
    
    //#define _DEB_LIMIT_FEATURELEVEL_
    #ifdef _DEB_LIMIT_FEATURELEVEL_
    D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;
    #endif
    
    /* Try to create Direct3D11 device and swap chain, if failed try to use the next device type */
    for (s32 i = 0; i < 3; ++i)
    {
        Result = D3D11CreateDeviceAndSwapChain(
            0,                              // Adapter (IDXGIAdapter)
            DriverTypes[i].Type,            // Driver type
            0,                              // Software module
            0,                              // Flags
            #ifdef _DEB_LIMIT_FEATURELEVEL_
            &FeatureLevels, 1,              // Feature levels
            #else
            0, 0,                           // Feature levels
            #endif
            D3D11_SDK_VERSION,              // SDK version
            &SwapChainDesc,                 // Swap chain description
            &SwapChain_,                    // Swap chain
            &D3DDevice_,                    // Direct3D11 device (main graphics device)
            &D3DRenderer->FeatureLevel_,    // Feature level
            &D3DDeviceContext_              // Direct3D11 device context (for rendering)
        );
        
        if (Result && i < 2)
        {
            io::Log::warning(
                "Could not create Direct3D11 device in " + DriverTypes[i].Name +
                " mode; using " + DriverTypes[i + 1].Name + " mode"
            );
        }
        else
            break;
    }
    
    /* Check for errors */
    if (Result)
    {
        io::Log::error("Creating Direct3D11 device and swap chain failed");
        return false;
    }
    if (!SwapChain_)
    {
        io::Log::error("Could not create Direct3D11 swap chain");
        return false;
    }
    if (!D3DDevice_)
    {
        io::Log::error("Could not create Direct3D11 device");
        return false;
    }
    if (!D3DDeviceContext_)
    {
        io::Log::error("Could not create Direct3D11 device context");
        return false;
    }
    
    /* Create render target view */
    if (SwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer_))
    {
        io::Log::error("Could not get back buffer");
        return false;
    }
    
    Result = D3DDevice_->CreateRenderTargetView(BackBuffer_, 0, &RenderTargetView_);
    BackBuffer_->Release();
    
    if (Result)
    {
        io::Log::error("Could not create render target view");
        return false;
    }
    
    /* Create depth stencil */
    D3D11_TEXTURE2D_DESC DepthDesc;
    ZeroMemory(&DepthDesc, sizeof(D3D11_TEXTURE2D_DESC));
    
    DepthDesc.Width                 = Resolution_.Width;
    DepthDesc.Height                = Resolution_.Height;
    DepthDesc.MipLevels             = 1;
    DepthDesc.ArraySize             = 1;
    DepthDesc.Format                = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthDesc.SampleDesc.Count      = SwapChainDesc.SampleDesc.Count;
    DepthDesc.SampleDesc.Quality    = 0;
    DepthDesc.Usage                 = D3D11_USAGE_DEFAULT;
    DepthDesc.BindFlags             = D3D11_BIND_DEPTH_STENCIL;
    DepthDesc.CPUAccessFlags        = 0;
    DepthDesc.MiscFlags             = 0;
    
    if (D3DDevice_->CreateTexture2D(&DepthDesc, 0, &DepthStencil_))
    {
        io::Log::error("Could not create depth-stencil texture");
        return false;
    }
    
    /* Create depth stencil view */
    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilDesc;
    ZeroMemory(&DepthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    
    DepthStencilDesc.Format             = DepthDesc.Format;
    DepthStencilDesc.ViewDimension      = (Flags_.isAntiAlias ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D);
    DepthStencilDesc.Texture2D.MipSlice = 0;
    
    if (D3DDevice_->CreateDepthStencilView(DepthStencil_, &DepthStencilDesc, &DepthStencilView_))
    {
        io::Log::error("Could not create depth-stencil view");
        return false;
    }
    
    /* Set render target view and depth stencil view */
    D3DDeviceContext_->OMSetRenderTargets(1, &RenderTargetView_, DepthStencilView_);
    
    D3DRenderer->OrigRenderTargetView_ = RenderTargetView_;
    D3DRenderer->OrigDepthStencilView_ = DepthStencilView_;
    
    SyncInterval_ = (Flags_.isVsync ? 1 : 0);
    
    /* Setup render system members */
    D3DRenderer->D3DDevice_         = D3DDevice_;
    D3DRenderer->D3DDeviceContext_  = D3DDeviceContext_;
    D3DRenderer->DepthStencil_      = DepthStencil_;
    D3DRenderer->DepthStencilView_  = DepthStencilView_;
    D3DRenderer->RenderTargetView_  = RenderTargetView_;
    
    D3DRenderer->setViewport(0, Resolution_);
    
    /* Show main window */
	if (Flags.isWindowVisible)
		showWindow();
    
    return true;
}

void Direct3D11RenderContext::releaseRenderContext()
{
    Direct3D11RenderSystem::releaseObject(BackBuffer_);
    Direct3D11RenderSystem::releaseObject(SwapChain_);
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
