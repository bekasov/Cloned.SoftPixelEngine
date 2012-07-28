/*
 * Network session login header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_SESSION_LOGIN_H__
#define __SP_NETWORK_SESSION_LOGIN_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkBaseUDP.hpp"
#include "Framework/Network/spNetworkAddress.hpp"

#include <boost/function.hpp>


namespace sp
{
namespace network
{


/**
Session answer callback.
\param ReceptionAddress: Specifies the the address of the reception (normally the server):
\param SessionDescription: Specifies the session description string. In this string the server
can store information about the running game session.
*/
typedef boost::function<void (const NetworkAddress &ReceptionAddress, const io::stringc &SessionDescription)> SessionAnswerCallback;


/**
The network session login is used to request open games sessions over the network. This should be done by the clients only.
\code
// Session request answer callback.
void SessionAnswerProc(const network::NetworkAddress &ServerAddress, const io::stringc &SessionDescription)
{
    // Store server address here ...
}

// ...

network::NetworkSessionLogin Login;

// The session key is used that only instances of your game can make requests to other instances of the same game.
Login.setSessionKey(SessionKey);
Login.setSessionAnswerCallback(SessionAnswerProc);

// Send a session request to all available broadcast IP addresses.
Login.request(SessionPort, NetSys->getBroadcastIPList());

// Wait a moment for answers
while (WaitingForAnswers)
{
    // When a request answer message has been received, your answer callback function will be called.
    Login.receiveAnswers();
    
    // Maybe do some render stuff here ...
}
\endcode
\see NetworkSessionReception
*/
class SP_EXPORT NetworkSessionLogin : public NetworkBaseUDP
{
    
    public:
        
        NetworkSessionLogin();
        ~NetworkSessionLogin();
        
        /* === Functions === */
        
        /**
        Sends a session request to the specified address.
        \param Address: Specifies the address to which the request is to be send.
        \param SessionKey: Specifies the session key string. This string must be equal to the
        sesssion reception's key string (which holds the server).
        \return True if the request succeeded.
        */
        bool request(const NetworkAddress &Address);
        
        //! Sends a session request to all specified IP addresses with the given port number.
        bool request(u16 Port, const std::list<io::stringc> &IPAddressList);
        
        /**
        Receives incomming session request answers. If an answer has been received
        the session answer callback will be called.
        \see setSessionAnswerCallback
        */
        void receiveAnswers();
        
        /* === Inline functions === */
        
        //! Sets the new session key.
        inline void setSessionKey(const io::stringc &Key)
        {
            SessionKey_ = Key;
        }
        inline io::stringc getSessionKey() const
        {
            return SessionKey_;
        }
        
        void setSessionAnswerCallback(const SessionAnswerCallback &Callback_)
        {
            AnswerCallback_ = Callback_;
        }
        
    private:
        
        /* === Functions === */
        
        bool sendRequestPacket(const NetworkAddress &Address);
        
        /* === Members === */
        
        io::stringc SessionKey_;
        
        SessionAnswerCallback AnswerCallback_;
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
