/*
 * Network system file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkSystem.hpp"

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


NetworkSystem::NetworkSystem() :
    Server_             (0      ),
    isSessionRunning_   (false  ),
    isConnected_        (false  ),
    hasOpenedServer_    (false  )
{
    io::Log::message(getVersion());
    io::Log::message("Copyright (c) 2012 - Lukas Hermanns");
    
    #if defined(SP_PLATFORM_WINDOWS)
    createWinSock();
    #endif
}
NetworkSystem::~NetworkSystem()
{
    #if defined(SP_PLATFORM_WINDOWS)
    deleteWinSock();
    #endif
}

io::stringc NetworkSystem::getVersion() const
{
    return "SoftPixel NetworkSystem - v.4.0";
}

void NetworkSystem::processPackets()
{
    NetworkPacket Packet;
    NetworkMember* Sender = 0;
    
    while (receivePacket(Packet, Sender))
    {
        // do nothing
    }
}

bool NetworkSystem::waitForConnection()
{
    NetworkPacket Packet;
    NetworkMember* Sender = 0;
    
    do
    {
        if (isConnected())
            return true;
    }
    while (receivePacket(Packet, Sender));
    
    return false;
}

io::stringc NetworkSystem::getHostIPAddress(const io::stringc &HostName) const
{
    /* Get host information */
    hostent* HostInfo = gethostbyname(HostName.c_str());
    
    if (HostInfo && HostInfo->h_addrtype == AF_INET)
    {
        /* Extract IP address */
        u8* IPAddress = reinterpret_cast<u8*>(HostInfo->h_addr);
        
        return (
            io::stringc(static_cast<s32>(IPAddress[0])) + "." +
            io::stringc(static_cast<s32>(IPAddress[1])) + "." +
            io::stringc(static_cast<s32>(IPAddress[2])) + "." +
            io::stringc(static_cast<s32>(IPAddress[3]))
        );
    }
    
    return "";
}

io::stringc NetworkSystem::getOfficialHostName(const io::stringc &HostName) const
{
    /* Get host information */
    hostent* HostInfo = gethostbyname(HostName.c_str());
    return (HostInfo && HostInfo->h_addrtype == AF_INET) ? io::stringc(HostInfo->h_name) : "";
}

std::list<io::stringc> NetworkSystem::getHostIPAddressList(const io::stringc &HostName) const
{
    std::list<io::stringc> AddressList;
    
    /* Get host information */
    hostent* HostInfo = gethostbyname(HostName.c_str());
    
    if (HostInfo && HostInfo->h_addrtype == AF_INET)
    {
        s32 i = 0;
        c8* pIPAddr;
        
        /* Extract each IP address */
        while ( ( pIPAddr = HostInfo->h_addr_list[i++] ) != 0 )
        {
            AddressList.push_back(
                io::stringc(static_cast<s32>(static_cast<u8>(pIPAddr[0]))) + "." +
                io::stringc(static_cast<s32>(static_cast<u8>(pIPAddr[1]))) + "." +
                io::stringc(static_cast<s32>(static_cast<u8>(pIPAddr[2]))) + "." +
                io::stringc(static_cast<s32>(static_cast<u8>(pIPAddr[3])))
            );
        }
    }
    
    return AddressList;
}

