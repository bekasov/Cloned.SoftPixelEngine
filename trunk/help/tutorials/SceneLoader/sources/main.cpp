//
// SoftPixel Engine Tutorial: SceneLoader - (21/07/2012)
//

#include <SoftPixelEngine.hpp>
#include <boost/foreach.hpp>

using namespace sp;

#include "../../common.hpp"

int main()
{
    SoftPixelDevice* spDevice = createGraphicsDevice(
        /*ChooseRenderer()*/video::RENDERER_OPENGL, dim::size2di(640, 480), 32, "SoftPixel Engine - SceneLoader Tutorial"
    );
    
    video::RenderSystem* spRenderer = spDevice->getRenderSystem();
    video::RenderContext* spContext = spDevice->getRenderContext();
    scene::SceneGraph* spScene      = spDevice->getSceneGraph();
    io::InputControl* spControl     = spDevice->getInputControl();
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    spScene->loadScene(
        "D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/DevmodeTestScene1.spsb"
        //"D:/SoftwareEntwicklung/C++/HLC/Spiele/QuarksGame/maps/tests/FirstGameMap-Prototype1.spsb"
        ,video::TEXPATH_IGNORE, scene::SCENEFORMAT_UNKNOWN,
        scene::SCENEFLAG_ALL ^ scene::SCENEFLAG_LIGHTMAPS
    );
    
    spScene->setLighting();
    
    foreach (scene::Animation* Anim, spScene->getAnimationList())
        Anim->play(scene::PLAYBACK_PINGPONG_LOOP);
    
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        #if 0
        if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree();
        #else
        if (spControl->mouseDown(io::MOUSE_LEFT))
        {
            static f32 x, y;
            x += static_cast<f32>(spControl->getCursorSpeed().Y)*0.25f;
            y += static_cast<f32>(spControl->getCursorSpeed().X)*0.25f;
            spScene->getActiveCamera()->setRotation(dim::vector3df(x, y, 0));
        }
        #endif
        
        spScene->updateAnimations();
        spScene->renderScene();
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}



// ============================================