/*
 * PhysX simulator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/PhysX/spPhysXSimulator.hpp"

#ifdef SP_COMPILE_WITH_PHYSX


#include "Framework/Physics/PhysX/spPhysXRigidBody.hpp"
#include "Framework/Physics/PhysX/spPhysXStaticObject.hpp"
#include "Framework/Physics/PhysX/spPhysXMaterial.hpp"
#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace physics
{


/*
 * PhysX error callback class
 */

class PhysXErrorCallback : public PxErrorCallback
{
    
    public:
        
        PhysXErrorCallback()
        {
        }
        ~PhysXErrorCallback()
        {
        }
        
        void reportError(PxErrorCode::Enum Code, const char *Message, const char *File, int Line)
        {
            io::Log::error("[ PhysX ] " + io::stringc(Message));
        }
        
};

static PxDefaultAllocator DefaultAllocatorCallback;
static PhysXErrorCallback DefaultErrorCallback;
static PxSimulationFilterShader DefaultFilterShader;


/*
 * PhysXSimulator class
 */

PhysXSimulator::PhysXSimulator() :
    PhysicsSimulator(SIMULATOR_PHYSX),
    PxDevice_       (0              ),
    PxFoundation_   (0              ),
    PxProfile_      (0              ),
    PxCooking_      (0              ),
    PxScene_        (0              )
{
    /* Print newton library information */
    io::Log::message(getVersion(), 0);
    io::Log::message("Copyright (c) 2012 - NVIDIA Corporation", 0);
    io::Log::message("", 0);
    
    /* Create foundation */
    PxFoundation_ = PxCreateFoundation(PX_PHYSICS_VERSION, DefaultAllocatorCallback, DefaultErrorCallback);
    
    if (!PxFoundation_)
    {
        io::Log::error("Coudl not create PhysX foundation");
        return;
    }
    
    /* Create physics device */
    /*PxProfile_ = &PxProfileZoneManager::createProfileZoneManager(PxFoundation_);
    
    if (!PxProfile_)
    {
        io::Log::error("Could not create PhysX profile zone manager");
        return;
    }*/
    
    PxDevice_ = PxCreatePhysics(PX_PHYSICS_VERSION, *PxFoundation_, PxTolerancesScale(), true);//, PxProfile_);
    
    if (!PxDevice_)
    {
        io::Log::error("Could not create PhysX device");
        return;
    }
    
    /* Initialize extensions */
    if (!PxInitExtensions(*PxDevice_))
    {
        io::Log::error("Could not initialize PhysX extensions");
        return;
    }
    
    /* Create cooking device */
    PxCooking_ = PxCreateCooking(PX_PHYSICS_VERSION, *PxFoundation_, PxCookingParams());
    
    if (!PxCooking_)
    {
        io::Log::error("Could not create PhysX cooking device");
        return;
    }
    
    /* Create base scene */
    PxScene_ = createScene();
}
PhysXSimulator::~PhysXSimulator()
{
    clearScene();
    MemoryManager::deleteList(MaterialList_);
    
    PxCloseExtensions();
    
    /* Release all PhysX objects */
    releaseObject(PxCooking_);
    releaseObject(PxDevice_);
    releaseObject(PxProfile_);
    releaseObject(PxFoundation_);
}

io::stringc PhysXSimulator::getVersion() const
{
    return "PhysX - v.3.2";
}

void PhysXSimulator::updateSimulation(const f32 StepTime)
{
    static const f32 DefaultStepTime = 1.0f / 60.0f;
    
    if (PxScene_)
    {
        /* Advance the simulation and wait for the results */
        PxScene_->simulate(StepTime);
        PxScene_->fetchResults(true);
    }
}

PhysicsMaterial* PhysXSimulator::createMaterial(
    f32 StaticFriction, f32 DynamicFriction, f32 Restitution)
{
    PhysicsMaterial* NewMaterial = new PhysXMaterial(PxDevice_, StaticFriction, DynamicFriction, Restitution);
    MaterialList_.push_back(NewMaterial);
    return NewMaterial;
}

StaticPhysicsObject* PhysXSimulator::createStaticObject(PhysicsMaterial* Material, scene::Mesh* Mesh)
{
    if (!PxScene_)
    {
        io::Log::error("Can not create rigid body without valid PhysX scene");
        return 0;
    }
    if (!Mesh || !Material || !PxCooking_)
        return 0;
    
    PhysXStaticObject* NewStaticObject = new PhysXStaticObject(
        PxDevice_, PxCooking_, static_cast<PhysXMaterial*>(Material), Mesh
    );
    StaticBodyList_.push_back(NewStaticObject);
    
    PxScene_->addActor(*NewStaticObject->PxActor_);
    
    return NewStaticObject;
}

RigidBody* PhysXSimulator::createRigidBody(
    PhysicsMaterial* Material, const ERigidBodies Type, scene::SceneNode* RootNode, const SRigidBodyConstruction &Construct)
{
    if (!PxScene_)
    {
        io::Log::error("Can not create rigid body without valid PhysX scene");
        return 0;
    }
    if (!Material || !RootNode)
        return 0;
    
    PhysXRigidBody* NewRigidBody = new PhysXRigidBody(
        PxDevice_, static_cast<PhysXMaterial*>(Material), Type, RootNode, Construct
    );
    RigidBodyList_.push_back(NewRigidBody);
    
    PxScene_->addActor(*NewRigidBody->PxActor_);
    
    return NewRigidBody;
}

RigidBody* PhysXSimulator::createRigidBody(PhysicsMaterial* Material, scene::Mesh* Mesh)
{
    return 0; //todo
}


/*
 * ======= Protected: =======
 */

PxScene* PhysXSimulator::createScene()
{
    /* Setup scene description */
    PxSceneDesc SceneDesc(PxDevice_->getTolerancesScale());
    
    SceneDesc.gravity       = convert(Gravity_);
    SceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
    SceneDesc.filterShader  = DefaultFilterShader;
    
    if (!SceneDesc.cpuDispatcher)
    {
        io::Log::error("Could not create PhysX CPU dispatcher");
        return 0;
    }
    
    /* Create scene with configured settings */
    PxScene* NewScene = PxDevice_->createScene(SceneDesc);
    
    if (!NewScene)
    {
        io::Log::error("Could not create PhysX scene");
        return 0;
    }
    
    /* Configure scene visualization */
    NewScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
    NewScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
    
    return NewScene;
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
