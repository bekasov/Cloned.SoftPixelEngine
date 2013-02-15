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
    
    //#define TEX_ARRAY
    #ifdef TEX_ARRAY
    for (u32 i = 1; i < 4; ++i)
        ColorMap[0]->getImageBuffer()->appendImageBuffer(ColorMap[i]->getImageBuffer());
    
    GrassTex->setDimension(video::TEXTURE_2D_ARRAY, 4);
    #endif
    
    // Create scene
    scene::Mesh* Terrain = spScene->createHeightField(HeightMap, 64);
    
    Terrain->setScale(dim::vector3df(100, 20, 100));
    Terrain->setPosition(dim::vector3df(0, -25, 0));
    
    #ifdef TEX_ARRAY
    Terrain->addTexture(ColorMap[0]);
    #else
    for (u32 i = 0; i < 4; ++i)
        Terrain->addTexture(ColorMap[i]);
    #endif
    
    Terrain->addTexture(LerpMap);
    
    // Load shader
    video::ShaderClass* ShdClass = spRenderer->createShaderClass();
    
    video::Shader* VertShd = spRenderer->loadShader(ShdClass, video::SHADER_VERTEX, video::GLSL_VERSION_1_20, "TerrainShader.glvert");
    video::Shader* FragShd = spRenderer->loadShader(ShdClass, video::SHADER_PIXEL, video::GLSL_VERSION_1_20, "TerrainShader.glfrag");
    
    if (ShdClass->link())
    {
        ShdClass->setObjectCallback(ShaderCallback);
        Terrain->setShaderClass(ShdClass);
        
        #ifndef TEX_ARRAY
        
        for (s32 i = 0; i < 4; ++i)
            FragShd->setConstant("ColorMap" + io::stringc(i + 1), i);
        
        FragShd->setConstant("LerpMap", 4);
        
        #endif
        
        FragShd->setConstant("TexScale", 0.1f);
    }
    
    // Main loop
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree();
        
        if (spControl->keyHit(io::KEY_TAB))
        {
            static bool Wire;
            Wire = !Wire;
            spScene->setWireframe(Wire ? video::WIREFRAME_LINES : video::WIREFRAME_SOLID);
        }
        
        spScene->renderScene();
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}
