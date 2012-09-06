/*
 * PhysX rigid body file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/PhysX/spPhysXRigidBody.hpp"

#ifdef SP_COMPILE_WITH_PHYSX


namespace sp
{
namespace physics
{


PhysXRigidBody::PhysXRigidBody(
    PxPhysics* PxDevice, PhysXMaterial* Material, const ERigidBodies Type,
    scene::SceneNode* RootNode, const SRigidBodyConstruction &Construct) :
    BaseObject          (               ),
    PhysicsBaseObject   (               ),
    RigidBody           (Type, Construct),
    PhysXBaseObject     (Material       ),
    PxActor_            (0              )
{
    if (!PxDevice || !RootNode || !Material)
        throw io::stringc("Invalid arguments for rigid body");
    
    /* Create dynamic rigid body */
    PxBaseActor_ = PxActor_ = PxDevice->createRigidDynamic(
        PxTransform(PxMat44(RootNode->getTransformMatrix(true).getArray()))
    );
    
    PxBaseActor_->userData = RootNode;
    
    if (!PxActor_)
        throw io::stringc("Could not create PhysX actor for rigid body");
    
    /* Create base shape */
    switch (Type)
    {
        case RIGIDBODY_BOX:         createBox       (Construct); break;
        case RIGIDBODY_SPHERE:      createSphere    (Construct); break;
        case RIGIDBODY_CAPSULE:     createCapsule   (Construct); break;
        default:
            throw io::stringc("Unsupported rigid body type");
    }
    
    /* Initialize root node and actor */
    setRootNode(RootNode);
    
    setMass(1.0f);
}
PhysXRigidBody::~PhysXRigidBody()
{
}

void PhysXRigidBody::setMass(f32 Mass)
{
    if (PxActor_)
        PxRigidBodyExt::setMassAndUpdateInertia(*PxActor_, Mass);
}

void PhysXRigidBody::setMassCenter(const dim::vector3df &LocalPoint)
{
    //todo
}
dim::vector3df PhysXRigidBody::getMassCenter() const
{
    return 0.0f; //todo
}

void PhysXRigidBody::addVelocity(const dim::vector3df &Direction)
{
    setVelocity(getVelocity() + Direction);
}
void PhysXRigidBody::setVelocity(const dim::vector3df &Direction)
{
    PxActor_->setLinearVelocity(VecSp2Px(Direction));
}
dim::vector3df PhysXRigidBody::getVelocity() const
{
    PxVec3 Velocity(PxActor_->getLinearVelocity());
    return VecPx2Sp(Velocity);
}

void PhysXRigidBody::addImpulse(const dim::vector3df &Direction, const dim::vector3df &PivotPoint)
{
    PxRigidBodyExt::addForceAtPos(*PxActor_, VecSp2Px(Direction), VecSp2Px(PivotPoint));
}
void PhysXRigidBody::setForce(const dim::vector3df &Direction)
{
    PxActor_->addForce(VecSp2Px(Direction));
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
