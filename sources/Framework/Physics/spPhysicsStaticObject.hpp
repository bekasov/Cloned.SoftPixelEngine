/*
 * Physics static object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_STATICOBJECT_H__
#define __SP_PHYSICS_STATICOBJECT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PHYSICS


#include "SceneGraph/spSceneMesh.hpp"
#include "Framework/Physics/spPhysicsBaseObject.hpp"


namespace sp
{
namespace physics
{


class SP_EXPORT StaticPhysicsObject : virtual public PhysicsBaseObject
{
    
    public:
        
        virtual ~StaticPhysicsObject()
        {
        }
        
    protected:
        
        StaticPhysicsObject(scene::Mesh* Mesh) :
            //PhysicsBaseObject   (       ),
            Mesh_               (Mesh   )
        {
        }
        
        /* Members */
        
        scene::Mesh* Mesh_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
