/*
 * Material node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spMaterialNode.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace scene
{


MaterialNode::MaterialNode(const ENodeTypes Type) :
    RenderNode      (Type   ),
    ShaderClass_    (0      ),
    EnableMaterial_ (true   )
{
}
MaterialNode::~MaterialNode()
{
}

void MaterialNode::setMaterial(const video::MaterialStates* Material)
{
    Material_.copy(Material);
}

bool MaterialNode::compare(const MaterialNode* Other) const
{
    /* Compare order */
    if (Order_ != Other->Order_)
        return Order_ > Other->Order_;
    
    /* Compare material alpha channel */
    if (Material_.getDiffuseColor().Alpha != Other->Material_.getDiffuseColor().Alpha)
        return Material_.getDiffuseColor().Alpha > Other->Material_.getDiffuseColor().Alpha;
    
    /* Compare blending */
    if (Material_.getBlendTarget() != Other->Material_.getBlendTarget())
        return Material_.getBlendTarget() > Other->Material_.getBlendTarget();
    
    /* Compare depth distance */
    if (scene::SceneGraph::ReverseDepthSorting_)
        return DepthDistance_ < Other->DepthDistance_;
    
    return DepthDistance_ > Other->DepthDistance_;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
