//
// SoftPixel Engine Tutorial: SceneLoader - (21/07/2012)
//

#include <SoftPixelEngine.hpp>
#include <boost/foreach.hpp>

using namespace sp;

//#include "../../common.hpp"

int main()
{
    SoftPixelDevice* spDevice = createGraphicsDevice(
        video::RENDERER_DIRECT3D9, dim::size2di(800, 600), 32, "SoftPixel Engine - SceneLoader Tutorial"
    );
    
    video::RenderSystem* spRenderer = spDevice->getRenderSystem();
    video::RenderContext* spContext = spDevice->getRenderContext();
    scene::SceneGraph* spScene      = spDevice->getSceneGraph();
    io::InputControl* spControl     = spDevice->getInputControl();
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    //scene::Camera* Cam = spScene->createCamera();
    //Cam->setRange(0.1f, 500.0f);
    
    scene::SceneGraph::setTextureLoadingState(false);
    
    spScene->loadScene(
        "D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/DevmodeTestScene1.spsb"
        //"D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/Trees.spsb"
        //"D:/SoftwareEntwicklung/C++/HLC/Spiele/QuarksGame/maps/tests/FirstGameMap-Prototype1.spsb"
    );
    
    spScene->setLighting();
    
    foreach (scene::Animation* Anim, spScene->getAnimationList())
        Anim->play(scene::PLAYBACK_PINGPONG_LOOP);
    
    scene::SceneNode* SkyBox = spScene->findNode("skybox");
    
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree();
        
        if (SkyBox)
            SkyBox->setPosition(spScene->getActiveCamera()->getPosition(true));
        
        spScene->updateAnimations();
        spScene->renderScene();
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}



// ============================================