//
// SoftPixel Engine Tutorial: GraphicsUserInterface - (06/05/2011)
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#ifdef SP_COMPILE_WITH_GUI

#include "../../common.hpp"

/* === Global members === */

SoftPixelDevice* spDevice       = 0;
io::InputControl* spControl     = 0;
video::RenderSystem* spRenderer = 0;
video::RenderContext* spContext = 0;
scene::SceneGraph* spScene      = 0;
gui::GUIManager* spGUI          = 0;
audio::SoundDevice* spListener  = 0;

const s32 ScrWidth = 1024, ScrHeight = 768;

bool isQuit = false;

scene::Camera* Cam  = 0;
scene::Light* Light = 0;
scene::Mesh* Obj    = 0;

video::Movie* Movie         = 0;
video::Texture* MovieTex    = 0;
audio::Sound* MovieSound    = 0;

gui::GUIMenuItem* ItemGeom = 0, * ItemMovie = 0, * ItemMusic = 0, * ItemQuit = 0, * ItemGame = 0;
gui::GUIWindow* WinGeom = 0, * WinMusic = 0, * WinSelect = 0, * WinGame = 0, * WinFileBrowser = 0, * WinMoviePlayer = 0;
gui::GUIButtonGadget* BtnOpen = 0, * BtnCancel = 0;
gui::GUITreeGadget* GadBrowserTree = 0;
gui::GUIListGadget* GadBrowserList = 0;

tool::PathGraph* Graph = 0;
std::list<tool::PathNode*> Path;
tool::PathNode* StartNode = 0, * EndNode = 0;
video::Texture* SmilieTex = 0;
dim::point2df SmiliePos;

enum EFileBrowserTypes
{
    FILEBROWER_NONE,
    FILEBROWER_MOVIE,
    FILEBROWER_MUSIC,
}
FileBrowserType = FILEBROWER_NONE;

// Declarations
void InitDevice();
void CleanUp();
void CreateScene();
void UpdateScene();


/* === All function definitions === */

int main()
{
    InitDevice();
    CreateScene();
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE) && !isQuit)
    {
        spRenderer->clearBuffers();
        
        UpdateScene();
        
        spContext->flipBuffers();
    }
    
    CleanUp();
    
    return 0;
}

void InitDevice()
{
    spDevice    = createGraphicsDevice(
        video::RENDERER_OPENGL, dim::size2di(ScrWidth, ScrHeight), 32, "SoftPixel Engine - GraphicsUserInterface tutorial"
    );
    
    spControl   = spDevice->getInputControl();
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    
    spScene     = spDevice->createSceneGraph();
    spGUI       = spDevice->getGUIManager();
    spListener  = spDevice->createSoundDevice(audio::SOUNDDEVICE_WINMM);
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    spDevice->setFrameRate(100);
    
    spRenderer->setClearColor(255);
    
    math::Randomizer::seedRandom();
}

void CleanUp()
{
    MemoryManager::deleteMemory(Graph);
    deleteDevice();
}

/**
 * GUI event callback procedure. Here we check each window or gadget event.
 * e.g. when a button has been pressed or a window has been closed.
 */
