/*
 * Mesh animation header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESH_ANIMATION_H__
#define __SP_MESH_ANIMATION_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/Animation/spAnimation.hpp"


namespace sp
{
namespace scene
{


//! Extension class for mesh based animations (MorphTargetAnimation and SkeletalAnimation).
class SP_EXPORT MeshAnimation : public Animation
{
    
    public:
        
        virtual ~MeshAnimation();
        
    protected:
        
        MeshAnimation(const EAnimationTypes Type);
        
        /* === Functions === */
        
        //! Returns true if the specified mesh object is inside a view frustum of any camera.
        virtual bool checkFrustumCulling(scene::Mesh* Object) const;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
