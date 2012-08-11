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


namespace sp
{

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

void DeferredRenderer::renderScene(scene::SceneGraph* Graph, video::Texture* RenderTarget)
{
    if ( Graph && GBufferShader_ && DeferredShader_ && ( !RenderTarget || RenderTarget->getRenderTarget() ) )
    {
        renderSceneIntoGBuffer(Graph);
        renderDeferredShading(RenderTarget);
    }
}


/*
 * ======= Protected: =======
 */

void DeferredRenderer::renderSceneIntoGBuffer(scene::SceneGraph* Graph)
{
    //todo
}

void DeferredRenderer::renderDeferredShading(video::Texture* RenderTarget)
{
    //todo
}


} // /namespace video

} // /namespace sp



// ================================================================================
