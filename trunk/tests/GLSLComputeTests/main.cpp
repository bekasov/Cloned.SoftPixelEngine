//
// SoftPixel Engine - GLSL Compute Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

int main()
{
    SP_TESTS_INIT("GLSL Compute")
    
    // Create compute shader
    video::ShaderClass* ShdClass = spRenderer->createShaderClass();
    
    spRenderer->loadShader(ShdClass, video::SHADER_COMPUTE, video::GLSL_VERSION_4_30, "GLComputeShader.glsl");
    
    if (!ShdClass->link())
    {
        io::Log::error("Loading GLSL compute shader failed");
        io::Log::pauseConsole();
        deleteDevice();
        return 0;
    }
    
    // Create R/W texture
    video::STextureCreationFlags CreationFlags;
    {
        CreationFlags.Type      = video::TEXTURE_2D_RW;
        CreationFlags.Size      = 128;
        CreationFlags.Format    = video::PIXELFORMAT_RGBA;
        CreationFlags.HWFormat  = video::HWTEXFORMAT_FLOAT32;
    }
    video::Texture* Tex = spRenderer->createTexture(CreationFlags);
    
    // Run compute shader
    Tex->bind(0);
    {
        spRenderer->runComputeShader(ShdClass, dim::vector3di(16, 16, 1));
    }
    Tex->unbind(0);
    
    SP_TESTS_MAIN_BEGIN
    {
        spScene->renderScene(Cam);
        
        spRenderer->draw2DImage(Tex, 0);
    }
    SP_TESTS_MAIN_END
}
