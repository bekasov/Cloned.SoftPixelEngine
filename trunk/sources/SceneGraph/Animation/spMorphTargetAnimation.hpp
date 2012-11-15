/*
 * Morph target animation header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MORPHTARGET_ANIMATION_H__
#define __SP_MORPHTARGET_ANIMATION_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/Animation/spMeshAnimation.hpp"
#include "SceneGraph/Animation/spAnimationBaseStructures.hpp"

#include <vector>


namespace sp
{
namespace scene
{


/**
Morph-Target animations interpolate each vertex of its mesh. This is a technique used by the games "Quake 1", "Quake 2" and "Quake III Arena".
The 'SoftPixel Engine' supports this animation model innately for the MD2 and MD3 file formats.
\ingroup group_animation
*/
class SP_EXPORT MorphTargetAnimation : public MeshAnimation
{
    
    public:
        
        MorphTargetAnimation();
        virtual ~MorphTargetAnimation();
        
        /* === Functions === */
        
        /**
        Adds a sequence of keyframes for the specified vertex. Because of performance efficiency you have to
        add all keyframes for each vertex at once.
        \param Surface: Specifies the MeshBuffer object to which the vertex belongs.
        \param Index: Specifies the vertex index.
        \param Keyframes: Specifies the container with all keyframe data (coordinate and normal).
        */
        void addKeyframeSequence(video::MeshBuffer* Surface, u32 Index, const std::vector<SVertexKeyframe> &Keyframes);
        
        //! Remvoes all keyframes of the specifies vertex.
        void removeKeyframeSequence(video::MeshBuffer* Surface, u32 Index);
        
        void clearKeyframes();
        
        virtual void setupManualAnimation(SceneNode* Node);
        
        /**
        Updates the morph target animation. If the animation is playing all keyframes will be performed.
        \param Object: This must be a Mesh scene node. Otherwise the function will do nothing.
        */
        virtual void updateAnimation(SceneNode* Node);
        
        virtual u32 getKeyframeCount() const;
        
        virtual void interpolate(u32 IndexFrom, u32 IndexTo, f32 Interpolation);
        
        virtual void copy(const Animation* Other);
        
    private:
        
        /* === Members === */
        
        std::list<SMorphTargetVertex> Vertices_;
        
        u32 MaxKeyframe_;
        
        bool isCulling_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
