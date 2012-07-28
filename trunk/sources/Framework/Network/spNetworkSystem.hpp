/*
 * Network system header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_SYSTEM_H__
#define __SP_NETWORK_SYSTEM_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkClient.hpp"
#include "Framework/Network/spNetworkServer.hpp"
#include "Framework/Network/spNetworkPacket.hpp"
#include "Framework/Network/spNetworkSocket.hpp"

#include <list>
#include <boost/function.hpp>


namespace sp
{
namespace network
{


static const u16 DEFAULT_PORT = 8100;


class SP_EXPORT NetworkSystem
{
    
    public:
        
        virtual ~NetworkSystem();
        
        /* === Functions === */
        
        //! Returns the network system version.
        virtual io::stringc getVersion() const;
        
        //! Hosts a new network server.
        virtual NetworkServer* hostServer(u16 Port = DEFAULT_PORT) = 0;
        
        //! Joins the network server.
        virtual NetworkServer* joinServer(const io::stringc &IPAddress, u16 Port = DEFAULT_PORT) = 0;
        
        //! Disconnects from the current network server and closes the network session.
        virtual void disconnect() = 0;
        
        /**
        Sends a network packet to the given network member.
        \param Packet: Specifies the network packet which is to be send.
        \param Receiver: Specifies the network member which is to receive the packet.
        By default 0 which means that each member gets the packet.
        \return True if the function succeeded.
        */
        virtual bool sendPacket(const NetworkPacket &Packet, NetworkMember* Receiver = 0) = 0;
        /**
        Receives a network packet and returns the sender.
        \param Packet: Returns a new incomming message.
        \param Sender: Returns the sender.
        \return True if a new network packet has been received.
        */
        virtual bool receivePacket(NetworkPacket &Packet, NetworkMember* &Sender) = 0;
        
        /**
        Processes each incomming network packet only for internal purposes.
        This is equivalent to the following code snippet:
        \code
        NetworkPacket Packet;
        NetworkMember* Sender = 0;
        while (spNetwork->receivePacket(Packet, Sender));
        \endcode
        */
        void processPackets();
        
        /**
        Processes each incomming netowrk packet and retuns true once the server accepted the connection.
        Use this as a UDP client after joining a server.
        */
        bool waitForConnection();
        
        /**
        Pops a client from the join-stack. Use this to determine when a new client joined the server.
        \param Client: Specifies where the client pointer is to be stored.
        \return True if a new client has joined the server.
        \code
        // Example:
        NetworkClient* NewClient;
        while (spNetwork->popClientJoinStack(NewClient))
            AddNewClientToGame(NewClient);
        \endcode
        */
        virtual bool popClientJoinStack(NetworkClient* &Client) = 0;
        
        /**
        Pops a client from the leave-stack. Use this to determine when a client left the server.
        \param Client: Specifies where the client pointer is to be stored.
        \return True if a client has left the server.
        \code
        // Example:
        NetworkClient* LeftClient;
        while (spNetwork->popClientLeaveStack(LeftClient))
            RemoveOldClientFromGame(LeftClient);
        \endcode
        \note Here you only have to use the pointer for comparision but no longer use the object,
        because it has already been deleted!
        */
        virtual bool popClientLeaveStack(NetworkClient* &Client) = 0;
        
        /**
        Returns a pointer to the network member with the given address or
        0 (null) if there is no member with this address.
        */
        virtual NetworkMember* getMemberByAddress(const NetworkAddress &Address) = 0;
        
        /**
        Returns the IP address of the specified host name.
        \param HostName: Specifies the host name (or rather local PC name).
        */
        io::stringc getHostIPAddress(const io::stringc &HostName) const;
        
        //! Returns the official host name (e.g. "www.google.com" to "www.l.google.com").
        io::stringc getOfficialHostName(const io::stringc &HostName) const;
        
        /**
        Gets all IP addresses of the specified host name.
        \param HostName: Specifies the host name (or rather local PC name):
        \return List of all IP addresses.
        */
        std::list<io::stringc> getHostIPAddressList(const io::stringc &HostName) const;
        
        /**
        Gets all network member host names (or rather local PC names).
        \return List of all host names.
        \note This function sometimes needs a couple of seconds and currently is only supported for MS/Windows.
        */
        std::list<io::stringc> getNetworkMembers() const;
        
        //! Returns all network adapters on the local computer.
        std::list<SNetworkAdapter> getNetworkAdapters() const;
        
        /**
        Returns a unique list of all available broadcast IP addresses. This depends on the count of network adapters.
        \see getNetworkAdapters
        */
        std::list<io::stringc> getBroadcastIPList() const;
        
        /* === Inline functions === */
        
        //! Returns pointer to the NetworkServer object.
        inline NetworkServer* getServer() const
        {
            return Server_;
        }
        //! Returns the whole client object list.
        inline std::list<NetworkClient*> getClientList() const
        {
            return ClientList_;
        }
        
        //! Returns true if a network session is currently running, i.e. a server has been opened or joined.
        inline bool isSessionRunning() const
        {
            return isSessionRunning_;
        }
        //! Returns true if a network session is currently connected, i.e. a server has been opened or an external joined server has accepted the connection.
        inline bool isConnected() const
        {
            return isConnected_;
        }
        //! Returns true if a network server was opened by this computer (in this case this network system is the server).
        inline bool isServer() const
        {
            return hasOpenedServer_;
        }
        
    protected:
        
        /* === Functions === */
        
        NetworkSystem();
        
        #if defined(SP_PLATFORM_WINDOWS)
        void createWinSock();
        void deleteWinSock();
        #endif
        
        void registerMember(NetworkMember* Member);
        NetworkMember* getMemberByAddress(const sockaddr_in &SenderAddr);
        
        NetworkClient* createClient(const NetworkAddress &ClientAddr);
        void deleteClient(NetworkClient* Client);
        
        void closeNetworkSession();
        
        /* === Members === */
        
        #if defined(SP_PLATFORM_WINDOWS)
        WSADATA WinSock_;
        #endif
        
        NetworkServer* Server_;
        std::list<NetworkClient*> ClientList_;
        
        std::list<NetworkClient*> ClientJointStack_;
        std::list<NetworkClient*> ClientLeaveStack_;
        
        std::map<u64, NetworkMember*> MemberMap_;
        
        bool isSessionRunning_;
        bool isConnected_;
        bool hasOpenedServer_;
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
