/*
 * Network member header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_MEMBER_H__
#define __SP_NETWORK_MEMBER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Base/spBaseObject.hpp"
#include "Framework/Network/spNetworkAddress.hpp"


namespace sp
{
namespace network
{


//! Network member types.
enum ENetworkMembers
{
    NETWORK_CLIENT, //!< Network client.
    NETWORK_SERVER, //!< Network server. Only one instance can exist.
};


/**
Network member interface class. This is the base class of NetworkClient and NetworkServer.
\ingroup group_network
*/
class SP_EXPORT NetworkMember : public BaseObject
{
    
    public:
        
        virtual ~NetworkMember();
        
        /* Inline functions */
        
        inline NetworkAddress getAddress() const
        {
            return Address_;
        }
        
        inline bool isServer() const
        {
            return Type_ == NETWORK_SERVER;
        }
        
    protected:
        
        NetworkMember(const ENetworkMembers Type, const NetworkAddress &Address);
        
    private:
        
        friend class NetworkSystemUDP;
        
        /* Members */
        
        ENetworkMembers Type_;
        NetworkAddress Address_;
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