std::list<io::stringc> NetworkSystem::getNetworkMembers() const
{
    std::list<io::stringc> MemberList;
    
    #if defined(SP_PLATFORM_WINDOWS)
    
    SERVER_INFO_100* Buffer = 0;
    DWORD Count, i;
    
    /* Get server enumeration */
    NET_API_STATUS NetStatus = NetServerEnum(0, 100, (LPBYTE*)&Buffer, MAX_PREFERRED_LENGTH, &i, &Count, SV_TYPE_ALL, 0, 0);
    
    if (NetStatus != NERR_Success)
    {
        io::stringc ErrorStr;
        
        switch (NetStatus)
        {
            case ERROR_ACCESS_DENIED:
                ErrorStr = "Access was denied"; break;
            case ERROR_INVALID_PARAMETER:
                ErrorStr = "The parameter is incorrect"; break;
            case ERROR_MORE_DATA:
                ErrorStr = "More entries are available"; break;
            case ERROR_NO_BROWSER_SERVERS_FOUND:
                ErrorStr = "No browser servers found"; break;
            case ERROR_NOT_SUPPORTED:
                ErrorStr = "The request is not supported"; break;
            case NERR_RemoteErr:
                ErrorStr = "A remote error occurred with no data returned by the server"; break;
            case NERR_ServerNotStarted:
                ErrorStr = "The server service is not started"; break;
            case NERR_ServiceNotInstalled:
                ErrorStr = "The service has not been started"; break;
            case NERR_WkstaNotStarted:
                ErrorStr = "The Workstation service has not been started"; break;
            default:
                ErrorStr = "Unknown error"; break;
        }
        
        io::Log::error("Network member enumeration failed (" + ErrorStr + ")");
        
        return MemberList;
    }
    
    /* Extract network members */
    for (i = 0; i < Count; ++i)
    {
        /* Add name to the list */
        MemberList.push_back(
            io::stringw(std::wstring(reinterpret_cast<c16*>(Buffer[i].sv100_name))).toAscii().c_str()
        );
    }
    
    #endif
    
    return MemberList;
}

std::list<SNetworkAdapter> NetworkSystem::getNetworkAdapters() const
{
    std::list<SNetworkAdapter> AdapterList;
    
    #if defined(SP_PLATFORM_WINDOWS)
    
    #define MALLOC(x)   HeapAlloc(GetProcessHeap(), 0, (x))
    #define FREE(x)     HeapFree(GetProcessHeap(), 0, (x))
    
    /* Get first network adapter */
    PIP_ADAPTER_INFO AdapterInfo, Adapter;
    ULONG BufferSize = sizeof(AdapterInfo);
    
    AdapterInfo = (IP_ADAPTER_INFO*)MALLOC(sizeof(IP_ADAPTER_INFO));
    
    if (!AdapterInfo)
    {
        io::Log::error("Error allocating memory needed to call GetAdaptersinfo");
        return AdapterList;
    }
    
    if (GetAdaptersInfo(AdapterInfo, &BufferSize) == ERROR_BUFFER_OVERFLOW)
    {
        FREE(AdapterInfo);
        AdapterInfo = (IP_ADAPTER_INFO*)MALLOC(BufferSize);
        
        if (!AdapterInfo)
        {
            io::Log::error("Error allocating memory needed to call GetAdaptersinfo");
            return AdapterList;
        }
    }
    
    DWORD Result = GetAdaptersInfo(AdapterInfo, &BufferSize);
    
    if (Result != NO_ERROR)
    {   
        io::stringc ErrorStr;
        
        switch (Result)
        {
            case ERROR_BUFFER_OVERFLOW:
                ErrorStr = "Buffer overflow"; break;
            case ERROR_INVALID_DATA:
                ErrorStr = "Invalid data"; break;
            case ERROR_INVALID_PARAMETER:
                ErrorStr = "Invalid parameter"; break;
            case ERROR_NO_DATA:
                ErrorStr = "No data"; break;
            case ERROR_NOT_SUPPORTED:
                ErrorStr = "Not supported"; break;
            default:
                ErrorStr = "Unknown error"; break;
        }
        
        io::Log::error("Could not get network adapter information (" + ErrorStr + ")");
        
        return AdapterList;
    }
    
    /* Iterate over all network adapters */
    for (Adapter = AdapterInfo; Adapter; Adapter = Adapter->Next)
    {
        /* Copy information */
        SNetworkAdapter Info;
        
        Info.Description    = Adapter->Description;
        Info.IPAddress      = Adapter->IpAddressList.IpAddress.String;
        Info.IPMask         = Adapter->IpAddressList.IpMask.String;
        Info.Enabled        = (Adapter->DhcpEnabled != 0);
        
        /* Get adapter type */
        switch (Adapter->Type)
        {
            case MIB_IF_TYPE_OTHER:
                Info.Type = NETADAPTER_OTHER; break;
            case MIB_IF_TYPE_ETHERNET:
                Info.Type = NETADAPTER_ETHERNET; break;
            case MIB_IF_TYPE_TOKENRING:
                Info.Type = NETADAPTER_TOKENRING; break;
            case MIB_IF_TYPE_FDDI:
                Info.Type = NETADAPTER_FDDI; break;
            case MIB_IF_TYPE_PPP:
                Info.Type = NETADAPTER_PPP; break;
            case MIB_IF_TYPE_LOOPBACK:
                Info.Type = NETADAPTER_LOOPBACK; break;
            case MIB_IF_TYPE_SLIP:
                Info.Type = NETADAPTER_SLIP; break;
            default:
                Info.Type = NETADAPTER_OTHER; break;
        }
        
        AdapterList.push_back(Info);
    }
    
    #undef MALLOC
    #undef FREE
    
    #endif
    
    return AdapterList;
}

