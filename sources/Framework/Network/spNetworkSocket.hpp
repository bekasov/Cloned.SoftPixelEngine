/*
 * Network socket header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_SOCKET_H__
#define __SP_NETWORK_SOCKET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkCore.hpp"
#include "Framework/Network/spNetworkAddress.hpp"


namespace sp
{
namespace network
{


class NetworkSocket
{
    
    public:
        
        NetworkSocket(const ENetworkProtocols Protocol = PROTOCOL_UDP);
        NetworkSocket(const ENetworkProtocols Protocol, SOCKET Socket);
        ~NetworkSocket();
        
        /* Functions */
        
        bool bindSocket(const NetworkAddress &Address);
        
        /* Inline functions */
        
        SOCKET getSocket() const
        {
            return Socket_;
        }
        
        inline ENetworkProtocols getProtocol() const
        {
            return Protocol_;
        }
        
    private:
        
        /* Functions */
        
        void createSocket();
        void closeSocket();
        
        void setupBehavior();
        
        void setConnectionReset(bool Enable);
        void setBlocking(bool Enable);
        void setBroadCasting(bool Enable);
        void setReUseAddress(bool Enable);
        
        /* Members */
        
        ENetworkProtocols Protocol_;
        
        SOCKET Socket_;
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
