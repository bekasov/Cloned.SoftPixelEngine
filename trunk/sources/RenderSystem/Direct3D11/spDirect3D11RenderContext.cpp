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

extern video::RenderSystem* GlbRenderSys;

namespace video
{


Direct3D11RenderContext::Direct3D11RenderContext() :
    DesktopRenderContext(   ),
    D3DDevice_          (0  ),
    Factory_            (0  ),
    SwapChain_          (0  ),
    D3DDeviceContext_   (0  ),
    BackBuffer_         (0  ),
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

void Direct3D11RenderContext::setFullscreen(bool Enable)
{
    /*
    Always call this function. No check if fullscreen is already active.
    If the user tabed out of the window, fullscreen mode will automatically disabled.
    */
    isFullscreen_ = Enable;
    SwapChain_->SetFullscreenState(Enable, 0);
}

void Direct3D11RenderContext::setVsync(bool Enable)
{
    Flags_.VSync.Enabled = Enable;
    SyncInterval_ = Flags_.VSync.getInterval();
}


/*
 * ======= Private: =======
 */

bool Direct3D11RenderContext::queryDxFactory()
{
    /* Get DirectX factory */
    IDXGIDevice* DxDevice = 0;
    if (D3DDevice_->QueryInterface(__uuidof(IDXGIDevice), (void**)&DxDevice) != S_OK)
    {
        io::Log::error("Querying DXGI device failed");
        return false;
    }
    
    IDXGIAdapter* DxAdapter = 0;
    if (DxDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&DxAdapter) != S_OK)
    {
        io::Log::error("Querying DXGI adapter failed");
        return false;
    }
    
    if (DxAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&Factory_) != S_OK)
    {
        io::Log::error("Querying DXGI factory failed");
        return false;
    }
    
    return true;
}

bool Direct3D11RenderContext::createRenderContext()
{
    Direct3D11RenderSystem* D3DRenderSys = static_cast<Direct3D11RenderSystem*>(GlbRenderSys);
    
    /* Temporary variables */
    HRESULT Result = 0;
    
    /* List for each possible Direct3D11 device type */
    #ifdef SP_DEBUGMODE
    
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
    
    #endif
    
    /* Setup feature level */
    const D3D_FEATURE_LEVEL* FeatureLvl = getFeatureLevel();
    
    #ifdef SP_DEBUGMODE
    /* Try to create Direct3D11 device and swap chain, if failed try to use the next device type */
    for (u32 i = 0; i < 3; ++i)
    #endif
    {
        Result = D3D11CreateDevice(
            0,                              // Adapter (IDXGIAdapter)
            #ifdef SP_DEBUGMODE
            DriverTypes[i].Type,            // Driver type
            #else
            D3D_DRIVER_TYPE_HARDWARE,
            #endif
            0,                              // Software module
            0,                              // Flags
            FeatureLvl,                     // Feature levels
            FeatureLvl ? 1 : 0,             // Num feature levels
            D3D11_SDK_VERSION,              // SDK version
            &D3DDevice_,                    // Direct3D11 device (main graphics device)
            &D3DRenderSys->FeatureLevel_,   // Output feature level
            &D3DDeviceContext_              // Direct3D11 device context (for rendering)
        );
        
        #ifdef SP_DEBUGMODE
        if (Result != S_OK && i < 2)
        {
            io::Log::warning(
                "Could not create Direct3D11 device in " + DriverTypes[i].Name +
                " mode; using " + DriverTypes[i + 1].Name + " mode"
            );
        }
        else
            break;
        #endif
    }
    
    /* Check for errors */
    if (Result != S_OK || !D3DDevice_ || !D3DDeviceContext_)
    {
        io::Log::error("Creating Direct3D11 device failed");
        return false;
    }
    
    /* Create swap chain, back-buffer render target view (RTV), depth-stencil and depth-stencil view (DSV) */
    if ( !queryDxFactory() ||
         !createSwapChain() ||
         !createBackBufferRTV() ||
         !createDepthStencil() ||
         !createDepthStencilView() )
    {
        return false;
    }
    
    /* Set render target view and depth stencil view */
    D3DDeviceContext_->OMSetRenderTargets(1, &RenderTargetView_, DepthStencilView_);
    
    D3DRenderSys->OrigRenderTargetView_ = RenderTargetView_;
    D3DRenderSys->OrigDepthStencilView_ = DepthStencilView_;
    
    SyncInterval_ = Flags_.VSync.getInterval();
    
    /* Setup render system members */
    D3DRenderSys->D3DDevice_         = D3DDevice_;
    D3DRenderSys->D3DDeviceContext_  = D3DDeviceContext_;
    D3DRenderSys->DepthStencil_      = DepthStencil_;
    D3DRenderSys->DepthStencilView_  = DepthStencilView_;
    D3DRenderSys->RenderTargetView_  = RenderTargetView_;
    
    D3DRenderSys->setViewport(0, Resolution_);
    
    /* Show main window */
	if (Flags_.Window.Visible)
		showWindow();
    
    return true;
}

