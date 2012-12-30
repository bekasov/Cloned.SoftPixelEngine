//
// SoftPixel Engine Tutorial: SceneLoader - (21/07/2012)
//

#include <SoftPixelEngine.hpp>
#include <boost/foreach.hpp>

using namespace sp;

//#include "../../common.hpp"

#if 1
#include <RenderSystem/DeferredRenderer/spShadowMapper.hpp>
#endif

tool::CommandLineUI* cmd = 0;

int main()
{
    io::Log::open();
    
    SoftPixelDevice* spDevice = createGraphicsDevice(
        video::RENDERER_OPENGL, dim::size2di(800, 600), 32, "SoftPixel Engine - SceneLoader Tutorial", false, DEVICEFLAG_HQ
    );
    
    if (!spDevice)
    {
        io::Log::pauseConsole();
        return 0;
    }
    
    video::RenderSystem* spRenderer     = spDevice->getRenderSystem();
    video::RenderContext* spContext     = spDevice->getRenderContext();
    io::InputControl* spControl         = spDevice->getInputControl();
    scene::SceneManager* spSceneMngr    = spDevice->getSceneManager();
    
    scene::SceneGraph* spScene          = spDevice->createSceneGraph();
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    //scene::Camera* Cam = spScene->createCamera();
    //Cam->setRange(0.1f, 500.0f);
    
    scene::SceneManager::setTextureLoadingState(false);
    
    spScene->loadScene(
        //"D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/DevmodeTestScene1.spsb"
        //"D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/Trees.spsb"
        //"D:/SoftwareEntwicklung/C++/HLC/Spiele/QuarksGame/maps/tests/FirstGameMap-Prototype1.spsb"
        "D:/SoftwareEntwicklung/C++/HLC/Spiele/KettenSaegenKurt/maps/Office.spsb"
    );
    
    spScene->setLighting();
    
    foreach (scene::Animation* Anim, spScene->getAnimationList())
        Anim->play(scene::PLAYBACK_PINGPONG_LOOP);
    
    scene::SceneNode* SkyBox = spScene->findNode("skybox");
    
    //scene::Camera* Cam = spScene->getActiveCamera();
    scene::Camera* Cam = spScene->createCamera();
    
    //#define SPHERE_TEST
    #ifdef SPHERE_TEST
    spScene->getActiveCamera()->setPosition(dim::vector3df(0, 0, -3.5f));
    
    video::Texture* CMTex = spRenderer->createCubeMap(256);
    
    scene::Mesh* Sphere = spScene->createMesh(scene::MESH_ICOSPHERE, 5);
    Sphere->setScale(2);
    Sphere->addTexture(CMTex);
    Sphere->getMeshBuffer(0)->setMappingGen(0, video::MAPGEN_REFLECTION_MAP);
    
    Sphere->getBoundingVolume().setType(scene::BOUNDING_SPHERE);
    Sphere->getBoundingVolume().setRadius(1.0f);
    #endif
    
    //#define RT_TEST
    #ifdef RT_TEST
    video::Texture* rt = spRenderer->createTexture(512);
    //rt->setMultiSamples(8);
    rt->setRenderTarget(true);
    scene::Mesh* rtObj = spScene->createMesh(scene::MESH_CUBE);
    rtObj->setScale(4);
    rtObj->setPosition(dim::vector3df(0, 0, 4));
    rtObj->getMaterial()->setLighting(false);
    rtObj->addTexture(rt);
    #endif
    
    tool::CommandLineUI* cmd = new tool::CommandLineUI();
    cmd->setBackgroundColor(video::color(0, 0, 0, 128));
    
    bool isCmdActive = false;
    spControl->setWordInput(isCmdActive);
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        if (!isCmdActive && spContext->isWindowActive())
            tool::Toolset::moveCameraFree();
        
        if (SkyBox)
            SkyBox->setPosition(Cam->getPosition(true));
        
        #ifndef RT_TEST
        
        #ifdef SPHERE_TEST
        if ( Sphere->getBoundingVolume().checkFrustumCulling(Cam->getViewFrustum(), Sphere->getTransformMatrix()) && 
             math::getDistanceSq(Cam->getPosition(), Sphere->getPosition()) < math::Pow2(25.0f) )
        {
            video::ShadowMapper::renderCubeMap(spScene, Cam, CMTex, Sphere->getPosition());
        }
        #endif
        
        #else
        
        spRenderer->setRenderTarget(rt);
        spRenderer->clearBuffers();
        
        rtObj->setVisible(false);
        spScene->renderScene();
        rtObj->setVisible(true);
        
        spRenderer->setRenderTarget(0);
        spRenderer->clearBuffers();
        
        #endif
        
        spSceneMngr->updateAnimations();
        spScene->renderScene(Cam);
        
        if (spControl->keyHit(io::KEY_F3))
        {
            isCmdActive = !isCmdActive;
            spControl->setWordInput(isCmdActive);
        }
        
        if (isCmdActive)
        {
            spRenderer->beginDrawing2D();
            {
                cmd->updateInput();
                cmd->draw();
            }
            spRenderer->endDrawing2D();
        }
        
        spContext->flipBuffers();
    }
    
    delete cmd;
    
    deleteDevice();
    
    return 0;
}



// ============================================