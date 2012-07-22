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
    NetworkSystem()
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
    openSocket();
    
    /* Create server object */
    Server_ = new NetworkServer(NetworkAddress(Port));
    
    /* Bind server */
    if (!bindSocketToPort(Port))
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
    openSocket();
    
    /* Create server object */
    NetworkAddress Addr(Port, IPAddress);
    
    Server_ = new NetworkServer(Addr);
    
    /* Bind server */
    if (!bindSocketToPort(Port))
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

void NetworkSystemUDP::requestNetworkSession(const io::stringc &IPAddress, u16 Port)
{
    /* Disconnect from previous connection and open new socket */
    disconnect();
    openSocket();
    
    /* General settings */
    isSessionRunning_ = true;
    
    /* Bind to port */
    if (bindSocketToPort(Port))
    {
        io::Log::message("Request network session for " + IPAddress + " : " + io::stringc(Port));
        
        NetworkPacket Packet(SessionCode_);
        Packet.setDescriptor(DESCRIPTOR_SESSION_REQUEST);
        
        /* Send session request message */
        sendPacketToAddress(Packet, NetworkAddress(Port, IPAddress).getSocketAddress());
    }
    else
        disconnect();
}

u32 NetworkSystemUDP::requestNetworkSessionBroadcast(u16 Port)
{
    /* Determine broadcast IP addresses */
    std::list<io::stringc> BroadcastIPList;
    
    foreach (const SNetworkAdapter &Adapter, getNetworkAdapters())
    {
        /* Only consider enabled ethernet adapters */
        if (!Adapter.Enabled || Adapter.IPAddress == "0.0.0.0")
            continue;
        
        /* Get IP address */
        u8 IPAddress[4];
        *((u32*)IPAddress) = inet_addr(Adapter.IPAddress.c_str());
        
        /* Get IP mask */
        u8 IPMask[4];
        *((u32*)IPMask) = inet_addr(Adapter.IPMask.c_str());
        
        /* Get broad cast IP address */
        io::stringc BroadcastIP;
        
        BroadcastIP += (IPMask[0] > 0 ? io::stringc(static_cast<s32>(IPAddress[0])) : "255");
        BroadcastIP += ".";
        BroadcastIP += (IPMask[1] > 0 ? io::stringc(static_cast<s32>(IPAddress[1])) : "255");
        BroadcastIP += ".";
        BroadcastIP += (IPMask[2] > 0 ? io::stringc(static_cast<s32>(IPAddress[2])) : "255");
        BroadcastIP += ".";
        BroadcastIP += (IPMask[3] > 0 ? io::stringc(static_cast<s32>(IPAddress[3])) : "255");
        
        BroadcastIPList.push_back(BroadcastIP);
    }
    
    /* Make broadcast IP list unique */
    BroadcastIPList.unique();
    
    /* Send all broadcast requests */
    foreach (const io::stringc IPAddress, BroadcastIPList)
        requestNetworkSession(IPAddress, Port);
    
    return BroadcastIPList.size();
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
    s32 SenderAddrSize = sizeof(SenderAddr);
    
    s32 Result = recvfrom(
        Socket_->getSocket(),
        NetworkSystem::RecvBuffer_,
        NetworkSystem::RECVBUFFER_SIZE,
        0,
        (SOCKADDR*)&SenderAddr,
        &SenderAddrSize
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
    Packet = NetworkPacket(NetworkSystem::RecvBuffer_, Result, true);
    
    /* Get sender client */
    Sender = getMemberByAddress(SenderAddr);
    
    /* Analyze network message */
    return examineReceivedPacket(Packet, Sender, SenderAddr);
}

void NetworkSystemUDP::processPackets()
{
    NetworkPacket Packet;
    NetworkMember* Sender = 0;
    while (receivePacket(Packet, Sender));
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

// !untested!
bool NetworkSystemUDP::transferServerPermission(NetworkClient* Client)
{
    /* Exit if this network system is not the server of no client has already joined it */
    if (!isServer() || ClientList_.empty())
        return false;
    
    if (!Client)
        Client = ClientList_.front();
    
    /* Send information of transfering server permissions to the affected client */
    sendPacket(NetworkPacket(DESCRIPTOR_BECOME_SERVER), Client);
    
    /* Send information of transfering server permissions to each other clients */
    NetworkPacket Packet(NetworkSystem::ADDR_SIZE);
    Packet.setDescriptor(DESCRIPTOR_INTRODUCE_CLIENT);
    
    writeAddressToBuffer(Packet.getBuffer(), Client->getAddress());
    
    foreach (NetworkClient* OtherClient, ClientList_)
    {
        if (OtherClient && OtherClient != Client)
            sendPacket(Packet, OtherClient);
    }
    
    /* Remove server permissions */
    hasOpenedServer_    = false;
    Server_->Address_   = Client->getAddress();
    
    /* Remove client -> because he has become the server */
    deleteClient(Client);
    
    return true;
}

NetworkMember* NetworkSystemUDP::getMemberByAddress(const NetworkAddress &Address)
{
    return getMemberByAddress(Address.getSocketAddress());
}


/*
 * ======= Private: =======
 */

bool NetworkSystemUDP::bindSocketToPort(u16 Port)
{
    /* Create connection-less address */
    if (!Socket_->bindSocket(NetworkAddress(Port)))
    {
        disconnect();
        return false;
    }
    return true;
}

void NetworkSystemUDP::registerMember(NetworkMember* Member)
{
    u64 AddrCode = convertAddress(Member->getAddress().getSocketAddress());
    MemberMap_[AddrCode] = Member;
}

NetworkMember* NetworkSystemUDP::getMemberByAddress(const sockaddr_in &SenderAddr)
{
    const u64 AddrCode = convertAddress(SenderAddr);
    
    /* Check if sender address was already regisitered as a client */
    std::map<u64, NetworkMember*>::iterator it = MemberMap_.find(AddrCode);
    
    if (it != MemberMap_.end())
        return it->second;
    
    return 0;
}

NetworkClient* NetworkSystemUDP::createClient(const NetworkAddress &ClientAddr)
{
    /* Create new client object */
    NetworkClient* NewClient = new NetworkClient(ClientAddr);
    
    /* Register new client */
    registerMember(NewClient);
    
    /* Add to all relevant lists */
    ClientList_.push_back(NewClient);
    ClientJointStack_.push_back(NewClient);
    
    return NewClient;
}

void NetworkSystemUDP::deleteClient(NetworkClient* Client)
{
    if (!Client)
        return;
    
    /* Remove client from all relevant lists */
    MemoryManager::removeElement(ClientList_, Client);
    MemoryManager::removeElement(ClientJointStack_, Client);
    
    std::map<u64, NetworkMember*>::iterator it = MemberMap_.find(
        convertAddress(Client->getAddress().getSocketAddress())
    );
    
    if (it != MemberMap_.end())
        MemberMap_.erase(it);
    
    /* Add to leave stack -> now the client must no longer be used */
    ClientLeaveStack_.push_back(Client);
    
    /* Delete client object */
    MemoryManager::deleteMemory(Client);
}

void NetworkSystemUDP::sendClientAllServerInfos(const sockaddr_in &SenderAddr)
{
    if (!isServer())
        return;
    
    /* Create packet for new client */
    const u32 BufferSize = ClientList_.size() * NetworkSystem::ADDR_SIZE;
    
    NetworkPacket Packet(BufferSize);
    
    c8* Buffer = Packet.getBuffer();
    Packet.setDescriptor(DESCRIPTOR_CONNECTION_ACCEPTED);
    
    /* Create packet for current clients */
    NetworkPacket IntroduceClientPacket(NetworkSystem::ADDR_SIZE);
    IntroduceClientPacket.setDescriptor(DESCRIPTOR_INTRODUCE_CLIENT);
    
    writeAddressToBuffer(IntroduceClientPacket.getBuffer(), SenderAddr);
    
    /* Fill packet */
    foreach (NetworkClient* Member, ClientList_)
    {
        /* Write port number and IP address to the buffer */
        writeAddressToBuffer(Buffer, Member->getAddress());
        Buffer += NetworkSystem::ADDR_SIZE;
        
        /* Send current client information about the new client */
        sendPacketToAddress(IntroduceClientPacket, Member->getAddress().getSocketAddress());
    }
    
    /* Send client the infos */
    sendPacketToAddress(Packet, SenderAddr);
}

s32 NetworkSystemUDP::sendPacketToAddress(const NetworkPacket &Packet, const sockaddr_in &Address)
{
    /* Send network packet to raw address */
    return sendto(
        Socket_->getSocket(),
        Packet.getRealBuffer(),
        Packet.getRealBufferSize(),
        0,
        (SOCKADDR*)(&Address),
        sizeof(sockaddr_in)
    );
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
            createClient(readAddressFromBuffer(Packet.getBuffer()));
        }
        break;
        
        case DESCRIPTOR_CONNECTION_ACCEPTED:
        {
            if (!isConnected_)
            {
                isConnected_ = true;
                
                /* Receive all client addresses which are available since start-up */
                const c8* Buffer = Packet.getBuffer();
                
                for (u32 i = 0, c = Packet.getBufferSize() / NetworkSystem::ADDR_SIZE; i < c; ++i)
                {
                    /* Create new client */
                    createClient(readAddressFromBuffer(Buffer));
                    Buffer += NetworkSystem::ADDR_SIZE;
                }
            }
        }
        break;
        
        case DESCRIPTOR_SERVER_DISCONNECTED:
        {
            disconnectedByServer();
        }
        break;
        
        case DESCRIPTOR_SESSION_REQUEST:
        {
            /* Compare session code */
            if (isServer() && Packet.compareString(SessionCode_))
            {
                /* Answer request with session description */
                NetworkPacket AnswerPacket(SessionDescription_);
                AnswerPacket.setDescriptor(DESCRIPTOR_SESSION_ANSWER);
                sendPacketToAddress(AnswerPacket, SenderAddr);
            }
        }
        break;
        
        case DESCRIPTOR_SESSION_ANSWER:
        {
            /* Callback session answer function */
            if (SessionAnswerCallback_)
                SessionAnswerCallback_(NetworkAddress(SenderAddr), Packet.getString());
        }
        break;
        
        // !untested!
        case DESCRIPTOR_BECOME_SERVER:
        {
            if (!isServer())
            {
                /* Add current server as client -> because he is no longer the server */
                createClient(Server_->getAddress());
                
                /* Get server permissions */
                hasOpenedServer_    = true;
                Server_->Address_   = NetworkAddress(Server_->getAddress().getPort());
            }
        }
        break;
        
        // !untested!
        case DESCRIPTOR_CLIENT_TRANSFERS_SERVER:
        {
            /* Read new server address */
            NetworkAddress NewServerAddr(readAddressFromBuffer(Packet.getBuffer()));
            NetworkMember* NewServer = getMemberByAddress(NewServerAddr);
            
            if (NewServer && !NewServer->isServer())
            {
                /* Add current server as client -> because he is no longer the server */
                createClient(Server_->getAddress());
                
                /* Give client server permissions and remove him as client */
                Server_->Address_ = NewServerAddr;
                deleteClient(static_cast<NetworkClient*>(NewServer));
            }
            else
                io::Log::error("Server permission transfer failed");
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

void NetworkSystemUDP::closeNetworkSession()
{
    /* Delete clients and server */
    MemoryManager::deleteList(ClientList_);
    MemoryManager::deleteMemory(Server_);
    
    ClientJointStack_.clear();
    ClientLeaveStack_.clear();
    
    MemberMap_.clear();
    
    /* Reset states */
    isSessionRunning_   = false;
    isConnected_        = false;
    hasOpenedServer_    = false;
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
