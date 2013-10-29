/*
 * Core header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_STANDARD_H__
#define __SP_STANDARD_H__


/* === Core configurations === */

#define __SOFTPIXELENGINE__         // SoftPixel Engine identity

#define __SP_EXPORT_TO_LIBRARY__    // Exports the SoftPixel Engine to a library

// Version macros

#define SOFTPIXEL_VERSION_MAJOR     3
#define SOFTPIXEL_VERSION_MINOR     3
#define SOFTPIXEL_VERSION_REVISION  0
#define SOFTPIXEL_VERSION_STATUS    "alpha" // "beta"/ "alpha" or undef

// Platform macros

#if defined(_ANDROID)
#   define SP_PLATFORM_ANDROID
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(WIN32) || defined(_WINDOWS) || defined(_MSC_VER)
#   define SP_PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__unix__)
#   define SP_PLATFORM_LINUX
#elif defined(__APPLE__)
#   include "TargetConditionals.h"
#   if defined(__MACOSX__)//TARGET_OS_MAC)
#       define SP_PLATFORM_MACOSX
#   elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#       define SP_PLATFORM_IOS
#   endif
#elif defined(__FreeBSD__)
#   define SP_PLATFORM_FREEBSD // unsupported yet
#   error The "SoftPixel Engine" is not supported under "FreeBSD" yet!
#elif defined(__sparc__) || defined(__sun__)
#   define SP_PLATFORM_SOLARIS // unsupported yet
#   error The "SoftPixel Engine" is not supported under "Sun Solaris" yet!
#elif defined(_XBOX)
#   define SP_PLATFORM_XBOX // unsupported yet
#   error The "SoftPixel Engine" is not supported under "XBox" yet!
#else
#   error Unknown platform, no support!
#endif

// Dll export macros

#if defined(__SP_EXPORT_TO_LIBRARY__) && defined(SP_PLATFORM_WINDOWS)
#   define SP_EXPORT __declspec(dllexport)
#else
#   define SP_EXPORT
#endif

// Debug mode macros

#if defined(SP_PLATFORM_WINDOWS) && defined(_DEBUG)
#   define SP_DEBUGMODE     // More debugging information
#   ifdef _MSC_VER
#       define SP_DEBUG_BREAK DebugBreak()
#   else
#       define SP_DEBUG_BREAK
#   endif
#endif

// Environment (32-/ 64 bit) macros

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
#   define SP_ENVIRONMENT_64BIT
#endif

#ifndef SP_ENVIRONMENT_64BIT
#   define SP_ENVIRONMENT_32BIT
#endif

// Compiler information macros

#if defined(__GNUC__)
#   define SP_COMPILER_GCC  // GNU Compiler Collection
#   ifdef SP_ENVIRONMENT_64BIT
#       define SP_SDK_VERSION 2
#   else
#       define SP_SDK_VERSION 1
#   endif
#elif defined(_MSC_VER)
#   define SP_COMPILER_VC   // VisualC++
#   ifdef SP_DEBUGMODE
#       ifdef SP_ENVIRONMENT_64BIT
#           define SP_SDK_VERSION (_MSC_VER + 3)
#       else
#           define SP_SDK_VERSION (_MSC_VER + 2)
#       endif
#   else
#       ifdef SP_ENVIRONMENT_64BIT
#           define SP_SDK_VERSION (_MSC_VER + 1)
#       else
#           define SP_SDK_VERSION (_MSC_VER)
#       endif
#   endif
#   if _MSC_VER >= 1600
#       define SP_COMPILER_SUPPORTS_MOVE_SEMANTIC
#   endif
#else
#   define SP_SDK_VERSION 0
#endif

// Sundries macros

#if defined(SP_COMPILER_VC)
#   define _USE_MATH_DEFINES
#endif

#ifndef SP_DONT_DEFINE_BOOST_MACROS
#   define foreach          BOOST_FOREACH
#   define foreach_reverse  BOOST_REVERSE_FOREACH
#endif


/* === Compilation configurations === */

#include "Base/spCompilationOptions.hpp"


/* === OS dependent configuration (do not edit!) === */

#if !defined(SP_PLATFORM_WINDOWS) && !defined(SP_PLATFORM_XBOX)
#   undef SP_COMPILE_WITH_DIRECT3D9
#   undef SP_COMPILE_WITH_XAUDIO2
#   undef SP_COMPILE_WITH_XBOX360GAMEPAD
#endif

#if !defined(SP_PLATFORM_WINDOWS)
#   undef SP_COMPILE_WITH_DIRECT3D11
#   undef SP_COMPILE_WITH_WINMM
#   undef SP_COMPILE_WITH_NEWTONPHYSICS
#endif

