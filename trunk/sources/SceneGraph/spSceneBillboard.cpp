/*
 * Billboard scene node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneBillboard.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Base/spInternalDeclarations.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace scene
{


Billboard::Billboard(video::Texture* BaseTexture) :
    MaterialNode    (NODE_BILLBOARD             ),
    BaseTexture_    (BaseTexture                ),
    NumInstances_   (1                          ),
    BaseRotation_   (0.0f                       ),
    Alignment_      (BILLBOARD_SCREEN_ALIGNED   ),
    UpVector_       (0, 1, 0                    )
{
    /* Initialize material states */
    Material_.setBlendingMode(video::BLEND_SRCALPHA, video::BLEND_ONE);
    Material_.setLighting(false);
    Material_.setColorMaterial(false);
}
Billboard::~Billboard()
{
}

Billboard* Billboard::copy() const
{
    /* Allocate a new sprite */
    Billboard* NewBillboard = new Billboard(BaseTexture_);
    
    /* Copy the root attributes */
    copyRoot(NewBillboard);
    
    /* Copy the sprite materials */
    NewBillboard->Material_     = Material_;
    NewBillboard->Order_        = Order_;
    
    NewBillboard->BaseTexture_  = BaseTexture_;
    NewBillboard->NumInstances_ = NumInstances_;
    NewBillboard->BasePosition_ = BasePosition_;
    NewBillboard->BaseRotation_ = BaseRotation_;
    NewBillboard->Alignment_    = Alignment_;
    NewBillboard->UpVector_     = UpVector_;

    /* Return the new sprite */
    return NewBillboard;
}

void Billboard::render()
{
    /* Setup material states */
    GlbRenderSys->setupMaterialStates(getMaterial());
    GlbRenderSys->setupShaderClass(this, getShaderClass());
    
    /* Matrix transformation */
    loadTransformation();
    
    /* Update the render matrix */
    GlbRenderSys->updateModelviewMatrix();
    
    /* Setup texture */
    video::MeshBuffer* HWBuffer = GlbRenderSys->getBillboardMeshBuffer();

    const bool isTexturing = __isTexturing;
    
    if (BaseTexture_)
        HWBuffer->setTexture(0, BaseTexture_);
    else
        __isTexturing = false;
    
    /* Render the billboard */
    HWBuffer->setHardwareInstancing(NumInstances_);
    GlbRenderSys->drawMeshBuffer(HWBuffer);
    
    /* Unbinding the shader */
    GlbRenderSys->unbindShaders();
    
    __isTexturing = isTexturing;
}

void Billboard::updateTransformation()
{
    /* Update billboard transformation */
    SceneNode::updateTransformation();
    
    const dim::matrix4f WorldMatrix(spViewMatrix * FinalWorldMatrix_);
    
    if (Alignment_ != BILLBOARD_SCREEN_ALIGNED)
    {
        FinalWorldMatrix_.getColumn(2) = dim::vector4df(FinalWorldMatrix_.getPosition() - spViewInvMatrix.getPosition(), 0);
        dim::normalize<3, f32>(FinalWorldMatrix_.getColumn(2).ptr());

        if (Alignment_ == BILLBOARD_UPVECTOR_ALIGNED)
        {
            FinalWorldMatrix_.getColumn(1) = dim::vector4df(UpVector_, 0);
            FinalWorldMatrix_.getColumn(0) = dim::cross(FinalWorldMatrix_.getColumn(1), FinalWorldMatrix_.getColumn(2));
            FinalWorldMatrix_.getColumn(2) = dim::cross(FinalWorldMatrix_.getColumn(0), FinalWorldMatrix_.getColumn(1));

            dim::normalize<3, f32>(FinalWorldMatrix_.getColumn(2).ptr());
            FinalWorldMatrix_.getColumn(2).W = 0;
        }
        else
        {
            FinalWorldMatrix_.getColumn(1) = dim::vector4df(0, 1, 0, 0);
            FinalWorldMatrix_.getColumn(0) = dim::cross(FinalWorldMatrix_.getColumn(1), FinalWorldMatrix_.getColumn(2));
            FinalWorldMatrix_.getColumn(1) = dim::cross(FinalWorldMatrix_.getColumn(2), FinalWorldMatrix_.getColumn(0));

            dim::normalize<3, f32>(FinalWorldMatrix_.getColumn(1).ptr());
            FinalWorldMatrix_.getColumn(1).W = 0;
        }

        dim::normalize<3, f32>(FinalWorldMatrix_.getColumn(0).ptr());

        FinalWorldMatrix_.getColumn(0).W = 0;

        FinalWorldMatrix_.scale(getScale());
    }
    else
        FinalWorldMatrix_ = spViewInvMatrix * WorldMatrix.getPositionScaleMatrix();
    
    /* Apply base translation and rotation */
    if (!math::equal(BaseRotation_, 0.0f))
        FinalWorldMatrix_.rotateZ(BaseRotation_);
    
    FinalWorldMatrix_.translate(BasePosition_);

    /* Store depth distance for sorting */
    DepthDistance_ = WorldMatrix.getPosition().Z;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
