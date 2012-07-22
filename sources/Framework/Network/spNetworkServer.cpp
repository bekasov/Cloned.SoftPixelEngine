/*
 * Network server file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkServer.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


namespace sp
{
namespace network
{


NetworkServer::NetworkServer(const NetworkAddress &Address) :
    NetworkMember(NETWORK_SERVER, Address)
{
}
NetworkServer::~NetworkServer()
{
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
