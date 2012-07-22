/*
 * iOS Devmode file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Platform/iOS/spDevmode(iOS).h"

#include "SoftPixelEngine.hpp"

#include <OpenGLES/ES2/gl.h>

using namespace sp;

SoftPixelDevice* spDevice           = 0;
video::RenderSystem* spRenderer     = 0;
video::RenderContext* spContext     = 0;
io::InputControl* spControl         = 0;
scene::SceneGraph* spScene          = 0;

scene::Camera* Cam = 0;
scene::Light* Lit = 0;
scene::Mesh* Obj = 0;

dim::size2di ScrSize;

#define IOS_MEDIA_PATH io::stringc("/Users/lukashermanns/SoftwareEntwicklung/SoftPixelEngine/")

void CreateSPEDevice()
{
    io::Log::open(IOS_MEDIA_PATH + "spDebugLog.txt");
    
    spDevice    = createGraphicsDevice(video::RENDERER_OPENGLES1);
    
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    spControl   = spDevice->getInputControl();
    spScene     = spDevice->getSceneGraph();
    
    spRenderer->setClearColor(video::color(222, 222, 255));
    
    ScrSize = spDevice->getResolution();
    
    // Create camera and light
    Cam = spScene->createCamera();
    Cam->setPosition(dim::vector3df(0, 0, -10));
    Cam->setRange(0.1, 100);
    
    Lit = spScene->createLight();
    spScene->setLighting();
    
    // Create scene
    Obj = spScene->createMesh(scene::MESH_CUBE);
    Obj->setScale(3);
    
    io::Log::close();
}

void DrawSPEFrame()
{
    spDevice->updateEvent();
    spRenderer->clearBuffers();
    
    Obj->turn(1.0f);
    
    spScene->renderScene();
    
    tool::Toolset::moveCameraFree();
    
    spContext->flipBuffers();
}
