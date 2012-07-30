//
// SoftPixel Engine Tutorial: Networking - (22/08/2011)
//

#include <SoftPixelEngine.hpp>

#include <iostream>
#include <boost/foreach.hpp>

using namespace sp;

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM

/* === Global members === */

SoftPixelDevice* spDevice           = 0;
io::InputControl* spControl         = 0;
video::RenderContext* spContext     = 0;
video::RenderSystem* spRenderer     = 0;

network::NetworkSystem* spNetwork               = 0;
network::NetworkSessionReception* spReception   = 0;
network::NetworkSessionLogin* spLogin           = 0;

video::Font* Font = 0;

video::Texture* BgTex   = 0;
video::Texture* CharTex = 0;

const s32 ScrWidth = 800, ScrHeight = 600;

// Structures
struct SNetPlayer
{
    SNetPlayer(network::NetworkMember* PlayerNetMember, const io::stringc &PlayerName = "");
    ~SNetPlayer();
    
    // Functions
    void Draw();
    void Move(f32 Speed);
    
    // Members
    io::stringc Name;
    dim::point2df Pos;
    f32 Angle;
    network::NetworkMember* NetMember;
};

struct SCharPacket
{
    dim::point2df Pos;
    f32 Angle;
};

std::list<SNetPlayer> PlayerList;
SNetPlayer* MainPlayer = 0;
dim::point2df ViewPos;

// Declarations
void InitDevice();
c8 SelectNetwork();
void OpenNetwork(c8 NetSelection);
void CreateScene();
void CleanUp();
void UpdateScene();
void DrawScene();
void DrawCenteredText(
    s32 PosY, const io::stringc &Text, const video::color &Color = video::color(255, 255, 255, 200)
);


/* === All function definitions === */

int main()
{
    c8 NetSelection = SelectNetwork();
    
    if (NetSelection == 'q')
        return 0;
    
    InitDevice();
    CreateScene();
    
    OpenNetwork(NetSelection);
    
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        UpdateScene();
        DrawScene();
        
        spContext->flipBuffers();
    }
    
    CleanUp();
    
    return 0;
}

io::stringc SessionIPAddress = "";

void SessionAnswerProc(const network::NetworkAddress &ServerAddress, const io::stringc &SessionDescription)
{
    SessionIPAddress = ServerAddress.getIPAddressName();
}

c8 SelectNetwork()
{
    // Select network type
    c8 Input = 0;
    
    while (Input != 'h' && Input != 'j' && Input != 'q')
    {
        io::Log::message("===============");
        io::Log::message("h.) Host server");
        io::Log::message("j.) Join server");
        io::Log::message("q.) Quit");
        io::Log::message("===============");
        
        std::cin >> Input;
        
        io::Log::message("");
    }
    
    return Input;
}

void OpenNetwork(c8 NetSelection)
{
    io::Log::message("");
    
	spNetwork = spDevice->createNetworkSystem(network::NETWORK_UDP);
    
    switch (NetSelection)
    {
        case 'h':
        {
            spNetwork->hostServer();
            
            spReception = new network::NetworkSessionReception();
            spReception->openSession(1000, "NetworkingTutorialSessionKey", "NetworkingTutorial");
        }
        break;
        
        case 'j':
        {
            spLogin = new network::NetworkSessionLogin();
            
            spLogin->setSessionKey("NetworkingTutorialSessionKey");
            spLogin->setSessionAnswerCallback(SessionAnswerProc);
            
            spLogin->request(1000, spNetwork->getBroadcastIPList());
            
            while (!SessionIPAddress.size())
                spLogin->receiveAnswers();
            
            spNetwork->joinServer(SessionIPAddress);
        }
        break;
    }
    
    // Create main character
    PlayerList.push_back(SNetPlayer(0, "MainPlayer"));
    
    if (!spNetwork->isServer())
        PlayerList.push_back(SNetPlayer(spNetwork->getServer(), "Host"));
    
    MainPlayer = &PlayerList.front();
}

void InitDevice()
{
    spDevice    = createGraphicsDevice(
        video::RENDERER_OPENGL, dim::size2di(ScrWidth, ScrHeight), 32, "Tutorial: Networking"
    );
    
    spControl   = spDevice->getInputControl();
    spContext   = spDevice->getRenderContext();
    spRenderer  = spDevice->getRenderSystem();
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
	
    spDevice->setFrameRate(100);
    
    spRenderer->setClearColor(255);
}

void CleanUp()
{
    MemoryManager::deleteMemory(spReception);
    MemoryManager::deleteMemory(spLogin);
    deleteDevice();
}

void CreateScene()
{
    // Load some resources
    //const io::stringc ResPath = "../../../repository/help/tutorials/Networking/media/";
    const io::stringc ResPath = "";
    
    // Load the font
    Font = spRenderer->loadFont("Arial", 20, video::FONT_BOLD);
    
    // Load the textures
    BgTex   = spRenderer->loadTexture(ResPath + "Ground.jpg");
    CharTex = spRenderer->loadTexture(ResPath + "Character.png");
    
    CharTex->setColorKey(dim::point2di(0));
}

/**
 * Update the in- and out comming network packets
 */
