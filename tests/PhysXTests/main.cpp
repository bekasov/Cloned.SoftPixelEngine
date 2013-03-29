//
// SoftPixel Engine - PhysX Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#ifdef SP_COMPILE_WITH_PHYSICS

#include "../common.hpp"

SP_TESTS_DECLARE

physics::PhysicsSimulator* spPhysics = 0;

physics::RigidBody* CreateRigidBox(
    physics::PhysicsMaterial* Material, const dim::vector3df &Pos,
    const video::color &Color = 255, const dim::vector3df &Rot = 0.0f,
    const physics::SRigidBodyConstruction &Construct = physics::SRigidBodyConstruction())
{
    scene::Mesh* MeshCube = spScene->createMesh(scene::MESH_CUBE);
    
    MeshCube->setPosition(Pos);
    MeshCube->setRotation(Rot);
    MeshCube->meshTransform(Construct.Size * 2.0f);
    
    video::MaterialStates* MatStates = MeshCube->getMaterial();
    MatStates->setColorMaterial(false);
    MatStates->setDiffuseColor(Color);
    
    return spPhysics->createRigidBody(Material, physics::RIGIDBODY_BOX, MeshCube, Construct);
}

int main()
{
    SP_TESTS_INIT("PhysX")
    
    Cam->setPosition(dim::vector3df(0, 0, -6));
    
    // Create visual scene
    scene::Mesh* MeshFloor = spScene->createMesh(scene::MESH_PLANE);
    MeshFloor->meshTransform(100);
    MeshFloor->setPosition(dim::vector3df(0, -3, 0));
    
    
    
    // Create physics scene
    spPhysics = spDevice->createPhysicsSimulator(physics::SIMULATOR_PHYSX);
    
    if (!spPhysics)
        return Fatal("Creating physics simulator failed");
    
    physics::PhysicsMaterial* Material = spPhysics->createMaterial(0.7f, 0.7f, 0.3f);
    
    physics::StaticPhysicsObject* Floor = spPhysics->createStaticObject(Material, MeshFloor);
    
    for (s32 i = 0; i < 5; ++i)
        CreateRigidBox(Material, dim::vector3df(0, -2 + 1.01f * i, 0), video::color(0, 255, 0));
    
    physics::RigidBody* Door = CreateRigidBox(
        Material, dim::vector3df(-4, 0, 0), video::color(255, 255, 0), 0.0f,
        physics::SRigidBodyConstruction(dim::vector3df(1.0f, 2.25f, 0.2f))
    );
    
    physics::PhysicsJoint* DoorJoint = spPhysics->createJoint(
        physics::JOINT_HINGE, Door,
        physics::SPhysicsJointConstruct(dim::vector3df(-5, 0, 0), dim::vector3df(0, 1, 0))
    );
    
    if (!DoorJoint)
        return Fatal("Creating physics joint failed");
    
    DoorJoint->setMotor(true);
    
    
    SP_TESTS_MAIN_BEGIN
    {
        if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree();
        
        if (spControl->mouseHit(io::MOUSE_LEFT))
        {
            physics::RigidBody* Obj = CreateRigidBox(
                Material, Cam->getPosition(), video::color(255, 0, 0), Cam->getRotation()
            );
            Obj->addVelocity(Cam->getTransformation().getDirection(dim::vector3df(0, 0, 15.0f)));
        }
        
        spPhysics->updateSimulation();
        
        if (spControl->keyDown(io::KEY_PAGEUP))
            DoorJoint->runMotor(1.0f);
        else if (spControl->keyDown(io::KEY_PAGEDOWN))
            DoorJoint->runMotor(-1.0f);
        else
            DoorJoint->runMotor(0.0f);
        
        
    }
    SP_TESTS_MAIN_END
}

#else

int main()
{
    io::Log::error("This engine was not compiled with physics simulators");
    io::Log::pauseConsole();
    return 0;
}

#endif
