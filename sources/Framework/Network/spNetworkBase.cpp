/*
 * Network base file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkBase.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkSocket.hpp"
#include "Base/spMemoryManagement.hpp"


namespace sp
{
namespace network
{


NetworkBase::NetworkBase() :
    Socket_(0)
{
}
NetworkBase::~NetworkBase()
{
    closeSocket();
}


/*
 * ======= Protected: =======
 */

void NetworkBase::openSocket(const ENetworkProtocols Protocol)
{
    if (!Socket_)
        Socket_ = new NetworkSocket(Protocol);
}
void NetworkBase::closeSocket()
{
    MemoryManager::deleteMemory(Socket_);
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
