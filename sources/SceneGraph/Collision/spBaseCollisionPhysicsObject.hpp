/*
 * Base collision physics object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_BASE_PHYSICS_OBJECT_H__
#define __SP_COLLISION_BASE_PHYSICS_OBJECT_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector3D.hpp"


namespace sp
{
namespace scene
{


class CollisionNode;

/**
This is the base class for simple collision-physics objects. This does not include actual physics simulation.
It only holds gravity, mass and force information which will affect the object's position.
\ingroup group_collision
*/
class SP_EXPORT BaseCollisionPhysicsObject
{
    
    public:
        
        virtual ~BaseCollisionPhysicsObject();
        
        /* === Functions === */
        
        void arrestGravityForces();
        void reduceVelocity(const dim::vector3df &FactorVec);
        
        /* === Inline functions === */
        
        //! Sets the gravity force which will be applied to the object.
        inline void setGravity(const dim::vector3df &Gravity)
        {
            Gravity_ = Gravity;
        }
        //! Returns the gravity used for this object. By default (0, 0, 0).
        inline dim::vector3df getGravity() const
        {
            return Gravity_;
        }
        
        /**
        Sets the object's mass. By default 1.0.
        \param Mass: Specifies the new mass. This value must be greater than 0.0.
        */
        inline void setMass(f32 Mass)
        {
            Mass_ = math::Max(math::ROUNDING_ERROR, Mass);
        }
        //! Returns the object's mass. By default 1.0.
        inline f32 getMass() const
        {
            return Mass_;
        }
        
        //! Sets the friction factor. This must be in the range [0.0 .. 1.0].
        inline void setFriction(f32 Friction)
        {
            Friction_ = math::MinMax(Friction, 0.0f, 1.0f);
        }
        //! Returns the friction factor. By default 0.5.
        inline f32 getFriction() const
        {
            return Friction_;
        }
        
        inline void setForce(const dim::vector3df &Force)
        {
            Force_ = Force;
        }
        inline void addForce(const dim::vector3df &Force)
        {
            Force_ += Force;
        }
        inline dim::vector3df getForce() const
        {
            return Force_;
        }
        
        inline void setVelocity(const dim::vector3df &Velocity)
        {
            Velocity_ = Velocity;
        }
        inline void addVelocity(const dim::vector3df &Velocity)
        {
            Velocity_ += Velocity;
        }
        inline dim::vector3df getVelocity() const
        {
            return Velocity_;
        }
        
    protected:
        
        /* === Functions === */
        
        BaseCollisionPhysicsObject();
        
        //! Compute physics integration.
        void integrate(CollisionNode* Node);
        
        void applyFriction();
        
    private:
        
        /* === Members === */
        
        dim::vector3df Gravity_;
        dim::vector3df Force_;
        dim::vector3df Velocity_;
        f32 Mass_;
        
        f32 Friction_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
