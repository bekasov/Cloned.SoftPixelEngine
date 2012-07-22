/*
 * Bullet static object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/Bullet/spBulletStaticObject.hpp"

#ifdef SP_COMPILE_WITH_BULLET


#include "Framework/Physics/spPhysicsRigidBodyConstruct.hpp"


namespace sp
{
namespace physics
{


BulletStaticObject::BulletStaticObject(scene::Mesh* Mesh) :
    BaseObject          (       ),
    PhysicsBaseObject   (       ),
    StaticPhysicsObject (Mesh   ),
    BulletBaseObject    (       )
{
    createBody(0.0f, false, Mesh, RIGIDBODY_MESH, SRigidBodyConstruction(dim::vector3df(20, 1, 20)));
}
BulletStaticObject::~BulletStaticObject()
{
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
