/*
 * OpenCL core header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENCL_COREHEADER_H__
#define __SP_OPENCL_COREHEADER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENCL)


#if defined(SP_PLATFORM_IOS) || defined(SP_PLATFORM_MACOSX)
#   include <OpenCL/opencl.h>
#else
#   include <CL/cl.h>
#endif


#endif

#endif



// ================================================================================
