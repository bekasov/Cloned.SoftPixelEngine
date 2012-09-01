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

int main()
{
    SoftPixelDevice* spDevice = createGraphicsDevice(
        video::RENDERER_OPENGL, dim::size2di(800, 600), 32, "SoftPixel Engine - SceneLoader Tutorial", false, DEVICEFLAG_HQ
    );
    
    if (!spDevice)
    {
        io::Log::pauseConsole();
        return 0;
    }
    
    video::RenderSystem* spRenderer = spDevice->getRenderSystem();
    video::RenderContext* spContext = spDevice->getRenderContext();
    io::InputControl* spControl     = spDevice->getInputControl();
    
    scene::SceneGraph* spScene      = spDevice->createSceneGraph();
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    //scene::Camera* Cam = spScene->createCamera();
    //Cam->setRange(0.1f, 500.0f);
    
    scene::SceneGraph::setTextureLoadingState(false);
    
    spScene->loadScene(
        //"D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/DevmodeTestScene1.spsb"
        "D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/Trees.spsb"
        //"D:/SoftwareEntwicklung/C++/HLC/Spiele/QuarksGame/maps/tests/FirstGameMap-Prototype1.spsb"
    );
    
    spScene->setLighting();
    
    foreach (scene::Animation* Anim, spScene->getAnimationList())
        Anim->play(scene::PLAYBACK_PINGPONG_LOOP);
    
    scene::SceneNode* SkyBox = spScene->findNode("skybox");
    
    scene::Camera* Cam = spScene->getActiveCamera();
    
    #if 1
    
    spScene->getActiveCamera()->setPosition(dim::vector3df(0, 0, -3.5f));
    
    video::Texture* CMTex = spRenderer->createCubeMap(256);
    
    scene::Mesh* Sphere = spScene->createMesh(scene::MESH_ICOSPHERE, 5);
    Sphere->setScale(2);
    Sphere->addTexture(CMTex);
    Sphere->getMeshBuffer(0)->setMappingGen(0, video::MAPGEN_REFLECTION_MAP);
    
    Sphere->getBoundingVolume().setType(scene::BOUNDING_SPHERE);
    Sphere->getBoundingVolume().setRadius(1.0f);
    
    #endif
    
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree();
        
        if (SkyBox)
            SkyBox->setPosition(Cam->getPosition(true));
        
        #if 1
        if ( Sphere->getBoundingVolume().checkFrustumCulling(Cam->getViewFrustum(), Sphere->getTransformMatrix()) && 
             math::getDistanceSq(Cam->getPosition(), Sphere->getPosition()) < math::Pow2(25.0f) )
        {
            video::ShadowMapper::renderCubeMap(spScene, Cam, CMTex, Sphere->getPosition());
        }
        #endif
        
        spScene->updateAnimations();
        spScene->renderScene(Cam);
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}



// ============================================