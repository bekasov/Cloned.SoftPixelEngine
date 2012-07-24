/*
 * Network structures header 
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_STRUCTURES_H__
#define __SP_NETWORK_STRUCTURES_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Base/spInputOutputString.hpp"


namespace sp
{
namespace network
{


/* === Enumerations === */

//! Supported network systems.
enum ENetworkSystems
{
    NETWORK_UDP, //!< Connection less UDP network. This is unsafe and should only be used if performance is more important than safety.
    NETWORK_TCP, //!< TCP client/server model. This is the most used network system for video games.
};

//! Network address classes.
enum ENetworkAddressClasses
{
    NETADDRESS_CLASS_UNKNOWN,   //!< Unknown network address class.
    NETADDRESS_CLASS_A,         //!< Network address class A has the net-mask 255.0.0.0.
    NETADDRESS_CLASS_B,         //!< Network address class B has the net-mask 255.255.0.0.
    NETADDRESS_CLASS_C,         //!< Network address class C has the net-mask 255.255.255.0.
};

//! Enumeration of some predefined port numbers.
enum ENetworkPorts
{
    NETPORT_HTTP    = 80,   //!< Port number for the "Hyper Text Transfer Protocol".
    NETPORT_SMTP    = 25,   //!< Port number for the "Simple Mail Transer Protocol".
    NETPORT_FTP     = 20,   //!< Port number for the "File Transer Protocol".
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

//! Network transport protocols.
enum ENetworkProtocols
{
    PROTOCOL_TCP, //!< Transmision Control Protocol (TCP). TCP guarantees safe transmisions but is slower than UDP.
    PROTOCOL_UDP, //!< User Datagram Protocol (UDP). UDP is unsafe and should only be used when performance is more important than safety (e.g. for video-transmision).
};


/* === Structures === */

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


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
