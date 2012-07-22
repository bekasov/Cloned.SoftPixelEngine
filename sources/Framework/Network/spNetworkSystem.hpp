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


typedef boost::function<void (const NetworkAddress &ServerAddress, const io::stringc &SessionDescription)> SessionAnswerCallback;

static const u16 DEFAULT_PORT = 8100;


//! Enumeration of some predefined port numbers.
enum ENetworkPorts
{
    NETPORT_HTTP    = 80,
    NETPORT_SMPT    = 25,
    NETPORT_FTP     = 20,
};

//! Types of network adapters.
enum ENetworkAdapterTypes
{
    NETADAPTER_OTHER,
    NETADAPTER_ETHERNET,    //!< Ethernet (e.g. for LAN connections).
    NETADAPTER_TOKENRING,   //!< Token Ring (IEEE 802.5).
    NETADAPTER_FDDI,        //!< Fiber Distributed Data Interface.
    NETADAPTER_PPP,         //!< Point to Point Protocol.
    NETADAPTER_LOOPBACK,
    NETADAPTER_SLIP,
};

//! Network adapter information structure.
struct SNetworkAdapter
{
    SNetworkAdapter() :
        Type        (NETADAPTER_OTHER   ),
        IPAddress   ("0.0.0.0"          ),
        IPMask      ("0.0.0.0"          ),
        Enabled     (false              )
    {
    }
    ~SNetworkAdapter()
    {
    }
    
    /* Members */
    ENetworkAdapterTypes Type;
    io::stringc IPAddress;      //!< IP address (e.g. "192.168.0.1").
    io::stringc IPMask;         //!< IP mask (e.g. "255.255.255.0").
    io::stringc Description;    //!< Description of the hardware/software network adapter.
    bool Enabled;               //!< True whether it's enabled.
};


