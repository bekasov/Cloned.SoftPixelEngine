/*
 * Network address header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_ADDRESS_H__
#define __SP_NETWORK_ADDRESS_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkCore.hpp"
#include "Base/spInputOutputString.hpp"


namespace sp
{
namespace network
{


/**
Network address holding the IP address (referencing to a computer) and the port number (referencing to a service).
\ingroup group_network
*/
class SP_EXPORT NetworkAddress
{
    
    public:
        
        /* === Macros === */
        
        static const size_t IP_SIZE;
        static const size_t PORT_SIZE;
        static const size_t ADDR_SIZE;
        
        /* === Constructors & destructor === */
        
        NetworkAddress(const sockaddr_in &SocketAddress);
        NetworkAddress(u16 Port);
        NetworkAddress(u16 Port, u32 IPAddress);
        NetworkAddress(u16 Port, const io::stringc &IPAddress);
        NetworkAddress(const NetworkAddress &Address);
        ~NetworkAddress();
        
        /* === Functions === */
        
        //! Returns the network port.
        u16 getPort() const;
        
        //! Returns the IP address name (e.g. "127.0.0.1").
        io::stringc getIPAddressName() const;
        
        //! Returns the address descriptions (e.g. "127.0.0.1 : 8000");
        io::stringc getDescription() const;
        
        //! Returns the network address class.
        ENetworkAddressClasses getAddressClass() const;
        
        //! Returns true if this is a valid network address.
        bool valid() const;
        
        /* === Static functions === */
        
        //! Returns the IP address as 64 bit integer.
        static u64 convert(const sockaddr_in &Addr);
        
        //! Reads a network address from the given buffe. This buffer must have at least a size of 6 bytes.
        static NetworkAddress read(const c8* Buffer);
        //! Writes the given network address to the given buffe. This buffer must have at least a size of 6 bytes.
        static void write(c8* Buffer, const NetworkAddress &Address);
        
        /* === Inline functions === */
        
        //! Returns the IP address value.
        inline u32 getIPAddress() const
        {
            return Addr_.sin_addr.s_addr;
        }
        
        inline const sockaddr_in& getSocketAddress() const
        {
            return Addr_;
        }
        inline sockaddr_in& getSocketAddress()
        {
            return Addr_;
        }
        
    private:
        
        /* === Members === */
        
        sockaddr_in Addr_;
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
