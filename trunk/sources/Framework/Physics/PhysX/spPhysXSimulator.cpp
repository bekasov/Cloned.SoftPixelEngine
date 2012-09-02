/*
 * PhysX simulator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/PhysX/spPhysXSimulator.hpp"

//#ifdef SP_COMPILE_WITH_PHYSX


#include "Framework/Physics/PhysX/spPhysXRigidBody.hpp"
#include "Framework/Physics/PhysX/spPhysXJoint.hpp"
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
static PxSimulationFilterShader DefaultFilterShader = PxDefaultSimulationFilterShader;


/*
 * PhysXSimulator class
 */

PhysXSimulator::PhysXSimulator() :
    PhysicsSimulator        (SIMULATOR_PHYSX),
    PxDevice_               (0              ),
    PxFoundation_           (0              ),
    PxProfile_              (0              ),
    PxCooking_              (0              ),
    #ifdef _DEBUG
    PxDebuggerConnection_   (0              ),
    #endif
    PxScene_                (0              )
{
    /* Print newton library information */
    io::Log::message(getVersion(), 0);
    io::Log::message("Copyright (c) 2012 - NVIDIA Corporation", 0);
    io::Log::message("", 0);
    
    /* Create foundation */
    PxFoundation_ = PxCreateFoundation(PX_PHYSICS_VERSION, DefaultAllocatorCallback, DefaultErrorCallback);
    
    if (!PxFoundation_)
        throw io::stringc("Could not create PhysX foundation");
    
    /* Create physics device */
    /*PxProfile_ = &PxProfileZoneManager::createProfileZoneManager(PxFoundation_);
    
    if (!PxProfile_)
        throw io::stringc("Could not create PhysX profile zone manager");
    */
    
    PxDevice_ = PxCreatePhysics(PX_PHYSICS_VERSION, *PxFoundation_, PxTolerancesScale(), true);//, PxProfile_);
    
    if (!PxDevice_)
        throw io::stringc("Could not create PhysX device");
    
    /* Initialize extensions */
    if (!PxInitExtensions(*PxDevice_))
        throw io::stringc("Could not initialize PhysX extensions");
    
    /* Create cooking device */
    PxCooking_ = PxCreateCooking(PX_PHYSICS_VERSION, *PxFoundation_, PxCookingParams());
    
    if (!PxCooking_)
        throw io::stringc("Could not create PhysX cooking device");
    
    #if defined(SP_DEBUGMODE) && 0
    
    if (PxDevice_->getPvdConnectionManager())
    {
        PxVisualDebuggerConnectionFlags ConnectFlags = PxVisualDebuggerExt::getAllConnectionFlags();
        
        const c8* PVDHostIp = "127.0.0.1";
        s32 Port            = 5425;
        u32 TimeOut         = 100;
        
        PxDebuggerConnection_ = PxVisualDebuggerExt::createConnection(
            PxDevice_->getPvdConnectionManager(), PVDHostIp, Port, TimeOut, ConnectFlags
        );
        
        if (!PxDebuggerConnection_)
            io::Log::warning("Could not connect to PhysX visual debugger");
    }
    else
        io::Log::warning("PhysX visual debugger is not available");
    
    #endif
    
    /* Create base scene */
    PxScene_ = createScene();
    
    if (!PxScene_)
        throw io::stringc("Unable to create PhysX scene");
}
PhysXSimulator::~PhysXSimulator()
{
    clearScene();
    MemoryManager::deleteList(MaterialList_);
    
    PxCloseExtensions();
    
    /* Release all PhysX objects */
    #ifdef SP_DEBUGMODE
    releaseObject(PxDebuggerConnection_);
    #endif
    
    releaseObject(PxCooking_);
    releaseObject(PxDevice_);
    releaseObject(PxProfile_);
    releaseObject(PxFoundation_);
}

io::stringc PhysXSimulator::getVersion() const
{
    return (
        "PhysX - v." + io::stringc(PX_PHYSICS_VERSION_MAJOR)
        + "." + io::stringc(PX_PHYSICS_VERSION_MINOR)
        #if PX_PHYSICS_VERSION_BUGFIX != 0
        + "." + io::stringc(PX_PHYSICS_VERSION_BUGFIX)
        #endif
    );
}