/**
This is the network system class. It currently only uses UDP/IP.
UDP is not safe like TCP but faster and good enough for game networking.
Here is a code sample on how to use the network system as server:
\code
// Configure the session-code which is required that other clients can
// send a broadcast request to know which servers are opened
spNetwork->setSessionCode("TestCode");
spNetwork->setSessionDescriptions("Some description about the running game session");

// Host a server (default Port is 8100)
spNetwork->hostServer();

// < Main loop ... >

network::NetworkPacket RecvPacket;
network::NetworkMember* Sender = 0;

while (spNetwork->receivePacket(RecvPacket, Sender))
{
    // < do something with the received data ... >
}

// Test message. You can send strings or any other buffers
network::NetworkPacket SendPacket("Test message");

// By default send it to all network members (which joined this server)
spNetwork->sendPacket(SendPacket);

// < Main loop end ... >

// Disconnect the server
spNetwork->disconnect();
\endcode
And here is a code sample on how to use it as client:
\code
// Global variable to store one IP address
io::stringc SessionIPAddress;

// Define a session-request answer callback function and store the IP address in a global variable
void SessionAnswerProc(const NetworkAddress &ServerAddress, const io::stringc &SessionDescription)
{
    io::Log::message("Session answer from " + ServerAddress.getIPAddressName() + ": " + SessionDescription);
    SessionIPAddress = ServerAddress.getIPAddressName();
}

// < main function ... >

// Configure the session-code that only servers will be found,
// which run an instance of the same game
spNetwork->setSessionCode("TestCode");

// Configure the session answer callback
spNetwork->setSessionAnswerCallback(SessionAnswerProc);

// Send broadcast request to find open game sessions (use default port)
if (spNetwork->requestNetworkSessionBroadcast() > 0)
{
    // Wait for session-request answers
    while (UserWaitsForServerAnswers)
    {
        // Just process packets to receive session-rueqest answers
        spNetwork->processPackets();
    }
    
    // Disconnect from the session-request part
    spNetwork->disconnect();
}

// Join the server (use default port)
spNetwork->joinServer(SessionIPAddress);

// Wait until the server accepted your connection
while (!spNetwork->isConnected())
{
    // Just process packets to receive server-request answers
    spNetwork->processPackets();
}

// < Main loop ... -> same as for the server >

spNetwork->disconnect();
\endcode
*/
class NetworkSystem
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
        
        //! Sends a network session request to the specified IP address.
        virtual void requestNetworkSession(const io::stringc &IPAddress, u16 Port = DEFAULT_PORT) = 0;
        
        /**
        Sends a network session request broadcast in the local area network and returns the number of requests.
        Server example:
        \code
        spNetwork->setSessionCode("ExampleGame_v.1.0");
        spNetwork->setSessionDescription("Map: 'ExampleMap'");
        \endcode
        Client example:
        \code
        void SessionAnswerProc(const NetworkAddress &Address, const io::stringc &Description)
        {
            // Store the information of the session answer (IP address and description)
        }
        
        //...
        
        spNetwork->setSessionCode("ExampleGame_v.1.0");
        spNetwork->setSessionAnswerCallback(SessionAnswerProc);
        
        if (spNetwork->requestNetworkSessionBroadcast() > 0)
        {
            io::Timer Timer(1000); // Wait for 1000 milliseconds
            while (!Timer.finish())
                spNetwork->processPackets();
        }
        \endcode
        */
        virtual u32 requestNetworkSessionBroadcast(u16 Port = DEFAULT_PORT) = 0;
        
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
        Processes each incomming network packet for internal purposes.
        This is equivalent to the following code snippet:
        \code
        NetworkPacket Packet;
        NetworkMember Sender;
        while (spNetwork->receivePacket(Packet, Sender));
        \endcode
        */
        virtual void processPackets() = 0;
        
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
        Transfers all server permission to the given client.
        \param Client: Specifies the client who is about to become the new server.
        By default 0 that the first client will be used.
        \return True if a client has been found wich became the new server.
        \note This only works if this network system has opened the server.
        */
        virtual bool transferServerPermission(NetworkClient* Client = 0) = 0;
        
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
        
        /**
        Sets the sessions code. This should be used that 'scanning the network' for open game sessions
        only works for your video game. e.g. "YourGameName" + "YourGameVersion" is a good session code.
        */
        inline void setSessionCode(const io::stringc &SessionCode)
        {
            SessionCode_ = SessionCode;
        }
        //! Returns the sessions code.
        inline io::stringc getSessionCode() const
        {
            return SessionCode_;
        }
        
        /**
        Sets the sessions description. This will be sent to a client who 'scans' the network for open game sessions.
        e.g. you can fill in the current map of your game with some other game-server information.
        */
        inline void setSessionDescription(const io::stringc &Description)
        {
            SessionDescription_ = Description;
        }
        //! Returns the sessions description.
        inline io::stringc getSessionDescription() const
        {
            return SessionDescription_;
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
        
        /**
        Sets the sessions answer callback. This will be called when
        your sessions request gets an answer from a running game server.
        */
        inline void setSessionAnswerCallback(const SessionAnswerCallback &Callback)
        {
            SessionAnswerCallback_ = Callback;
        }
        
    protected:
        
        /* === Macros === */
        
        static const u32 RECVBUFFER_SIZE = 4096;
        
        static const size_t IP_SIZE;
        static const size_t PORT_SIZE;
        static const size_t ADDR_SIZE;
        
        /* === Functions === */
        
        NetworkSystem();
        
        #if defined(SP_PLATFORM_WINDOWS)
        void createWinSock();
        void deleteWinSock();
        #endif
        
        void openSocket();
        void closeSocket();
        
        u64 convertAddress(const sockaddr_in &Addr) const;
        
        NetworkAddress readAddressFromBuffer(const c8* Buffer) const;
        void writeAddressToBuffer(c8* Buffer, const NetworkAddress &Address);
        
        /* === Members === */
        
        #if defined(SP_PLATFORM_WINDOWS)
        WSADATA WinSock_;
        #endif
        
        NetworkSocket* Socket_;
        
        NetworkServer* Server_;
        std::list<NetworkClient*> ClientList_;
        
        std::list<NetworkClient*> ClientJointStack_;
        std::list<NetworkClient*> ClientLeaveStack_;
        
        std::map<u64, NetworkMember*> MemberMap_;
        
        io::stringc SessionCode_;
        io::stringc SessionDescription_;
        
        bool isSessionRunning_;
        bool isConnected_;
        bool hasOpenedServer_;
        
        SessionAnswerCallback SessionAnswerCallback_;
        
        static c8 RecvBuffer_[RECVBUFFER_SIZE];
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
