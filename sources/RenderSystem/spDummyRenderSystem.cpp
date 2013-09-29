/*
 * Dummy render system file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spDummyRenderSystem.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern scene::SceneGraph* GlbSceneGraph;

namespace video
{


DummyRenderSystem::DummyRenderSystem() :
    RenderSystem(RENDERER_DUMMY)
{
}
DummyRenderSystem::~DummyRenderSystem()
{
}


/*
 * ======= Initialization functions =======
 */

void DummyRenderSystem::setupConfiguration()
{
    // dummy
}


/*
 * ======= Renderer information =======
 */

io::stringc DummyRenderSystem::getRenderer() const
{
    return "Null Device";
}
io::stringc DummyRenderSystem::getVersion() const
{
    return "Dummy";
}
io::stringc DummyRenderSystem::getVendor() const
{
    return "SoftPixel Engine";
}
io::stringc DummyRenderSystem::getShaderVersion() const
{
    return "";
}

bool DummyRenderSystem::queryVideoSupport(const EVideoFeatureSupport Query) const
{
    return false;
}

s32 DummyRenderSystem::getMultitexCount() const
{
    return 0;
}
s32 DummyRenderSystem::getMaxAnisotropicFilter() const
{
    return 0;
}
s32 DummyRenderSystem::getMaxLightCount() const
{
    return 0;
}


/*
 * ======= Video buffer control functions =======
 */

void DummyRenderSystem::clearBuffers(const s32 ClearFlags)
{
    // dummy
}
void DummyRenderSystem::flipBuffers()
{
    // dummy
}


/*
 * ======= Rendering functions =======
 */

bool DummyRenderSystem::setupMaterialStates(const MaterialStates* Material, bool Forced)
{
    if ( !Material || ( !Forced && ( PrevMaterial_ == Material || Material->compare(PrevMaterial_) ) ) )
        return false;
    
    PrevMaterial_ = Material;
    
    return true;
}

void DummyRenderSystem::createVertexBuffer(void* &BufferID)
{
    // dummy
}
void DummyRenderSystem::createIndexBuffer(void* &BufferID)
{
    // dummy
}

void DummyRenderSystem::deleteVertexBuffer(void* &BufferID)
{
    // dummy
}
void DummyRenderSystem::deleteIndexBuffer(void* &BufferID)
{
    // dummy
}

void DummyRenderSystem::updateVertexBuffer(
    void* BufferID, const dim::UniversalBuffer &BufferData, const VertexFormat* Format, const EHWBufferUsage Usage)
{
    // dummy
}
void DummyRenderSystem::updateIndexBuffer(
    void* BufferID, const dim::UniversalBuffer &BufferData, const IndexFormat* Format, const EHWBufferUsage Usage)
{
    // dummy
}

void DummyRenderSystem::updateVertexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index)
{
    // dummy
}
void DummyRenderSystem::updateIndexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index)
{
    // dummy
}

void DummyRenderSystem::drawMeshBuffer(const MeshBuffer* MeshBuffer)
{
    // dummy
}

void DummyRenderSystem::setRenderState(const video::ERenderStates Type, s32 State)
{
    switch (Type)
    {
        case RENDER_TEXTURE:
            __isTexturing = (State != 0); break;
        default:
            if (Type >= 0 && Type < 16)
                RenderStates_[Type] = State;
            break;
    }
}

s32 DummyRenderSystem::getRenderState(const video::ERenderStates Type) const
{
    switch (Type)
    {
        case RENDER_TEXTURE:
            return (s32)__isTexturing;
        default:
            if (Type >= 0 && Type < 16)
                return RenderStates_[Type];
            break;
    }
    return 0;
}


/*
 * ======= Texture loading and creating =======
 */

Texture* DummyRenderSystem::createTexture(const STextureCreationFlags &CreationFlags)
{
    /* Create new dummy texture */
    Texture* NewTexture = new Texture(CreationFlags);
    TextureList_.push_back(NewTexture);
    return NewTexture;
}


/*
 * ======= Matrix controll =======
 */

void DummyRenderSystem::updateModelviewMatrix()
{
    // dummy
}


} // /namespace video

} // /namespace sp



// ================================================================================
