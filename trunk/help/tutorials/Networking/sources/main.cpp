//
// SoftPixel Engine Tutorial: Networking - (22/08/2011)
//

#include <SoftPixelEngine.hpp>

#include <iostream>
#include <boost/foreach.hpp>

using namespace sp;

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
struct SCharacter
{
    SCharacter(network::NetworkMember* CharClient, const io::stringc &CharName = "");
    ~SCharacter();
    
    // Functions
    void Draw();
    
    // Members
    io::stringc Name;
    dim::point2df Pos;
    f32 Angle;
    network::NetworkMember* Client;
};

struct SCharPacket
{
    dim::point2df Pos;
    f32 Angle;
};

std::list<SCharacter*> CharList;
SCharacter* MainChar = 0;
const dim::point2df* WorldPos = 0;

// Declarations
void SelectNetwork();
void InitDevice();
void CleanUp();
void CreateScene();
void UpdateScene();
void DrawScene();
void DrawCenteredText(
    s32 PosY, const io::stringc &Text, const video::color &Color = video::color(255, 255, 255, 200)
);


/* === All function definitions === */

int main()
{
    InitDevice();
    SelectNetwork();
    CreateScene();
    
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

void SelectNetwork()
{
	spNetwork = spDevice->createNetworkSystem(network::NETWORK_UDP);
    
    // Select network type
    std::string Input;
    
    while (1)
    {
        io::Log::message("===============");
        io::Log::message("h.) Host server");
        io::Log::message("j.) Join server");
        io::Log::message("q.) Quit");
        io::Log::message("===============");
        
        std::cin >> Input;
        
        if (Input == "h")
        {
            spNetwork->hostServer();
            
            spReception = new network::NetworkSessionReception();
            spReception->openSession(1000, "NetworkingTutorialSessionKey", "NetworkingTutorial");
            
            break;
        }
        else if (Input == "j")
        {
            spLogin = new network::NetworkSessionLogin();
            
            spLogin->setSessionKey("NetworkingTutorialSessionKey");
            spLogin->setSessionAnswerCallback(SessionAnswerProc);
            
            spLogin->request(1000, spNetwork->getBroadcastIPList());
            
            while (!SessionIPAddress.size())
                spLogin->receiveAnswers();
            
            spNetwork->joinServer(SessionIPAddress);
            
            break;
        }
        else if (Input == "q")
        {
            CleanUp();
            exit(0);
        }
    }
    
    io::Log::clearConsole();
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
    MemoryManager::deleteList(CharList);
    MemoryManager::deleteMemory(spReception);
    MemoryManager::deleteMemory(spLogin);
    deleteDevice();
}

void CreateScene()
{
    // Load some resources
    const io::stringc ResPath = "../../../repository/help/tutorials/Networking/media/";
    
    // Load the font
    Font = spRenderer->loadFont("Arial", 20, video::FONT_BOLD);
    
    // Load the textures
    BgTex   = spRenderer->loadTexture(ResPath + "Ground.jpg");
    CharTex = spRenderer->loadTexture(ResPath + "Character.png");
    
    CharTex->setColorKey(dim::point2di(0));
    
    // Create main character
    if (!spNetwork->isServer())
    {
        CharList.push_back(new SCharacter(0, "Host"));
        CharList.push_back(MainChar = new SCharacter(0, "MainChar"));
    }
    else
        CharList.push_back(MainChar = new SCharacter(0, "MainChar"));
    
    WorldPos = &MainChar->Pos;
}

/**
 * Update the in- and out comming network packets
 */
void UpdateScene()
{
    // Update character movement
    static const f32 CHAR_TURN_SPEED = 5.0f;
    static const f32 CHAR_MOVE_SPEED = 4.0f;
    
    if (spControl->keyDown(io::KEY_LEFT))
        MainChar->Angle -= CHAR_TURN_SPEED;
    if (spControl->keyDown(io::KEY_RIGHT))
        MainChar->Angle += CHAR_TURN_SPEED;
    if (spControl->keyDown(io::KEY_UP))
    {
        MainChar->Pos.X += math::Sin(-MainChar->Angle) * CHAR_MOVE_SPEED;
        MainChar->Pos.Y += math::Cos(-MainChar->Angle) * CHAR_MOVE_SPEED;
    }
    if (spControl->keyDown(io::KEY_DOWN))
    {
        MainChar->Pos.X -= math::Sin(-MainChar->Angle) * CHAR_MOVE_SPEED;
        MainChar->Pos.Y -= math::Cos(-MainChar->Angle) * CHAR_MOVE_SPEED;
    }
    
    // Receive network packets
    network::NetworkPacket Packet;
    network::NetworkMember* Sender = 0;
    
    while (spNetwork->receivePacket(Packet, Sender))
    {
        SCharPacket* CharPacket = reinterpret_cast<SCharPacket*>(Packet.getBuffer());
        network::NetworkMember* Client = Sender;
        
        foreach (SCharacter* Char, CharList)
        {
            if (Char->Client == Client)
            {
                Char->Pos      = CharPacket->Pos;
                Char->Angle    = CharPacket->Angle;
                break;
            }
        }
    }
    
    // Listen new clients
    network::NetworkClient* Client = 0;
    
    while (spNetwork->popClientJoinStack(Client))
        CharList.push_back(new SCharacter(Client));
    
    // Send network packets
    SCharPacket CharPacket;
    
    CharPacket.Pos      = MainChar->Pos;
    CharPacket.Angle    = MainChar->Angle;
    
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
            -WorldPos->X / BgTex->getSize().Width,
            -WorldPos->Y / BgTex->getSize().Height,
            (-WorldPos->X + ScrWidth) / BgTex->getSize().Width,
            (-WorldPos->Y + ScrHeight) / BgTex->getSize().Height
        )
    );
    
    // Draw characters
    foreach_reverse (SCharacter* Char, CharList)
        Char->Draw();
    
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

SCharacter::SCharacter(network::NetworkMember* CharClient, const io::stringc &CharName)
{
    static u32 CharCount;
    
    Client  = CharClient;
    Angle   = 0.0f;
    
    if (CharName.size())
        Name = CharName;
    else
        Name = "Char #" + io::stringc(++CharCount);
}
SCharacter::~SCharacter()
{
}

/**
 * This function draws the character object.
 */
void SCharacter::Draw()
{
    const dim::point2di Point(
        ScrWidth/2 + static_cast<s32>(WorldPos->X - Pos.X),
        ScrHeight/2 + static_cast<s32>(WorldPos->Y - Pos.Y)
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



// ============================================
