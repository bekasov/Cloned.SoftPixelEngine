//
// SoftPixel Engine - GLSL Compute Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

void DrawObjCallback(video::ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    ShdClass->getVertexShader()->setConstant(
        "WVPMatrix",
        spRenderer->getProjectionMatrix() * spRenderer->getViewMatrix() * spRenderer->getWorldMatrix()
    );
}

int main()
{
    SP_TESTS_INIT("GLSL Compute")
    
    if (!spRenderer->queryVideoSupport(video::QUERY_SHADER))
        return Fatal("Shaders are not supported");
    
    // Create compute shader
    video::ShaderClass* CompShdClass = spRenderer->createShaderClass();
    
    video::Shader* CompShd = spRenderer->loadShader(
        CompShdClass, video::SHADER_COMPUTE, video::GLSL_VERSION_4_30, "GLComputeShader.glsl"
    );
    
    if (!CompShdClass->link())
        return Fatal("Loading compute shader failed");

    // Initialize shader
    CompShd->setConstant("DestTex", 0);
    
    // Create R/W texture
    video::STextureCreationFlags CreationFlags;
    {
        CreationFlags.Type      = video::TEXTURE_2D_RW;
        CreationFlags.Size      = 128;
        CreationFlags.Format    = video::PIXELFORMAT_RGBA;
        CreationFlags.HWFormat  = video::HWTEXFORMAT_FLOAT32;
    }
    video::Texture* Tex = spRenderer->createTexture(CreationFlags);
    
    CompShdClass->addRWTexture(Tex);
    
    // Create shader resource
    video::ShaderResource* ShdRes = spRenderer->createShaderResource();
    
    ShdRes->setupBuffer<dim::float4>(32);
    
    // Run compute shader
    spRenderer->dispatch(CompShdClass, dim::vector3d<u32>(16, 16, 1));

    Tex->generateMipMap();

    // Create vertex format
    video::VertexFormatUniversal* VertexFmt = spRenderer->createVertexFormat<video::VertexFormatUniversal>();
    VertexFmt->addUniversal(video::DATATYPE_FLOAT, 3, "VertexPos", false, video::VERTEXFORMAT_COORD);
    VertexFmt->addUniversal(video::DATATYPE_FLOAT, 2, "VertexTexCoord", false, video::VERTEXFORMAT_TEXCOORDS);

    // Load draw shader
    video::ShaderClass* DrawShdClass = spRenderer->createShaderClass(VertexFmt);

    spRenderer->loadShader(DrawShdClass, video::SHADER_VERTEX, video::GLSL_VERSION_4_30, "Draw.glvert");
    spRenderer->loadShader(DrawShdClass, video::SHADER_PIXEL, video::GLSL_VERSION_4_30, "Draw.glfrag");

    if (!DrawShdClass->link())
        return Fatal("Loading draw shader failed");

    DrawShdClass->setObjectCallback(DrawObjCallback);

    // Create small scene
    scene::SceneManager::setDefaultVertexFormat(VertexFmt);

    scene::Mesh* Obj = spScene->createMesh(scene::MESH_CUBE);

    Obj->addTexture(Tex);
    Obj->setShaderClass(DrawShdClass);

    Cam->setPosition(dim::vector3df(0, 0, -3));
    
    SP_TESTS_MAIN_BEGIN
    {
        tool::Toolset::presentModel(Obj);

        spScene->renderScene(Cam);
    }
    SP_TESTS_MAIN_END
}
