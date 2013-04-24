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
    video::Texture* Tex = spRenderer->loadTexture("../Media/SkyboxNorth.jpg");
    
    Tex->setMipMapping(false);
    Tex->setDimension(video::TEXTURE_RECTANGLE);
    
    // Create scene
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_CUBE);
    
    Cam->setPosition(dim::vector3df(0, 0, -2));
    
    Obj->addTexture(Tex);

    // Load shaders
    video::ShaderClass* ShdClass = spRenderer->createShaderClass();

    spRenderer->loadShader(ShdClass, video::SHADER_VERTEX, video::GLSL_VERSION_1_20, "TBOShader.glvert");
    spRenderer->loadShader(ShdClass, video::SHADER_PIXEL, video::GLSL_VERSION_1_20, "TBOShader.glfrag");

    if (ShdClass->link())
        Obj->setShaderClass(ShdClass);

    // Draw scene
    SP_TESTS_MAIN_BEGIN
    {
        tool::Toolset::presentModel(Obj);
        
        spScene->renderScene();
    }
    SP_TESTS_MAIN_END
}
