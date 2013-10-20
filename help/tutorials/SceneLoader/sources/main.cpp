//
// SoftPixel Engine Tutorial: SceneLoader - (21/07/2012)
//

#include <SoftPixelEngine.hpp>
#include <boost/foreach.hpp>

using namespace sp;

#include "../../common.hpp"

#if 1
#include <RenderSystem/AdvancedRenderer/spShadowMapper.hpp>
#endif

tool::CommandLineUI* cmd = 0;

SoftPixelDevice* spDevice           = 0;
video::RenderSystem* spRenderer     = 0;
video::RenderContext* spContext     = 0;
io::InputControl* spControl         = 0;
scene::SceneManager* spSceneMngr    = 0;
scene::SceneGraph* spScene          = 0;

int main()
{
    io::Log::open();
    
    spDevice = createGraphicsDevice(
        video::RENDERER_DIRECT3D9,
        //video::RENDERER_OPENGL,
        //ChooseRenderer(),
        dim::size2di(1024, 600), 32, "SoftPixel Engine - SceneLoader Tutorial", false, DEVICEFLAG_HQ
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
    
    spScene->loadScene(
        "D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/Trees.spsb"
        //ROOT_PATH + "SceneLoader/media/SmallDemoScene.spsb"
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

    // Create billboards
    video::Texture* BlBrdTex1 = spRenderer->loadTexture("../../../tests/Media/LightBulb.png");
    BlBrdTex1->setWrapMode(video::TEXWRAP_CLAMP);

    video::Texture* BlBrdTex2 = spRenderer->loadTexture("../../../tests/Media/LenseFlare1.jpg");
    BlBrdTex2->setWrapMode(video::TEXWRAP_CLAMP);

    foreach (scene::Light* Lit, spScene->getLightList())
    {
        if (Lit->getLightModel() == scene::LIGHT_POINT)
        {
            // Create 1st billboard
            scene::Billboard* BlBrd1 = spScene->createBillboard(BlBrdTex1);
            BlBrd1->setPosition(Lit->getPosition());
            BlBrd1->setAlignment(scene::BILLBOARD_UPVECTOR_ALIGNED);
            BlBrd1->setBasePosition(dim::vector3df(0, 0.5f, 0));

            BlBrd1->getMaterial()->setAlphaMethod(video::CMPSIZE_GREATER, 0.5f);
            BlBrd1->getMaterial()->setBlending(false);

            // Create 2nd billboard
            scene::Billboard* BlBrd2 = spScene->createBillboard(BlBrdTex2);
            BlBrd2->setPosition(Lit->getPosition());
            BlBrd2->setAlignment(scene::BILLBOARD_VIEWPOINT_ALIGNED);
            BlBrd2->setBasePosition(dim::vector3df(0, 0, -0.1f));
        }
    }
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        if (!isCmdActive && spContext->isWindowActive())
            tool::Toolset::moveCameraFree(0, spControl->keyDown(io::KEY_SHIFT) ? 0.5f : 0.25f);
        
        if (SkyBox)
            SkyBox->setPosition(Cam->getPosition(true));
        
        #if 0
        foreach (scene::Billboard* BlBrd, spScene->getBillboardList())
            BlBrd->setBaseRotation(BlBrd->getBaseRotation() + 0.1f);
        #endif

        #ifndef RT_TEST
        
        #ifdef SPHERE_TEST
        if ( Sphere->getBoundingVolume().checkFrustumCulling(Cam->getViewFrustum(), Sphere->getTransformMatrix()) && 
             math::getDistanceSq(Cam->getPosition(), Sphere->getPosition()) < math::pow2(25.0f) )
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
        
        #if 1
        tool::Toolset::drawDebugInfo(cmd->getFont(), spControl->keyHit(io::KEY_RETURN));
        #else
        spRenderer->draw2DText(cmd->getFont(), 15, "FPS: " + io::stringc(io::Timer::getFPS()));
        #endif
        
        spContext->flipBuffers();
    }
    
    delete cmd;
    
    deleteDevice();
    
    return 0;
}



// ============================================
