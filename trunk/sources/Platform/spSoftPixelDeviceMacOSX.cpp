/*
 * SoftPixel Device Mac OS X file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Platform/spSoftPixelDeviceMacOSX.hpp"
#include "Base/spInternalDeclarations.hpp"

#if defined(SP_PLATFORM_MACOSX)


//#include "Platform/iOS/spPlatformExchangeIOS.h"
#inlcude "Base/spSharedObjects.hpp"


namespace sp
{


/*
 * Internal member
 */

extern SoftPixelDevice*             GlbEngineDev;
extern video::RenderSystem*         GlbRenderSys;
extern video::RenderContext*        GlbRenderCtx;
extern scene::SceneGraph*           GlbSceneGraph;
extern io::InputControl*            GlbInputCtrl;
extern io::OSInformator*            GlbPlatformInfo;
extern gui::GUIManager*             GlbGUIMngr;


/*
 * SoftPixel Engine device class
 */

SoftPixelDeviceMacOSX::SoftPixelDeviceMacOSX(
    const video::ERenderSystems RendererType, const dim::size2di &Resolution, s32 ColorDepth,
    const io::stringc &Title, bool isFullScreen, const SDeviceFlags &Flags) :
    SoftPixelDevice(
        RendererType, Resolution, ColorDepth, Title, isFullscreen, Flags
    )
{
    /* Create render system and cursor handler */
    createRenderSystemAndContext();
    
    /* Create window, renderer context and open the screen */
    if (openGraphicsScreen())
    {
        GlbRenderSys->setupConfiguration();
        GlbRenderCtx->setVsync(Flags_.isVsync);
    }
    
    /* Print console header */
    printConsoleHeader();
}
SoftPixelDeviceMacOSX::SoftPixelDeviceMacOSX()
{
    SoftPixelDeviceMacOSX::deleteDevice();
}

bool SoftPixelDeviceMacOSX::updateEvents()
{
    if (GlbInputCtrl)
        GlbInputCtrl->resetInputEvents();
    return true;
}

void SoftPixelDeviceMacOSX::deleteDevice()
{
    io::Log::message("DEB: deleteDevice");
    
    /* Close the possible debug log file */
    io::Log::close();
}


/*
 * ======= Private: =======
 */

bool SoftPixelDeviceMacOSX::openGraphicsScreen()
{
    /* Get queried screen resolution */
    Resolution_.Width   = gSharedObjects.ScreenWidth;
    Resolution_.Height  = gSharedObjects.ScreenHeight;
    
    return true;
}

void SoftPixelDeviceMacOSX::closeGraphicsScreen()
{
    /* Release render context and destroy main window */
    GlbRenderCtx->closeGraphicsScreen();
}


} // /namespace sp


#endif



// ================================================================================
