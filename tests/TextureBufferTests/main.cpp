//
// SoftPixel Engine - Texture Buffer Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

int main()
{
    SP_TESTS_INIT_EX(
        video::RENDERER_OPENGL, dim::size2di(800, 600), "TextureBuffer", false
    )
    
    // Create textures
    video::Texture* BufTex = spRenderer->createTexture(dim::size2di(4096), video::PIXELFORMAT_RGBA);
    video::ImageBuffer* ImgBuf = BufTex->getImageBuffer();
    {
        ImgBuf->setPixelColor(0, video::color(0, 255, 0));
    }
    BufTex->setDimension(video::TEXTURE_BUFFER);
    
    video::Texture* RectTex = spRenderer->loadTexture("../Media/SkyboxNorth.jpg");
    RectTex->setDimension(video::TEXTURE_RECTANGLE);
    
    // Create scene
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_CUBE);
    
    Cam->setPosition(dim::vector3df(0, 0, -2));
    
    Obj->addTexture(RectTex);
    Obj->addTexture(BufTex);
    
    // Load shaders
    video::ShaderClass* ShdClass = spRenderer->createShaderClass();
    
    video::Shader* ShdVert = spRenderer->loadShader(ShdClass, video::SHADER_VERTEX, video::GLSL_VERSION_1_20, "TBOShader.glvert");
    video::Shader* ShdFrag = spRenderer->loadShader(ShdClass, video::SHADER_PIXEL, video::GLSL_VERSION_1_20, "TBOShader.glfrag");
    
    if (ShdClass->link())
        Obj->setShaderClass(ShdClass);
    
    ShdFrag->setConstant("RectTex", 0);
    ShdFrag->setConstant("BufTex", 1);
    
    // Draw scene
    SP_TESTS_MAIN_BEGIN
    {
        tool::Toolset::presentModel(Obj);
        
        spScene->renderScene();
    }
    SP_TESTS_MAIN_END
}
