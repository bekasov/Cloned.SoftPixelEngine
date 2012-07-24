/*
 * Network base UDP header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_BASE_UDP_H__
#define __SP_NETWORK_BASE_UDP_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkBase.hpp"
#include "Base/spThreadManager.hpp"

struct sockaddr_in;


namespace sp
{
namespace network
{


class NetworkPacket;
class NetworkSocket;

class NetworkBaseUDP : public NetworkBase
{
    
    public:
        
        virtual ~NetworkBaseUDP();
        
    protected:
        
        friend THREAD_PROC(NetworkSessionReceptionThread);
        
        /* === Macros === */
        
        static const u32 RECVBUFFER_SIZE = 4096;
        
        /* === Functions === */
        
        NetworkBaseUDP();
        
        bool bindSocketToPort(u16 Port);
        
        s32 sendPacketToAddress(const NetworkPacket &Packet, const sockaddr_in &Address);
        bool receivePacketFromAddress(NetworkPacket &Packet, sockaddr_in &Address);
        
        static s32 sendPacketToAddress(
            SOCKET Socket, const NetworkPacket &Packet, const sockaddr_in &Address
        );
        static bool receivePacketFromAddress(
            SOCKET Socket, NetworkPacket &Packet, sockaddr_in &Address,
            c8* RecvBuffer, u32 RecvBufferSize
        );
        
        /* === Members === */
        
        static c8 RecvBuffer_[RECVBUFFER_SIZE];
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
