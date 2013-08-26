/*
 * SoftPixel Device iOS file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Platform/spSoftPixelDeviceIOS.hpp"
#include "Base/spInternalDeclarations.hpp"

#if defined(SP_PLATFORM_IOS)


#include "Platform/iOS/spPlatformExchangeIOS.h"
#include "Base/spSharedObjects.hpp"


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

SoftPixelDeviceIOS::SoftPixelDeviceIOS(
    const video::ERenderSystems RendererType, const io::stringc &Title, bool isFullscreen) :
    SoftPixelDevice(
        RendererType, 0, 32, isFullscreen, SDeviceFlags()
    )
{
    /* Create render system and cursor handler */
    createRenderSystemAndContext();
    
    /* Create window, renderer context and open the screen */
    if (!openGraphicsScreen())
        throw io::stringc("Could not open graphics screen");
    
    GlbRenderSys->setupConfiguration();
    GlbRenderSys->createDefaultResources();
    GlbRenderCtx->setVsync(Flags_.VSync.Enabled);
    
    /* Print console header */
    printConsoleHeader();
}
SoftPixelDeviceIOS::~SoftPixelDeviceIOS()
{
    SoftPixelDeviceIOS::deleteDevice();
}

bool SoftPixelDeviceIOS::updateEvents()
{
    if (GlbInputCtrl)
        GlbInputCtrl->resetInputEvents();
    return true;
}

void SoftPixelDeviceIOS::deleteDevice()
{
    io::Log::message("DEB: deleteDevice");
    
    /* Close the possible debug log file */
    io::Log::close();
}


/*
 * ======= Private: =======
 */

bool SoftPixelDeviceIOS::openGraphicsScreen()
{
    /* Set static screen resolution for iDevices */
    iOS_SetupScreenSize();
    
    /* Get queried screen resolution */
    Resolution_.Width   = gSharedObjects.ScreenWidth;
    Resolution_.Height  = gSharedObjects.ScreenHeight;
    
    return GlbRenderCtx->openGraphicsScreen(0, Resolution_, "", ColorDepth_, isFullscreen_, Flags_);
}

void SoftPixelDeviceIOS::closeGraphicsScreen()
{
    /* Release render context and destroy main window */
    GlbRenderCtx->closeGraphicsScreen();
}


} // /namespace sp


#endif



// ================================================================================
