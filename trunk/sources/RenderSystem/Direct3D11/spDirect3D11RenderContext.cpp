/*
 * Direct3D11 render context file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11RenderContext.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "Base/spInternalDeclarations.hpp"
#include "Base/spSharedObjects.hpp"
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

bool Direct3D11RenderContext::activate()
{
    if (RenderContext::ActiveRenderContext_ != this)
    {
        RenderContext::setActiveRenderContext(this);
        makeCurrent();
    }
    return true;
}

bool Direct3D11RenderContext::deactivate()
{
    RenderContext::ActiveRenderContext_ = 0;
    return true;
}

void Direct3D11RenderContext::setFullscreen(bool Enable)
{
    if (!ParentWindow_)
    {
        /*
        Always call this function (if no parent window is used). No check if fullscreen is already active.
        If the user tabed out of the window, fullscreen mode will automatically disabled.
        */
        isFullscreen_ = Enable;
        SwapChain_->SetFullscreenState(Enable, 0);
    }
}

bool Direct3D11RenderContext::setResolution(const dim::size2di &Resolution)
{
    if (Resolution_ == Resolution)
        return true;
    
    /* Setup new resolution value */
    Resolution_ = Resolution;
    
    gSharedObjects.ScreenWidth  = Resolution.Width;
    gSharedObjects.ScreenHeight = Resolution.Height;
    
    /* Update window dimension */
    if (!ParentWindow_)
        updateWindowStyleAndDimension();
    
    /* Disbale render targets */
    if (activated())
        D3DDeviceContext_->OMSetRenderTargets(0, 0, 0);
    
    /* Release previous back buffer render target view (RTV) */
    RenderTargetView_->Release();
    
    /*
    Resize swap chain buffers.
    Let DXGI find out the client window area and preserve buffer count and format.
    */
    if (SwapChain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0) != S_OK)
    {
        io::Log::error("Resizing swawp chain buffers failed");
        return false;
    }
    
    /* ReCreate back buffer render target view (RTV) */
    if (!createBackBufferRTV())
        return false;
    
    /* Activate this render context */
    if (activated())
        makeCurrent();
    
    return true;
}

void Direct3D11RenderContext::setVsync(bool Enable)
{
    Flags_.VSync.Enabled = Enable;
    SyncInterval_ = Flags_.VSync.getInterval();
}


/*
 * ======= Private: =======
 */

bool Direct3D11RenderContext::createRenderContext()
{
    /* Get render system main objects */
    Direct3D11RenderSystem* D3DRenderSys = static_cast<Direct3D11RenderSystem*>(GlbRenderSys);
    
    D3DDevice_          = D3DRenderSys->D3DDevice_;
    D3DDeviceContext_   = D3DRenderSys->D3DDeviceContext_;
    Factory_            = D3DRenderSys->Factory_;
    
    /* Create swap chain, back-buffer render target view (RTV), depth-stencil and depth-stencil view (DSV) */
    if ( !createSwapChain       () ||
         !createBackBufferRTV   () ||
         !createDepthStencil    () ||
         !createDepthStencilView() )
    {
        return false;
    }
    
    /* Make this render context to the current (like 'glMakeCurrent') */
    makeCurrent();
    
    D3DRenderSys->setViewport(0, Resolution_);
    
    SyncInterval_ = Flags_.VSync.getInterval();
    
    /* Show main window */
	if (!ParentWindow_ && Flags_.Window.Visible)
		showWindow();
    
    return true;
}

void Direct3D11RenderContext::releaseRenderContext()
{
    /* Release shader views */
    Direct3D11RenderSystem::releaseObject(DepthStencilView_     );
    Direct3D11RenderSystem::releaseObject(RenderTargetView_     );
    
    /* Release context objects */
    Direct3D11RenderSystem::releaseObject(DepthStencil_);
    Direct3D11RenderSystem::releaseObject(BackBuffer_);
    Direct3D11RenderSystem::releaseObject(SwapChain_);
}

bool Direct3D11RenderContext::createSwapChain()
{
    /* Setup swap chain description */
    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
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
    
    /* Create shwap chain with main DXGI factory */
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

void Direct3D11RenderContext::makeCurrent()
{
    Direct3D11RenderSystem* D3DRenderSys = static_cast<Direct3D11RenderSystem*>(GlbRenderSys);
    
    /* Set render target view and depth stencil view */
    D3DDeviceContext_->OMSetRenderTargets(1, &RenderTargetView_, DepthStencilView_);
    
    /* Setup render system members */
    D3DRenderSys->OrigRenderTargetView_ = RenderTargetView_;
    D3DRenderSys->OrigDepthStencilView_ = DepthStencilView_;
    
    D3DRenderSys->DepthStencilView_     = DepthStencilView_;
    D3DRenderSys->RenderTargetView_     = RenderTargetView_;
}

u32 Direct3D11RenderContext::getSwapChainSampleCount() const
{
    return Flags_.AntiAliasing.Enabled ? Flags_.AntiAliasing.MultiSamples : 1;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
