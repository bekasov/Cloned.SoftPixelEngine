/*
 * Collision line based header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_LINEBASED_H__
#define __SP_COLLISION_LINEBASED_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/Collision/spCollisionNode.hpp"


namespace sp
{
namespace scene
{


class SP_EXPORT CollisionLineBased : public CollisionNode
{
    
    public:
        
        virtual ~CollisionLineBased();
        
        /* === Functions === */
        
        virtual s32 getSupportFlags() const = 0;
        virtual f32 getMaxMovement() const;
        
        /**
        Returns the line representing the capsule, cylinder or cone.
        This line is transformed by the last updated collision-node transformation
        \see CollisionNode::updateTransformation
        */
        dim::line3df getLine() const;
        
        /* === Static functions === */
        
        static dim::obbox3df getBoundBoxFromLine(const dim::line3df &Line, f32 Radius);
        
        /* === Inline functions === */
        
        //! Sets the capsule's radius.
        inline void setRadius(f32 Radius)
        {
            Radius_ = Radius;
        }
        //! Returns the capsule's radius.
        inline f32 getRadius() const
        {
            return Radius_;
        }
        
        /**
        Sets the nodes's height.
        \note The capsule collision model is not centered like a basic cylinder mesh.
        */
        inline void setHeight(f32 Height)
        {
            Height_ = Height;
        }
        //! Returns the nodes's height.
        inline f32 getHeight() const
        {
            return Height_;
        }
        
    protected:
        
        /* === Functions === */
        
        CollisionLineBased(
            CollisionMaterial* Material, SceneNode* Node, const ECollisionModels Type,
            f32 Radius, f32 Height
        );
        
        /* === Members === */
        
        f32 Radius_, Height_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
