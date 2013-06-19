//
// SoftPixel Engine - Texture Buffer Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

void ShaderCallbackD3D11(video::ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    /* Setup vertex shader constant buffer */
    dim::matrix4f WVPMatrix(spRenderer->getProjectionMatrix());
    WVPMatrix *= spRenderer->getViewMatrix();
    WVPMatrix *= spRenderer->getWorldMatrix();
    
    ShdClass->getVertexShader()->setConstantBuffer(0, &WVPMatrix);
}

int main()
{
    SP_TESTS_INIT_EX(
        video::RENDERER_DIRECT3D11, dim::size2di(800, 600), "TextureBuffer", false
    )
    
    // Create textures
    video::Texture* BufTex = 0;
    video::Texture* RectTex = 0;
    
    #if 1
    
    BufTex = spRenderer->createTexture(dim::size2di(100, 1), video::PIXELFORMAT_RGBA);
    video::ImageBuffer* ImgBuf = BufTex->getImageBuffer();
    {
        for (s32 i = 0; i < 100;)
        {
            ImgBuf->setPixelColor(i++, video::color(255, 0, 0));
            ImgBuf->setPixelColor(i++, video::color(0, 255, 0));
            ImgBuf->setPixelColor(i++, video::color(255, 255, 0));
            ImgBuf->setPixelColor(i++, video::color(0, 0, 255));
        }
    }
    BufTex->setDimension(video::TEXTURE_BUFFER);
    
    #endif

    if (spRenderer->getRendererType() == video::RENDERER_OPENGL)
    {
        RectTex = spRenderer->loadTexture("../Media/SkyboxNorth.jpg");
        RectTex->setDimension(video::TEXTURE_RECTANGLE);
    }
    
    // Create scene
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_CUBE);
    
    Cam->setPosition(dim::vector3df(0, 0, -2));
    
    if (RectTex)
        Obj->addTexture(RectTex);
    
    if (BufTex)
        Obj->addTexture(BufTex);
    
    #if 1

    video::STextureCreationFlags CreationFlags;
    {
        CreationFlags.Size          = 1;
        CreationFlags.Dimension     = video::TEXTURE_BUFFER;
        CreationFlags.BufferType    = video::IMAGEBUFFER_FLOAT;
        CreationFlags.HWFormat      = video::HWTEXFORMAT_FLOAT32;
        CreationFlags.Format        = video::PIXELFORMAT_GRAY;
    }
    video::Texture* BufTex2 = spRenderer->createTexture(CreationFlags);
    BufTex2->getImageBuffer()->setPixelColor(0, video::color(128));
    BufTex2->updateImageBuffer();
    //BufTex2->setDimension(video::TEXTURE_BUFFER);
    Obj->addTexture(BufTex2);

    #endif

    // Load shaders
    video::ShaderClass* ShdClass = spRenderer->createShaderClass(scene::SceneManager::getDefaultVertexFormat());
    
    video::Shader* ShdVert = 0;
    video::Shader* ShdFrag = 0;
    
    if (spRenderer->getRendererType() == video::RENDERER_OPENGL)
    {
        ShdVert = spRenderer->loadShader(ShdClass, video::SHADER_VERTEX, video::GLSL_VERSION_1_20, "TBOShader.glvert");
        ShdFrag = spRenderer->loadShader(ShdClass, video::SHADER_PIXEL, video::GLSL_VERSION_1_20, "TBOShader.glfrag");
    }
    else if (spRenderer->getRendererType() == video::RENDERER_DIRECT3D11)
    {
        ShdVert = spRenderer->loadShader(ShdClass, video::SHADER_VERTEX, video::HLSL_VERTEX_5_0, "TBOShader.hlsl");
        ShdFrag = spRenderer->loadShader(ShdClass, video::SHADER_PIXEL, video::HLSL_PIXEL_5_0, "TBOShader.hlsl");
    }
    
    if (ShdClass->link())
    {
        Obj->setShaderClass(ShdClass);
        
        if (spRenderer->getRendererType() == video::RENDERER_OPENGL)
        {
            ShdFrag->setConstant("RectTex", 0);
            ShdFrag->setConstant("BufTex", 1);
        }
        else if (spRenderer->getRendererType() == video::RENDERER_DIRECT3D11)
            ShdClass->setObjectCallback(ShaderCallbackD3D11);
    }
    
    // Draw scene
    SP_TESTS_MAIN_BEGIN
    {
        tool::Toolset::presentModel(Obj);
        
        spScene->renderScene();
    }
    SP_TESTS_MAIN_END
}
