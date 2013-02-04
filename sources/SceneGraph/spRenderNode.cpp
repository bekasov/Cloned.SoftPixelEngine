/*
 * Render node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spRenderNode.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern scene::SceneGraph* __spSceneManager;

namespace scene
{


RenderNode::RenderNode(const ENodeTypes Type) :
    SceneNode       (Type           ),
    DepthDistance_  (0.0f           ),
    Order_          (ORDER_NORMAL   )
{
}
RenderNode::~RenderNode()
{
}

bool RenderNode::compare(const RenderNode* Other) const
{
    /* Compare order */
    if (Order_ != Other->Order_)
        return Order_ > Other->Order_;
    
    /* Compare depth distance */
    if (scene::SceneGraph::ReverseDepthSorting_)
        return DepthDistance_ < Other->DepthDistance_;
    
    return DepthDistance_ > Other->DepthDistance_;
}

void RenderNode::updateTransformation()
{
    SceneNode::updateTransformation();
    
    DepthDistance_ = (spViewMatrix * FinalWorldMatrix_.getPosition()).Z;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
