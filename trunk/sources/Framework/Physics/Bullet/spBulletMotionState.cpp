/*
 * Bullet motion state file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/Bullet/spBulletMotionState.hpp"

#ifdef SP_COMPILE_WITH_BULLET


namespace sp
{
namespace physics
{


BulletMotionState::BulletMotionState(const btTransform &OriginTrans, scene::SceneNode* Node) :
    Trans_  (OriginTrans),
    Node_   (Node       )
{
}
BulletMotionState::~BulletMotionState()
{
}

void BulletMotionState::getWorldTransform(btTransform &WorldTrans) const
{
    WorldTrans = Trans_;
}
void BulletMotionState::setWorldTransform(const btTransform &WorldTrans)
{
    Trans_ = WorldTrans;
    
    if (Node_)
    {
        const btVector3 Pos = WorldTrans.getOrigin();
        Node_->setPosition(dim::vector3df(Pos.x(), Pos.y(), Pos.z()));
        
        const btQuaternion Rot = WorldTrans.getRotation();
        Node_->setRotationMatrix(dim::quaternion(Rot.x(), Rot.y(), Rot.z(), Rot.w()).getMatrix());
    }
}

dim::matrix4f BulletMotionState::getMatrix(const btTransform &Trans)
{
    dim::matrix4f Mat;
    Trans.getOpenGLMatrix(Mat.getArray());
    return Mat;
}
btTransform BulletMotionState::getTransform(const dim::matrix4f &Matrix)
{
    btTransform Trans;
    Trans.setFromOpenGLMatrix(Matrix.getArray());
    return Trans;
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
