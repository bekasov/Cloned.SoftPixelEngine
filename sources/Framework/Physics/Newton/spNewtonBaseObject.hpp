/*
 * Newton base object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_NEWTON_BASEOBJECT_H__
#define __SP_PHYSICS_NEWTON_BASEOBJECT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NEWTON


#include "Base/spDimension.hpp"
#include "Framework/Physics/spPhysicsBaseObject.hpp"


class NewtonCollision;
class NewtonBody;

namespace sp
{
namespace physics
{


class SP_EXPORT NewtonBaseObject : virtual public PhysicsBaseObject
{
    
    public:
        
        virtual ~NewtonBaseObject();
        
        /* Functions */
        
        virtual void setTransformation(const dim::matrix4f &Transformation);
        virtual dim::matrix4f getTransformation() const;
        
        /* Inline functions */
        
        //! Returns the pointer to the NewtonCollision object. Use this when you want to extend the physics system.
        inline NewtonCollision* getCollision() const
        {
            return NtCollision_;
        }
        //! Returns the pointer to the NewtonBody object. Use this when you want to extend the physics system.
        inline NewtonBody* getBody() const
        {
            return NtBody_;
        }
        
    protected:
        
        friend class NewtonDynamicsJoint;
        
        /* Functions */
        
        NewtonBaseObject();
        
        /* Members */
        
        NewtonCollision* NtCollision_;
        NewtonBody* NtBody_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
