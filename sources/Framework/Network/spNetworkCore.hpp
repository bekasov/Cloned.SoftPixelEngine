/*
 * Network core 
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_CORE_H__
#define __SP_NETWORK_CORE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#if defined(SP_PLATFORM_WINDOWS)
#   if defined(SP_COMPILER_GCC)
#       include <unistd.h>
#       include <winsock2.h>
#       include <ws2tcpip.h>
#   elif defined(SP_COMPILER_VC)
#       include <windows.h>
#       include <process.h>
#   endif
#   include <lmserver.h>
#elif defined(SP_PLATFORM_LINUX)
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <sys/select.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <unistd.h>
#endif

#include <map>
#include <vector>

#if defined(SP_PLATFORM_LINUX)
#   ifndef SOCKET
#       define SOCKET           int
#   endif
#   ifndef INVALID_SOCKET
#       define INVALID_SOCKET   -1
#   endif
#   ifndef SOCKET_ERROR
#       define SOCKET_ERROR     -1
#   endif
#endif


#endif

#endif



// ================================================================================
