/*
 * Network base UDP file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkBaseUDP.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Base/spMemoryManagement.hpp"
#include "Framework/Network/spNetworkSocket.hpp"
#include "Framework/Network/spNetworkPacket.hpp"


namespace sp
{
namespace network
{


c8 NetworkBaseUDP::RecvBuffer_[RECVBUFFER_SIZE] = { 0 };

NetworkBaseUDP::NetworkBaseUDP() :
    NetworkBase()
{
}
NetworkBaseUDP::~NetworkBaseUDP()
{
}


/*
 * ======= Private: =======
 */

bool NetworkBaseUDP::bindSocketToPort(u16 Port)
{
    /* Create connection-less address */
    return Socket_->bindSocket(NetworkAddress(Port));
}

s32 NetworkBaseUDP::sendPacketToAddress(
    SOCKET Socket, const NetworkPacket &Packet, const sockaddr_in &Address)
{
    /* Send network packet to raw address */
    return sendto(
        Socket,
        Packet.getRealBuffer(),
        Packet.getRealBufferSize(),
        0,
        (sockaddr*)&Address,
        sizeof(sockaddr_in)
    );
}

bool NetworkBaseUDP::receivePacketFromAddress(
    SOCKET Socket, NetworkPacket &Packet, sockaddr_in &Address, c8* RecvBuffer, u32 RecvBufferSize)
{
    s32 AddressSize = sizeof(Address);
    
    /* Receive network packet from raw address */
    s32 Result = recvfrom(
        Socket,
        RecvBuffer,
        RecvBufferSize,
        0,
        (sockaddr*)&Address,
        &AddressSize
    );
    
    if (Result == SOCKET_ERROR)
    {
        /* Check for error */
        #if defined(SP_PLATFORM_WINDOWS)
        const s32 ErrorCode = WSAGetLastError();
        if (ErrorCode != WSAEWOULDBLOCK)
            io::Log::error("Receiving network packet failed (Error code: " + io::stringc(ErrorCode) + ")");
        #endif
        
        return false;
    }
    
    /* Copy network packet */
    Packet = NetworkPacket(RecvBuffer, Result, 0, true);
    
    return true;
}

s32 NetworkBaseUDP::sendPacketToAddress(const NetworkPacket &Packet, const sockaddr_in &Address)
{
    return NetworkBaseUDP::sendPacketToAddress(Socket_->getSocket(), Packet, Address);
}

bool NetworkBaseUDP::receivePacketFromAddress(NetworkPacket &Packet, sockaddr_in &Address)
{
    return NetworkBaseUDP::receivePacketFromAddress(
        Socket_->getSocket(), Packet, Address,
        NetworkBaseUDP::RecvBuffer_, NetworkBaseUDP::RECVBUFFER_SIZE
    );
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
