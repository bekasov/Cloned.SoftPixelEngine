/*
 * Core header
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __SPS_CORE_H__
#define __SPS_CORE_H__


/* Global macros */

#define __SPS_MAJOR__       1
#define __SPS_MINOR__       0
#define __SPS_REVISION__    0
#define __SPS_STATUS__      "alpha"

//#define __SPS_DEBUG__

#ifdef _WIN32
#   define SPS_EXPORT __declspec(dllexport)
#   ifdef _MSC_VER
#       pragma warning(disable : 4251) // Disable warning for dll-export.
#   endif
#else
#   define SPS_EXPORT
#endif


/* Global include files */

#include <string>


/* Typedefinitions */

namespace sps
{


typedef bool                boolean;
typedef char                byte;

typedef signed char         int8;
typedef signed short        int16;
typedef signed int          int32;
typedef signed long int     int64;

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long int   uint64;

typedef float               float32;
typedef double              float64;


//! File seek modes.
enum EFileSeekModes
{
    SEEKMODE_BEGIN,
    SEEKMODE_CURRENT,
    SEEKMODE_END,
};


} // /namespace sps


#endif



// ================================================================================
