//
// SoftPixel Engine - Getting started
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#ifdef SP_COMPILE_WITH_OPENCL

#include <Framework/OpenCL/spOpenCLDevice.hpp>

#include "../../common.hpp"


/* === Global members === */

SoftPixelDevice* spDevice       = 0;

video::RenderSystem* spRenderer = 0;
video::RenderContext* spContext = 0;
io::InputControl* spControl     = 0;

scene::SceneGraph* spScene      = 0;


/* === Functions === */

void ParticleShaderCallback(video::ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    ShdClass->getVertexShader()->setConstant("WorldMatrix", spRenderer->getWorldMatrix());
    ShdClass->getVertexShader()->setConstant("ViewMatrix", spRenderer->getViewMatrix());
    
    ShdClass->getGeometryShader()->setConstant("ProjectionMatrix", spRenderer->getProjectionMatrix());
}

void ParticleMaterialCallback(scene::Mesh* Obj, bool isBegin)
{
    spRenderer->setDepthMask(!isBegin);
}

int main()
{
    //io::Log::open();
    
    spDevice    = createGraphicsDevice(
        video::RENDERER_OPENGL, dim::size2di(800, 600), 32, "SoftPixel Engine - GPGPU Tutorial"
        //video::RENDERER_OPENGL, video::VideoModeEnumerator().getDesktop().Resolution, 32, "", true, DEVICEFLAG_HQ
    );
    
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    spControl   = spDevice->getInputControl();
    
    spScene     = spDevice->createSceneGraph();
    
    spControl->setCursorVisible(true);
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    #if 1
    spContext->setVsync(false);
    #endif
    
    math::Randomizer::seedRandom();
    
    /* Create OpenCL device */
    video::OpenCLDevice* CLDev = 0;
    
    try
    {
        CLDev = new video::OpenCLDevice();
    }
    catch (const std::string &ErrorStr)
    {
        io::Log::error(ErrorStr);
        io::Log::pauseConsole();
        return 0;
    }
    
    /* Create vertex format */
    video::VertexFormatUniversal* VertFmt = spRenderer->createVertexFormat<video::VertexFormatUniversal>();
    
    VertFmt->addCoord();
    VertFmt->addTexCoord(video::DATATYPE_FLOAT, 2);
    VertFmt->addTexCoord(video::DATATYPE_FLOAT, 4);
    
    /* Create scene */
    scene::Camera* Cam = spScene->createCamera();
    Cam->setPosition(dim::vector3df(0, 0, -3));
    
    scene::Light* Lit = spScene->createLight();
    spScene->setLighting();
    
    video::Texture* ParticleTex = spRenderer->loadTexture(ROOT_PATH + "GPGPU/media/Particle.jpg");
    video::Font* Fnt = spRenderer->createFont("", 20, video::FONT_BOLD);
    
    /* Create particle mesh */
    scene::Mesh* Obj = spScene->createMesh();
    
    video::MaterialStates* Material = Obj->getMaterial();
    
    Material->setBlendingMode(video::BLEND_BRIGHT);
    Material->setLighting(false);
    Material->setMaterialCallback(ParticleMaterialCallback);
    
    /* Load GLSL shader */
    video::ShaderClass* ShdClass = spRenderer->createShaderClass(VertFmt);
    
    video::Shader* VertShd = spRenderer->loadShader(
        ShdClass, video::SHADER_VERTEX, video::GLSL_VERSION_1_20, ROOT_PATH + "GPGPU/shaders/ParticleShader.glvert"
    );
    video::Shader* GeomShd = spRenderer->loadShader(
        ShdClass, video::SHADER_GEOMETRY, video::GLSL_VERSION_1_50, ROOT_PATH + "GPGPU/shaders/ParticleShader.glgeom"
    );
    video::Shader* PixelShd = spRenderer->loadShader(
        ShdClass, video::SHADER_PIXEL, video::GLSL_VERSION_1_50, ROOT_PATH + "GPGPU/shaders/ParticleShader.glfrag"
    );
    
    if (ShdClass->link())
    {
        ShdClass->setObjectCallback(ParticleShaderCallback);
        
        Obj->setShaderClass(ShdClass);
        
        PixelShd->setConstant("Tex", 0);
    }
    
    /* Load reference mesh */
    scene::Mesh* RefMesh = spScene->createMesh(scene::MESH_TEAPOT);
    RefMesh->setOrder(scene::ORDER_BACKGROUND);
    
    Obj->setParent(RefMesh);
    
    /* Create mesh surface */
    video::MeshBuffer* Surface = Obj->createMeshBuffer(VertFmt, video::DATATYPE_UNSIGNED_INT);
    
    Surface->setPrimitiveType(video::PRIMITIVE_POINTS);
    Surface->addTexture(ParticleTex);
    
    const u32 PointCount = RefMesh->getVertexCount();
    const u32 MultiCount = 5;
    
    Surface->addVertices(PointCount*MultiCount);
    
    video::MeshBuffer* RefSurface = RefMesh->getMeshBuffer(0);
    
    for (u32 i = 0, c = 0; i < PointCount; ++i)
    {
        const dim::vector3df Pos(RefSurface->getVertexCoord(i));
        
        for (u32 j = 0; j < MultiCount; ++j)
        {
            const dim::vector4df Vec(Pos.X, Pos.Y - 0.7f, Pos.Z, math::Randomizer::randFloat(0.5f, 1.5f));
            
            Surface->setVertexCoord(c, Pos);
            Surface->setVertexTexCoord(c, dim::point2df(math::Randomizer::randFloat(0.5f), 0.0f), 0);
            Surface->setVertexAttribute(c, VertFmt->getTexCoords()[1], &Vec.X, sizeof(f32)*4);
            
            Surface->addPrimitiveIndex(c++);
        }
    }
    
    Surface->updateMeshBuffer();
    
    /* Load OpenCL program */
    video::OpenCLBuffer* CLBuf = CLDev->createBuffer(video::OCLBUFFER_READ_WRITE, Surface);
    
    u64 Time = io::Timer::millisecs();
    
    //#define TEST_RAYTRACING
    
    video::OpenCLProgram* CLShader = CLDev->loadProgram(
        #ifdef TEST_RAYTRACING
        "../../../sources/Resources/RayTracingShader.cl"
        #else
        ROOT_PATH + "GPGPU/shaders/OpenCLShader.cl"
        #endif
    );
    
    io::Log::message("Compilation time: " + io::stringc(io::Timer::millisecs() - Time) + " ms.");
    
    const io::stringc KernelName =
        #ifdef TEST_RAYTRACING
        "RenderRayTracing";
        #else
        "MainKernel";
        #endif
    
    CLShader->addKernel(KernelName);
    
    const s32 NumExecutionCores = 100;
    const s32 VertexCount       = static_cast<s32>(Surface->getVertexCount());
    const s32 OffsetSize        = (VertexCount / NumExecutionCores);
    
    CLShader->setParameter(KernelName, 0, CLBuf);
    CLShader->setParameter(KernelName, 1, VertexCount);
    CLShader->setParameter(KernelName, 2, OffsetSize);
    
    const u32 TriangleCount = RefMesh->getTriangleCount() + Obj->getVertexCount()*2;
    io::Timer Timer(true);
    
    #ifdef TEST_RAYTRACING
    io::Log::pauseConsole();
    deleteDevice();
    return 0;
    #endif
    
    //spRenderer->setVsync(false);
    
    /* Main loop */
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        tool::Toolset::presentModel(RefMesh);
        
        if (spControl->keyHit(io::KEY_SPACE))
            RefMesh->setVisible(!RefMesh->getVisible());
        
        if (spControl->keyHit(io::KEY_W))
        {
            static bool Wire;
            Wire = !Wire;
            spScene->setWireframe(Wire ? video::WIREFRAME_LINES : video::WIREFRAME_SOLID);
        }
        if (spControl->keyHit(io::KEY_V))
            spContext->setVsync(!spContext->getVsync());
        
        const dim::matrix4f ObjMatrix(RefMesh->getTransformMatrix(true));
        CLShader->setParameter(KernelName, 3, ObjMatrix);
        
        CLBuf->lock();
        CLShader->run(KernelName, NumExecutionCores, NumExecutionCores);
        CLBuf->unlock();
        
        spScene->renderScene();
        
        spRenderer->beginDrawing2D();
        spRenderer->draw2DText(Fnt, dim::point2di(15, 15), "Triangles: " + io::stringc(TriangleCount));
        spRenderer->draw2DText(Fnt, dim::point2di(15, 40), "FPS: " + io::stringc(Timer.getFPS()));
        spRenderer->draw2DText(Fnt, dim::point2di(15, 65), "Press W -> Wireframe Switch");
        spRenderer->draw2DText(Fnt, dim::point2di(15, 90), "Press V -> Vsync Switch");
        spRenderer->endDrawing2D();
        
        spContext->flipBuffers();
    }
    
    delete CLDev;
    
    deleteDevice();
    
    return 0;
}

#else

#error The engine was not compiled with OpenCL

#endif
