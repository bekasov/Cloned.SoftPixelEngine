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
    const KeyframeTransformation &OriginTransformation, const io::stringc Name) :
    BaseObject              (Name                   ),
    isEnable_               (true                   ),
    Parent_                 (0                      ),
    OriginTransformation_   (OriginTransformation   ),
    Transformation_         (OriginTransformation   )
{
}
AnimationJoint::~AnimationJoint()
{
}

dim::matrix4f AnimationJoint::getGlobalTransformation() const
{
    if (Parent_)
        return Parent_->getGlobalTransformation() * Transformation_.getMatrix();
    return Transformation_.getMatrix();
}


/*
 * ======= Protected: =======
 */

void AnimationJoint::transformVertices(dim::matrix4f BaseMatrix, bool useTangentSpace)
{
    /* Update the transformation */
    BaseMatrix *= Transformation_.getMatrix();
    
    const dim::matrix4f WorldMatrix(BaseMatrix * OriginMatrix_);
    const dim::matrix4f NormalMatrix(WorldMatrix.getRotationMatrix());
    
    /* Transform each influenced vertex */
    foreach (const SVertexGroup &Vert, VertexGroups_)
    {
        /* Setup new vertex coordinate and normal */
        Vert.Surface->setVertexCoord(
            Vert.Index,
            Vert.Surface->getVertexCoord(Vert.Index) + (WorldMatrix * Vert.Position) * Vert.Weight
        );
        Vert.Surface->setVertexNormal(
            Vert.Index,
            Vert.Surface->getVertexNormal(Vert.Index) + (NormalMatrix * Vert.Normal) * Vert.Weight
        );
        
        /*if (useTangentSpace)
        {
            //todo
        }*/
    }
    
    /* Transform children vertices */
    foreach (AnimationJoint* Child, Children_)
        Child->transformVertices(BaseMatrix, useTangentSpace);
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