void UpdateScene()
{
    // Update character movement
    static const f32 CHAR_TURN_SPEED = 5.0f;
    static const f32 CHAR_MOVE_SPEED = 4.0f;
    
    static const f32 WORLD_LIMIT_X = 300.0f;
    static const f32 WORLD_LIMIT_Y = 300.0f;
    
    if (spControl->keyDown(io::KEY_LEFT))
        MainPlayer->Angle -= CHAR_TURN_SPEED;
    if (spControl->keyDown(io::KEY_RIGHT))
        MainPlayer->Angle += CHAR_TURN_SPEED;
    if (spControl->keyDown(io::KEY_UP))
        MainPlayer->Move(CHAR_MOVE_SPEED);
    if (spControl->keyDown(io::KEY_DOWN))
        MainPlayer->Move(-CHAR_MOVE_SPEED);
    
    // Limit world view and main player position
    ViewPos = MainPlayer->Pos;
    
    const f32 PlayerLimitX = WORLD_LIMIT_X + ScrWidth/2 - 50;
    const f32 PlayerLimitY = WORLD_LIMIT_Y + ScrHeight/2 - 50;
    
    math::Clamp(ViewPos.X, -WORLD_LIMIT_X, WORLD_LIMIT_X);
    math::Clamp(ViewPos.Y, -WORLD_LIMIT_Y, WORLD_LIMIT_Y);
    
    math::Clamp(MainPlayer->Pos.X, -PlayerLimitX, PlayerLimitX);
    math::Clamp(MainPlayer->Pos.Y, -PlayerLimitY, PlayerLimitY);
    
    // Receive network packets
    network::NetworkPacket Packet;
    network::NetworkMember* Sender = 0;
    
    while (spNetwork->receivePacket(Packet, Sender))
    {
        SCharPacket* CharPacket = reinterpret_cast<SCharPacket*>(Packet.getBuffer());
        
        // Find character
        foreach (SNetPlayer &Player, PlayerList)
        {
            if (Player.NetMember == Sender)
            {
                Player.Pos      = CharPacket->Pos;
                Player.Angle    = CharPacket->Angle;
                break;
            }
        }
    }
    
    if (!spNetwork->isSessionRunning())
    {
        io::Log::message("Network session has been disconnected by the server", io::LOG_MSGBOX);
        exit(0);
    }
    
    // Listen new clients
    network::NetworkClient* Client = 0;
    
    while (spNetwork->popClientJoinStack(Client))
    {
        io::Log::message("CLIENT JOINED");
        
        // Add new network member
        PlayerList.push_back(SNetPlayer(Client));
    }
    
    while (spNetwork->popClientLeaveStack(Client))
    {
        io::Log::message("CLIENT LEFT");
        
        // Find network member which is to be removed
        for (std::list<SNetPlayer>::iterator it = PlayerList.begin(); it != PlayerList.end(); ++it)
        {
            if (it->NetMember == Client)
            {
                PlayerList.erase(it);
                break;
            }
        }
    }
    
    // Send network packet with information about main player to all network members
    SCharPacket CharPacket;
    
    CharPacket.Pos      = MainPlayer->Pos;
    CharPacket.Angle    = MainPlayer->Angle;
    
    spNetwork->sendPacket(
        network::NetworkPacket(reinterpret_cast<const c8*>(&CharPacket), sizeof(SCharPacket))
    );
}

void DrawScene()
{
    spRenderer->beginDrawing2D();
    
    // Draw background
    spRenderer->draw2DImage(
        BgTex,
        dim::rect2di(0, 0, ScrWidth, ScrHeight),
        dim::rect2df(
            -ViewPos.X / BgTex->getSize().Width,
            -ViewPos.Y / BgTex->getSize().Height,
            (-ViewPos.X + ScrWidth) / BgTex->getSize().Width,
            (-ViewPos.Y + ScrHeight) / BgTex->getSize().Height
        )
    );
    
    // Draw characters
    foreach_reverse (SNetPlayer &Player, PlayerList)
        Player.Draw();
    
    spRenderer->endDrawing2D();
}

/**
 * Function to draw centered text easyly.
 */
void DrawCenteredText(s32 PosY, const io::stringc &Text, const video::color &Color)
{
    // Get the text size
    const dim::size2di TextSize(Font->getStringSize(Text));
    
    // Draw the text (background black and foreground with specified color)
    spRenderer->draw2DText(
        Font, dim::point2di(ScrWidth/2 - TextSize.Width/2 + 2, PosY + 2), Text, video::color(0, 0, 0, Color.Alpha)
    );
    spRenderer->draw2DText(
        Font, dim::point2di(ScrWidth/2 - TextSize.Width/2, PosY), Text, Color
    );
}

SNetPlayer::SNetPlayer(network::NetworkMember* PlayerNetMember, const io::stringc &PlayerName) :
    Angle       (0.0f           ),
    NetMember   (PlayerNetMember)
{
    static u32 CharCount;
    
    if (PlayerName.size())
        Name = PlayerName;
    else
        Name = "Client (ID " + io::stringc(++CharCount) + ")";
}
SNetPlayer::~SNetPlayer()
{
}

/**
 * This function draws the character object.
 */
void SNetPlayer::Draw()
{
    const dim::point2di Point(
        ScrWidth/2 + static_cast<s32>(ViewPos.X - Pos.X),
        ScrHeight/2 + static_cast<s32>(ViewPos.Y - Pos.Y)
    );
    
    // Draw character
    spRenderer->draw2DImage(
        CharTex, Point, Angle, 0.75f * CharTex->getSize().Width
    );
    
    // Draw name
    const dim::size2di TextSize(Font->getStringSize(Name));
    
    spRenderer->draw2DText(
        Font, dim::point2di(Point.X - TextSize.Width/2, Point.Y - 50), Name, video::color(128, 200, 255)
    );
}

void SNetPlayer::Move(f32 Speed)
{
    Pos.X += math::Sin(-Angle) * Speed;
    Pos.Y += math::Cos(-Angle) * Speed;
}

#else

int main()
{
    io::Log::error("Network system part was not compiled with this engine");
    return 0;
}

#endif



// ============================================
