/*
 * Network system TCP header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_SYSTEM_TCP_H__
#define __SP_NETWORK_SYSTEM_TCP_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkSystem.hpp"
#include "Framework/Network/spNetworkBase.hpp"


namespace sp
{
namespace network
{


//! TCP/IP network system class.
class NetworkSystemTCP : public NetworkBase, public NetworkSystem
{
    
    public:
        
        NetworkSystemTCP();
        ~NetworkSystemTCP();
        
        /* === Functions === */
        
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
        
        
        
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
