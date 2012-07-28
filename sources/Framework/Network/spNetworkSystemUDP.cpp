/*
 * Network system UDP file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkSystemUDP.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Base/spInputOutputLog.hpp"
#include "Base/spTimer.hpp"
#include "Base/spMemoryManagement.hpp"

#include <lm.h>
#include <Iphlpapi.h>
#include <boost/foreach.hpp>


namespace sp
{
namespace network
{


NetworkSystemUDP::NetworkSystemUDP() :
    NetworkBaseUDP  (),
    NetworkSystem   ()
{
}
NetworkSystemUDP::~NetworkSystemUDP()
{
    disconnect();
}

NetworkServer* NetworkSystemUDP::hostServer(u16 Port)
{
    /* Disconnect from previous connection and open new socket */
    disconnect();
    openSocket(PROTOCOL_UDP);
    
    /* Create server object */
    Server_ = new NetworkServer(NetworkAddress(Port));
    
    /* Bind server */
    if (!bindToPort(Port))
        return 0;
    
    /* General settings */
    isSessionRunning_   = true;
    hasOpenedServer_    = true;
    isConnected_        = true;
    
    io::Log::message("Opened network server ( " + io::stringc(Port) + " )");
    
    return Server_;
}

NetworkServer* NetworkSystemUDP::joinServer(const io::stringc &IPAddress, u16 Port)
{
    /* Disconnect from previous connection and open new socket */
    disconnect();
    openSocket(PROTOCOL_UDP);
    
    /* Create server object */
    NetworkAddress Addr(Port, IPAddress);
    
    Server_ = new NetworkServer(Addr);
    
    /* Bind server */
    if (!bindToPort(Port))
        return 0;
    
    /* Register server */
    registerMember(Server_);
    
    /* Send registration message */
    sendPacket(NetworkPacket(DESCRIPTOR_CLIENT_JOIN), Server_);
    
    /* General settings */
    isSessionRunning_ = true;
    
    io::Log::message("Joined network server ( " + Addr.getDescription() + " )");
    
    return Server_;
}

void NetworkSystemUDP::disconnect()
{
    if (!isSessionRunning_)
        return;
    
    /* Disconnect from the server */
    sendPacket(NetworkPacket(isServer() ? DESCRIPTOR_SERVER_DISCONNECTED : DESCRIPTOR_CLIENT_LEFT));
    
    /* Wait a moment that the last packet can be send */
    io::Timer::sleep(100);
    
    /* Close network session and socket */
    closeNetworkSession();
    closeSocket();
    
    io::Log::message("Disconnected from server");
}

