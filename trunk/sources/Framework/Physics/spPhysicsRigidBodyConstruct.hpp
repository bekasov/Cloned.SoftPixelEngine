/*
 * Physics rigid body construction header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_RIGIDBODY_CONSTRUCT_H__
#define __SP_PHYSICS_RIGIDBODY_CONSTRUCT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PHYSICS


#include "Base/spDimensionVector3D.hpp"


namespace sp
{
namespace physics
{


//! Rigid body types.
enum ERigidBodies
{
    RIGIDBODY_BOX,              //!< Normal box.
    RIGIDBODY_CONE,             //!< Normal cone.
    RIGIDBODY_SPHERE,           //!< Normal sphere.
    RIGIDBODY_CYLINDER,         //!< Normal cylinder.
    RIGIDBODY_CHAMFERCYLINDER,  //!< Cylinder with smooth edges at top and bottom.
    RIGIDBODY_CAPSULE,          //!< Cylinder with half spheres at top and bottom.
    RIGIDBODY_MESH,             //!< Universal mesh object.
    RIGIDBODY_CUSTOM,           //!< Custom rigid body. Used for inherited classes of physics::RigidBody.
};


//! Rigid body construction data.
struct SRigidBodyConstruction
{
    SRigidBodyConstruction(f32 InitRadius = 0.5f, f32 InitHeight = 0.5f) :
        Size    (0.5f       ),
        Radius  (InitRadius ),
        Height  (InitHeight )
    {
    }
    SRigidBodyConstruction(
        const dim::vector3df InitSize, f32 InitRadius = 0.5f, f32 InitHeight = 0.5f) :
        Size    (InitSize   ),
        Radius  (InitRadius ),
        Height  (InitHeight )
    {
    }
    ~SRigidBodyConstruction()
    {
    }
    
    /* Members */
    dim::vector3df Size;
    f32 Radius, Height;
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
