/*
 * Collision plane header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_PLANE_H__
#define __SP_COLLISION_PLANE_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/Collision/spCollisionNode.hpp"


namespace sp
{
namespace scene
{


//! CollisionPlane is one of the collision models with only a position and normal vector.
class SP_EXPORT CollisionPlane : public CollisionNode
{
    
    public:
        
        CollisionPlane(CollisionMaterial* Material, SceneNode* Node, const dim::plane3df &Plane);
        ~CollisionPlane();
        
        /* Functions */
        
        s32 getSupportFlags() const;
        
        bool checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const;
        bool checkIntersection(const dim::line3df &Line) const;
        
        /* Inline functions */
        
        inline void setPlane(const dim::plane3df &Plane)
        {
            Plane_ = Plane;
        }
        inline dim::plane3df getPlane() const
        {
            return Plane_;
        }
        
    private:
        
        /* Members */
        
        dim::plane3df Plane_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
