/*
 * Blender camera header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#if 0 //!unfinished!

#ifndef __SP_SCENE_CAMERA_BLENDER_H__
#define __SP_SCENE_CAMERA_BLENDER_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/spSceneCamera.hpp"


namespace sp
{
namespace scene
{


class SP_EXPORT BlenderCamera : public Camera
{
    
    public:
        
        BlenderCamera();
        virtual ~BlenderCamera();
        
        /* Functions */
        
        /* Inline functions */
        
        inline scene::SceneNode* getRootNode() const
        {
            return Root_;
        }
        
    protected:
        
        /* Functions */
        
        
        
        /* Members */
        
        scene::SceneNode* Root_;
        f32 Distance_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
