/*
 * Base collision physics object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spBaseCollisionPhysicsObject.hpp"
#include "SceneGraph/Collision/spCollisionNode.hpp"
#include "Base/spTimer.hpp"


namespace sp
{
namespace scene
{


BaseCollisionPhysicsObject::BaseCollisionPhysicsObject() :
    Mass_       (1.0f   ),
    Friction_   (0.25f  )
{
}
BaseCollisionPhysicsObject::~BaseCollisionPhysicsObject()
{
}

void BaseCollisionPhysicsObject::arrestGravityForces()
{
    //Velocity_.Y *= 0.1f; //!!!
    //Velocity_.Y = 0.0f;
}

void BaseCollisionPhysicsObject::reduceVelocity(const dim::vector3df &FactorVec)
{
    Velocity_ *= FactorVec;
}


/*
 * ======= Protected: =======
 */

void BaseCollisionPhysicsObject::integrate(CollisionNode* Node)
{
    if (!Node)
        return;
    
    const f32 dt = io::Timer::getGlobalSpeed();
    
    /* Acceleration is force / mass */
    dim::vector3df Acceleration(Gravity_);
    Acceleration += Force_;
    Acceleration /= Mass_;
    
    /* Apply velocity and forces */
    Node->translate(Velocity_ * dt);
    Velocity_ += (Acceleration / Mass_);// * dt;
    
    /* Reset forces */
    Force_ = 0.0f;
}

void BaseCollisionPhysicsObject::applyFriction()
{
    /* Reduce velocity by friction */
    Velocity_ *= (1.0f - Friction_);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
