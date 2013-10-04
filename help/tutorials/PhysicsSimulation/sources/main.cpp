//
// SoftPixel Engine Tutorial: PhysicsSimulation - (27/05/2011)
//

#include <SoftPixelEngine.hpp>

#ifdef SP_COMPILE_WITH_PHYSICS

using namespace sp;

#include "../../common.hpp"

/* === Global members === */

SoftPixelDevice* spDevice               = 0;
io::InputControl* spControl             = 0;
video::RenderSystem* spRenderer         = 0;
video::RenderContext* spContext         = 0;
scene::SceneGraph* spScene              = 0;
physics::PhysicsSimulator* spPhysics    = 0;
audio::SoundDevice* spListener          = 0;

scene::Camera* Cam  = 0;
scene::Light* Light = 0;
scene::Mesh* World  = 0;

video::Font* Font = 0;

video::Texture* WoodTex     = 0;
video::Texture* MetalTex    = 0;
video::Texture* CraneTex    = 0;
video::Texture* StoneTex    = 0;

audio::Sound* SoundContact[6]   = { 0 };
std::vector<dim::matrix4f> ResetLocations;

physics::PhysicsMaterial* PhysicsMat    = 0;
physics::RigidBody* WreckingBall        = 0;

const s32 ScrWidth = 800, ScrHeight = 600;
const s32 CountOfJoints = 4;

// Declarations
void InitDevice();
void CleanUp();
void CreateScene();
void UpdateScene();
void DrawMenu();
void DrawCenteredText(
    s32 PosY, const io::stringc &Text, const video::color &Color = video::color(255, 255, 255, 200)
);


/* === All function definitions === */

int main()
{
    InitDevice();
    CreateScene();
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        UpdateScene();
        
        spScene->renderScene();
        
        DrawMenu();
        
        spContext->flipBuffers();
    }
    
    CleanUp();
    
    return 0;
}

void InitDevice()
{
    spDevice    = createGraphicsDevice(
        ChooseRenderer(), dim::size2di(ScrWidth, ScrHeight), 32, "Tutorial: PhysicsSimulation"
    );
    
    spControl   = spDevice->getInputControl();
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    
    spListener  = spDevice->createSoundDevice();
    spScene     = spDevice->createSceneGraph();
    spPhysics   = spDevice->createPhysicsSimulator(physics::SIMULATOR_NEWTON);
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
	
    spDevice->setFrameRate(100);
    
    spRenderer->setClearColor(255);
    
    /**
    The "setSolverModel" functions (NewtonSetSolverModel) specifies the degree of accuracy.
    0 is the exact accuracy but is very slow when lots of objects are stacked.
    Greater than 0 is for a less accuracy when speed is more important.
    The higher the value the higher the accuracy.
    In This example we have a tower of wood blocks so we set the accuracy lower.
    By default 0.
    */
    spPhysics->setSolverModel(3);
    
    math::Randomizer::seedRandom();
}

void CleanUp()
{
    deleteDevice();
}

/**
 * Collision contact callback for playing sounds when the metal boxes collide with each other.
 */
void ContactCallback(const dim::vector3df &Point, const dim::vector3df &Normal, f32 Velocity)
{
    static io::Timer Time(25ul);
    
    if (Velocity > 5.0f && Time.finish())
    {
        Time.reset();
        
        s32 SndNr = math::Randomizer::randInt(5);
        
        SoundContact[SndNr]->setVolume((Velocity - 5.0f) * 0.05f);
        SoundContact[SndNr]->play();
    }
}

/**
 * Create the physics simulation scene. Create some rigid bodies and joints.
 */
