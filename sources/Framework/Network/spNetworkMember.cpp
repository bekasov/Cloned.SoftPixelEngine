/*
 * Network member file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkSystem.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


namespace sp
{
namespace network
{


NetworkMember::NetworkMember(
    const ENetworkMembers Type, const NetworkAddress &Address) :
    BaseObject  (       ),
    Type_       (Type   ),
    Address_    (Address)
{
}
NetworkMember::~NetworkMember()
{
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
