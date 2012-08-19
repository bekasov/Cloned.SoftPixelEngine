/*
 * Newton static object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_NEWTON_STATICOBJECT_H__
#define __SP_PHYSICS_NEWTON_STATICOBJECT_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NEWTON


#include "Framework/Physics/spPhysicsStaticObject.hpp"
#include "Framework/Physics/Newton/spNewtonBaseObject.hpp"


namespace sp
{
namespace physics
{


class SP_EXPORT NewtonStaticObject : public StaticPhysicsObject, public NewtonBaseObject
{
    
    public:
        
        NewtonStaticObject(scene::Mesh* Mesh);
        virtual ~NewtonStaticObject();
        
    protected:
        
        /* Functions */
        
        void createMesh();
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
