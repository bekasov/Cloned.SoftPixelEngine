/*
 * Animation joint group file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spAnimationJointGroup.hpp"


namespace sp
{
namespace scene
{


AnimationJointGroup::AnimationJointGroup()
{
}
AnimationJointGroup::~AnimationJointGroup()
{
}

void AnimationJointGroup::clearJoints()
{
    JointKeyframesRef_.clear();
    Playback_.stop();
}


} // /namespace scene

} // /namespace sp



// ================================================================================
