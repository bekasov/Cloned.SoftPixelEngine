/*
 * Render node header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_RENDERNODE_H__
#define __SP_SCENE_RENDERNODE_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/spSceneNode.hpp"


namespace sp
{
namespace scene
{


//! Interface for all renderable objects: visible 3d geometry and light sources or a whole scene graph.
class SP_EXPORT RenderNode : public SceneNode
{
    
    public:
        
        virtual ~RenderNode();
        
        //! Renders the object. In this case rendering does not only mean drawing a 3d mesh. It can also be a light source.
        virtual void render() = 0;
        
        //! This functions compares this and the specified render node for depth sorting.
        virtual bool compare(RenderNode* other);
        
        //! Updates the objects transformation and sets the depth distance.
        virtual void updateTransformation();
        
        /* Depth distance sorting */
        
        //! Sets the depth distance. This value is used for sorting the objects by its distance to the camera view.
        inline void setDepthDistance(f32 Distance)
        {
            DepthDistance_ = Distance;
        }
        inline f32 getDepthDistance() const
        {
            return DepthDistance_;
        }
        
        //! Sets the order for sorting (by default 0).
        inline void setOrder(s32 Order)
        {
            Order_ = Order;
        }
        inline s32 getOrder() const
        {
            return Order_;
        }
        
    protected:
        
        RenderNode(const ENodeTypes Type);
        
        /* Members */
        
        f32 DepthDistance_;
        s32 Order_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
