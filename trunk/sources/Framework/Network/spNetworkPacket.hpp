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
    DESCRIPTOR_SERVER_DISCONNECTED,     //!< Server has disconnected. This introduces a disconnection for each client.
    DESCRIPTOR_SESSION_REQUEST,         //!< Will be sent to 'scan' the network for open game sessions.
    DESCRIPTOR_SESSION_ANSWER,          //!< Answer from a sessions request.
    DESCRIPTOR_BECOME_SERVER,           //!< Will be sent to a client who becomes the new server.
    DESCRIPTOR_CLIENT_TRANSFERS_SERVER, //!< Will be sent to all other clients that one client becomes the new server.
};


class NetworkPacket
{
    
    public:
        
        NetworkPacket(u32 BufferSize = 0);
        NetworkPacket(const EReservedDescriptors Descriptor);
        NetworkPacket(const c8* Buffer, u32 BufferSize, bool isSetLowLevel = false);
        NetworkPacket(const NetworkPacket &Other);
        NetworkPacket(const io::stringc &String);
        ~NetworkPacket();
        
        /* Functions */
        
        c8* getBuffer();
        const c8* getBuffer() const;
        
        io::stringc getString() const;
        bool compareString(const io::stringc &Str) const;
        
        void setDescriptor(u32 Descriptor);
        u32 getDescriptor() const;
        
        /* Inline functions */
        
        inline const c8* getRealBuffer() const
        {
            return Buffer_.get();
        }
        inline u32 getRealBufferSize() const
        {
            return BufferSize_ + NetworkPacket::RESERVED_OFFSET;
        }
        
        inline u32 getBufferSize() const
        {
            return BufferSize_;
        }
        
    private:
        
        /* Macros */
        
        static u32 RESERVED_OFFSET;
        
        /* Functions */
        
        void createBuffer(u32 Size);
        void createBufferNonOffset(u32 Size);
        
        /* Members */
        
        boost::shared_array<c8> Buffer_;
        
        u32 BufferSize_;
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
