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

extern video::RenderSystem* __spVideoDriver;

namespace scene
{


video::MeshBuffer* Billboard::MeshBuffer_ = 0;

Billboard::Billboard(video::Texture* BaseTexture) :
    MaterialNode    (NODE_BILLBOARD ),
    BaseTexture_    (BaseTexture    ),
    InstanceCount_  (1              )
{
    /* Create mesh buffer if not already done */
    if (!Billboard::MeshBuffer_)
        Billboard::createDefaultMeshBuffer();
    
    /* Material */
    Material_->setBlendingMode(video::BLEND_SRCALPHA, video::BLEND_ONE);
    Material_->setColorMaterial(false);
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
    NewBillboard->BasePosition_ = BasePosition_;
    NewBillboard->Order_        = Order_;
    
    /* Return the new sprite */
    return NewBillboard;
}

/*
 * Rendering
 */

void Billboard::render()
{
    /* Setup material states */
    if (EnableMaterial_)
        __spVideoDriver->setupMaterialStates(Material_);
    __spVideoDriver->setupShaderClass(this, ShaderObject_);
    
    /* Matrix transformation */
    loadTransformation();
    
    /* Update the render matrix */
    __spVideoDriver->updateModelviewMatrix();
    
    /* Setup texture */
    const bool isTexturing = __isTexturing;
    
    if (BaseTexture_)
        MeshBuffer_->setTexture(0, BaseTexture_);
    else
        __isTexturing = false;
    
    /* Render the billboard */
    Billboard::MeshBuffer_->setHardwareInstancing(InstanceCount_);
    __spVideoDriver->drawMeshBuffer(Billboard::MeshBuffer_);
    
    /* Unbinding the shader */
    __spVideoDriver->unbindShaders();
    
    __isTexturing = isTexturing;
}

void Billboard::createDefaultMeshBuffer()
{
    MeshBuffer_ = new video::MeshBuffer(__spVideoDriver->getVertexFormatReduced());
    MeshBuffer_->createMeshBuffer();
    
    const dim::vector3df Normal(0, 0, -1);
    
    MeshBuffer_->addVertex(dim::vector3df(-1, -1, 0), Normal, dim::point2df(0, 1));
    MeshBuffer_->addVertex(dim::vector3df(-1,  1, 0), Normal, dim::point2df(0, 0));
    MeshBuffer_->addVertex(dim::vector3df( 1,  1, 0), Normal, dim::point2df(1, 0));
    MeshBuffer_->addVertex(dim::vector3df( 1, -1, 0), Normal, dim::point2df(1, 1));
    
    MeshBuffer_->updateVertexBuffer();
    MeshBuffer_->setIndexBufferEnable(false);
    MeshBuffer_->setPrimitiveType(video::PRIMITIVE_TRIANGLE_FAN);
    
    MeshBuffer_->addTexture(__spVideoDriver->createTexture(1));
}
void Billboard::deleteDefaultMeshBuffer()
{
    MemoryManager::deleteMemory(MeshBuffer_);
}

void Billboard::updateTransformation()
{
    SceneNode::updateTransformation();
    
    const dim::matrix4f WorldMatrix(spViewMatrix * Transformation_);
    Transformation_ = spViewInvMatrix * WorldMatrix.getPositionScaleMatrix();
    
    DepthDistance_ = WorldMatrix.getPosition().Z;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
