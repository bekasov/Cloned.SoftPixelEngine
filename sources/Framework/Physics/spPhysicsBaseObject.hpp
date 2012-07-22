/*
 * Physics base object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_BASEOBJECT_H__
#define __SP_PHYSICS_BASEOBJECT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PHYSICS


#include "Base/spDimension.hpp"
#include "Base/spBaseObject.hpp"


namespace sp
{
namespace physics
{


class SP_EXPORT PhysicsBaseObject : virtual public BaseObject
{
    
    public:
        
        virtual ~PhysicsBaseObject();
        
        /* Functions */
        
        virtual void setTransformation(const dim::matrix4f &Transformation) = 0;
        virtual dim::matrix4f getTransformation() const = 0;
        
        virtual void setPosition(const dim::vector3df &Position);
        virtual dim::vector3df getPosition() const;
        
        virtual void setRotation(const dim::matrix4f &Rotation);
        virtual dim::matrix4f getRotation() const;
        
    protected:
        
        PhysicsBaseObject();
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
