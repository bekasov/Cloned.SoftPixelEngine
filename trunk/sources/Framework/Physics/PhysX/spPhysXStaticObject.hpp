/*
 * PhysX static object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_PHYSX_STATICOBJECT_H__
#define __SP_PHYSICS_PHYSX_STATICOBJECT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PHYSX


#include "Framework/Physics/spPhysicsStaticObject.hpp"
#include "Framework/Physics/PhysX/spPhysXBaseObject.hpp"


namespace sp
{
namespace physics
{


class SP_EXPORT PhysXStaticObject : public StaticPhysicsObject, public PhysXBaseObject
{
    
    public:
        
        PhysXStaticObject(
            PxPhysics* PxDevice, PxCooking* PxCookDevice, PhysXMaterial* Material, scene::Mesh* Mesh
        );
        virtual ~PhysXStaticObject();
        
    protected:
        
        friend class PhysXSimulator;
        
        /* === Members === */
        
        PxRigidStatic* PxActor_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
