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

extern SoftPixelDevice*             __spDevice;
extern video::RenderSystem*         __spVideoDriver;
extern video::RenderContext*        __spRenderContext;
extern scene::SceneGraph*           __spSceneManager;
extern scene::CollisionDetector*    __spCollisionDetector;
extern io::InputControl*            __spInputControl;
extern io::OSInformator*            __spOSInformator;
extern gui::GUIManager*             __spGUIManager;


/*
 * SoftPixel Engine device class
 */

SoftPixelDeviceIOS::SoftPixelDeviceIOS(
    const video::ERenderSystems RendererType, const io::stringc &Title, bool isFullscreen)
    : SoftPixelDevice(RendererType, 0, 32, Title, isFullscreen, SDeviceFlags())
{
    /* Create render system and cursor handler */
    createRenderSystem();
    
    /* Create window, renderer context and open the screen */
    if (openGraphicsScreen())
    {
        __spVideoDriver->setupConfiguration();
        __spVideoDriver->setVsync(Flags_.isVsync);
    }
    
    /* Print console header */
    printConsoleHeader();
}
SoftPixelDeviceIOS::~SoftPixelDeviceIOS()
{
    SoftPixelDeviceIOS::deleteDevice();
}

bool SoftPixelDeviceIOS::updateEvent()
{
    if (__spInputControl)
        __spInputControl->resetInputEvents();
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
    
    return true;
}

void SoftPixelDeviceIOS::closeGraphicsScreen()
{
    /* Release render context and destroy main window */
    __spRenderContext->closeGraphicsScreen();
}


} // /namespace sp


#endif



// ================================================================================
