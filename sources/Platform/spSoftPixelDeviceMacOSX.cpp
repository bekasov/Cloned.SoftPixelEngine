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

extern SoftPixelDevice*             __spDevice;
extern video::RenderSystem*         __spVideoDriver;
extern video::RenderContext*        __spRenderContext;
extern scene::SceneGraph*           __spSceneManager;
extern io::InputControl*            __spInputControl;
extern io::OSInformator*            __spOSInformator;
extern gui::GUIManager*             __spGUIManager;


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
        __spVideoDriver->setupConfiguration();
        __spRenderContext->setVsync(Flags_.isVsync);
    }
    
    /* Print console header */
    printConsoleHeader();
}
SoftPixelDeviceMacOSX::SoftPixelDeviceMacOSX()
{
    SoftPixelDeviceMacOSX::deleteDevice();
}

bool SoftPixelDeviceMacOSX::updateEvent()
{
    if (__spInputControl)
        __spInputControl->resetInputEvents();
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
    __spRenderContext->closeGraphicsScreen();
}


} // /namespace sp


#endif



// ================================================================================
