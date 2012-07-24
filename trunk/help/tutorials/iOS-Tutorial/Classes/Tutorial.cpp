/*
 *  Tutorial.cpp
 *  SPE-Tutorial1
 *
 *  Created by Lukas Hermanns on 03.01.12.
 *  Copyright 2012 TU  Darmstadt. All rights reserved.
 *
 */

#include <SoftPixelEngine.hpp>

#include <boost/foreach.hpp>

using namespace sp;

SoftPixelDevice* spDevice           = 0;
io::InputControl* spControl         = 0;
video::RenderContext* spContext     = 0;
video::RenderSystem* spRenderer     = 0;
scene::SceneGraph* spScene          = 0;
scene::CollisionDetector* spColl    = 0;

s32 ScrWidth = 0, ScrHeight = 0;

scene::Camera* Cam  = 0;

scene::Mesh* Cube = 0;

/**
 * Derive from the "SceneLoaderSPSB" to manipulate all resource paths. We only use the filenames
 * and no paths i.e. we change "Textures/Tex1.jpg" to "Tex1.jpg". That's easier for iOS development
 * when using resources. With Android it's the same thing.
 */
class SceneLoader : public scene::SceneLoaderSPSB
{
    public:
        SceneLoader() : scene::SceneLoaderSPSB()
        {
        }
        virtual ~SceneLoader()
        {
        }
        
    protected:
        // This is the only function we overwrite.
        virtual io::stringc getAbsolutePath(const io::stringc &Path) const
        {
            // Just return the filename part without a file path.
            return Path.getFilePart();
        }
        
};

void CreateTutorial()
{
    // Create device. Here we use OpenGL|ES 1 because we don't want to use any shaders in this example.
    spDevice    = createGraphicsDevice(video::RENDERER_OPENGLES1);
    
    ScrWidth    = spDevice->getResolution().Width;
    ScrHeight   = spDevice->getResolution().Height;
    
    spControl   = spDevice->getInputControl();
    spRenderer  = spDevice->getRenderSystem();
    spScene     = spDevice->getSceneGraph();
    spColl      = spDevice->getCollisionDetector();
    
    // Load the "SoftPixel Sandbox Scene" from our resources.
    io::Log::message("Load scene: \"DemoScene.spsb\"");
    io::Log::upperTab();
    
    SceneLoader().loadScene(
        "DemoScene.spsb", "",
        scene::SCENEFLAG_CAMERAS | scene::SCENEFLAG_TEXTURES | scene::SCENEFLAG_LIGHTMAPS
    );
    
    io::Log::lowerTab();
    
    Cam = spScene->getActiveCamera();
    
    // Create a simple collision model
    scene::Collision* WorldColl = spColl->createCollision();
    scene::Collision* CamColl   = spColl->createCollision();
    
    CamColl->addCollisionMaterial(WorldColl, scene::COLLISION_SPHERE_TO_POLYGON);
    
    foreach (scene::Mesh* Obj, spScene->getMeshList())
        spColl->addCollisionMesh(Obj, WorldColl);
    
    spColl->addCollisionObject(Cam, CamColl, 0.7);
    
    // Create small object
    Cube = spScene->createMesh(scene::MESH_CUBE);
    Cube->addTexture(spRenderer->loadTexture("SoftPixelLogo.jpg"));
    Cube->setPosition(dim::vector3df(-5, 4, 5));
    Cube->paint(130);
}

void DrawTutorial()
{
    // We don't have our own main loop so call "updateEvent" in this function at first.
    spDevice->updateEvent();
    spRenderer->clearBuffers();
    
    spColl->updateScene();
    
    // Just render the scene for the last activated camera.
    spScene->renderScene(Cam);
    
    // For Android and iOS the "moveCameraFree" function also provides two controller.
    // These controllers are need to be drawn after all rendering.
    tool::Toolset::moveCameraFree();
    
    // Animate the cube
    static f32 Deg;
    Deg += 3.5f;
    
    Cube->setPosition(dim::vector3df(-5, 4 + SIN(Deg)*0.25f, 5));
    Cube->turn(dim::vector3df(0, 1.5f, 0));
}

void ClearTutorial()
{
    deleteDevice();
}

