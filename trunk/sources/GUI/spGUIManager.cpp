/*
 * GUI manager file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUIManager.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Base/spTimer.hpp"
#include "Base/spSharedObjects.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;
extern io::InputControl* __spInputControl;
extern SoftPixelDevice* __spDevice;

namespace gui
{


/*
 * Internal members
 */

video::Font* __spGUIFont = 0;


/*
 * GUIManager class
 */

GUIManager::GUIManager() :
    TexInterface_       (0      ),
    EventCallback_      (0      ),
    FocusedController_  (0      ),
    FocusUsing_         (false  ),
    MouseWheel_         (0      ),
    Time_               (0      ),
    HatchedFace_        (0      ),
    ArrowTex_           (0      )
{
    /* Create interface texture where the whole GUI will be drawn into */
    TexInterface_ = __spVideoDriver->createTexture(
        dim::size2di(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight), video::PIXELFORMAT_RGBA
    );
    
    TexInterface_->setMipMapping(false);
    TexInterface_->setRenderTarget(true);
    
    /* Load GUI font */
    __spGUIFont = __spVideoDriver->loadFont("arial", 15, video::FONT_BOLD);
    
    /* Make sure the input control has been created */
    __spDevice->getInputControl();
    
    /* Create basic textures */
    createHatchedFace();
    createArrowTex();
}
GUIManager::~GUIManager()
{
    /* Delete all GUI controller elements */
    MemoryManager::deleteList(ControllerList_);
    
    /* Delete GUI base textures */
    __spVideoDriver->deleteTexture(TexInterface_);
    __spVideoDriver->deleteTexture(HatchedFace_);
    __spVideoDriver->deleteTexture(ArrowTex_);
}

io::stringc GUIManager::getVersion() const
{
    return "BoxGUI - v.2.0.1 beta";
}

void GUIManager::update()
{
    CursorSpeed_    = __spInputControl->getCursorSpeed();
    CursorPos_      = __spInputControl->getCursorPosition();
    MouseWheel_     = __spInputControl->getMouseWheel();
    Time_           = io::Timer::millisecs();
    InputStr_       = __spDevice->getUserCharList();
    
    /*video::Texture* LastRenderTarget = __spVideoDriver->getRenderTarget();
    __spVideoDriver->setRenderTarget(TexInterface_);
    
    __spVideoDriver->setClearColor(video::color(255, 255, 255, 0));
    __spVideoDriver->clearBuffers();*/
    __spVideoDriver->beginDrawing2D();
    
    //__spVideoDriver->draw2DImage(TexInterface_, 0); // !!!
    
    /* Update each GUI controller */
    u32 i = 0;
    foreach_reverse (GUIController* Obj, ParentControllerList_)
    {
        Obj->isForeground_ = (i == 0);
        Obj->updateClipping();
        ++i;
    }
    
    foreach_reverse (GUIController* Obj, ParentControllerList_)
    {
        if (Obj->update())
            break;
    }
    
    /* Draw each GUI controller */
    foreach (GUIController* Obj, ParentControllerList_)
        Obj->draw();
    
    __spVideoDriver->endDrawing2D();
    
    __spVideoDriver->setClipping(false, 0, 0);
    //__spVideoDriver->setRenderTarget(LastRenderTarget);
    
    /* Update focus using */
    if (FocusUsing_ && __wasMouseKey[io::MOUSE_LEFT])
    {
        FocusUsing_ = false;
        if (FocusedController_)
            FocusedController_->FocusUsage_ = -1;
    }
}

GUIWindow* GUIManager::addWindow(
    const dim::point2di &Position, const dim::size2di &Size, const io::stringc &Title, s32 Flags)
{
    GUIWindow* NewWindow = new GUIWindow();
    {
        NewWindow->setPosition(Position);
        NewWindow->setSize(Size);
        NewWindow->setFlags(Flags);
        NewWindow->setText(Title);
    }
    addController(NewWindow, true);
    
    return NewWindow;
}
void GUIManager::removeWindow(GUIWindow* Window)
{
    removeController(Window, !Window->getParent());
}


/*
 * ======= Private: =======
 */

void GUIManager::createHatchedFace()
{
    HatchedFace_ = __spVideoDriver->createTexture(2, video::PIXELFORMAT_RGBA);
    
    const u32 ImageBuffer[4] = { 0x90000000, 0x00000000, 0x00000000, 0x90000000 };
    
    HatchedFace_->setupImageBuffer(&ImageBuffer);
    HatchedFace_->setFilter(video::FILTER_LINEAR);
}
void GUIManager::createArrowTex()
{
    ArrowTex_ = __spVideoDriver->createTexture(32, video::PIXELFORMAT_RGBA);
    
    u8 ImageBuffer[32*32*4];
    
    for (s32 y = 0, x, i = 0; y < 32; ++y)
    {
        for (x = 0; x < 32; ++x, ++i)
        {
            *((s32*)ImageBuffer + i) = (
                (x > 32/2 - y + 8 && x < 32/2 + y - 8 && y > 8 && y < 20) ? 0xFF000000 : 0
            );
        }
    }
    
    ArrowTex_->setupImageBuffer(ImageBuffer);
}

void GUIManager::addController(GUIController* Controller, bool isParent)
{
    if (isParent)
        ParentControllerList_.push_back(Controller);
    ControllerList_.push_back(Controller);
}

void GUIManager::removeController(GUIController* Controller, bool isParent)
{
    if (isParent)
        removeParentController(Controller);
    
    MemoryManager::removeElement(ControllerList_, Controller, true);
}

void GUIManager::removeParentController(GUIController* Controller)
{
    MemoryManager::removeElement(ParentControllerList_, Controller);
}


} // /namespace gui

} // /namespace sp


#endif



// ================================================================================
