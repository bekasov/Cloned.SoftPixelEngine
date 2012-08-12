//
// SoftPixel Engine Tutorial: Tessellation - (15/09/2010)
//

#include <SoftPixelEngine.hpp>

#include "../../common.hpp"

#ifndef SP_PLATFORM_WINDOWS
#   error Tessellation using Direct3D11 is only supported on a Windows(c) platform yet!
#endif

using namespace sp;
using namespace std;

/* === Global members === */

SoftPixelDevice* spDevice       = 0;
io::InputControl* spControl     = 0;
video::RenderSystem* spRenderer = 0;
video::RenderContext* spContext = 0;
scene::SceneGraph* spScene      = 0;

const s32 ScrWidth = 800, ScrHeight = 600;

scene::Camera* Cam  = 0;
scene::Light* Light = 0;
scene::Mesh* Room   = 0;

video::Texture* ColorMap[3] = { 0 };
video::Texture* BumpMap[3]  = { 0 };

video::ShaderClass* TessShdClass = 0;

const io::stringc ResPath = "media/";


// Declarations
bool InitDevice();
bool CreateScene();
void UpdateScene();
bool LoadTessellationShader(const io::stringc &Filename, video::VertexFormat* Format);


/* === All function definitions === */

int main()
{
    if (!InitDevice())
        return 0;
    
    if (!CreateScene())
    {
        deleteDevice();
        return 0;
    }
    
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        UpdateScene();
        
        spScene->renderScene();
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}

/**
 * In this case we only allow Direct3D11 video driver because tessellation is currently only supported for this renderer.
 */
bool InitDevice()
{
    io::Log::open("spDebugLog.txt");
    
    spDevice    = createGraphicsDevice(
        video::RENDERER_DIRECT3D11, dim::size2di(ScrWidth, ScrHeight), 32, "SoftPixel Engine - Tessellation tutorial"
    );
    
    spControl   = spDevice->getInputControl();
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    
    spScene     = spDevice->createSceneGraph();
    
    // If Direct3D11 is not supported exit the program.
    if (spRenderer->getRendererType() != video::RENDERER_DIRECT3D11 || spRenderer->getVersion() != "Direct3D 11.0")
    {
        io::Log::error("Valid rendering device is not supported");
        io::Log::pauseConsole();
        return false;
    }
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    spDevice->setFrameRate(100);
    
    spRenderer->setClearColor(255);
    
    math::Randomizer::seedRandom();
    
    return true;
}

/**
 * Create the whole scene. Load the test chamber and the tessellation shader.
 */
bool CreateScene()
{
    // Create the vertex format
    video::VertexFormatUniversal* VertFormat = spRenderer->createVertexFormat<video::VertexFormatUniversal>();
    
    VertFormat->addCoord();
    VertFormat->addTangent();
    VertFormat->addBinormal();
    VertFormat->addNormal();
    VertFormat->addTexCoord(video::DATATYPE_FLOAT, 3);
    
    scene::SceneGraph::setDefaultVertexFormat(VertFormat);
    
    // Create a camera to see the scene.
    Cam = spScene->createCamera();
    Cam->setRange(0.1f, 500.0f);
    Cam->setPosition(dim::vector3df(0, 3, 0));
    
    // Create a point light.
    Light = spScene->createLight(scene::LIGHT_POINT);
    Light->setPosition(dim::vector3df(0, 4, 0));
    
    // Set texture filter to anisotropic 16x.
    spRenderer->setTextureGenFlags(video::TEXGEN_MIPMAPFILTER, video::FILTER_ANISOTROPIC);
    spRenderer->setTextureGenFlags(video::TEXGEN_ANISOTROPY, 16);
    
    // Load color map and bump maps. We call it bump maps because they consist
    // of a normal- and height map where the height map data is stored in the alpha channel.
    const io::stringc TexNames[3] = { "Wall", "Rocks", "Stones" };
    
    for (s32 i = 0; i < 3; ++i)
    {
        ColorMap[i] = spRenderer->loadTexture(ResPath + TexNames[i] + "ColorMap.jpg");
        BumpMap[i]  = spRenderer->loadTexture(ResPath + TexNames[i] + "BumpMap.dds");
    }
    
    // Load the tessellation shader.
    if (!LoadTessellationShader(ResPath + "TessellationShader.hlsl", VertFormat))
    {
        io::Log::error("Shader compilation failed");
        io::Log::pauseConsole();
        return false;
    }
    
	// Load the test chamber.
    Room = spScene->loadMesh(ResPath + "Room.spm");
    
    if (Room->getMeshBufferCount() != 3)
    {
        io::Log::error("Tessellation tutorial expects a room mesh with three surfaces");
        return false;
    }
    
    // Add the color- and bump map for each surface.
    for (u32 i = 0; i < 3; ++i)
    {
        Room->getMeshBuffer(i)->addTexture(ColorMap[i]);
        Room->getMeshBuffer(i)->addTexture(BumpMap[i]);
    }
    
    // Set the shader table.
    Room->setShaderClass(TessShdClass);
    
    // Update the tangent space for correct bump mapping.
    Room->updateTangentSpace();
    
    return true;
}

/**
 * Update camera movement and user input. In this tutorial the focus is on the tessellation and
 * not on interactivity. Thus you only can move around the scene and switch between wireframe mode.
 */
void UpdateScene()
{
    // Switch between the wireframe modes.
    if (spControl->keyHit(io::KEY_TAB))
    {
        static bool Frames;
        Frames = !Frames;
        spScene->setWireframe(Frames ? video::WIREFRAME_LINES : video::WIREFRAME_SOLID);
    }
    
    // Move the camera free in the world.
    if (spContext->isWindowActive())
        tool::Toolset::moveCameraFree(Cam, 0.15f);
}

