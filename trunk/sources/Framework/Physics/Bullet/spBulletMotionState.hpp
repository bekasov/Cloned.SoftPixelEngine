/*
 * Bullet motion state header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PHYSICS_BULLET_MOTIONSTATE_H__
#define __SP_PHYSICS_BULLET_MOTIONSTATE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_BULLET


#include "Base/spDimension.hpp"
#include "SceneGraph/spSceneNode.hpp"

#include <btBulletDynamicsCommon.h>


namespace sp
{
namespace physics
{


class BulletMotionState : public btMotionState
{
    
    public:
        
        BulletMotionState(const btTransform &OriginTrans, scene::SceneNode* Node);
        ~BulletMotionState();
        
        /* Functions */
        
        void getWorldTransform(btTransform &WorldTrans) const;
        void setWorldTransform(const btTransform &WorldTrans);
        
        /* Static functions */
        
        static dim::matrix4f getMatrix(const btTransform &Trans);
        static btTransform getTransform(const dim::matrix4f &Matrix);
        
        /* Inline functions */
        
        inline void setNode(scene::SceneNode* Node)
        {
            Node_ = Node;
        }
        inline scene::SceneNode* getNode() const
        {
            return Node_;
        }
        
    private:
        
        /* Members */
        
        btTransform Trans_;
        scene::SceneNode* Node_;
        
};


} // /namespace physics

} // /namespace sp


#endif

#endif



// ================================================================================
