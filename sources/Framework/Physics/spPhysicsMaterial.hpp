/*
 * Physics material header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_MATERIAL_H__
#define __SP_PHYSICS_MATERIAL_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PHYSICS


namespace sp
{
namespace physics
{


class SP_EXPORT PhysicsMaterial
{
    
    public:
        
        virtual ~PhysicsMaterial()
        {
        }
        
        virtual void setStaticFriction(f32 Factor) = 0;
        virtual f32 getStaticFriction() const = 0;
        
        virtual void setDynamicFriction(f32 Factor) = 0;
        virtual f32 getDynamicFriction() const = 0;
        
        virtual void setRestitution(f32 Factor) = 0;
        virtual f32 getRestitution() const = 0;
        
    protected:
        
        PhysicsMaterial()
        {
        }
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
