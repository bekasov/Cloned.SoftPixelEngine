/*
 * Bullet static object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_BULLET_STATICOBJECT_H__
#define __SP_PHYSICS_BULLET_STATICOBJECT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_BULLET


#include "Framework/Physics/spPhysicsStaticObject.hpp"
#include "Framework/Physics/Bullet/spBulletBaseObject.hpp"


namespace sp
{
namespace physics
{


class SP_EXPORT BulletStaticObject : public StaticPhysicsObject, public BulletBaseObject
{
    
    public:
        
        BulletStaticObject(scene::Mesh* Mesh);
        virtual ~BulletStaticObject();
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
