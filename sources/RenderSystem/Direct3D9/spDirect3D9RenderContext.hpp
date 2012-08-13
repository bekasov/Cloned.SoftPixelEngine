/*
 * Direct3D9 render context header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERCONTEXT_DIRECT3D9_H__
#define __SP_RENDERCONTEXT_DIRECT3D9_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "RenderSystem/spDesktopRenderContext.hpp"

#include <d3d9.h>


namespace sp
{
namespace video
{


//! Direct3D9 render context.
class SP_EXPORT Direct3D9RenderContext : public DesktopRenderContext
{
    
    public:
        
        Direct3D9RenderContext();
        ~Direct3D9RenderContext();
        
        /* Functions */
        
        bool openGraphicsScreen(
            void* ParentWindow, const dim::size2di &Resolution, const io::stringc &Title,
            s32 ColorDepth, bool isFullscreen, const SDeviceFlags &Flags
        );
        void closeGraphicsScreen();
        
        void flipBuffers();
        
        bool activate();
        bool deactivate();
        
    private:
        
        /* Functions */
        
        bool createRenderContext();
        void releaseRenderContext();
        
        void switchFullscreenMode(bool isFullscreen);
        
        bool checkDepthStencilFormat();
        bool checkAntiAliasFormat();
        
        /* Members */
        
        IDirect3D9* D3DInstance_;
        IDirect3DDevice9* D3DDevice_;
        
        D3DPRESENT_PARAMETERS Presenter_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
