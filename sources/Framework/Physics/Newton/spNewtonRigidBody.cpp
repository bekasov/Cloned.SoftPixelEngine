/*
 * Newton rigid body file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/Newton/spNewtonRigidBody.hpp"

#ifdef SP_COMPILE_WITH_NEWTON


#include "Framework/Physics/Newton/spNewtonSimulator.hpp"
#include "Framework/Physics/Newton/spNewtonJoint.hpp"
#include "Base/spMemoryManagement.hpp"

#include <newton.h>


namespace sp
{
namespace physics
{


void PhysicsTransformCallback(const NewtonBody* NtBody, const f32* Transformation, s32 ThreadIndex);
void PhysicsForceAndTorqueCallback(const NewtonBody* NtBody, f32 TimeStep, s32 ThreadIndex);


NewtonRigidBody::NewtonRigidBody(
    const ERigidBodies Type, const SRigidBodyConstruction &Construct) :
    BaseObject          (               ),
    PhysicsBaseObject   (               ),
    RigidBody           (Type, Construct),
    NewtonBaseObject    (               )
{
    /* Create the specified rigid body type */
    switch (Type_)
    {
        case RIGIDBODY_BOX:             createBox               (); break;
        case RIGIDBODY_CONE:            createCone              (); break;
        case RIGIDBODY_SPHERE:          createSphere            (); break;
        case RIGIDBODY_CYLINDER:        createCylinder          (); break;
        case RIGIDBODY_CHAMFERCYLINDER: createChamferCylinder   (); break;
        case RIGIDBODY_CAPSULE:         createCapsule           (); break;
        default:
            io::Log::error("Unsupported rigid body type");
            break;
    }
}
NewtonRigidBody::NewtonRigidBody(scene::Mesh* Mesh) :
    PhysicsBaseObject   (                                           ),
    RigidBody           (RIGIDBODY_MESH, SRigidBodyConstruction()   ),
    NewtonBaseObject    (                                           )
{
    createMesh(Mesh);
    setRootNode(Mesh);
}
NewtonRigidBody::~NewtonRigidBody()
{
}

void NewtonRigidBody::setMass(f32 Mass)
{
    DynamicPhysicsObject::setMass(Mass);
    
    /* Compute the moment of inertia for all three axles */
    f32 Inertia[3], Origin[3];
    NewtonConvexCollisionCalculateInertialMatrix(NtCollision_, Inertia, Origin);
    
    NewtonBodySetMassMatrix(
        NtBody_, Mass_,
        Inertia[0] * Mass_, Inertia[1] * Mass_, Inertia[2] * Mass_
    );
}

void NewtonRigidBody::setAutoSleep(bool Enable)
{
    DynamicPhysicsObject::setAutoSleep(Enable);
    
    NewtonBodySetFreezeState(NtBody_, Enable ? 1 : 0);
    NewtonBodySetAutoSleep(NtBody_, Enable ? 1 : 0);
}

void NewtonRigidBody::setMassCenter(const dim::vector3df &LocalPoint)
{
    NewtonBodySetCentreOfMass(NtBody_, &LocalPoint.X);
}
dim::vector3df NewtonRigidBody::getMassCenter() const
{
    dim::vector3df Center;
    NewtonBodyGetCentreOfMass(NtBody_, &Center.X);
    return Center;
}

void NewtonRigidBody::addVelocity(const dim::vector3df &Direction)
{
    setVelocity(getVelocity() + Direction);
}
void NewtonRigidBody::setVelocity(const dim::vector3df &Direction)
{
    NewtonBodySetVelocity(NtBody_, &Direction.X);
}
dim::vector3df NewtonRigidBody::getVelocity() const
{
    dim::vector3df Velocity;
    NewtonBodyGetVelocity(NtBody_, &Velocity.X);
    return Velocity;
}

void NewtonRigidBody::addImpulse(const dim::vector3df &Direction, const dim::vector3df &PivotPoint)
{
    NewtonBodyAddImpulse(NtBody_, &Direction.X, &PivotPoint.X);
}
void NewtonRigidBody::setForce(const dim::vector3df &Direction)
{
    NewtonBodySetForce(NtBody_, &Direction.X);
}


