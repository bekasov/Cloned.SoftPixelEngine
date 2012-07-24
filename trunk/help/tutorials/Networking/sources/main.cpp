//
// SoftPixel Engine Tutorial: Networking - (22/08/2011)
//

#include <SoftPixelEngine.hpp>

#include <iostream>

using namespace sp;

/* === Global members === */

SoftPixelDevice* spDevice           = 0;
io::InputControl* spControl         = 0;
video::RenderSystem* spRenderer     = 0;
network::NetworkSystem* spNetwork   = 0;

video::Font* Font = 0;

video::Texture* BgTex   = 0;
video::Texture* CharTex = 0;

const s32 ScrWidth = 800, ScrHeight = 600;

// Structures
struct SCharacter
{
    SCharacter(network::NetworkClient* CharClient, const io::stringc &CharName = "");
    ~SCharacter();
    
    // Functions
    void Draw();
    
    // Members
    io::stringc Name;
    dim::point2df Pos;
    f32 Angle;
    network::NetworkClient* Client;
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
    SelectNetwork();
    InitDevice();
    CreateScene();
    
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        UpdateScene();
        DrawScene();
        
        spRenderer->flipBuffers();
    }
    
    CleanUp();
    
    return 0;
}

void SelectNetwork()
{
	spNetwork = new network::NetworkSystem();
    
    // Select network type
    std::string Input;
    
    while (1)
    {
        io::Log::message("================");
        io::Log::message("o.) Open server");
        io::Log::message("j.) Join server");
        io::Log::message("s.) Scan network");
        io::Log::message("q.) Quit");
        io::Log::message("================");
        
        std::cin >> Input;
        
        if (Input == "o")
        {
            spNetwork->openServer();
            break;
        }
        else if (Input == "j")
        {
            io::Log::message("Enter IP address or host name:");
            
            std::cin >> Input;
            
            io::stringc HostName(Input);
            
            if (HostName.find(".") == -1)
                HostName = spNetwork->getHostIPAddress(HostName);
            
            spNetwork->joinServer(Input);
            break;
        }
        else if (Input == "s")
        {
            io::Log::message("");
            io::Log::message("Please wait ...");
            io::Log::message("");
            
            std::list<io::stringc> Members = spNetwork->getNetworkMembers();
            
            for (std::list<io::stringc>::iterator it = Members.begin(); it != Members.end(); ++it)
            {
                io::Log::message("NetworkMember: \"" + *it + "\"");
                
                std::list<io::stringc> Addresses = spNetwork->getHostIPAddressList(*it);
                
                for (std::list<io::stringc>::iterator it2 = Addresses.begin(); it2 != Addresses.end(); ++it2)
                    io::Log::message("\tIP Address: \"" + *it2 + "\"");
            }
            
            io::Log::message("");
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
        video::RENDERER_AUTODETECT, dim::size2di(ScrWidth, ScrHeight), 32, "Tutorial: Networking"
    );
    
    spControl   = spDevice->getInputControl();
    spRenderer  = spDevice->getRenderSystem();
    
    spDevice->setWindowTitle(
        spDevice->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
	
    spDevice->setFrameRate(100);
    
    spRenderer->setClearColor(255);
}

void CleanUp()
{
    MemoryManager::deleteMemory(spNetwork);
    MemoryManager::deleteList(CharList);
    deleteDevice();
}

void CreateScene()
{
    // Load some resources
    const io::stringc ResPath = "../media/";
    
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
        MainChar->Pos.X += SIN(-MainChar->Angle) * CHAR_MOVE_SPEED;
        MainChar->Pos.Y += COS(-MainChar->Angle) * CHAR_MOVE_SPEED;
    }
    if (spControl->keyDown(io::KEY_DOWN))
    {
        MainChar->Pos.X -= SIN(-MainChar->Angle) * CHAR_MOVE_SPEED;
        MainChar->Pos.Y -= COS(-MainChar->Angle) * CHAR_MOVE_SPEED;
    }
    
    // Receive network packets
    network::SNetworkPacket Packet;
    
    while (spNetwork->pickPacket(Packet))
    {
        switch (Packet.Type)
        {
            case network::PACKET_CLIENTJOIN:
            {
                io::Log::message("CLIENT JOIN");
                
                network::NetworkClient* Client = static_cast<network::NetworkClient*>(Packet.Buffer);
                
                CharList.push_back(new SCharacter(Client));
            }
            break;
            
            /*case network::PACKET_CLIENTLEFT:
            {
                io::Log::message("CLIENT LEFT");
                
                network::NetworkClient* Client = static_cast<network::NetworkClient*>(Packet.Buffer);
                
                for (std::list<SCharacter*>::iterator it = CharList.begin(); it != CharList.end(); ++it)
                {
                    if ((*it)->Client == Client)
                    {
                        delete *it;
                        CharList.erase(it);
                        break;
                    }
                }
            }
            break;*/
            
            case network::PACKET_SERVEROFF:
            {
                io::Log::message("SERVER OFF");
                
                spNetwork->disconnect();
                CleanUp();
                exit(0);
            }
            break;
            
            case network::PACKET_CLIENTDATA:
            {
                io::Log::message("CLIENT DATA");
                
                SCharPacket* CharPacket = static_cast<SCharPacket*>(Packet.Buffer);
                network::NetworkClient* Client = Packet.Sender;
                
                for (std::list<SCharacter*>::iterator it = CharList.begin(); it != CharList.end(); ++it)
                {
                    if ((*it)->Client == Client)
                    {
                        (*it)->Pos      = CharPacket->Pos;
                        (*it)->Angle    = CharPacket->Angle;
                        break;
                    }
                }
                
            }
            break;
            
            default:
                break;
        }
        
        Packet.deleteBuffer();
    }
    
    // Send network packets
    SCharPacket CharPacket;
    
    CharPacket.Pos      = MainChar->Pos;
    CharPacket.Angle    = MainChar->Angle;
    
    spNetwork->sendPacket(&CharPacket, sizeof(SCharPacket));
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
    for (std::list<SCharacter*>::reverse_iterator it = CharList.rbegin(); it != CharList.rend(); ++it)
        (*it)->Draw();
    
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

SCharacter::SCharacter(network::NetworkClient* CharClient, const io::stringc &CharName)
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
    const dim::point2di Point(ScrWidth/2 + WorldPos->X - Pos.X, ScrHeight/2 + WorldPos->Y - Pos.Y);
    
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