void CreateScene()
{
    // Load some resources
    const io::stringc ResPath = ROOT_PATH + "PhysicsSimulation/media/";
    
    // Load the font
    Font = spRenderer->createFont("Arial", 20, video::FONT_BOLD);
    
    // Load the textures
    WoodTex     = spRenderer->loadTexture(ResPath + "Wood.jpg");
    MetalTex    = spRenderer->loadTexture(ResPath + "Metal.jpg");
    CraneTex    = spRenderer->loadTexture(ResPath + "Crane.png");
    StoneTex    = spRenderer->loadTexture(ResPath + "Stone.jpg");
    
    video::ImageBuffer* ImgBuffer = CraneTex->getImageBuffer();
    
    for (s32 y = 0, x; y < ImgBuffer->getSize().Height; ++y)
    {
        for (x = 0; x < ImgBuffer->getSize().Width; ++x)
        {
            if (ImgBuffer->getPixelColor(dim::point2di(x, y)) == video::color(0, 255, 0))
                ImgBuffer->setPixelColor(dim::point2di(x, y), video::color(0, 0, 0, 0));
        }
    }
    
    CraneTex->updateImageBuffer();
    
    // Load the sounds
    for (s32 i = 0; i < 6; ++i)
        SoundContact[i] = spListener->loadSound(ResPath + "Impact" + io::stringc(i + 1) + ".wav", 10);
    
    // Create the small 3D scene
    Cam = spScene->createCamera();
    Cam->setRange(0.1f, 250);
    Cam->setPosition(dim::vector3df(0, 15, -25));
    
    Light = spScene->createLight();
    Light->setRotation(dim::vector3df(45, 10, 0));
    
    spScene->setLighting(true);
    
    // Create the wolrd
	World = spScene->createMesh(scene::MESH_PLANE);
	World->setPosition(dim::vector3df(0, -5, 0));
	World->setScale(250);
	World->addTexture(WoodTex);
	World->textureAutoMap(0, 0.5f);
    
    // Create physics material describing the behavior of friction etc.
    PhysicsMat = spPhysics->createMaterial();
    
    // Create the physics blocks
    physics::RigidBody* Body = 0;
    
    physics::PhysicsBaseObject* PrevBody = spPhysics->createStaticObject(PhysicsMat, World);
    
    const dim::vector3df BlockSize(3, 1, 0.5);
    const f32 TowerRadius = 4;
    dim::vector3df BlockPos;
    
    scene::Mesh* RefBlock = spScene->createMesh(scene::MESH_CUBE);
    RefBlock->addTexture(MetalTex);
    RefBlock->setScale(BlockSize);
    RefBlock->textureAutoMap(0, 0.4f);
    RefBlock->getMeshBuffer(0)->textureTransform(0, dim::point2df(0.75f, 1.0f));
    RefBlock->getMeshBuffer(0)->textureTranslate(0, 0.45f);
    RefBlock->setVisible(false);
    
    spPhysics->setGravity(spPhysics->getGravity() * 5);
    physics::PhysicsSimulator::setContactCallback(ContactCallback);
    
    for (s32 h = 0; h < 30; ++h)
    {
        for (s32 i = 0, c = 7; i < c; ++i)
        {
            scene::Mesh* Block = spScene->createMesh();
            
            f32 Angle = 360.0f * (0.5f * h + i) / c;
            
            BlockPos.X = math::Sin(Angle) * TowerRadius;
            BlockPos.Y = -4.5f + h;
            BlockPos.Z = math::Cos(Angle) * TowerRadius;
            
            Block->setReference(RefBlock);
            Block->setScale(BlockSize);
            Block->setPosition(BlockPos);
            Block->setRotation(dim::vector3df(0, Angle, 0));
            
            Body = spPhysics->createRigidBody(
                PhysicsMat, physics::RIGIDBODY_BOX, Block, physics::SRigidBodyConstruction(BlockSize * 0.5f)
            );
            
            Body->setAutoSleep(true);
        }
    }
    
    // Create joints
    scene::Mesh* RefCylinder = spScene->createMesh(scene::MESH_CYLINDER);
    RefCylinder->meshTurn(dim::vector3df(0, 0, 90));
    RefCylinder->setVisible(false);
    
    for (s32 i = 0; i < CountOfJoints; ++i)
    {
        scene::Mesh* Cylinder = spScene->createMesh();
        Cylinder->setReference(RefCylinder);
        
        Cylinder->setPosition(dim::vector3df(10.0f + i * 5, 30, 0));
        Cylinder->setScale(dim::vector3df(5.0f, 0.5f, 0.5f));
        
        Body = spPhysics->createRigidBody(
            PhysicsMat, physics::RIGIDBODY_CYLINDER, Cylinder, physics::SRigidBodyConstruction(0.5f, 5.0f)
        );
        spPhysics->createJoint(
            physics::JOINT_BALL,
            Body,
            PrevBody,
            physics::SPhysicsJointConstruct(dim::vector3df(7.5f + i * 5, 30, 0))
        );
        
        PrevBody = Body;
    }
    
    // Create wrecking ball
    scene::Mesh* Sphere = spScene->createMesh(scene::MESH_SPHERE);
    Sphere->setPosition(dim::vector3df(30, 30, 0));
    Sphere->setScale(5);
    Sphere->addTexture(MetalTex);
    
    WreckingBall = spPhysics->createRigidBody(
        PhysicsMat, physics::RIGIDBODY_SPHERE, Sphere, physics::SRigidBodyConstruction(dim::vector3df(2.5f))
    );
    WreckingBall->setMass(math::Randomizer::randFloat(250, 1500));
    
    spPhysics->createJoint(
        physics::JOINT_BALL,    // Physics joint type
        WreckingBall,           // First rigid body
        PrevBody,               // Second rigid body
        physics::SPhysicsJointConstruct(dim::vector3df(7.5f + CountOfJoints * 5, 30, 0))
    );
    
    // Store reset positions
    std::list<physics::RigidBody*> RigidBodyList = spPhysics->getRigidBodyList();
    
    ResetLocations.resize(RigidBodyList.size());
    
    s32 i = 0;
    for (std::list<physics::RigidBody*>::iterator it = RigidBodyList.begin(); it != RigidBodyList.end(); ++it, ++i)
        ResetLocations[i] = (*it)->getTransformation();
    
    // Create crane
    scene::Mesh* CraneObj[4] = { 0 };
    
    CraneObj[0] = spScene->createMesh(scene::MESH_CUBE);
    CraneObj[0]->setPosition(dim::vector3df(7.5f, 31.5f, 0));
    CraneObj[0]->setScale(dim::vector3df(1, 3, 1));
    
    CraneObj[1] = spScene->createMesh(scene::MESH_CUBE);
    CraneObj[1]->setPosition(dim::vector3df(22.5f, 33, 0));
    CraneObj[1]->setScale(dim::vector3df(30, 2, 2));
    
    CraneObj[2] = spScene->createMesh(scene::MESH_CUBE);
    CraneObj[2]->setPosition(dim::vector3df(35, 13, 0));
    CraneObj[2]->setScale(dim::vector3df(2, 40, 3));
    
    CraneObj[3] = spScene->createMesh(scene::MESH_CUBE);
    CraneObj[3]->setPosition(dim::vector3df(40, 33, 0));
    CraneObj[3]->setScale(dim::vector3df(7, 5, 5));
    
    CraneObj[3]->addTexture(StoneTex);
    CraneObj[3]->textureAutoMap(0, 0.25f);
    
    for (s32 i = 1; i <= 2; ++i)
    {
        CraneObj[i]->addTexture(CraneTex);
        CraneObj[i]->textureAutoMap(0, 0.5f);
        
        CraneObj[i]->getMaterial()->setRenderFace(video::FACE_BOTH);
        CraneObj[i]->getMaterial()->setAlphaMethod(video::CMPSIZE_GREATER, 0.5f);
        CraneObj[i]->getMaterial()->setBlending(false);
    }
    
    #if 0
    Cam->setPosition(0);
    Cam->setRotation(dim::vector3df(90, -90, 0));
    Cam->setParent(Sphere);
    #endif
}

