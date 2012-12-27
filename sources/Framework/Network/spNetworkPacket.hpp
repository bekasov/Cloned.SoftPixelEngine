/*
 * Network packet header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_PACKET_H__
#define __SP_NETWORK_PACKET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkCore.hpp"

#include <boost/shared_array.hpp>


namespace sp
{
namespace network
{


//! Reserved network packet descriptors for internal network communication. Don't use these values!
enum EReservedDescriptors
{
    DESCRIPTOR_CLIENT_JOIN = 1000,      //!< A client has joined the server. This packet will only be sent to the server.
    DESCRIPTOR_CLIENT_LEFT,             //!< A client has left the server. This packet will sent to each network member.
    DESCRIPTOR_INTRODUCE_CLIENT,        //!< Server introduces a new client who has joined.
    DESCRIPTOR_CONNECTION_ACCEPTED,     //!< Server accepted the connection of the new client who is about to join.
    DESCRIPTOR_SERVER_DISCONNECTED,     //!< Server has been disconnected. This results in a disconnection for each client.
    
    DESCRIPTOR_SESSION_REQUEST,         //!< Will be sent to 'scan' the network for open game sessions.
    DESCRIPTOR_SESSION_ANSWER,          //!< Answer from a sessions request.
};


/**
NetworkPacket is used as wrapping class around a packet which can be send and/or received over the network.
\ingroup group_network
*/
class SP_EXPORT NetworkPacket
{
    
    public:
        
        NetworkPacket(u32 Descriptor = 0);
        NetworkPacket(const NetworkPacket &Other);
        NetworkPacket(const void* Buffer, u32 BufferSize, u32 Descriptor = 0, bool isSetLowLevel = false);
        NetworkPacket(const io::stringc &String, u32 Descriptor = 0);
        ~NetworkPacket();
        
        /* === Functions === */
        
        //! Returns the packet as a string but only if this packet is a null-terminated string.
        io::stringc getString() const;
        //! Returns true if the given string equals the network packet data as string.
        bool compareString(const io::stringc &Str) const;
        
        //! Sets the descriptor. The descriptor is used to identify the purpose of the packet when received by another network member.
        void setDescriptor(u32 Descriptor);
        //! Returns the descriptor.
        u32 getDescriptor() const;
        
        //! Sets the receiver IP address. This is only used for a client/server model.
        void setReceiverAddress(u32 IPAddress);
        //! Returns the receiver IP address. This is only used for a client/server model.
        u32 getReceiverAddress() const;
        
        /* === Inline functions === */
        
        //! Returns a byte (or rather char) pointer to the packet data.
        inline c8* getBuffer()
        {
            return Buffer_.get() + NetworkPacket::RESERVED_OFFSET;
        }
        inline const c8* getBuffer() const
        {
            return Buffer_.get() + NetworkPacket::RESERVED_OFFSET;
        }
        
        /**
        Returns a constant pointer to the real buffer. This is used internally
        and should not be used to get the buffer of transmitted network data. use "getBuffer" instead.
        \see getBuffer
        */
        inline const c8* getRealBuffer() const
        {
            return Buffer_.get();
        }
        /**
        Returns the real buffer size. This is the size (in bytes) of the
        actual network packet (including protocol information). This is used internally
        and should not be used to get the buffer size of send network data.
        \see getBufferSize
        */
        inline u32 getRealBufferSize() const
        {
            return LogicalBufferSize_ + NetworkPacket::RESERVED_OFFSET;
        }
        
        //! Returns the local buffer size. This is the size (in bytes) of the packet data field.
        inline u32 getBufferSize() const
        {
            return LogicalBufferSize_;
        }
        
    private:
        
        /* === Macros === */
        
        static const u32 RESERVED_OFFSET = sizeof(u32);
        
        /* === Functions === */
        
        void createBuffer(u32 Size);
        void createBufferNonOffset(u32 Size);
        
        /* === Members === */
        
        boost::shared_array<c8> Buffer_;    //!< Network packet buffer.
        u32 LogicalBufferSize_;             //!< Buffer size used for the data field (excluding protocol data fields).
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
