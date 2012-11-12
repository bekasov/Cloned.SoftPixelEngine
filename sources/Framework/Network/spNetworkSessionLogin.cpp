/*
 * Network session login file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkSessionLogin.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkPacket.hpp"
#include "Base/spInputOutputLog.hpp"
#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace network
{


NetworkSessionLogin::NetworkSessionLogin() :
    NetworkBaseUDP()
{
    openSocket(PROTOCOL_UDP);
}
NetworkSessionLogin::~NetworkSessionLogin()
{
}

bool NetworkSessionLogin::request(const NetworkAddress &Address)
{
    if (bindSocketToPort(Address.getPort()))
        return sendRequestPacket(Address);
    return false;
}

bool NetworkSessionLogin::request(u16 Port, const std::list<io::stringc> &IPAddressList)
{
    if (!IPAddressList.empty() && bindSocketToPort(Port))
    {
        bool Result = false;
        
        foreach (const io::stringc &IPAddress, IPAddressList)
        {
            if (sendRequestPacket(NetworkAddress(Port, IPAddress)))
                Result = true;
        }
        
        return Result;
    }
    return false;
}

bool NetworkSessionLogin::receiveAnswer()
{
    if (!AnswerCallback_)
        return false;
    
    /* Receive network packet */
    NetworkPacket Packet;
    sockaddr_in SenderAddr;
    
    if (!receivePacketFromAddress(Packet, SenderAddr))
        return false;
    
    /* Examine network packet for session request answer */
    if (Packet.getDescriptor() == DESCRIPTOR_SESSION_ANSWER)
        AnswerCallback_(NetworkAddress(SenderAddr), Packet.getString());
    
    return true;
}


/*
 * ======= Private: =======
 */

bool NetworkSessionLogin::sendRequestPacket(const NetworkAddress &Address)
{
    io::Log::message("Request network session for " + Address.getDescription());
    
    /* Setup session request packet */
    NetworkPacket Packet(SessionKey_);
    Packet.setDescriptor(DESCRIPTOR_SESSION_REQUEST);
    
    /* Send session request packet */
    return sendPacketToAddress(Packet, Address.getSocketAddress()) > 0;
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
