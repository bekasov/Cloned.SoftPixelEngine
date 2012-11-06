/*
 * Network system UDP header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_SYSTEM_UDP_H__
#define __SP_NETWORK_SYSTEM_UDP_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkBaseUDP.hpp"
#include "Framework/Network/spNetworkSystem.hpp"


namespace sp
{
namespace network
{


//! UDP/IP network system class.
class SP_EXPORT NetworkSystemUDP : public NetworkBaseUDP, public NetworkSystem
{
    
    public:
        
        NetworkSystemUDP();
        ~NetworkSystemUDP();
        
        /* === Functions === */
        
        io::stringc getDescription() const;
        
        NetworkServer* hostServer(u16 Port = DEFAULT_PORT);
        NetworkServer* joinServer(const io::stringc &IPAddress, u16 Port = DEFAULT_PORT);
        
        void disconnect();
        
        bool sendPacket(const NetworkPacket &Packet, NetworkMember* Receiver = 0);
        bool receivePacket(NetworkPacket &Packet, NetworkMember* &Sender);
        
        bool popClientJoinStack(NetworkClient* &Client);
        bool popClientLeaveStack(NetworkClient* &Client);
        
        NetworkMember* getMemberByAddress(const NetworkAddress &Address);
        
    private:
        
        /* === Functions === */
        
        bool bindToPort(u16 Port);
        
        void sendClientAllServerInfos(const sockaddr_in &SenderAddr);
        
        bool examineReceivedPacket(NetworkPacket &Packet, NetworkMember* &Sender, const sockaddr_in &SenderAddr);
        
        void disconnectedByServer();
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