/**
 * Update the physics simulation and user interaction.
 */
void UpdateScene()
{
	// Update the simulation with the speed of 100 FPS.
    spPhysics->updateSimulation(
        static_cast<f32>(1.0 / io::Timer::getFPS())
    );
    
    if (!Cam->getParent() && spContext->isWindowActive())
        tool::Toolset::moveCameraFree();
    
    // Reset simulation
    if (spControl->mouseHit(io::MOUSE_RIGHT))
    {
        std::list<physics::RigidBody*> RigidBodyList = spPhysics->getRigidBodyList();
        
        #if 1
        
        WreckingBall->setMass(math::Randomizer::randFloat(250, 1500));
        
        s32 i = 0;
        for (std::list<physics::RigidBody*>::iterator it = RigidBodyList.begin(); it != RigidBodyList.end(); ++it, ++i)
            (*it)->setTransformation(ResetLocations[i]);
        
        #else
        
        WreckingBall->setMass(100);
        
        s32 i = 0;
        for (std::list<physics::RigidBody*>::iterator it = RigidBodyList.begin(); it != RigidBodyList.end(); ++it, ++i)
        {
            if (i >= ResetLocations.size() - CountOfJoints - 1)
                (*it)->setTransformation(ResetLocations[i]);
        }
        
        #endif
    }
}

void DrawMenu()
{
    spRenderer->beginDrawing2D();
    
    DrawCenteredText(15, "Wrecking ball mass: " + io::stringc(WreckingBall->getMass()));
    
    spRenderer->endDrawing2D();
}


/**
 * Function to draw centered text easyly.
 */
void DrawCenteredText(s32 PosY, const io::stringc &Text, const video::color &Color)
{
    // Get the text size
    const dim::size2di TextSize(Font->getStringSize(Text));
    
    // Draw the text (background black and foreground with specified color)
    spRenderer->draw2DText(
        Font, dim::point2di(ScrWidth/2 - TextSize.Width/2 + 2, PosY + 2), Text, video::color(0, 0, 0, Color.Alpha)
    );
    spRenderer->draw2DText(
        Font, dim::point2di(ScrWidth/2 - TextSize.Width/2, PosY), Text, Color
    );
}

#else

#   error The engine was not compiled with the physics system!

#endif



// ============================================
