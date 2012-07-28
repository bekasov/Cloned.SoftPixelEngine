/*
 * Network client header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_CLIENT_H__
#define __SP_NETWORK_CLIENT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkMember.hpp"


namespace sp
{
namespace network
{


class SP_EXPORT NetworkClient : public NetworkMember
{
    
    public:
        
        NetworkClient(const NetworkAddress &Address);
        ~NetworkClient();
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
