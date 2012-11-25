/*
 * Animation bone file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spAnimationJoint.hpp"

#include <vector>
#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


AnimationJoint::AnimationJoint(
    const Transformation &OriginTransform, const io::stringc Name) :
    BaseObject      (Name           ),
    isEnable_       (true           ),
    Parent_         (0              ),
    OriginTransform_(OriginTransform),
    Transform_      (OriginTransform)
{
}
AnimationJoint::~AnimationJoint()
{
}

dim::matrix4f AnimationJoint::getGlobalTransformation() const
{
    if (Parent_)
        return Parent_->getGlobalTransformation() * Transform_.getMatrix();
    return Transform_.getMatrix();
}


/*
 * ======= Protected: =======
 */

void AnimationJoint::transformVertices(
    scene::Mesh* MeshObj, dim::matrix4f BaseMatrix, bool useTangentSpace)
{
    /* Update the transformation */
    BaseMatrix *= Transform_.getMatrix();
    
    const dim::matrix4f WorldMatrix(BaseMatrix * OriginMatrix_);
    const dim::matrix4f NormalMatrix(WorldMatrix.getRotationMatrix());
    
    /* Transform each influenced vertex */
    u32 PrevSurface = static_cast<u32>(-1);
    video::MeshBuffer* Surf = 0;
    
    foreach (const SVertexGroup &Vert, VertexGroups_)
    {
        /* Get current mesh buffer */
        if (Vert.Surface != PrevSurface || !Surf)
        {
            Surf = MeshObj->getMeshBuffer(Vert.Surface);
            
            if (!Surf)
                continue;
            
            PrevSurface = Vert.Surface;
        }
        
        /* Setup new vertex coordinate and normal */
        Surf->setVertexCoord(
            Vert.Index,
            Surf->getVertexCoord(Vert.Index) + (WorldMatrix * Vert.Position) * Vert.Weight
        );
        Surf->setVertexNormal(
            Vert.Index,
            Surf->getVertexNormal(Vert.Index) + (NormalMatrix * Vert.Normal) * Vert.Weight
        );
        
        /*if (useTangentSpace)
        {
            //todo
        }*/
    }
    
    /* Transform children vertices */
    foreach (AnimationJoint* Child, Children_)
        Child->transformVertices(MeshObj, BaseMatrix, useTangentSpace);
}

bool AnimationJoint::checkParentIncest(AnimationJoint* Joint) const
{
    if (!Parent_)
        return true;
    if (Parent_ == Joint)
        return false;
    return Parent_->checkParentIncest(Joint);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