#if defined(SP_PLATFORM_ANDROID) || defined(SP_PLATFORM_IOS)
#    undef SP_COMPILE_WITH_OPENGL
#   undef SP_COMPILE_WITH_NETWORKSYSTEM
#   undef SP_COMPILE_WITH_SOUNDSYSTEM
#   undef SP_COMPILE_WITH_GUI
#   undef SP_COMPILE_WITH_PHYSICS
#   undef SP_COMPILE_WITH_NEWTON
#   undef SP_COMPILE_WITH_BULLET
#   undef SP_COMPILE_WITH_PHYSX
#   undef SP_COMPILE_WITH_CG
#   undef SP_COMPILE_WITH_ADVANCEDRENDERER
#   undef SP_COMPILE_WITH_OPENCL
#   undef SP_COMPILE_WITH_INPUTSERVICE
#else
#   undef SP_COMPILE_WITH_OPENGLES1
#   undef SP_COMPILE_WITH_OPENGLES2
#   undef SP_COMPILE_WITH_OPENSLES
#endif

#if defined(SP_PLATFORM_IOS)
#   undef SP_COMPILE_WITH_OPENSLES
#endif

#if defined(SP_PLATFORM_ANDROID)
#   define SP_USE_GLES_EGL
#endif

#if defined(SP_PLATFORM_MACOSX) || defined(SP_PLATFORM_LINUX)
#   undef SP_COMPILE_WITH_CG
#endif

#ifndef SP_COMPILE_WITH_XMLPARSER
#   undef SP_COMPILE_WITH_WEBPAGERENDERER
#endif


/* === Compiler dependent configuration (do not edit!) === */

#if defined(SP_COMPILER_VC)
#   define _USE_MATH_DEFINES
#else
#   undef SP_COMPILE_WITH_DIRECT3D11
#   undef SP_COMPILE_WITH_XAUDIO2
#   undef SP_COMPILE_WITH_XBOX360GAMEPAD
#endif


/* === Doxygen configuration === */

#ifdef __DOXYGEN__
#   define SP_COMPILE_WITH_OPENGL
#   define SP_COMPILE_WITH_OPENGLES1
#   define SP_COMPILE_WITH_OPENGLES2
#   define SP_COMPILE_WITH_DIRECT3D9
#   define SP_COMPILE_WITH_DIRECT3D11
#   define SP_COMPILE_WITH_ADVANCEDRENDERER

#   define SP_COMPILE_WITH_SCENEGRAPH_SIMPLE
#   define SP_COMPILE_WITH_SCENEGRAPH_SIMPLE_STREAM
#   define SP_COMPILE_WITH_SCENEGRAPH_FAMILY_TREE

#   define SP_COMPILE_WITH_WINMM
#   define SP_COMPILE_WITH_OPENAL
#   define SP_COMPILE_WITH_XAUDIO2
#   define SP_COMPILE_WITH_OPENSLES

#   define SP_COMPILE_WITH_NEWTON
#   define SP_COMPILE_WITH_PHYSX
#   define SP_COMPILE_WITH_BULLET

#   define SP_COMPILE_WITH_LIGHTMAPGENERATOR
#   define SP_COMPILE_WITH_MODELCOMBINER
#   define SP_COMPILE_WITH_SCRIPTLOADER
#   define SP_COMPILE_WITH_PARTICLEANIMATOR
#   define SP_COMPILE_WITH_IMAGEMODIFIER
#   define SP_COMPILE_WITH_XMLPARSER
#   define SP_COMPILE_WITH_PATHFINDER
#   define SP_COMPILE_WITH_INPUTSERVICE
#   define SP_COMPILE_WITH_MATRIXTEXTURE
#   define SP_COMPILE_WITH_STORYBOARD
#   define SP_COMPILE_WITH_TOKENSCANNER
#   define SP_COMPILE_WITH_COMMANDLINE
#   define SP_COMPILE_WITH_WEBPAGERENDERER

#   define SP_COMPILE_WITH_NETWORKSYSTEM
#   define SP_COMPILE_WITH_GUI
#   define SP_COMPILE_WITH_CG
#   define SP_COMPILE_WITH_OPENCL
#   define SP_COMPILE_WITH_XBOX360GAMEPAD

#   define SP_PLATFORM_WINDOWS
#   define SP_PLATFORM_MACOSX
#   define SP_PLATFORM_LINUX
#   define SP_PLATFORM_ANDROID
#   define SP_PLATFORM_IOS
#endif

/* === Base types === */

#include "Base/spBaseTypes.hpp"


#endif



// ================================================================================
