/*
 * Network system TCP file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkSystemTCP.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Base/spInputOutputLog.hpp"
#include "Base/spTimer.hpp"
#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace network
{


NetworkSystemTCP::NetworkSystemTCP() :
    NetworkBase     (),
    NetworkSystem   ()
{
}
NetworkSystemTCP::~NetworkSystemTCP()
{
    disconnect();
}

io::stringc NetworkSystemTCP::getDescription() const
{
    return "TCP/IP Network";
}

NetworkServer* NetworkSystemTCP::hostServer(u16 Port)
{
    return 0;
}

NetworkServer* NetworkSystemTCP::joinServer(const io::stringc &IPAddress, u16 Port)
{
    return 0;
}

void NetworkSystemTCP::disconnect()
{
    
    //todo
    
}

bool NetworkSystemTCP::sendPacket(const NetworkPacket &Packet, NetworkMember* Receiver)
{
    if (!Socket_)
        return false;
    
    if (Receiver)
    {
        
        //todo
        
    }
    else
    {
        /* Send packet to each client */
        foreach (NetworkClient* Client, ClientList_)
        {
            if (Client && !sendPacket(Packet, Client))
                return false;
        }
        
        /* Send packet to the server */
        if (!isServer() && Server_ && !sendPacket(Packet, Server_))
            return false;
    }
    
    return true;
}

bool NetworkSystemTCP::receivePacket(NetworkPacket &Packet, NetworkMember* &Sender)
{
    if (!Socket_)
        return false;
    
    
    return false;
}

bool NetworkSystemTCP::popClientJoinStack(NetworkClient* &Client)
{
    if (!ClientJointStack_.empty())
    {
        Client = ClientJointStack_.front();
        ClientJointStack_.pop_front();
        return true;
    }
    return false;
}

bool NetworkSystemTCP::popClientLeaveStack(NetworkClient* &Client)
{
    if (!ClientLeaveStack_.empty())
    {
        Client = ClientLeaveStack_.front();
        ClientLeaveStack_.pop_front();
        return true;
    }
    return false;
}

NetworkMember* NetworkSystemTCP::getMemberByAddress(const NetworkAddress &Address)
{
    return getMemberByAddress(Address.getSocketAddress());
}


/*
 * ======= Private: =======
 */




} // /namespace network

} // /namespace sp


#endif



// ================================================================================
