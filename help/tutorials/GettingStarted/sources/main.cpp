//
// SoftPixel Engine - Getting started
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../../common.hpp"

video::RenderSystem* spRenderer = 0;

//#define RT_TEST
#ifdef RT_TEST

void ShdCallback(video::ShaderClass* ShdClass, const scene::MaterialNode* Node)
{
    ShdClass->getVertexShader()->setConstant(
        "WorldViewProjectionMatrix",
        spRenderer->getProjectionMatrix() * spRenderer->getViewMatrix() * spRenderer->getWorldMatrix()
    );
    ShdClass->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
}

#endif

int main()
{
    SoftPixelDevice* spDevice = createGraphicsDevice(
        /*ChooseRenderer()*/video::RENDERER_OPENGL, dim::size2di(640, 480), 32, "Getting Started"             // Create the graphics device to open the screen (in this case windowed screen).
    );
    
    /*video::RenderSystem* */spRenderer = spDevice->getRenderSystem();                  // Render system for drawing, rendering and general graphics hardware control.
    video::RenderContext* spContext = spDevice->getRenderContext();                 // Render context is basically only used to flip the video buffers.
    io::InputControl* spControl     = spDevice->getInputControl();                  // Input control to check for user inputs: keyboard, mouse etc.
    
    scene::SceneGraph* spScene      = spDevice->createSceneGraph();                 // Scene graph for creating cameras, lights, meshes and handling the whole scene.
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"       // Change the window title to display the type of renderer
    );
    
    #if 0
    tool::XMLParser Parser;
    Parser.loadFile("D:/SoftwareEntwicklung/C++/HLC/Spiele/KettenSaegenKurt/scripts/LanguageEN.xml");
    #endif
    
    //#define MULTI_CONTEXT
    #ifdef MULTI_CONTEXT
    video::RenderContext* SecondContext = spDevice->createRenderContext(0, dim::size2di(640, 480));
    SecondContext->setWindowPosition(0);
    spContext->activate();
    #endif
    
    scene::Camera* Cam  = spScene->createCamera();                                  // Create a camera to make our scene visible.
    scene::Light* Lit   = spScene->createLight();                                   // Create a light (by default directional light) to shade the scene.
    spScene->setLighting(true);                                                     // Activate global lighting
    
    //Cam->setOrtho(true);
    
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_TEAPOT);                     // Create one of the standard meshes
    Obj->setPosition(dim::vector3df(0, 0, 3));                                      // Sets the object's position (x, y, z)
    
    #ifdef SP_COMPILE_WITH_XBOX360GAMEPAD
    io::XBox360GamePadPtr GamePad = spControl->getXBox360GamePad(0);
    #endif
    
    #ifdef RT_TEST
    
    video::Texture* RtTex = spRenderer->createTexture(256, video::PIXELFORMAT_RGB);
    RtTex->setRenderTarget(true);
    RtTex->setMultiSamples(8);
    
    std::vector<io::stringc> ShdCode(1);
    
    video::ShaderClass* ShdClass = spRenderer->createShaderClass();
    
    ShdCode[0] =
        "#version 120\n"
        "uniform mat4 WorldViewProjectionMatrix;\n"
        "uniform mat4 WorldMatrix;\n"
        "varying vec3 Normal;"
        "void main() {\n"
        "    gl_Position = WorldViewProjectionMatrix * gl_Vertex;\n"
        "    Normal = mat3(WorldMatrix) * gl_Normal;"
        "}\n";
    
    spRenderer->createShader(ShdClass, video::SHADER_VERTEX, video::GLSL_VERSION_1_20, ShdCode);
    
    ShdCode[0] =
        "#version 120\n"
        "varying vec3 Normal;"
        "void main() {\n"
        "    float NdotL = max(0.1, -dot(normalize(Normal), vec3(0.0, 0.0, 1.0)));\n"
        "    gl_FragColor = vec4(0.0, NdotL, 0.0, 1.0);\n"
        "}\n";
    
    spRenderer->createShader(ShdClass, video::SHADER_PIXEL, video::GLSL_VERSION_1_20, ShdCode);
    
    if (ShdClass->link())
    {
        ShdClass->setObjectCallback(ShdCallback);
        Obj->setShaderClass(ShdClass);
    }
    
    #endif
    
    video::Texture* Tex = spRenderer->loadTexture("media/SphereMap.jpg");           // Load a texture. With a texture 2D images can be mapped onto 3D objects.
    
    #if 0
    video::STextureCreationFlags CreationFlags;
    {
        CreationFlags.BufferType    = video::IMAGEBUFFER_FLOAT;
        CreationFlags.HWFormat      = video::HWTEXFORMAT_FLOAT32;
    }
    video::Texture* Tex2 = spRenderer->createTexture(CreationFlags);
    
    Tex2->getImageBuffer()->copy(Tex->getImageBuffer());
    Tex2->updateImageBuffer();
    #endif
    
    Obj->addTexture(Tex);                                                           // Map the texture onto the mesh.
    Obj->getMeshBuffer(0)->setMappingGen(0, video::MAPGEN_SPHERE_MAP);              // Set texture coordinate generation (mapping gen) to sphere mapping.
    
    #if 0
    
    // scene::Mesh::mergeMeshBuffers test
    Obj->meshTranslate(dim::vector3df(1, 0, 0));
    
    video::MeshBuffer* Surf = Obj->createMeshBuffer();
    scene::MeshGenerator::createCube(*Surf, 0.5f, 1);
    Surf->updateMeshBuffer();
    
    Surf->addTexture(Tex);
    Surf->setMappingGen(0, video::MAPGEN_SPHERE_MAP);
    
    Obj->mergeMeshBuffers();
    io::Log::message("Surfaces: " + io::stringc(Obj->getMeshBufferCount()));
    
    #endif
    
    //#define FONT_TEST
    #ifdef FONT_TEST
    
    const io::stringc Path = "D:/Anwendungen/Dev-Cpp/irrlicht-1.7/tools/IrrFontTool/newFontTool/";
    
    video::Texture* FontTex = spRenderer->loadTexture(Path + "TestFont.png");
    video::Font* FontObj = spRenderer->createFont(FontTex, Path + "TestFont.xml");
    
    spRenderer->setClearColor(255);
    
    #endif
    
    video::Font* FontObj = spRenderer->createFont();
    
    bool isCmdActive = false;
    
    #define CMD_TEST
    #ifdef CMD_TEST
    tool::CommandLineUI* Cmd = new tool::CommandLineUI();
    spControl->setWordInput(isCmdActive);
    #endif
    
    f32 Pitch = 0.0f, Yaw = 0.0f;

    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))         // The main loop will update our device
    {
        #ifdef MULTI_CONTEXT
        spContext->activate();
        #endif
        
        spRenderer->clearBuffers();                                                 // Clear the color- and depth buffer.
        
        tool::Toolset::presentModel(Obj, !isCmdActive);                             // Present the model so that the user can turn the model by clicking and moving the mouse.
        
        spScene->renderScene();                                                     // Render the whole scene. In our example only one object (the teapot).
        
        if (spControl->keyHit(io::KEY_F1))
            spContext->setFullscreen(!spContext->getFullscreen());
        
        #ifdef SP_COMPILE_WITH_XBOX360GAMEPAD
        
        if (GamePad->buttonHit(io::GAMEPAD_BUTTON_A))
        {
            GamePad->setLeftVibration(io::MAX_GAMEPAD_VIRBATION, 250);
            GamePad->setRightVibration(io::MAX_GAMEPAD_VIRBATION, 250);
        }
        
        //GamePad->setLeftVibration(static_cast<s32>(GamePad->getLeftTrigger()) * io::MAX_GAMEPAD_VIRBATION / 256);
        //GamePad->setRightVibration(static_cast<s32>(GamePad->getRightTrigger()) * io::MAX_GAMEPAD_VIRBATION / 256);
        
        const dim::point2di JoystickLN(GamePad->getLeftJoystickNative());
        const dim::point2df JoystickL(GamePad->getLeftJoystick());
        
        if (math::Abs(JoystickLN.X) > 4000 || math::Abs(JoystickLN.Y) > 4000)
        {
            dim::matrix4f Rot;
            
            Rot.rotateY(-JoystickL.X * 2.5f);
            Rot.rotateX(JoystickL.Y * 2.5f);
            
            Obj->setRotationMatrix(Rot * Obj->getRotationMatrix());
        }
        
        #endif
        
        #ifdef RT_TEST
        spRenderer->setRenderTarget(RtTex);
        {
            spRenderer->setClearColor(255);
            spRenderer->clearBuffers();
            spRenderer->setClearColor(0);
            
            Cam->setViewport(dim::rect2di(0, 0, RtTex->getSize().Width, RtTex->getSize().Height));
            spScene->renderScene();
            Cam->setViewport(dim::rect2di(0, 0, 640, 480));
        }
        spRenderer->setRenderTarget(0);
        
        spRenderer->beginDrawing2D();
        spRenderer->draw2DImage(RtTex, 0);
        spRenderer->endDrawing2D();
        #endif
        
        #ifdef MULTI_CONTEXT
        spContext->flipBuffers();
        SecondContext->activate();
        spRenderer->clearBuffers();
        spScene->renderScene();
        SecondContext->flipBuffers();
        #endif
        
        #ifdef FONT_TEST
        
        spRenderer->beginDrawing2D();
        spRenderer->draw2DText(FontObj, 15, "[ This is a test string! ]", video::color(255, 0, 0));
        spRenderer->endDrawing2D();
        
        #endif
        
        #ifdef CMD_TEST
        if (spControl->keyHit(io::KEY_F3))
        {
            isCmdActive = !isCmdActive;
            spControl->setWordInput(isCmdActive);
        }
        
        if (isCmdActive)
            Cmd->render();
        #endif
        
        #ifndef MULTI_CONTEXT
        spContext->flipBuffers();                                                   // Swap the video buffer to make the current frame visible.
        #endif
    }
    
    #if 1
    spContext->activate();
    #endif
    
    #ifdef CMD_TEST
    delete Cmd;
    #endif
    
    deleteDevice();                                                                 // Delete the device context. This will delete and release all objects allocated by the engine.
    
    return 0;
}
