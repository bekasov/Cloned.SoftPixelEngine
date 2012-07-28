/*
 * Network session reception header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_SESSION_RECEPTION_H__
#define __SP_NETWORK_SESSION_RECEPTION_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkBaseUDP.hpp"
#include "Base/spThreadManager.hpp"


namespace sp
{
namespace network
{


/**
The network session reception is used to answer game session requests. This should be done by the server only.
\code
network::NetworkSessionReception Reception;

// Open a session with the reception. This will start a thread which is running in the background. This thread has only a low priority.
Reception.openSession(SessionPort, SessionKey, "This is a test session description from the server");
\endcode
*/
class SP_EXPORT NetworkSessionReception : public NetworkBaseUDP
{
    
    public:
        
        NetworkSessionReception();
        ~NetworkSessionReception();
        
        /* === Functions === */
        
        /**
        Opens a new network session.
        \param Port: Specifies the port number on which the session requests can be transmitted.
        \param SessionKey: Specifies the session key to guarantee that only your individual game
        can communicate with other instances of the same game.
        \param SessionDescription: Specifies the session description which can store information
        about the running game instance.
        */
        bool openSession(
            u16 Port, const io::stringc &SessionKey, const io::stringc &SessionDescription
        );
        void closeSession();
        
        /* === Inline functions === */
        
        inline io::stringc getSessionKey() const
        {
            return SessionKey_;
        }
        inline io::stringc getSessionDescription() const
        {
            return SessionDescription_;
        }
        
    private:
        
        /* === Members === */
        
        io::stringc SessionKey_;
        io::stringc SessionDescription_;
        
        ThreadManager* Thread_;
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