bool NetworkSystemUDP::sendPacket(const NetworkPacket &Packet, NetworkMember* Receiver)
{
    if (!Socket_)
        return false;
    
    if (Receiver)
    {
        /* Send network packet */
        s32 Result = sendPacketToAddress(Packet, Receiver->getAddress().getSocketAddress());
        
        /* Check for error */
        if (Result == SOCKET_ERROR)
        {
            io::Log::error("Could not send network packet to " + Receiver->getAddress().getIPAddressName());
            return false;
        }
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

bool NetworkSystemUDP::receivePacket(NetworkPacket &Packet, NetworkMember* &Sender)
{
    if (!Socket_)
        return false;
    
    /* Receive network packet */
    sockaddr_in SenderAddr;
    
    if (!receivePacketFromAddress(Packet, SenderAddr))
        return false;
    
    /* Get sender client */
    Sender = getMemberByAddress(SenderAddr);
    
    /* Analyze network message */
    return examineReceivedPacket(Packet, Sender, SenderAddr);
}

bool NetworkSystemUDP::popClientJoinStack(NetworkClient* &Client)
{
    if (!ClientJointStack_.empty())
    {
        Client = ClientJointStack_.front();
        ClientJointStack_.pop_front();
        return true;
    }
    return false;
}

bool NetworkSystemUDP::popClientLeaveStack(NetworkClient* &Client)
{
    if (!ClientLeaveStack_.empty())
    {
        Client = ClientLeaveStack_.front();
        ClientLeaveStack_.pop_front();
        return true;
    }
    return false;
}

NetworkMember* NetworkSystemUDP::getMemberByAddress(const NetworkAddress &Address)
{
    return NetworkSystem::getMemberByAddress(Address.getSocketAddress());
}


/*
 * ======= Private: =======
 */

bool NetworkSystemUDP::bindToPort(u16 Port)
{
    /* Create connection-less address */
    if (!bindSocketToPort(Port))
    {
        disconnect();
        return false;
    }
    return true;
}

void NetworkSystemUDP::sendClientAllServerInfos(const sockaddr_in &SenderAddr)
{
    if (!isServer())
        return;
    
    /* Create packet for new client */
    const u32 BufferSize = ClientList_.size() * NetworkAddress::ADDR_SIZE;
    
    NetworkPacket Packet(BufferSize);
    
    c8* Buffer = Packet.getBuffer();
    Packet.setDescriptor(DESCRIPTOR_CONNECTION_ACCEPTED);
    
    /* Create packet for current clients */
    NetworkPacket IntroduceClientPacket(NetworkAddress::ADDR_SIZE);
    IntroduceClientPacket.setDescriptor(DESCRIPTOR_INTRODUCE_CLIENT);
    
    NetworkAddress::write(IntroduceClientPacket.getBuffer(), SenderAddr);
    
    /* Fill packet */
    foreach (NetworkClient* Member, ClientList_)
    {
        /* Write port number and IP address to the buffer */
        NetworkAddress::write(Buffer, Member->getAddress());
        Buffer += NetworkAddress::ADDR_SIZE;
        
        /* Send current client information about the new client */
        sendPacketToAddress(IntroduceClientPacket, Member->getAddress().getSocketAddress());
    }
    
    /* Send client the infos */
    sendPacketToAddress(Packet, SenderAddr);
}

bool NetworkSystemUDP::examineReceivedPacket(
    NetworkPacket &Packet, NetworkMember* &Sender, const sockaddr_in &SenderAddr)
{
    switch (Packet.getDescriptor())
    {
        case DESCRIPTOR_CLIENT_JOIN:
        {
            /* Check if a new client joined the server */
            if (!Sender)
            {
                /* Send client all server information */
                sendClientAllServerInfos(SenderAddr);
                
                /* Create and register new client */
                Sender = createClient(SenderAddr);
            }
        }
        break;
        
        case DESCRIPTOR_CLIENT_LEFT:
        {
            /* Delete client */
            if (Sender && !Sender->isServer())
                deleteClient(static_cast<NetworkClient*>(Sender));
        }
        break;
        
        case DESCRIPTOR_INTRODUCE_CLIENT:
        {
            /* Create new client which has been introduced by the server */
            createClient(NetworkAddress::read(Packet.getBuffer()));
        }
        break;
        
        case DESCRIPTOR_CONNECTION_ACCEPTED:
        {
            if (!isConnected_)
            {
                isConnected_ = true;
                
                /* Receive all client addresses which are available since start-up */
                const c8* Buffer = Packet.getBuffer();
                
                for (u32 i = 0, c = Packet.getBufferSize() / NetworkAddress::ADDR_SIZE; i < c; ++i)
                {
                    /* Create new client */
                    createClient(NetworkAddress::read(Buffer));
                    Buffer += NetworkAddress::ADDR_SIZE;
                }
            }
        }
        break;
        
        case DESCRIPTOR_SERVER_DISCONNECTED:
        {
            disconnectedByServer();
        }
        break;
        
        default:
            /* No reserved message -> custom network packet */
            return Sender != 0;
    }
    
    /* No message or a reserved message has been received */
    return false;
}

void NetworkSystemUDP::disconnectedByServer()
{
    if (isSessionRunning_)
    {
        /* Close network session */
        closeNetworkSession();
        io::Log::message("Disconnected by server");
    }
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
