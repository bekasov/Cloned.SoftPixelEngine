/*
 * PhysX material header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_PHYSX_MATERIAL_H__
#define __SP_PHYSICS_PHYSX_MATERIAL_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PHYSX


#include "Framework/Physics/PhysX/spPhysXCoreHeader.hpp"
#include "Framework/Physics/spPhysicsMaterial.hpp"


namespace sp
{
namespace physics
{


class SP_EXPORT PhysXMaterial : public PhysicsMaterial
{
    
    public:
        
        PhysXMaterial(
            PxPhysics* PxDevice,
            f32 StaticFriction, f32 DynamicFriction, f32 Restitution
        );
        virtual ~PhysXMaterial();
        
        virtual void setStaticFriction(f32 Factor);
        virtual f32 getStaticFriction() const;
        
        virtual void setDynamicFriction(f32 Factor);
        virtual f32 getDynamicFriction() const;
        
        virtual void setRestitution(f32 Factor);
        virtual f32 getRestitution() const;
        
    protected:
        
        friend class PhysXBaseObject;
        
        /* Members */
        
        PxMaterial* PxMaterial_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