/*
 * ======= Protected: =======
 */

void NewtonRigidBody::createBox()
{
    setupCollision(NewtonCreateBox(
        NewtonSimulator::getNewtonWorld(), Construct_.Size.X*2, Construct_.Size.Y*2, Construct_.Size.Z*2, 0, 0
    ));
}
void NewtonRigidBody::createCone()
{
    setupCollision(NewtonCreateCone(
        NewtonSimulator::getNewtonWorld(), Construct_.Radius, Construct_.Height, 0, 0
    ));
}
void NewtonRigidBody::createSphere()
{
    setupCollision(NewtonCreateSphere(
        NewtonSimulator::getNewtonWorld(), Construct_.Size.X, Construct_.Size.Y, Construct_.Size.Z, 0, 0
    ));
}
void NewtonRigidBody::createCylinder()
{
    setupCollision(NewtonCreateCylinder(
        NewtonSimulator::getNewtonWorld(), Construct_.Radius, Construct_.Height, 0, 0
    ));
}
void NewtonRigidBody::createChamferCylinder()
{
    setupCollision(NewtonCreateChamferCylinder(
        NewtonSimulator::getNewtonWorld(), Construct_.Radius, Construct_.Height, 0, 0
    ));
}
void NewtonRigidBody::createCapsule()
{
    setupCollision(NewtonCreateCapsule(
        NewtonSimulator::getNewtonWorld(), Construct_.Radius, Construct_.Height, 0, 0
    ));
}

void NewtonRigidBody::createMesh(scene::Mesh* MeshGeom)
{
    static const f32 OptimizeTolerance = 0.002f;
    
    /* Temporary storage */
    u32 s, i, j = 0;
    
    const dim::matrix4f ScaleMatrix(MeshGeom->getScaleMatrix());
    
    /* Create the vertex coordinates buffer */
    const u32 VertexCount = MeshGeom->getVertexCount();
    dim::vector3df* VertexBuffer = MemoryManager::createBuffer<dim::vector3df>(VertexCount, "Newton Hull Vertex Buffer");
    
    for (s = 0; s < MeshGeom->getMeshBufferCount(); ++s)
    {
        const video::MeshBuffer* Surface = MeshGeom->getMeshBuffer(s)->getReference();
        
        for (i = 0; i < Surface->getVertexCount(); ++i, ++j)
            VertexBuffer[j] = ScaleMatrix * Surface->getVertexCoord(i);
    }
    
    /* Create the convex hull collision and delete the buffer */
    setupCollision(NewtonCreateConvexHull(
        NewtonSimulator::getNewtonWorld(), VertexCount,
        &VertexBuffer[0].X, sizeof(dim::vector3df), OptimizeTolerance, 0, 0
    ));
    
    delete [] VertexBuffer;
}

void NewtonRigidBody::setupCollision(NewtonCollision* NtCollision)
{
    NtCollision_ = NtCollision;
    
    if (!NtCollision_)
    {
        io::Log::error("Could not create physics collision");
        return;
    }
    
    /* Create the physics body */
    dim::matrix4f InitMatrix;
    NtBody_ = NewtonCreateBody(NewtonSimulator::getNewtonWorld(), NtCollision_, InitMatrix.getArray());
    
    if (!NtBody_)
    {
        io::Log::error("Could not create physics body");
        return;
    }
    
    /* Disable by default auto sleep and freeze */
    setAutoSleep(false);
    
    /* Setup the callback procedures for transformation, force and torque */
    NewtonBodySetTransformCallback(NtBody_, PhysicsTransformCallback);
    NewtonBodySetForceAndTorqueCallback(NtBody_, PhysicsForceAndTorqueCallback);
    
    /* Setup user data */
    NewtonBodySetUserData(NtBody_, this);
    
    setGravity(dim::vector3df(0, -9.81f, 0));
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
