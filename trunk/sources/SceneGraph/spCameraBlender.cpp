/*
 * Blender camera file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#if 0 //!unfinished!

#include "SceneGraph/spCameraBlender.hpp"


namespace sp
{
namespace scene
{


BlenderCamera::BlenderCamera() : Camera()
{
    Root_ = new scene::SceneNode(scene::NODE_BASICNODE);
    setParent(Root_);
    
    Distance_ = 1.0f;
}
BlenderCamera::~BlenderCamera()
{
    MemoryManager::deleteMemory(Root_);
}


} // /namespace scene

} // /namespace sp

#endif



// ================================================================================
