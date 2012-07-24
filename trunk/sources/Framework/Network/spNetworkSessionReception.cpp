/*
 * Network session reception file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkSessionReception.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkPacket.hpp"
#include "Framework/Network/spNetworkSocket.hpp"
#include "Base/spInputOutputLog.hpp"
#include "Base/spMemoryManagement.hpp"

#include "Base/spTimer.hpp"


namespace sp
{
namespace network
{


THREAD_PROC(NetworkSessionReceptionThread);

NetworkSessionReception::NetworkSessionReception() :
    NetworkBaseUDP  (   ),
    Thread_         (0  )
{
    openSocket(PROTOCOL_UDP);
}
NetworkSessionReception::~NetworkSessionReception()
{
    closeSession();
}

bool NetworkSessionReception::openSession(
    u16 Port, const io::stringc &SessionKey, const io::stringc &SessionDescription)
{
    closeSession();
    
    /* Set new session key and description */
    SessionKey_ = SessionKey;
    SessionDescription_ = SessionDescription;
    
    /* Bind to port and create thread */
    if (bindSocketToPort(Port))
    {
        Thread_ = new ThreadManager(NetworkSessionReceptionThread, this);
        Thread_->setPriority(THREADPRIORITY_LOW);
        return true;
    }
    
    return false;
}
void NetworkSessionReception::closeSession()
{
    MemoryManager::deleteMemory(Thread_);
}


/*
 * Thead functions
 */

THREAD_PROC(NetworkSessionReceptionThread)
{
    /* Get network session reception object */
    NetworkSessionReception* Reception = static_cast<NetworkSessionReception*>(Arguments);
    
    /* Packet and sender storage */
    NetworkPacket Packet;
    sockaddr_in SenderAddr;
    
    /* Store local memory to avoid threading problems */
    SOCKET Socket = Reception->getSocket()->getSocket();
    
    static const u32 RecvBufferSize = 512;
    c8 RecvBuffer[RecvBufferSize] = { 0 };
    
    const io::stringc SessionKey = Reception->getSessionKey();
    const io::stringc SessionDescription = Reception->getSessionDescription();
    
    /* Run until session thread will be terminated */
    while (1)
    {
        /* Receive network packet */
        while (NetworkBaseUDP::receivePacketFromAddress(Socket, Packet, SenderAddr, RecvBuffer, RecvBufferSize))
        {
            /* Examine network packet for session request answer */
            if (Packet.getDescriptor() == DESCRIPTOR_SESSION_REQUEST)
            {
                /* Check if session key is correct */
                if (Packet.compareString(SessionKey))
                {
                    /* Answer request with session description */
                    NetworkPacket AnswerPacket(SessionDescription);
                    AnswerPacket.setDescriptor(DESCRIPTOR_SESSION_ANSWER);
                    
                    NetworkBaseUDP::sendPacketToAddress(Socket, AnswerPacket, SenderAddr);
                }
            }
        }
        
        /* Yield the thread to give other processes time to run */
        io::Timer::yield();
    }
    
    return 0;
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
