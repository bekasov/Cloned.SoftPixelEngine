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
    DeferredShader_ (0)
{
}
DeferredRenderer::~DeferredRenderer()
{
}

void DeferredRenderer::renderScene(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, video::Texture* RenderTarget)
{
    if ( Graph && GBufferShader_ && DeferredShader_ && ( !RenderTarget || RenderTarget->getRenderTarget() ) )
    {
        video::ShaderClass* PrevShaderClass = __spVideoDriver->getGlobalShaderClass();
        
        renderSceneIntoGBuffer(Graph, ActiveCamera);
        renderDeferredShading(RenderTarget);
        
        __spVideoDriver->setGlobalShaderClass(PrevShaderClass);
    }
}


/*
 * ======= Protected: =======
 */

void DeferredRenderer::renderSceneIntoGBuffer(scene::SceneGraph* Graph, scene::Camera* ActiveCamera)
{
    __spVideoDriver->setGlobalShaderClass(GBufferShader_);
    
    __spVideoDriver->setRenderTarget(GBuffer_.getTexture(GBuffer::RENDERTARGET_DIFFUSE_AND_SPECULAR));
    __spVideoDriver->clearBuffers();
    
    __spDevice->setActiveSceneGraph(Graph);
    Graph->renderScene(ActiveCamera);
    //todo
}

void DeferredRenderer::renderDeferredShading(video::Texture* RenderTarget)
{
    __spVideoDriver->setGlobalShaderClass(DeferredShader_);
    
    
    //todo
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
