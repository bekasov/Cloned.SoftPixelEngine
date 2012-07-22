/*
 * Morph target animation file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spMorphTargetAnimation.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


MorphTargetAnimation::MorphTargetAnimation() :
    MeshAnimation   (ANIMATION_MORPHTARGET  ),
    MaxKeyframe_    (0                      ),
    isCulling_      (false                  )
{
}
MorphTargetAnimation::~MorphTargetAnimation()
{
}

void MorphTargetAnimation::addKeyframeSequence(
    video::MeshBuffer* Surface, u32 Index, const std::vector<SVertexKeyframe> &Keyframes)
{
    Vertices_.push_back(SMorphTargetVertex(Surface, Index, Keyframes));
    
    if (MaxKeyframe_ < Keyframes.size())
        MaxKeyframe_ = Keyframes.size();
}
void MorphTargetAnimation::removeKeyframeSequence(video::MeshBuffer* Surface, u32 Index)
{
    MaxKeyframe_ = 0;
    
    for (std::list<SMorphTargetVertex>::iterator it = Vertices_.begin(); it != Vertices_.end();)
    {
        if (it->Surface == Surface && it->Index == Index)
            it = Vertices_.erase(it);
        else
        {
            if (MaxKeyframe_ < it->Keyframes.size())
                MaxKeyframe_ = it->Keyframes.size();
            ++it;
        }
    }
}

void MorphTargetAnimation::clearKeyframes()
{
    Vertices_.clear();
    MaxKeyframe_ = 0;
}

void MorphTargetAnimation::setupManualAnimation(SceneNode* Node)
{
    isCulling_ = true;
}

void MorphTargetAnimation::updateAnimation(scene::SceneNode* Node)
{
    /* Get valid mesh object */
    if (!Node || Node->getType() != scene::NODE_MESH || !playing())
        return;
    
    Mesh* Object = static_cast<Mesh*>(Node);
    
    /* Update playback process */
    isCulling_ = checkFrustumCulling(Object);
    
    updatePlayback(getSpeed());
    
    /* Update the vertex transformation if the object is inside a view frustum of any camera */
    if (isCulling_)
        Object->updateVertexBuffer();
}

u32 MorphTargetAnimation::getKeyframeCount() const
{
    return MaxKeyframe_;
}

void MorphTargetAnimation::interpolate(u32 IndexFrom, u32 IndexTo, f32 Interpolation)
{
    if (!isCulling_)
        return;
    
    foreach (SMorphTargetVertex &Vert, Vertices_)
    {
        if (IndexFrom < Vert.Keyframes.size() && IndexTo < Vert.Keyframes.size())
        {
            /* Get frame references */
            SVertexKeyframe* From   = &Vert.Keyframes[IndexFrom];
            SVertexKeyframe* To     = &Vert.Keyframes[IndexTo];
            
            /* Update transformation for vertex coordinate and normal */
            Vert.Surface->setVertexCoord(
                Vert.Index, From->Position + (To->Position - From->Position) * Interpolation
            );
            Vert.Surface->setVertexNormal(
                Vert.Index, From->Normal + (To->Normal - From->Normal) * Interpolation
            );
        }
    }
}


} // /namespace scene

} // /namespace sp



// ================================================================================
