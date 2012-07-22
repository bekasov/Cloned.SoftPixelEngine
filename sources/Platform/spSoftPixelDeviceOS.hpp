/*
 * SoftPixelDevice OS header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SOFTPIXELDEVICE_OS_H__
#define __SP_SOFTPIXELDEVICE_OS_H__


#include "Base/spStandard.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include "Platform/spSoftPixelDeviceWindows.hpp"
#elif defined(SP_PLATFORM_MACOSX)
#   include "Platform/spSoftPixelDeviceMacOSX.hpp"
#elif defined(SP_PLATFORM_LINUX)
#   include "Platform/spSoftPixelDeviceLinux.hpp"
#elif defined(SP_PLATFORM_ANDROID)
#   include "Platform/spSoftPixelDeviceAndroid.hpp"
#elif defined(SP_PLATFORM_IOS)
#   include "Platform/spSoftPixelDeviceIOS.hpp"
#endif


#endif



// ================================================================================
