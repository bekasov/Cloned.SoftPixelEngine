/*
 * PhysX material file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/PhysX/spPhysXMaterial.hpp"

#ifdef SP_COMPILE_WITH_PHYSX


#include "Base/spInputOutputLog.hpp"


namespace sp
{
namespace physics
{


PhysXMaterial::PhysXMaterial(
    PxPhysics* PxDevice, f32 StaticFriction, f32 DynamicFriction, f32 Restitution) :
    PhysicsMaterial (   ),
    PxMaterial_     (0  )
{
    if (PxDevice)
    {
        PxMaterial_ = PxDevice->createMaterial(StaticFriction, DynamicFriction, Restitution);
        if (!PxMaterial_)
            io::Log::error("Could not create PhysX material");
    }
}
PhysXMaterial::~PhysXMaterial()
{
    if (PxMaterial_)
        PxMaterial_->release();
}

void PhysXMaterial::setStaticFriction(f32 Factor)
{
    PxMaterial_->setStaticFriction(Factor);
}
f32 PhysXMaterial::getStaticFriction() const
{
    return PxMaterial_->getStaticFriction();
}

void PhysXMaterial::setDynamicFriction(f32 Factor)
{
    PxMaterial_->setDynamicFriction(Factor);
}
f32 PhysXMaterial::getDynamicFriction() const
{
    return PxMaterial_->getDynamicFriction();
}

void PhysXMaterial::setRestitution(f32 Factor)
{
    PxMaterial_->setRestitution(Factor);
}
f32 PhysXMaterial::getRestitution() const
{
    return PxMaterial_->getRestitution();
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
