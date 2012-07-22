/*
 * Bullet rigid body file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/Bullet/spBulletRigidBody.hpp"

#ifdef SP_COMPILE_WITH_BULLET


namespace sp
{
namespace physics
{


BulletRigidBody::BulletRigidBody(
    const ERigidBodies Type, scene::SceneNode* RootNode, const SRigidBodyConstruction &Construct) :
    BaseObject          (               ),
    PhysicsBaseObject   (               ),
    RigidBody           (Type, Construct),
    BulletBaseObject    (               )
{
    createBody(Mass_, true, RootNode, Type, Construct);
}
BulletRigidBody::~BulletRigidBody()
{
}

void BulletRigidBody::setGravity(const dim::vector3df &Gravity)
{
    DynamicPhysicsObject::setGravity(Gravity);
    if (BtBody_)
        BtBody_->setGravity(btVector3(Gravity_.X, Gravity_.Y, Gravity_.Z));
}

void BulletRigidBody::setMass(f32 Mass)
{
    DynamicPhysicsObject::setMass(Mass);
    
    if (BtBody_)
    {
        btVector3 LocalInertia(0, 0, 0);
        BtShape_->calculateLocalInertia(Mass_, LocalInertia);
        LocalInertia_ = dim::vector3df(LocalInertia.x(), LocalInertia.y(), LocalInertia.z());
        
        BtBody_->setMassProps(Mass_, LocalInertia);
    }
}

void BulletRigidBody::setAutoSleep(bool Enable)
{
    if (BtBody_)
        BtBody_->setActivationState(Enable ? WANTS_DEACTIVATION : DISABLE_DEACTIVATION);
}

void BulletRigidBody::setMassCenter(const dim::vector3df &LocalPoint)
{
    if (BtBody_)
        BtBody_->setMassProps(Mass_, btVector3(LocalPoint.X, LocalPoint.Y, LocalPoint.Z));
    LocalInertia_ = LocalPoint;
}
dim::vector3df BulletRigidBody::getMassCenter() const
{
    return LocalInertia_;
}

void BulletRigidBody::addVelocity(const dim::vector3df &Direction)
{
    if (BtBody_)
    {
        BtBody_->applyCentralImpulse(
            btVector3(Direction.X * Mass_, Direction.Y * Mass_, Direction.Z * Mass_)
        );
    }
}
void BulletRigidBody::setVelocity(const dim::vector3df &Direction)
{
    //todo
}
dim::vector3df BulletRigidBody::getVelocity() const
{
    return 0.0f; //todo
}

void BulletRigidBody::addImpulse(const dim::vector3df &Direction, const dim::vector3df &PivotPoint)
{
    if (BtBody_)
    {
        BtBody_->applyImpulse(
            btVector3(Direction.X * Mass_, Direction.Y * Mass_, Direction.Z * Mass_),
            btVector3(PivotPoint.X, PivotPoint.Y, PivotPoint.Z)
        );
    }
}
void BulletRigidBody::setForce(const dim::vector3df &Direction)
{
    //todo
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
