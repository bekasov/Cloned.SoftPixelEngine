/*
 * Network base header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_BASE_H__
#define __SP_NETWORK_BASE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkStructures.hpp"


namespace sp
{
namespace network
{


class NetworkSocket;


//! Pure base class for network systems.
class SP_EXPORT NetworkBase
{
    
    public:
        
        virtual ~NetworkBase();
        
        /* Inline functions */
        
        NetworkSocket* getSocket() const
        {
            return Socket_;
        }
        
    protected:
        
        /* Functions */
        
        NetworkBase();
        
        void openSocket(const ENetworkProtocols Protocol = PROTOCOL_UDP);
        void closeSocket();
        
        /* Members */
        
        NetworkSocket* Socket_;
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
