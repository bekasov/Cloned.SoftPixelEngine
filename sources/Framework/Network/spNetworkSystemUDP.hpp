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


#include "Framework/Network/spNetworkSystem.hpp"


namespace sp
{
namespace network
{


//! UDP/IP network system class.
class NetworkSystemUDP : public NetworkSystem
{
    
    public:
        
        NetworkSystemUDP();
        ~NetworkSystemUDP();
        
        /* === Functions === */
        
        NetworkServer* hostServer(u16 Port = DEFAULT_PORT);
        NetworkServer* joinServer(const io::stringc &IPAddress, u16 Port = DEFAULT_PORT);
        
        void disconnect();
        
        void requestNetworkSession(const io::stringc &IPAddress, u16 Port = DEFAULT_PORT);
        u32 requestNetworkSessionBroadcast(u16 Port = DEFAULT_PORT);
        
        bool sendPacket(const NetworkPacket &Packet, NetworkMember* Receiver = 0);
        bool receivePacket(NetworkPacket &Packet, NetworkMember* &Sender);
        
        void processPackets();
        
        bool popClientJoinStack(NetworkClient* &Client);
        bool popClientLeaveStack(NetworkClient* &Client);
        
        bool transferServerPermission(NetworkClient* Client = 0);
        
        NetworkMember* getMemberByAddress(const NetworkAddress &Address);
        
    private:
        
        /* === Functions === */
        
        bool bindSocketToPort(u16 Port);
        
        void registerMember(NetworkMember* Member);
        NetworkMember* getMemberByAddress(const sockaddr_in &SenderAddr);
        
        NetworkClient* createClient(const NetworkAddress &ClientAddr);
        void deleteClient(NetworkClient* Client);
        
        void sendClientAllServerInfos(const sockaddr_in &SenderAddr);
        
        s32 sendPacketToAddress(const NetworkPacket &Packet, const sockaddr_in &Address);
        bool examineReceivedPacket(NetworkPacket &Packet, NetworkMember* &Sender, const sockaddr_in &SenderAddr);
        
        void closeNetworkSession();
        void disconnectedByServer();
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
