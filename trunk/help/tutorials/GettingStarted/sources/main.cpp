//
// SoftPixel Engine - Getting started
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../../common.hpp"

#include <boost/foreach.hpp>

#define USE_CONSOLE
#ifdef USE_CONSOLE
scene::Camera* Cam = 0;
tool::CommandLineUI* cmd = 0;
#endif

void ResizeCallback(video::RenderContext* Context)
{
    const dim::rect2di Rect(0, 0, Context->getResolution().Width, Context->getResolution().Height);
    cmd->setRect(Rect);
    Cam->setViewport(Rect);
}

void DropFileCallback(video::RenderContext* Context, const io::stringc &Filename, u32 Index, u32 NumFiles)
{
    io::Log::message("Droped Filed [ " + io::stringc(Index) + " ]: \"" + Filename + "\"");
}

int main(void)
{
    SDeviceFlags DevFlags;
    //DevFlags.RendererProfile.UseExtProfile = true;
    //DevFlags.RendererProfile.UseGLCoreProfile = true;
    //DevFlags.RendererProfile.D3DFeatureLevel = DIRECT3D_FEATURE_LEVEL_10_0;
    DevFlags.Window.Resizable = true;
    DevFlags.Window.DropFileAccept = true;

    // Create the graphics device to open the screen (in this case windowed screen).
    SoftPixelDevice* spDevice = createGraphicsDevice(
        //ChooseRenderer(),
        #if 1
        video::RENDERER_DIRECT3D11,
        #elif 0
        video::RENDERER_DIRECT3D9,
        #else
        video::RENDERER_OPENGL,
        #endif
        dim::size2di(800, 600), 32, "Getting Started", false, DevFlags
    );
    
    // Check for errors while creating the graphics device
    if (!spDevice)
    {
        io::Log::pauseConsole();
        return 0;
    }
    
    video::RenderSystem* spRenderer = spDevice->getRenderSystem();                  // Render system for drawing, rendering and general graphics hardware control.
    video::RenderContext* spContext = spDevice->getRenderContext();                 // Render context is basically only used to flip the video buffers.
    io::InputControl* spControl     = spDevice->getInputControl();                  // Input control to check for user inputs: keyboard, mouse etc.
    
    scene::SceneGraph* spScene      = spDevice->createSceneGraph();                 // Scene graph for creating cameras, lights, meshes and handling the whole scene.
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"       // Change the window title to display the type of renderer
    );
    
    #if 1
    spContext->setResizeCallback(ResizeCallback);
    spContext->setDropFileCallback(DropFileCallback);
    #endif
    
    /*scene::Camera* */Cam  = spScene->createCamera();                                  // Create a camera to make our scene visible.
    scene::Light* Lit   = spScene->createLight();                                   // Create a light (by default directional light) to shade the scene.
    spScene->setLighting(true);                                                     // Activate global lighting
    
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_TEAPOT);                     // Create one of the standard meshes
    Obj->setPosition(dim::vector3df(0, 0, 3));                                      // Sets the object's position (x, y, z)
    
    video::Texture* Tex = spRenderer->loadTexture(
        //ROOT_PATH + "GettingStarted/media/SphereMap.jpg"                            // Load a texture. With a texture 2D images can be mapped onto 3D objects.
        "media/SphereMap.jpg"
    );
    
    Obj->addTexture(Tex);                                                           // Map the texture onto the mesh.
    Obj->getMeshBuffer(0)->setMappingGen(0, video::MAPGEN_SPHERE_MAP);              // Set texture coordinate generation (mapping gen) to sphere mapping.
    
    //!!!
    //#define SVO_TEST
    #define POSTPROCESS_TEST

    #if defined(SVO_TEST) || defined(POSTPROCESS_TEST)
    {
    #endif

    #ifdef SVO_TEST
    video::SparseOctreeVoxelizer Voxelizer;
    Voxelizer.createResources(64);

    Cam->setPosition(Obj->getPosition());
    Obj->setPosition(0.0f);

    dim::aabbox3df BBox(Obj->getMeshBoundingBox());
    BBox.Min *= 1.5f;
    BBox.Max *= 1.5f;

    #endif
    
    #if 0//!!!
    
    video::VertexFormatUniversal* fm = spRenderer->createVertexFormat<video::VertexFormatUniversal>();
    fm->addCoord();
    
    video::ShaderClass* sc = spRenderer->createShaderClass(fm);
    
    const io::stringc path = "D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelEngine/trunk/sources/Framework/Tools/LightmapGenerator/spLightmapGenerationShader.hlsl";
    spRenderer->loadShader(sc, video::SHADER_COMPUTE, video::HLSL_VERTEX_5_0, path, "ComputeMain", video::SHADERFLAG_ALLOW_INCLUDES);
    
    sc->compile();
    
    #endif

    #define USE_CONSOLE
    #ifdef USE_CONSOLE
    cmd = new tool::CommandLineUI();
    
    cmd->setBackgroundColor(video::color(0, 0, 0, 128));
    cmd->setRect(dim::rect2di(0, 0, spContext->getResolution().Width, spContext->getResolution().Height));

    bool isCmdActive = false;
    spControl->setWordInput(isCmdActive);
    #endif
    
    //#define QUERY_TEST
    #ifdef QUERY_TEST
    video::Font* Fnt = spRenderer->createFont();
    video::Query* MyQuery = spRenderer->createQuery(video::QUERY_SAMPLES_PASSED);
    #endif

    #ifdef POSTPROCESS_TEST
    video::RadialBlur postProcess;
    postProcess.createResources();

    video::STextureCreationFlags cf;
    {
        cf.Size = spContext->getResolution();
        cf.Filter.HasMIPMaps = false;
        cf.Filter.Mag = video::FILTER_LINEAR;
        cf.Filter.Min = video::FILTER_LINEAR;
        cf.Filter.WrapMode = video::TEXWRAP_CLAMP;
    }
    video::Texture* rt = spRenderer->createTexture(cf);
    rt->setRenderTarget(true);
    #endif
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))         // The main loop will update our device
    {
        spRenderer->clearBuffers();                                                 // Clear the color- and depth buffer.
        
        #ifdef USE_CONSOLE
        if (!isCmdActive)
        #endif
        {
            tool::Toolset::presentModel(Obj);                                           // Present the model so that the user can turn the model by clicking and moving the mouse.
        }
        
        #ifdef SVO_TEST
        Voxelizer.generateSparseOctree(spScene, BBox);
        #endif

        #ifdef QUERY_TEST
        if (MyQuery)
            MyQuery->begin();
        #endif
        
        #ifdef POSTPROCESS_TEST
        spRenderer->setRenderTarget(rt);
        spRenderer->clearBuffers();
        #endif

        spScene->renderScene();                                                     // Render the whole scene. In our example only one object (the teapot).
        
        #ifdef POSTPROCESS_TEST
        postProcess.drawEffect(rt);
        spRenderer->setRenderTarget(0);
        #endif

        #ifdef USE_CONSOLE
        if (spControl->keyHit(io::KEY_F3))
        {
            isCmdActive = !isCmdActive;
            spControl->setWordInput(isCmdActive);
        }
        if (isCmdActive)
            cmd->render();
        #endif
        
        #ifdef QUERY_TEST
        if (MyQuery)
        {
            MyQuery->end();
            spRenderer->draw2DText(Fnt, 15, "Samples Passed: " + io::stringc(MyQuery->result()));
        }
        #endif
        
        spContext->flipBuffers();                                                   // Swap the video buffer to make the current frame visible.
    }
    
    #if defined(SVO_TEST) || defined(POSTPROCESS_TEST)
    }
    #endif

    #ifdef USE_CONSOLE
    delete cmd;
    #endif
    
    deleteDevice();                                                                 // Delete the device context. This will delete and release all objects allocated by the engine.
    
    return 0;
}