void EventProc(const gui::SGUIEvent &Event)
{
    /**
     * Check menu item events.
     */
    if (Event.Object == gui::EVENT_MENUITEM && Event.MenuItem)
    {
        if (Event.MenuItem == ItemGeom)
        {
            ItemGeom->setState(!ItemGeom->getState());
            WinGeom->setVisible(ItemGeom->getState());
            if (ItemGeom->getState())
                WinGeom->foreground();
        }
        else if (Event.MenuItem == ItemGame)
        {
            ItemGame->setState(!ItemGame->getState());
            WinGame->setVisible(ItemGame->getState());
            if (ItemGame->getState())
                WinGame->foreground();
        }
        else if (Event.MenuItem == ItemMovie && !WinFileBrowser->getVisible())
        {
            if (!ItemMovie->getState())
            {
                WinFileBrowser->setVisible(true);
                WinFileBrowser->foreground();
                FileBrowserType = FILEBROWER_MOVIE;
            }
            else
            {
                ItemMovie->setState(false);
                WinMoviePlayer->setVisible(false);
                FileBrowserType = FILEBROWER_NONE;
                
                if (Movie)
                    Movie->stop();
                if (MovieSound)
                    MovieSound->stop();
            }
        }
        else if (Event.MenuItem == ItemQuit)
            isQuit = true;
    }
    /**
     * Check gadget controller events.
     */
    else if (Event.Object == gui::EVENT_GADGET && Event.Gadget)
    {
        if (Event.Gadget == GadBrowserTree)
        {
            GadBrowserList->clearItems();
            GadBrowserList->addDirectoryItems(
                GadBrowserTree->getExplorerFullPath(GadBrowserTree->getSelectedItem()) + "/*.avi"
            );
        }
        else if (Event.Gadget == BtnCancel)
        {
            WinFileBrowser->setVisible(false);
            FileBrowserType = FILEBROWER_NONE;
        }
        else if (Event.Gadget == BtnOpen && GadBrowserList->getSelectedItem())
        {
            if (FileBrowserType == FILEBROWER_MOVIE)
            {
                const io::stringc Filename(
                    GadBrowserTree->getExplorerFullPath(GadBrowserTree->getSelectedItem()) + "/" + GadBrowserList->getSelectedItem()->getText()
                );
                
                if (io::FileSystem().findFile(Filename))
                {
                    if (Movie)
                    {
                        Movie->stop();
                        spRenderer->deleteMovie(Movie);
                    }
                    if (MovieSound)
                    {
                        MovieSound->stop();
                        spListener->deleteSound(MovieSound);
                    }
                    
                    Movie = spRenderer->loadMovie(Filename, 512);
                    
                    MovieSound = spListener->loadSound(Filename, 1);
                    
                    if (!Movie->valid())
                    {
                        io::Log::error("Could not load movie. Maybe the correct video codec is not installed");
                        return;
                    }
                    
                    MovieSound->play();
                    Movie->play();
                    
                    WinMoviePlayer->setVisible(true);
                    ItemMovie->setState(true);
                }
            }
            
            WinFileBrowser->setVisible(false);
            FileBrowserType = FILEBROWER_NONE;
        }
    }
    /**
     * Window events
     */
    else if (Event.Object == gui::EVENT_WINDOW && Event.Window)
    {
        if (Event.Window == WinFileBrowser && Event.Type == gui::EVENT_RESIZE)
        {
            const dim::size2di WinSize(Event.Window->getLocalViewArea().getSize());
            
            BtnOpen->setPosition(dim::point2di(WinSize.Width - 180, WinSize.Height - 35));
            BtnCancel->setPosition(dim::point2di(WinSize.Width - 90, WinSize.Height - 35));
            
            GadBrowserTree->setSize(dim::size2di((WinSize.Width - 20)/2 - 5, WinSize.Height - 55));
            
            GadBrowserList->setPosition(dim::point2di((WinSize.Width - 20)/2 + 15, 10));
            GadBrowserList->setSize(dim::size2di((WinSize.Width - 20)/2 - 5, WinSize.Height - 55));
        }
    }
}

/**
 * Owner draw callback procedures.
 */
void OwnerDrawGeom(gui::GUIController* Ctrl)
{
    dim::rect2di Rect(Ctrl->getRect());
    
    spRenderer->draw2DRectangle(Rect, 0);
    
    Rect.Right -= Rect.Left;
    Rect.Bottom -= Rect.Top;
    
    Cam->setViewport(Rect);
    
    Obj->turn(1);
    
    spScene->renderScene();
}

/**
 * Owner draw callback for the labyrinth game.
 */