/**
 * Tessellation shader constant buffer structures.
 * Since Shader Model 4.0 there are no longer uniform constants. There are only constant buffer
 * with a set of data. You can use multiple constant buffers.
 */
struct SConstantBufferObject
{
    dim::matrix4f WorldMatrix;                      //!< World matrix (Object space).
    dim::matrix4f ViewProjectionMatrix;             //!< View projection matrix for deferred projection.
    
    // All structure members have to have a
    // size of 4 * 4 bytes (= 4 floats).
    // This is requiered since Shader Model 4.0
    // and if members are smaller additional
    // members need to exist to fill this gap.
    dim::vector4df CameraPosition;                  //!< Camera position (Object space).
    
    dim::vector4df LightPosition;                   //!< Light position (Object space).
    dim::vector4df LightDiffuse;                    //!< Light diffuse color.
    dim::vector4df LightSpecular;                   //!< Light specular color.
    
    // Only 4 bytes (= 1 float). Thus we have
    // to fill the other 3 * 4 bytes with a
    // dummy member (Dummy[3]).
    f32 Shininess;                                  //!< Material shininess factor.
    f32 Dummy[3];                                   //!< Dummy container to fill the 4 * 4 bytes block.
};

struct SConstantBufferSurface
{
    f32 HeightFactor;                               //!< Height field factor.
    f32 Dummy[3];                                   //!< Dummy container.
};

/**
 * Object shader callback. This is called for each object before it will be rendered.
 */
void ShaderCallbackObject(video::ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    SConstantBufferObject TessBuffer;
    
    TessBuffer.WorldMatrix          = spRenderer->getWorldMatrix();
    TessBuffer.ViewProjectionMatrix = spRenderer->getProjectionMatrix() * spRenderer->getViewMatrix();
    
    TessBuffer.CameraPosition       = spScene->getActiveCamera()->getPosition(true);
    
    TessBuffer.LightPosition        = dim::vector4df(Light->getPosition(true));
    
    // Set the light's diffuse and specular color. Here we have to set the colors using 4 components vectors.
    TessBuffer.LightDiffuse         = dim::vector4df(1.0f);
    TessBuffer.LightSpecular        = dim::vector4df(0.4f);
    
    TessBuffer.Shininess            = 90.0f;
    
    // Set the constant buffer. Because we don't have so much buffers we don't have to care about the index number
    // we can just set the index to 0. This function is overloaded and we can also specify the name
    // of the constant buffer instead of the index. But this would be a little but slower.
    // The name of our constant buffer in the tessellation shader is "MainBuffer".
    ShdClass->getVertexShader()->setConstantBuffer(0, &TessBuffer);
    ShdClass->getHullShader()->setConstantBuffer(0, &TessBuffer);
    ShdClass->getDomainShader()->setConstantBuffer(0, &TessBuffer);
    ShdClass->getPixelShader()->setConstantBuffer(0, &TessBuffer);
}

/**
 * Surface shader callback. This is called for each surface before it will be rendered.
 * Here we set configurations which depends on the texture lists. In this case we only set
 * the height field factor. The rocks shall be higher then the stones.
 */
void ShaderCallbackSurface(video::ShaderClass* ShdClass, const std::vector<video::SMeshSurfaceTexture>* TextureList)
{
    if (!TextureList || TextureList->empty())
        return;
    
    SConstantBufferSurface TessBuffer;
    
    video::Texture* Tex = (*TextureList)[0].TextureObject;
    
    // Set the individual height factors in dependency to the texture.
    if (Tex == ColorMap[0])
        TessBuffer.HeightFactor = 0.1f;
    else if (Tex == ColorMap[1])
        TessBuffer.HeightFactor = 0.125f;
    else if (Tex == ColorMap[2])
        TessBuffer.HeightFactor = 0.05f;
    
    // Set the constant number two (index = 1).
    ShdClass->getVertexShader()->setConstantBuffer(1, &TessBuffer);
    ShdClass->getHullShader()->setConstantBuffer(1, &TessBuffer);
    ShdClass->getDomainShader()->setConstantBuffer(1, &TessBuffer);
    ShdClass->getPixelShader()->setConstantBuffer(1, &TessBuffer);
}

/**
 * Load the tessellation shader and set the object and surface callbacks.
 */
bool LoadTessellationShader(const io::stringc &Filename, video::VertexFormat* Format)
{
    // Create the shader table to hold all needed shader programs.
    TessShdClass = spRenderer->createShaderClass(Format);
    
    // Load the shader programs (for more information about the tessellation shader
    // look inside the "media/TessellationShader.hlsl" file).
    video::Shader* VertexShader = spRenderer->loadShader(
        TessShdClass, video::SHADER_VERTEX, video::HLSL_VERTEX_5_0, Filename, "VertexMain"
    );
    video::Shader* HullShader = spRenderer->loadShader(
        TessShdClass, video::SHADER_HULL, video::HLSL_HULL_5_0, Filename, "HullMain"
    );
    video::Shader* DomainShader = spRenderer->loadShader(
        TessShdClass, video::SHADER_DOMAIN, video::HLSL_DOMAIN_5_0, Filename, "DomainMain"
    );
    video::Shader* PixelShader = spRenderer->loadShader(
        TessShdClass, video::SHADER_PIXEL, video::HLSL_PIXEL_5_0, Filename, "PixelMain"
    );
    
    // Link the shader and check for errors.
    if (!TessShdClass->link())
        return false;
    
    // Set the shader callback where the constant buffer will be filled.
    TessShdClass->setObjectCallback(ShaderCallbackObject);
    TessShdClass->setSurfaceCallback(ShaderCallbackSurface);
    
    return true;
}



// ============================================
