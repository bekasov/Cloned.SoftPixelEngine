/*
 * Network socket file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

// This is a little bit weird -> "Winsock2.h" etc. must be included at first!
#include "Base/spStandard.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <Winsock2.h>
#   ifdef SP_COMPILER_VC
#       include <Mstcpip.h>
#   endif
#   include <Mswsock.h>
#endif

#include "Framework/Network/spNetworkSocket.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Base/spInputOutputLog.hpp"


namespace sp
{
namespace network
{


NetworkSocket::NetworkSocket(const ENetworkProtocols Protocol) :
    Protocol_   (Protocol       ),
    Socket_     (INVALID_SOCKET )
{
    createSocket();
    setupBehavior();
}
NetworkSocket::NetworkSocket(const ENetworkProtocols Protocol, SOCKET Socket) :
    Protocol_   (Protocol   ),
    Socket_     (Socket     )
{
    setupBehavior();
}
NetworkSocket::~NetworkSocket()
{
    closeSocket();
}

bool NetworkSocket::bindSocket(const NetworkAddress &Address)
{
    s32 Result = bind(
        Socket_,
        (SOCKADDR*)&(Address.getSocketAddress()),
        sizeof(sockaddr_in)
    );
    
    /* Check for error */
    if (Result != NO_ERROR)
    {
        io::Log::error("Could not bind server to port " + io::stringc(Address.getPort()));
        return false;
    }
    
    return true;
}


/*
 * ======= Private: =======
 */

void NetworkSocket::createSocket()
{
    /* Create socket object */
    switch (Protocol_)
    {
        case PROTOCOL_TCP:
            Socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            break;
        case PROTOCOL_UDP:
            Socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            break;
    }
    
    /* Check for errors */
    if (Socket_ == INVALID_SOCKET)
    {
        #if defined(SP_PLATFORM_WINDOWS)
        io::Log::error("Could not create socket (error code = " + io::stringc(WSAGetLastError()) + ")");
        #elif defined(SP_PLATFORM_LINUX)
        io::Log::error("Could not create socket");
        #endif
    }
}

void NetworkSocket::closeSocket()
{
    /* Delete socket object */
    #if defined(SP_PLATFORM_WINDOWS)
    closesocket(Socket_);
    #elif defined(SP_PLATFORM_LINUX)
    close(Socket_);
    #endif
    Socket_ = INVALID_SOCKET;
}

void NetworkSocket::setupBehavior()
{
    setBlocking(false);
    setConnectionReset(false);
    //setBroadCasting(true);
    setReUseAddress(true);
}

void NetworkSocket::setConnectionReset(bool Enable)
{
    #if defined(SP_PLATFORM_WINDOWS) && defined(SP_COMPILER_VC)
    
    DWORD BytesReturned = 0;
    BOOL NewBehavior = Enable;
    
    DWORD Status = WSAIoctl(
        Socket_, SIO_UDP_CONNRESET, &NewBehavior, sizeof(NewBehavior), 0, 0, &BytesReturned, 0, 0
    );
    
    if (Status == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
        io::Log::error("Could not disable IOCTRL socket behvior");
    
    #endif
}

void NetworkSocket::setBlocking(bool Enable)
{
    unsigned long Flags = (Enable ? 0 : 1);
    
    if (ioctlsocket(Socket_, FIONBIO, &Flags) != NO_ERROR)
        io::Log::error("Could not set socket non-blocking flags");
}

void NetworkSocket::setBroadCasting(bool Enable)
{
    const s32 Flags = (Enable ? 1 : 0);
    setsockopt(Socket_, SOL_SOCKET, SO_BROADCAST, (c8*)Flags, sizeof(Flags));
}

void NetworkSocket::setReUseAddress(bool Enable)
{
    const BOOL Flags = (Enable ? TRUE : FALSE);
    if (setsockopt(Socket_, SOL_SOCKET, SO_REUSEADDR, (const c8*)&Flags, sizeof(Flags)) == SOCKET_ERROR)
        io::Log::error("Could not set socket option for re-usable address");
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