void OwnerDrawGame(gui::GUIController* Ctrl)
{
    static const s32 RasterSize = 10;
    static f32 Morphing;
    static tool::PathNode* LastNode;
    static bool FollowRnd;
    
    const dim::rect2di Rect(Ctrl->getRect());
    const dim::point2di MousePos(spControl->getCursorPosition());
    
    std::list<tool::PathNode*> Nodes = Graph->getNodeList();
    
    if (StartNode && LastNode && Path.empty())
    {
        FollowRnd = true;
        s32 RndIndex = math::Randomizer::randInt(Nodes.size()), i = 0;
        
        for (std::list<tool::PathNode*>::iterator it = Nodes.begin(); it != Nodes.end(); ++it, ++i)
        {
            if (i == RndIndex)
            {
                StartNode = LastNode;
                EndNode = *it;
                
                Path = Graph->findPath(StartNode, EndNode);
                Path.reverse();
                
                LastNode = *Path.begin();
                Path.pop_front();
                
                break;
            }
        }
    }
    
    if (WinGame->hasForeground() && Rect.overlap(MousePos) && spControl->mouseHit(io::MOUSE_LEFT))
    {
        for (std::list<tool::PathNode*>::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
        {
            const dim::point2df NodePos((*it)->getPosition());
            const dim::rect2di NodeRect(dim::rect2df(
                NodePos.X + Rect.Left, NodePos.Y + Rect.Top,
                NodePos.X + Rect.Left + RasterSize, NodePos.Y + Rect.Top + RasterSize
            ).cast<s32>());
            
            if (NodeRect.overlap(MousePos))
            {
                if (!StartNode)
                {
                    StartNode = LastNode = *it;
                    SmiliePos = StartNode->getPosition();
                }
                else
                {
                    FollowRnd = false;
                    
                    StartNode = LastNode;
                    EndNode = *it;
                    
                    Path = Graph->findPath(StartNode, EndNode);
                    Path.reverse();
                    
                    LastNode = *Path.begin();
                    Path.pop_front();
                }
                break;
            }
        }
    }
    
    spRenderer->beginDrawing2D();
    
    spRenderer->draw2DRectangle(Rect, 0);
    
    /* Draw the grid nodes */
    for (std::list<tool::PathNode*>::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
    {
        const dim::point2df Pos((*it)->getPosition());
        s32 UserData = reinterpret_cast<long>((*it)->getUserData());
        
        video::color Color(255);
        
        if (EndNode == *it)
        {
            if (FollowRnd)
                Color = video::color(64, 200, 32);
            else
                Color = video::color(255, 64, 32);
        }
        else if (UserData > 0)
        {
            if (FollowRnd)
                Color = video::color(255 - UserData/2, 255, 255 - UserData);
            else
                Color = video::color(255, 255 - UserData, 255 - UserData);
        }
        
        spRenderer->draw2DRectangle(
            dim::rect2di(dim::rect2df(
                Pos.X + Rect.Left, Pos.Y + Rect.Top,
                Pos.X + Rect.Left + RasterSize, Pos.Y + Rect.Top + RasterSize
            ).cast<s32>()),
            Color
        );
        
        UserData -= 5;
        if (UserData <= 0)
            (*it)->setUserData(0);
        else
            (*it)->setUserData(reinterpret_cast<void*>(UserData));
    }
    
    /* Draw the smilie */
    spRenderer->draw2DImage(
        SmilieTex, dim::point2di(Rect.Left, Rect.Top) + SmiliePos.cast<s32>()
    );
    
    spRenderer->endDrawing2D();
    
    /* Move the smilie */
    if (!Path.empty())
    {
        tool::PathNode* CurNode = *Path.begin();
        
        Morphing += 0.25f;
        SmiliePos = (LastNode->getPosition() + (CurNode->getPosition() - LastNode->getPosition()) * Morphing);
        
        if (Morphing >= 1.0f)
        {
            CurNode->setUserData(reinterpret_cast<void*>(255));
            LastNode = CurNode;
            Path.pop_front();
            Morphing = 0;
        }
    }
}

/**
 * Owner draw callback for the movie.
 */
void OwnerDrawMovie(gui::GUIController* Ctrl)
{
    if (!Movie)
        return;
    
    const dim::rect2di Rect(Ctrl->getLocalViewArea());
    const dim::size2di Size(Rect.getSize());
    
    Movie->renderTexture(MovieTex);
    
    spRenderer->beginDrawing2D();
    spRenderer->draw2DImage(
        MovieTex, dim::rect2di(Rect.Left, Rect.Top, Size.Width, Size.Height), dim::rect2df(0, 1, 1, 0)
    );
    spRenderer->endDrawing2D();
}

/**
 * Create the scene or rather all window views with all its buttons and controls.
 */
void CreateScene()
{
    // Load some resources
    const io::stringc ResPath = "media/";
    
    MovieTex = spRenderer->createTexture(dim::size2di(512), video::PIXELFORMAT_BGR);
    
    SmilieTex = spRenderer->loadTexture(ResPath + "Smilie.png");
    SmilieTex->setColorKey(dim::point2di(0));
    
    // Create the small 3D scene
    Cam = spScene->createCamera();
    Cam->setRange(0.1f, 100);
    
    Light = spScene->createLight();
    Light->setRotation(dim::vector3df(45, 10, 0));
    
    spScene->setLighting(true);
    
    Obj = spScene->createMesh(scene::MESH_TEAPOT);
    Obj->setPosition(dim::vector3df(0, 0, 3));
    
    /**
     * Create the GUI. At first we set the event callback function.
     */
    spGUI->setEventCallback(EventProc);
    
    /**
     * Create the first windows.
     */
    gui::GUIWindow* Win = spGUI->addWindow(
        dim::point2di(50, 50), dim::size2di(350, 250), "Example Explorer", gui::GUIFLAG_CLOSEBUTTON
    );
    
    /**
     * Create a menu interface.
     */
    gui::GUIMenuItem* Item1, * Item2, * Item3, * Item4;
    
    Item1 = new gui::GUIMenuItem(gui::MENUITEM_ROOT);
    
    Item1->addChild(Item2 = new gui::GUIMenuItem(gui::MENUITEM_TITLE, "iPear OS"));
        Item2->addChild(ItemMusic = new gui::GUIMenuItem(gui::MENUITEM_ENTRY, "Play Music\tCtrl+M"));
        Item2->addChild(ItemMovie = new gui::GUIMenuItem(gui::MENUITEM_ENTRY, "Play Movie\tCtrl+F"));
        Item2->addChild(ItemGeom = new gui::GUIMenuItem(gui::MENUITEM_ENTRY, "Show 3D Geometry\tCtrl+G"));
        Item2->addChild(ItemGame = new gui::GUIMenuItem(gui::MENUITEM_ENTRY, "Labyrinth Game"));
        Item2->addChild(new gui::GUIMenuItem(gui::MENUITEM_SEPARATOR));
        Item2->addChild(Item3 = new gui::GUIMenuItem(gui::MENUITEM_ENTRY, "Last Used Programs"));
            Item3->addChild(Item4 = new gui::GUIMenuItem(gui::MENUITEM_ENTRY, "SPE Browser")); Item4->setEnable(false);
            Item3->addChild(new gui::GUIMenuItem(gui::MENUITEM_ENTRY, "SPE Info"));
            Item3->addChild(new gui::GUIMenuItem(gui::MENUITEM_ENTRY, "SPE License"));
        Item2->addChild(new gui::GUIMenuItem(gui::MENUITEM_SEPARATOR));
        Item2->addChild(ItemQuit = new gui::GUIMenuItem(gui::MENUITEM_ENTRY, "Quit\tCtrl+Q"));
    Item1->addChild(Item2 = new gui::GUIMenuItem(gui::MENUITEM_TITLE, "Help"));
        Item2->addChild(new gui::GUIMenuItem(gui::MENUITEM_ENTRY, "Forum"));
        Item2->addChild(new gui::GUIMenuItem(gui::MENUITEM_SEPARATOR));
        Item2->addChild(new gui::GUIMenuItem(gui::MENUITEM_ENTRY, "About"));
    
    Win->setMenuRoot(Item1);
    
    /**
     * Create the geometry window.
     */
    WinGeom = spGUI->addWindow(dim::point2di(300, 150), 350, "3D Geometry");
    
    gui::GUIContainerGadget* Con = WinGeom->addContainerGadget(15, WinGeom->getLocalViewArea().getSize() - dim::size2di(30));
    Con->setOwnerDrawCallback(OwnerDrawGeom);
    
    WinGeom->setVisible(false);
    
    /**
     * Create the labyrinth game window.
     */
    WinGame = spGUI->addWindow(dim::point2di(100, 50), dim::size2di(593, 496), "Labyrinth Game");
    
    Con = WinGame->addContainerGadget(15, WinGame->getLocalViewArea().getSize() - dim::size2di(30));
    Con->setOwnerDrawCallback(OwnerDrawGame);
    
    WinGame->setVisible(false);
    
    /**
     * Create the labyrinth game path graph.
     */
    Graph = new tool::PathGraph();
    
    io::FileSystem FileSys;
    io::File* BitmapFile = FileSys.openFile(ResPath + "LabyrinthBitmap.txt", io::FILE_READ);
    
    std::vector<bool> Bitmap;
    
    if (BitmapFile)
    {
        while (!BitmapFile->isEOF())
        {
            io::stringc Line = BitmapFile->readString();
            
            s32 Offset = Bitmap.size();
            Bitmap.resize(Bitmap.size() + Line.size());
            
            for (u32 i = 0; i < Line.size(); ++i)
                Bitmap[Offset + i] = (Line[i] == ' ');
        }
        
        FileSys.closeFile(BitmapFile);
    }
    
    Graph->createGrid(
        dim::point2df(5, 5), dim::point2df(544, 423), dim::vector3di(50, 39, 1), Bitmap, false
    );
    
    /**
     * Create the open file window
     */
    WinFileBrowser = spGUI->addWindow(
        dim::point2di(100, 50), dim::size2di(300, 250), "Open file", gui::GUIFLAG_SIZEBUTTON | gui::GUIFLAG_CLOSEBUTTON
    );
    
    WinFileBrowser->setMinSize(dim::size2di(194, 200));
    WinFileBrowser->setVisible(false);
    
    const dim::size2di WinSize(WinFileBrowser->getLocalViewArea().getSize());
    
    BtnOpen     = WinFileBrowser->addButtonGadget(dim::point2di(WinSize.Width - 180, WinSize.Height - 35), dim::size2di(80, 25), "Open");
    BtnCancel   = WinFileBrowser->addButtonGadget(dim::point2di(WinSize.Width - 90, WinSize.Height - 35), dim::size2di(80, 25), "Cancel");
    
    GadBrowserTree = WinFileBrowser->addTreeGadget(
        10, dim::size2di((WinSize.Width - 20)/2 - 5, WinSize.Height - 55), gui::GUIFLAG_HOLDSELECTION
    );
    GadBrowserTree->setExplorer(true);
    
    GadBrowserList = WinFileBrowser->addListGadget(
        dim::point2di((WinSize.Width - 20)/2 + 15, 10), dim::size2di((WinSize.Width - 20)/2 - 5, WinSize.Height - 55), gui::GUIFLAG_HOLDSELECTION
    );
    GadBrowserList->addColumn("Filename", 200);
    
    /**
     * Create the movie player
     */
    WinMoviePlayer = spGUI->addWindow(dim::point2di(100, 50), dim::size2di(600, 450), "Movie Player");
    
    Con = WinMoviePlayer->addContainerGadget(15, WinMoviePlayer->getLocalViewArea().getSize() - dim::size2di(30));
    Con->setOwnerDrawCallback(OwnerDrawMovie);
    
    WinMoviePlayer->setVisible(false);
}

/**
 * Only update the GUI.
 */
void UpdateScene()
{
    spGUI->update();
}

#else

int main()
{
    io::Log::error("GUI was not compiled in this version");
    return 0;
}

#endif



// ============================================
