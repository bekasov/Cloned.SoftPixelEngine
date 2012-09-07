/*
 * Deferred renderer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spDeferredRenderer.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Platform/spSoftPixelDevice.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern video::RenderSystem* __spVideoDriver;

namespace video
{


DeferredRenderer::DeferredRenderer() :
    GBufferShader_  (0),
    DeferredShader_ (0),
    Flags_          (0)
{
}
DeferredRenderer::~DeferredRenderer()
{
    deleteShaders();
}

bool DeferredRenderer::generateShaders(s32 Flags)
{
    Flags_ = Flags;
    
    /* Create new vertex formats and delete old shaders */
    createVertexFormats();
    deleteShaders();
    
    /* Generate g-buffer shader */
    GBufferShader_ = __spVideoDriver->createCgShaderClass(&VertexFormat_);
    
    if (GBufferShader_)
        return false;
    
    //...
    
    if (!GBufferShader_->link())
    {
        io::Log::error("Compiling g-buffer shader failed");
        return false;
    }
    
    /* Generate deferred shader */
    DeferredShader_ = __spVideoDriver->createCgShaderClass(&ImageVertexFormat_);
    
    if (!DeferredShader_)
        return false;
    
    //...
    
    if (!DeferredShader_->link())
    {
        io::Log::error("Compiling deferred shader failed");
        return false;
    }
    
    return true;
}

void DeferredRenderer::renderScene(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, video::Texture* RenderTarget, bool UseDefaultGBufferShader)
{
    if ( Graph && GBufferShader_ && DeferredShader_ && ( !RenderTarget || RenderTarget->getRenderTarget() ) )
    {
        renderSceneIntoGBuffer(Graph, ActiveCamera, UseDefaultGBufferShader);
        renderDeferredShading(RenderTarget);
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("DeferredRenderer::renderScene");
    #endif
}


/*
 * ======= Protected: =======
 */

void DeferredRenderer::renderSceneIntoGBuffer(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, bool UseDefaultGBufferShader)
{
    video::ShaderClass* PrevShaderClass = 0;
    
    if (UseDefaultGBufferShader)
    {
        PrevShaderClass = __spVideoDriver->getGlobalShaderClass();
        __spVideoDriver->setGlobalShaderClass(GBufferShader_);
    }
    
    GBuffer_.bindRenderTarget();
    __spVideoDriver->clearBuffers();
    
    __spDevice->setActiveSceneGraph(Graph);
    
    if (ActiveCamera)
        Graph->renderScene(ActiveCamera);
    else
        Graph->renderScene();
    
    if (UseDefaultGBufferShader)
        __spVideoDriver->setGlobalShaderClass(PrevShaderClass);
}

void DeferredRenderer::renderDeferredShading(video::Texture* RenderTarget)
{
    __spVideoDriver->setRenderTarget(RenderTarget);
    {
        DeferredShader_->bind();
        {
            GBuffer_.draw2DImage();
        }
        DeferredShader_->unbind();
    }
    __spVideoDriver->setRenderTarget(0);
}

void DeferredRenderer::deleteShaders()
{
    __spVideoDriver->deleteShaderClass(GBufferShader_,  true);
    __spVideoDriver->deleteShaderClass(DeferredShader_, true);
}

void DeferredRenderer::createVertexFormats()
{
    /* Create object vertex format */
    VertexFormat_.clear();
    
    VertexFormat_.addCoord();
    VertexFormat_.addNormal();
    VertexFormat_.addTexCoord();
    
    if (Flags_ & DEFERREDFLAG_NORMAL_MAPPING)
    {
        /* Add information for normal-mapping (tangent and binormal is texture-coordinates) */
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 3);
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 3);
    }
    
    /* Create 2D image vertex format */
    ImageVertexFormat_.clear();
    
    ImageVertexFormat_.addCoord(DATATYPE_FLOAT, 2);
    ImageVertexFormat_.addTexCoord();
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