std::list<io::stringc> NetworkSystem::getBroadcastIPList() const
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
    
    return BroadcastIPList;
}


/*
 * ======= Private: =======
 */

#if defined(SP_PLATFORM_WINDOWS)

void NetworkSystem::createWinSock()
{
    /* Windows socket startup */
    const WORD VersionRequest = MAKEWORD(2, 2);
    
    const s32 ErrorCode = WSAStartup(VersionRequest, &WinSock_);
    
    /* Check for error */
    if (ErrorCode)
    {
        switch (ErrorCode)
        {
            case WSASYSNOTREADY:
                io::Log::error("WinSock startup error: WinSock not ready"); break;
            case WSAVERNOTSUPPORTED:
                io::Log::error("WinSock startup error: Requested WinSock version not supported"); break;
            case WSAEINPROGRESS:
                io::Log::error("WinSock startup error: Blocking WinSock 1.1 operation in progress"); break;
            case WSAEPROCLIM:
                io::Log::error("WinSock startup error: Maximum WinSock tasks reached"); break;
            case WSAEFAULT:
                io::Log::error("WinSock startup error: lpWSAData is not a valid pointer"); break;
            default:
                io::Log::error("WinSock startup error: Unknown error code: " + io::stringc(ErrorCode)); break;
        }
        
        return;
    }
    
    /* Print information about the socket */
    io::Log::message(io::stringc(WinSock_.szDescription) + " " + io::stringc(WinSock_.szSystemStatus));
}

void NetworkSystem::deleteWinSock()
{
    WSACleanup();
}

#endif

void NetworkSystem::registerMember(NetworkMember* Member)
{
    u64 AddrCode = NetworkAddress::convert(Member->getAddress().getSocketAddress());
    MemberMap_[AddrCode] = Member;
}

NetworkMember* NetworkSystem::getMemberByAddress(const sockaddr_in &SenderAddr)
{
    const u64 AddrCode = NetworkAddress::convert(SenderAddr);
    
    /* Check if sender address was already regisitered as a client */
    std::map<u64, NetworkMember*>::iterator it = MemberMap_.find(AddrCode);
    
    if (it != MemberMap_.end())
        return it->second;
    
    return 0;
}

NetworkClient* NetworkSystem::createClient(const NetworkAddress &ClientAddr)
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

void NetworkSystem::deleteClient(NetworkClient* Client)
{
    if (!Client)
        return;
    
    /* Remove client from all relevant lists */
    MemoryManager::removeElement(ClientList_, Client);
    MemoryManager::removeElement(ClientJointStack_, Client);
    
    std::map<u64, NetworkMember*>::iterator it = MemberMap_.find(
        NetworkAddress::convert(Client->getAddress().getSocketAddress())
    );
    
    if (it != MemberMap_.end())
        MemberMap_.erase(it);
    
    /* Add to leave stack -> now the client must no longer be used */
    ClientLeaveStack_.push_back(Client);
    
    /* Delete client object */
    MemoryManager::deleteMemory(Client);
}

void NetworkSystem::closeNetworkSession()
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
