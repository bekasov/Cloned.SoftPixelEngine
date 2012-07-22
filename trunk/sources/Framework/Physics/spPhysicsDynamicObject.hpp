/*
 * Physics dynamic object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_DYNAMICOBJECT_H__
#define __SP_PHYSICS_DYNAMICOBJECT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PHYSICS


#include "Framework/Physics/spPhysicsBaseObject.hpp"

#include <boost/function.hpp>


namespace sp
{
namespace physics
{


class DynamicPhysicsObject;

typedef boost::function<void (DynamicPhysicsObject* PhysicsObject)> PhysicsGravityCallback;


class SP_EXPORT DynamicPhysicsObject : virtual public PhysicsBaseObject
{
    
    public:
        
        virtual ~DynamicPhysicsObject();
        
        /* === Functions === */
        
        //! Sets the individual gravity for this rigid body.
        virtual void setGravity(const dim::vector3df &Gravity);
        
        //! Sets the body's mass factor (by default 25.0).
        virtual void setMass(f32 Mass);
        
        //! Enables or disables the auto sleep and freeze mode. By default disabled. This can be used for optimization purposes.
        virtual void setAutoSleep(bool Enable);
        
        //! Sets the gravity callback. By default 0.
        virtual void setGravityCallback(const PhysicsGravityCallback &Callback);
        
        //! Sets the balance point (or rather center of mass) of the rigid body in object space. By default (0 | 0 | 0).
        virtual void setMassCenter(const dim::vector3df &LocalPoint) = 0;
        //! Returns the balance point (or rather center of mass).
        virtual dim::vector3df getMassCenter() const = 0;
        
        virtual void addVelocity(const dim::vector3df &Direction) = 0;
        virtual void setVelocity(const dim::vector3df &Direction) = 0;
        virtual dim::vector3df getVelocity() const = 0;
        
        /**
        Adds the specified impulse to the body. E.g. use this when you shoot at a box and the box shall throw away.
        \param Direction: Specifies the direction in which the impulse is to be pointed.
        \param PivotPoint: Specifies the global point where the impulse is to be performed.
        */
        virtual void addImpulse(const dim::vector3df &Direction, const dim::vector3df &PivotPoint) = 0;
        
        /**
        Sets the force for this object. This is actually only used to apply the g-force.
        \note This function can only be used internally. Otherwise it has no effect.
        */
        virtual void setForce(const dim::vector3df &Direction) = 0;
        
        /* === Inline functions === */
        
        //! Returns the individual gravity for this rigid body.
        inline dim::vector3df getGravity() const
        {
            return Gravity_;
        }
        //! Returns the body's mass factor.
        inline f32 getMass() const
        {
            return Mass_;
        }
        //! Returns true if the auto sleep and freeze mode is enabled.
        inline bool getAutoSleep() const
        {
            return AutoSleep_;
        }
        //! Returns the gravity callback.
        inline PhysicsGravityCallback getGravityCallback() const
        {
            return GravityCallback_;
        }
        
    protected:
        
        DynamicPhysicsObject();
        
        /* === Members === */
        
        dim::vector3df Gravity_;
        f32 Mass_;
        bool AutoSleep_;
        
        PhysicsGravityCallback GravityCallback_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
