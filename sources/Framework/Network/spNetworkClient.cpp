/*
 * Network client file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkClient.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


namespace sp
{
namespace network
{


NetworkClient::NetworkClient(const NetworkAddress &Address) :
    NetworkMember(NETWORK_CLIENT, Address)
{
}
NetworkClient::~NetworkClient()
{
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
