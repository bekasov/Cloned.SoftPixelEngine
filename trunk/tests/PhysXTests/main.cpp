//
// SoftPixel Engine - Getting started
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

physics::PhysicsSimulator* spPhysics = 0;

physics::RigidBody* CreateRigidBox(
    physics::PhysicsMaterial* Material, const dim::vector3df &Pos,
    const video::color &Color = 255, const dim::vector3df &Rot = 0.0f)
{
    scene::Mesh* MeshCube = spScene->createMesh(scene::MESH_CUBE);
    
    MeshCube->setPosition(Pos);
    MeshCube->setRotation(Rot);
    
    video::MaterialStates* MatStates = MeshCube->getMaterial();
    MatStates->setColorMaterial(false);
    MatStates->setDiffuseColor(Color);
    
    return spPhysics->createRigidBody(Material, physics::RIGIDBODY_BOX, MeshCube);
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
    {
        io::Log::error("Creating physics simulator failed");
        io::Log::pauseConsole();
        return 0;
    }
    
    physics::PhysicsMaterial* Material = spPhysics->createMaterial(1.0f, 1.0f, 0.3f);
    
    physics::StaticPhysicsObject* Floor = spPhysics->createStaticObject(Material, MeshFloor);
    
    for (s32 i = 0; i < 5; ++i)
        CreateRigidBox(Material, dim::vector3df(0, -2 + 1.01f * i, 0), video::color(0, 255, 0));
    
    
    
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
        
        
    }
    SP_TESTS_MAIN_END
}
