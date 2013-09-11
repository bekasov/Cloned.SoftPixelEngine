/*
 * Direct3D11 render context header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERCONTEXT_DIRECT3D11_H__
#define __SP_RENDERCONTEXT_DIRECT3D11_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/spDesktopRenderContext.hpp"

#include <d3d11.h>
#include <d3dx11.h>


namespace sp
{
namespace video
{


//! Direct3D11 render context.
class SP_EXPORT Direct3D11RenderContext : public DesktopRenderContext
{
    
    public:
        
        Direct3D11RenderContext();
        ~Direct3D11RenderContext();
        
        /* Functions */
        
        bool openGraphicsScreen(
            void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title,
            s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags
        );
        void closeGraphicsScreen();
        
        void flipBuffers();
        void setFullscreen(bool Enable);
        void setVsync(bool Enable);
        
    private:
        
        /* Functions */
        
        bool createRenderContext();
        void releaseRenderContext();
        
        /* Members */
        
        IDXGISwapChain* SwapChain_;
        ID3D11Texture2D* BackBuffer_;
        
        ID3D11Device* D3DDevice_;
        ID3D11DeviceContext* D3DDeviceContext_;
        
        ID3D11RenderTargetView* RenderTargetView_;
        ID3D11Texture2D* DepthStencil_;
        ID3D11DepthStencilView* DepthStencilView_;
        
        u32 SyncInterval_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
