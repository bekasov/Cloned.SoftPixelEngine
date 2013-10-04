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
    BufTex->setType(video::TEXTURE_BUFFER);
    
    #endif

    if (spRenderer->getRendererType() == video::RENDERER_OPENGL)
    {
        RectTex = spRenderer->loadTexture("../Media/SkyboxNorth.jpg");
        RectTex->setType(video::TEXTURE_RECTANGLE);
    }
    
    // Create scene
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_CUBE);
    
    Cam->setPosition(dim::vector3df(0, 0, -2));
    
    #if 0
    
    if (RectTex)
        Obj->addTexture(RectTex);
    
    if (BufTex)
        Obj->addTexture(BufTex);
    
    #endif

    #if 0

    video::STextureCreationFlags CreationFlags;
    {
        CreationFlags.Size          = 1;
        CreationFlags.Type          = video::TEXTURE_BUFFER;
        CreationFlags.BufferType    = video::IMAGEBUFFER_FLOAT;
        CreationFlags.HWFormat      = video::HWTEXFORMAT_FLOAT32;
        CreationFlags.Format        = video::PIXELFORMAT_GRAY;
    }
    video::Texture* BufTex2 = spRenderer->createTexture(CreationFlags);
    BufTex2->getImageBuffer()->setPixelColor(0, video::color(128));
    BufTex2->updateImageBuffer();
    //BufTex2->setType(video::TEXTURE_BUFFER);
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
    
    if (ShdClass->compile())
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

    #if 1

    if (spRenderer->getRendererType() == video::RENDERER_DIRECT3D11)
    {
        // Load compute shader
        video::ShaderClass* ShdClassCS = spRenderer->createShaderClass();

        video::Shader* ShdCS = spRenderer->loadShader(
            ShdClassCS, video::SHADER_COMPUTE, video::HLSL_COMPUTE_5_0, "TestComputeShader.hlsl", "ComputeMain"
        );

        ShdClassCS->compile();

        // Create shader resource
        video::ShaderResource* ShdResOut = spRenderer->createShaderResource();
        video::ShaderResource* ShdResIn = spRenderer->createShaderResource();

        const u32 Count = 4*4;

        ShdResOut->setupBufferRW<dim::vector4df>(Count);
        ShdResIn->setupBuffer<dim::vector4df>(Count);

        ShdClassCS->addShaderResource(ShdResOut);
        ShdClass->addShaderResource(ShdResIn);

        // Run compute shader
        spRenderer->dispatch(ShdClassCS, dim::vector3d<u32>(4, 4, 1));

        ShdResIn->copyBuffer(ShdResOut);
    }

    #endif
    
    // Draw scene
    SP_TESTS_MAIN_BEGIN
    {
        tool::Toolset::presentModel(Obj);
        
        spScene->renderScene();
    }
    SP_TESTS_MAIN_END
}
