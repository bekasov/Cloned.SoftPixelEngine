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

SoftPixelDevice* spDevice           = 0;
video::RenderSystem* spRenderer     = 0;
video::RenderContext* spContext     = 0;
io::InputControl* spControl         = 0;
scene::SceneManager* spSceneMngr    = 0;
scene::SceneGraph* spScene          = 0;

class CustomSceneLoader : public scene::SceneLoaderSPSB
{
    
    public:
        
        CustomSceneLoader() : scene::SceneLoaderSPSB()
        {
        }
        ~CustomSceneLoader()
        {
        }
        
    private:
        
        scene::Mesh* createMeshResource(const SpMeshConstructionResource &Construct)
        {
            return spScene->getMesh(getFinalPath(Construct.Filename));
        }
        
};

int main()
{
    io::Log::open();
    
    spDevice = createGraphicsDevice(
        //video::RENDERER_DIRECT3D9,
        video::RENDERER_OPENGL,
        dim::size2di(1280, 768), 32, "SoftPixel Engine - SceneLoader Tutorial", false, DEVICEFLAG_HQ
    );
    
    if (!spDevice)
    {
        io::Log::pauseConsole();
        return 0;
    }
    
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    spControl   = spDevice->getInputControl();
    spSceneMngr = spDevice->getSceneManager();
    
    spScene     = spDevice->createSceneGraph();
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    //scene::Camera* Cam = spScene->createCamera();
    //Cam->setRange(0.1f, 500.0f);
    
    scene::SceneManager::setTextureLoadingState(false);
    
    CustomSceneLoader Loader;
    
    Loader.loadScene(
        //"D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/StoryboardTest1.spsb"
        //"D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/DevmodeTestScene1.spsb"
        "D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/Trees.spsb"
        //"D:/SoftwareEntwicklung/C++/HLC/Spiele/QuarksGame/maps/tests/FirstGameMap-Prototype1.spsb"
        
        ,video::TEXPATH_IGNORE, scene::DEF_SCENE_FLAGS
    );
    
    spScene->setDepthSorting(false);
    spScene->sortRenderList(scene::RENDERLIST_SORT_MESHBUFFER);
    
    spScene->setLighting();
    
    foreach (scene::Animation* Anim, spScene->getAnimationList())
        Anim->play(scene::PLAYBACK_PINGPONG_LOOP);
    
    scene::SceneNode* SkyBox = spScene->findNode("skybox");
    
    //scene::Camera* Cam = spScene->getActiveCamera();
    scene::Camera* Cam = spScene->createCamera();
    
    #define SPHERE_TEST
    #ifdef SPHERE_TEST
    Cam->setPosition(dim::vector3df(0, 0, -3.5f));
    
    video::Texture* CMTex = spRenderer->createCubeMap(256);
    
    scene::Mesh* Sphere = spScene->createMesh(scene::MESH_ICOSPHERE, 5);
    Sphere->setScale(2);
    Sphere->addTexture(CMTex);
    Sphere->getMeshBuffer(0)->setMappingGen(0, video::MAPGEN_REFLECTION_MAP);
    Sphere->getMaterial()->setAmbientColor(128);
    
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
    cmd->setRect(dim::rect2di(0, 0, spContext->getResolution().Width, spContext->getResolution().Height));
    
    bool isCmdActive = false;
    spControl->setWordInput(isCmdActive);
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
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
        
        spRenderer->clearBuffers();
        
        spSceneMngr->updateAnimations();
        spScene->renderScene(Cam);
        
        if (spControl->keyHit(io::KEY_F3))
        {
            isCmdActive = !isCmdActive;
            spControl->setWordInput(isCmdActive);
        }
        
        if (isCmdActive)
            cmd->render();
        
        spRenderer->draw2DText(cmd->getFont(), 15, "FPS: " + io::stringc(io::Timer::getFPS()));
        
        spContext->flipBuffers();
    }
    
    delete cmd;
    
    deleteDevice();
    
    return 0;
}



// ============================================