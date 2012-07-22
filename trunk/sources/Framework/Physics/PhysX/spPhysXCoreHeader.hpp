/*
 * PhysX core header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_PHYSX_CORE_H__
#define __SP_PHYSICS_PHYSX_CORE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PHYSX


#include <PxPhysicsAPI.h>

using namespace physx;

#define VecPx2Sp(v) dim::vector3df(v.x, v.y, v.z)
#define VecSp2Px(v) PxVec3(v.X, v.Y, v.Z)


#endif

#endif



// ================================================================================
