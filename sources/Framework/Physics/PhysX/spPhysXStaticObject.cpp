/*
 * PhysX static object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/PhysX/spPhysXStaticObject.hpp"

#ifdef SP_COMPILE_WITH_PHYSX


namespace sp
{
namespace physics
{


PhysXStaticObject::PhysXStaticObject(
    PxPhysics* PxDevice, PxCooking* PxCookDevice, PhysXMaterial* Material, scene::Mesh* Mesh) :
    BaseObject          (           ),
    PhysicsBaseObject   (           ),
    StaticPhysicsObject (Mesh       ),
    PhysXBaseObject     (Material   ),
    PxActor_            (0          )
{
    if (!PxDevice || !Mesh || !Material)
        return;
    
    /* Create dynamic rigid body */
    PxBaseActor_ = PxActor_ = PxDevice->createRigidStatic(
        //PxTransform(PxMat44(Mesh->getTransformation(true).getArray()))
        PxTransform(PxVec3(0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)))
    );
    
    if (!PxActor_)
    {
        io::Log::error("Could not create PhysX actor for static object");
        return;
    }
    
    /* Create base shape */
    //createMesh(PxDevice, PxCookDevice, Mesh);
    addShape(PxPlaneGeometry());
}
PhysXStaticObject::~PhysXStaticObject()
{
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