void PhysXSimulator::updateSimulation(const f32 StepTime)
{
    if (!PxScene_)
        return;
    
    /* Advance the simulation and wait for the results */
    PxScene_->simulate(StepTime);
    PxScene_->fetchResults(true);
    
    /* Retrieve array of actors that moved */
    PxU32 ActiveTransformsCount;
    PxActiveTransform* ActiveTransforms = PxScene_->getActiveTransforms(ActiveTransformsCount);
    
    /* Update each render object with the new transform */
    for (PxU32 i = 0; i < ActiveTransformsCount; ++i)
    {
        scene::SceneNode* Node = static_cast<scene::SceneNode*>(ActiveTransforms[i].userData);
        Node->setTransformation(PhysXSimulator::convert(ActiveTransforms[i].actor2World));
    }
}

PhysicsMaterial* PhysXSimulator::createMaterial(
    f32 StaticFriction, f32 DynamicFriction, f32 Restitution)
{
    try
    {
        PhysicsMaterial* NewMaterial = new PhysXMaterial(PxDevice_, StaticFriction, DynamicFriction, Restitution);
        MaterialList_.push_back(NewMaterial);
        return NewMaterial;
    }
    catch (const std::string &ErrorStr)
    {
        io::Log::error(ErrorStr);
    }
    return 0;
}

StaticPhysicsObject* PhysXSimulator::createStaticObject(PhysicsMaterial* Material, scene::Mesh* Mesh)
{
    if (!PxScene_)
    {
        io::Log::error("Can not create rigid body without valid PhysX scene");
        return 0;
    }
    if (!Mesh || !Material || !PxCooking_)
    {
        io::Log::error("Invalid arguments for static physics object");
        return 0;
    }
    
    try
    {
        PhysXStaticObject* NewStaticObject = new PhysXStaticObject(
            PxDevice_, PxCooking_, static_cast<PhysXMaterial*>(Material), Mesh
        );
        StaticBodyList_.push_back(NewStaticObject);
        
        PxScene_->addActor(*NewStaticObject->PxActor_);
        
        return NewStaticObject;
    }
    catch (const std::string &ErrorStr)
    {
        io::Log::error(ErrorStr);
    }
    
    return 0;
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
    {
        io::Log::error("Invalid arguments for rigid body");
        return 0;
    }
    
    try
    {
        PhysXRigidBody* NewRigidBody = new PhysXRigidBody(
            PxDevice_, static_cast<PhysXMaterial*>(Material), Type, RootNode, Construct
        );
        RigidBodyList_.push_back(NewRigidBody);
        
        PxScene_->addActor(*NewRigidBody->PxActor_);
        
        return NewRigidBody;
    }
    catch (const std::string &ErrorStr)
    {
        io::Log::error(ErrorStr);
    }
    
    return 0;
}

RigidBody* PhysXSimulator::createRigidBody(PhysicsMaterial* Material, scene::Mesh* Mesh)
{
    return 0; //todo
}

PhysicsJoint* PhysXSimulator::createJoint(
    const EPhysicsJoints Type, PhysicsBaseObject* Object, const SPhysicsJointConstruct &Construct)
{
    return createJoint(Type, Object, 0, Construct);
}

PhysicsJoint* PhysXSimulator::createJoint(
    const EPhysicsJoints Type, PhysicsBaseObject* ObjectA, PhysicsBaseObject* ObjectB, const SPhysicsJointConstruct &Construct)
{
    try
    {
        PhysXJoint* NewJoint = new PhysXJoint(
            PxDevice_, Type, dynamic_cast<PhysXRigidBody*>(ObjectA), ObjectB, Construct
        );
        JointList_.push_back(NewJoint);
        return NewJoint;
    }
    catch (const std::string &ErrorStr)
    {
        io::Log::error(ErrorStr);
    }
    return 0;
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
    SceneDesc.flags         = PxSceneFlag::Enum::eENABLE_ACTIVETRANSFORMS;
    
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

scene::Transformation PhysXSimulator::convert(const PxTransform &Transform)
{
    return scene::Transformation(
        *((dim::vector3df*)&Transform.p),
        ((dim::quaternion*)&Transform.q)->getInverse(),
        1.0f
    );
}


} // /namespace physics

} // /namespace sp


//#endif



// ================================================================================
