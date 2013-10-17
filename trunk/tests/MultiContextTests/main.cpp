//
// SoftPixel Engine - Multi Context Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

int main()
{
    const dim::size2di ScrSize(640, 480);
    const io::stringc Title = "Multi Context";
    
    SDeviceFlags DevFlags;
    DevFlags.AntiAliasing.Enabled;
    DevFlags.AntiAliasing.MultiSamples = 4;
    DevFlags.Window.Resizable = true;
    
    spDevice = createGraphicsDevice(
        #if 0
        video::RENDERER_OPENGL,
        #else
        video::RENDERER_DIRECT3D11,
        #endif
        ScrSize, 32, "Tests: " + Title, false, DevFlags
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
    
    // Create 2nd context
    video::RenderContext* spContext2 = spDevice->createRenderContext(0, ScrSize, Title + " [ 2nd Context ]");
    
    dim::point2di WinPos(spContext2->getWindowPosition());
    WinPos.X -= ScrSize.Width;
    spContext2->setWindowPosition(WinPos);
    
    // Create scene
    scene::SceneGraph* spScene2 = spDevice->createSceneGraph();
    
    Cam = spScene->createCamera();
    Cam->setPosition(dim::vector3df(0, 0, -3));
    
    Lit = spScene->createLight();
    Lit->setRotation(dim::vector3df(25, 25, 0));
    
    //scene::Light* Lit2 = spScene2->createLight();
    //Lit2->setRotation(dim::vector3df(-10, 15, 0));
    
    spScene->setLighting();
    
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_CUBE);
    Obj->getMaterial()->setColorMaterial(false);
    Obj->getMaterial()->setDiffuseColor(video::color(50, 255, 30));
    
    scene::Mesh* Obj2 = spScene2->createMesh(scene::MESH_CUBE);
    Obj2->getMaterial()->setColorMaterial(false);
    Obj2->getMaterial()->setDiffuseColor(video::color(255, 50, 30));
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        // Update scene
        if (spContext->isWindowActive())
            tool::Toolset::presentModel(Obj);
        if (spContext2->isWindowActive())
            tool::Toolset::presentModel(Obj2);
        
        // Render into 1st context
        spContext->activate();
        spRenderer->clearBuffers();
        {
            Cam->setViewport(dim::rect2di(0, 0, spContext->getResolution().Width, spContext->getResolution().Height));
            spScene->renderScene(Cam);
        }
        spContext->flipBuffers();
        
        // Render into 2nd context
        spContext2->activate();
        spRenderer->clearBuffers();
        {
            Cam->setViewport(dim::rect2di(0, 0, spContext2->getResolution().Width, spContext2->getResolution().Height));
            spScene2->renderScene(Cam);
        }
        spContext2->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}
