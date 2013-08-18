/*
 * Network address file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkAddress.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Base/spMathCore.hpp"


namespace sp
{
namespace network
{


const size_t NetworkAddress::IP_SIZE    = sizeof(u32);
const size_t NetworkAddress::PORT_SIZE  = sizeof(u16);
const size_t NetworkAddress::ADDR_SIZE  = NetworkAddress::IP_SIZE + NetworkAddress::PORT_SIZE;

NetworkAddress::NetworkAddress(const sockaddr_in &SocketAddress)
{
    memcpy(&Addr_, &SocketAddress, sizeof(sockaddr_in));
}
NetworkAddress::NetworkAddress(u16 Port)
{
    /* Fill address structure */
    memset(&Addr_, 0, sizeof(sockaddr_in));
    
    Addr_.sin_family        = AF_INET;
    Addr_.sin_port          = htons(Port);
    Addr_.sin_addr.s_addr   = htonl(INADDR_ANY);
}
NetworkAddress::NetworkAddress(u16 Port, u32 IPAddress)
{
    /* Fill address structure */
    memset(&Addr_, 0, sizeof(sockaddr_in));
    
    Addr_.sin_family        = AF_INET;
    Addr_.sin_port          = htons(Port);
    Addr_.sin_addr.s_addr   = IPAddress;
}
NetworkAddress::NetworkAddress(u16 Port, const io::stringc &IPAddress)
{
    /* Fill address structure */
    memset(&Addr_, 0, sizeof(sockaddr_in));
    
    Addr_.sin_family        = AF_INET;
    Addr_.sin_port          = htons(Port);
    Addr_.sin_addr.s_addr   = inet_addr(IPAddress.c_str());
}
NetworkAddress::NetworkAddress(const NetworkAddress &Address)
{
    memcpy(&Addr_, &Address.Addr_, sizeof(sockaddr_in));
}
NetworkAddress::~NetworkAddress()
{
}

u16 NetworkAddress::getPort() const
{
    return ntohs(Addr_.sin_port);
}

io::stringc NetworkAddress::getIPAddressName() const
{
    io::stringc Str;
    
    const u8* Addr = (const u8*)&Addr_.sin_addr.s_addr;
    
    Str += io::stringc(static_cast<s32>(Addr[0])) + ".";
    Str += io::stringc(static_cast<s32>(Addr[1])) + ".";
    Str += io::stringc(static_cast<s32>(Addr[2])) + ".";
    Str += io::stringc(static_cast<s32>(Addr[3]));
    
    return Str;
}

io::stringc NetworkAddress::getDescription() const
{
    return getIPAddressName() + " : " + io::stringc(getPort());
}

ENetworkAddressClasses NetworkAddress::getAddressClass() const
{
    const u32 IPAddr = getIPAddress();
    
    if (!math::getBitL2R(IPAddr, 0))
        return NETADDRESS_CLASS_A;
    else if (!math::getBitL2R(IPAddr, 1))
        return NETADDRESS_CLASS_B;
    else if (!math::getBitL2R(IPAddr, 2))
        return NETADDRESS_CLASS_C;
    
    return NETADDRESS_CLASS_UNKNOWN;
}

bool NetworkAddress::valid() const
{
    return true; //todo
}

u64 NetworkAddress::convert(const sockaddr_in &Addr)
{
    u64 Result = u64(0);
    
    /* Shift port and IP address in the 64-bit integer */
    Result |= Addr.sin_port;
    Result <<= 32;
    Result |= Addr.sin_addr.s_addr;
    
    return Result;
}

NetworkAddress NetworkAddress::read(const c8* Buffer)
{
    if (!Buffer)
        return NetworkAddress(0);
    
    u32 IPAddress = 0;
    u16 Port = 0;
    
    /* Read port number and IP address */
    memcpy(&Port, Buffer, NetworkAddress::PORT_SIZE);
    Buffer += NetworkAddress::PORT_SIZE;
    
    memcpy(&IPAddress, Buffer, NetworkAddress::IP_SIZE);
    Buffer += NetworkAddress::IP_SIZE;
    
    return NetworkAddress(Port, IPAddress);
}

void NetworkAddress::write(c8* Buffer, const NetworkAddress &Address)
{
    if (Buffer)
    {
        /* Write port number and IP address */
        const u16 Port = Address.getPort();
        memcpy(Buffer, &Port, NetworkAddress::PORT_SIZE);
        
        const u32 IPAddress = Address.getIPAddress();
        memcpy(Buffer + NetworkAddress::PORT_SIZE, &IPAddress, NetworkAddress::IP_SIZE);
    }
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
