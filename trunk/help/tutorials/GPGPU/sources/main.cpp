//
// SoftPixel Engine - Getting started
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include <Framework/OpenCL/spOpenCLDevice.hpp>


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
    spDevice    = createGraphicsDevice(
        video::RENDERER_OPENGL, dim::size2di(800, 600), 32, "SoftPixel Engine - GPGPU Tutorial"
    );
    
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    spControl   = spDevice->getInputControl();
    
    spScene     = spDevice->createSceneGraph();
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
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
    
    video::Texture* ParticleTex = spRenderer->loadTexture("media/Particle.jpg");
    
    /* Create particle mesh */
    scene::Mesh* Obj = spScene->createMesh();
    
    video::MaterialStates* Material = Obj->getMaterial();
    
    Material->setBlendingMode(video::BLEND_BRIGHT);
    Material->setLighting(false);
    Material->setMaterialCallback(ParticleMaterialCallback);
    
    /* Load GLSL shader */
    video::ShaderClass* ShdClass = spRenderer->createShaderClass();
    
    video::Shader* VertShd = spRenderer->loadShader(
        ShdClass, video::SHADER_VERTEX, video::GLSL_VERSION_1_20, "shaders/ParticleShader.glvert"
    );
    video::Shader* GeomShd = spRenderer->loadShader(
        ShdClass, video::SHADER_GEOMETRY, video::GLSL_VERSION_1_50, "shaders/ParticleShader.glgeom"
    );
    video::Shader* PixelShd = spRenderer->loadShader(
        ShdClass, video::SHADER_PIXEL, video::GLSL_VERSION_1_50, "shaders/ParticleShader.glfrag"
    );
    
    if (ShdClass->link())
    {
        ShdClass->setObjectCallback(ParticleShaderCallback);
        
        Obj->setShaderClass(ShdClass);
        
        PixelShd->setConstant("Tex", 0);
    }
    
    /* Load reference mesh */
    scene::Mesh* RefMesh = spScene->createMesh(scene::MESH_SPHERE, 15);
    RefMesh->setOrder(scene::ORDER_BACKGROUND);
    
    Obj->setParent(RefMesh);
    
    /* Create mesh surface */
    video::MeshBuffer* Surface = Obj->createMeshBuffer(VertFmt, video::DATATYPE_UNSIGNED_INT);
    
    Surface->setPrimitiveType(video::PRIMITIVE_POINTS);
    Surface->addTexture(ParticleTex);
    
    const u32 PointCount = RefMesh->getVertexCount();
    
    Surface->addVertices(PointCount);
    
    video::MeshBuffer* RefSurface = RefMesh->getMeshBuffer(0);
    
    for (u32 i = 0; i < PointCount; ++i)
    {
        const dim::vector3df Pos(RefSurface->getVertexCoord(i));
        const dim::vector4df Vec(Pos.X, Pos.Y - 0.7f, Pos.Z, math::Randomizer::randFloat(0.5f, 1.5f));
        
        Surface->setVertexCoord(i, Pos);
        Surface->setVertexTexCoord(i, dim::point2df(math::Randomizer::randFloat(0.5f), 0.0f), 0);
        Surface->setVertexAttribute(i, VertFmt->getTexCoords()[1], &Vec.X, sizeof(f32)*4);
        
        Surface->addPrimitiveIndex(i);
    }
    
    Surface->updateMeshBuffer();
    
    /* Load OpenCL program */
    video::OpenCLBuffer* CLBuf = CLDev->createBuffer(video::OCLBUFFER_READ_WRITE, Surface);
    
    u64 Time = io::Timer::millisecs();
    
    video::OpenCLProgram* CLShader = CLDev->loadProgram("shaders/OpenCLShader.cl");
    
    io::Log::message("Compilation time: " + io::stringc(io::Timer::millisecs() - Time) + " ms.");
    
    CLShader->addKernel("MainKernel");
    
    CLShader->setParameter("MainKernel", 0, CLBuf);
    CLShader->setParameter("MainKernel", 1, static_cast<s32>(Surface->getVertexCount()));
    
    /* Main loop */
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        tool::Toolset::presentModel(RefMesh);
        
        if (spControl->keyHit(io::KEY_SPACE))
            RefMesh->setVisible(!RefMesh->getVisible());
        
        if (spControl->keyHit(io::KEY_TAB))
        {
            static bool Wire;
            Wire = !Wire;
            spScene->setWireframe(Wire ? video::WIREFRAME_LINES : video::WIREFRAME_SOLID);
        }
        
        const dim::matrix4f ObjMatrix(RefMesh->getTransformation(true));
        CLShader->setParameter("MainKernel", 2, ObjMatrix.getArray(), sizeof(f32)*16);
        
        CLBuf->lock();
        CLShader->run("MainKernel", 1, 1);
        CLBuf->unlock();
        
        spScene->renderScene();
        
        spContext->flipBuffers();
    }
    
    delete CLDev;
    
    deleteDevice();
    
    return 0;
}
