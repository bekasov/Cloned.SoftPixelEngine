//
// SoftPixel Engine - Terrain Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

void ShaderCallback(video::ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    ShdClass->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
}

int main()
{
    SP_TESTS_INIT("Terrain")
    
    // Load textures
    const io::stringc ResPath("../Media/");
    
    video::Texture* HeightMap   = spRenderer->loadTexture(ResPath + "HeightMap.jpg");
    video::Texture* LerpMap     = spRenderer->loadTexture("LerpMap.png");
    
    video::Texture* ColorMap[4] =
    {
        spRenderer->loadTexture(ResPath + "Grass1.jpg"),
        spRenderer->loadTexture(ResPath + "FloorBricks1.jpg"),
        spRenderer->loadTexture(ResPath + "DryGround1.jpg"),
        spRenderer->loadTexture(ResPath + "GrassSnow1.jpg"),
    };
    
    // Create scene
    scene::Terrain* HeightField = spScene->createTerrain(video::SHeightMapTexture(), 8, 4);
    
    HeightField->setScale(dim::vector3df(100, 20, 100));
    HeightField->setPosition(dim::vector3df(0, -25, 0));
    
    HeightField->getMaterial()->setLighting(false);
    
    // Main loop
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree(0, 0.5f);
        
        if (spControl->keyHit(io::KEY_TAB))
        {
            static bool Wire;
            Wire = !Wire;
            spScene->setWireframe(Wire ? video::WIREFRAME_LINES : video::WIREFRAME_SOLID);
        }
        
        spScene->renderScene();
        
        tool::Toolset::drawDebugInfo(Fnt);
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}