void Direct3D11RenderContext::releaseRenderContext()
{
    Direct3D11RenderSystem::releaseObject(BackBuffer_);
    Direct3D11RenderSystem::releaseObject(SwapChain_);
}

void Direct3D11RenderContext::setupSwapChainDesc(DXGI_SWAP_CHAIN_DESC &SwapChainDesc) const
{
    ZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));
    
    SwapChainDesc.BufferCount                           = 1;
    SwapChainDesc.BufferDesc.Width                      = Resolution_.Width;
    SwapChainDesc.BufferDesc.Height                     = Resolution_.Height;
    SwapChainDesc.BufferDesc.Format                     = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator      = Flags_.VSync.RefreshRate;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator    = 1;
    SwapChainDesc.BufferUsage                           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.OutputWindow                          = Window_;
    SwapChainDesc.SampleDesc.Count                      = getSwapChainSampleCount();
    SwapChainDesc.SampleDesc.Quality                    = 0;
    SwapChainDesc.Windowed                              = !isFullscreen_;
}

bool Direct3D11RenderContext::createSwapChain()
{
    /* Create the Direct3D11 device, swap-chain and device-context */
    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    setupSwapChainDesc(SwapChainDesc);
    
    if (Factory_->CreateSwapChain(D3DDevice_, &SwapChainDesc, &SwapChain_) != S_OK)
    {
        io::Log::error("Could not create Direct3D11 swap chain");
        return false;
    }
    
    return true;
}

bool Direct3D11RenderContext::createBackBufferRTV()
{
    /* Get back-buffer from swap chain */
    if (SwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer_))
    {
        io::Log::error("Could not get back buffer");
        return false;
    }
    
    /* Create render target view and release reference */
    HRESULT Result = D3DDevice_->CreateRenderTargetView(BackBuffer_, 0, &RenderTargetView_);
    BackBuffer_->Release();
    
    /* Check for errors */
    if (Result != S_OK)
    {
        io::Log::error("Could not create render target view");
        return false;
    }
    
    return true;
}

bool Direct3D11RenderContext::createDepthStencil()
{
    D3D11_TEXTURE2D_DESC DepthDesc;
    ZeroMemory(&DepthDesc, sizeof(D3D11_TEXTURE2D_DESC));
    
    DepthDesc.Width                 = Resolution_.Width;
    DepthDesc.Height                = Resolution_.Height;
    DepthDesc.MipLevels             = 1;
    DepthDesc.ArraySize             = 1;
    DepthDesc.Format                = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthDesc.Usage                 = D3D11_USAGE_DEFAULT;
    DepthDesc.BindFlags             = D3D11_BIND_DEPTH_STENCIL;
    DepthDesc.CPUAccessFlags        = 0;
    DepthDesc.MiscFlags             = 0;
    
    DepthDesc.SampleDesc.Count      = getSwapChainSampleCount();
    DepthDesc.SampleDesc.Quality    = 0;
    
    if (D3DDevice_->CreateTexture2D(&DepthDesc, 0, &DepthStencil_))
    {
        io::Log::error("Could not create depth-stencil texture");
        return false;
    }
    
    return true;
}

bool Direct3D11RenderContext::createDepthStencilView()
{
    /* Create depth stencil view */
    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilDesc;
    ZeroMemory(&DepthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    
    DepthStencilDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;//DepthDesc.Format;
    DepthStencilDesc.ViewDimension      = (Flags_.AntiAliasing.Enabled ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D);
    DepthStencilDesc.Texture2D.MipSlice = 0;
    
    if (D3DDevice_->CreateDepthStencilView(DepthStencil_, &DepthStencilDesc, &DepthStencilView_))
    {
        io::Log::error("Could not create depth-stencil view");
        return false;
    }
    
    return true;
}

u32 Direct3D11RenderContext::getSwapChainSampleCount() const
{
    return Flags_.AntiAliasing.Enabled ? Flags_.AntiAliasing.MultiSamples : 1;
}

const D3D_FEATURE_LEVEL* Direct3D11RenderContext::getFeatureLevel() const
{
    const ED3DFeatureLevels FeatureLevel = Flags_.RendererProfile.D3DFeatureLevel;
    
    static const D3D_FEATURE_LEVEL D3DFeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_9_1,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_11_0
    };
    
    if (FeatureLevel >= DIRECT3D_FEATURE_LEVEL_9_1 && FeatureLevel <= DIRECT3D_FEATURE_LEVEL_11_0)
        return &D3DFeatureLevels[FeatureLevel - DIRECT3D_FEATURE_LEVEL_9_1];
    
    return 0;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
