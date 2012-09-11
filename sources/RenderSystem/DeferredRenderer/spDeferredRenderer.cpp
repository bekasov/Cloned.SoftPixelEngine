/*
 * Deferred renderer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spDeferredRenderer.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Platform/spSoftPixelDevice.hpp"
#include "Base/spSharedObjects.hpp"


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern video::RenderSystem* __spVideoDriver;

namespace video
{


static void GBufferShaderCallback(video::ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    video::Shader* VertShd = ShdClass->getVertexShader();
    video::Shader* FragShd = ShdClass->getPixelShader();
    
    VertShd->setConstant(
        "WorldViewProjectionMatrix",
        __spVideoDriver->getProjectionMatrix() * __spVideoDriver->getViewMatrix() * __spVideoDriver->getWorldMatrix()
    );
    VertShd->setConstant(
        "WorldMatrix",
        __spVideoDriver->getWorldMatrix()
    );
    
    
}

static void DeferredShaderCallback(video::ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    video::Shader* VertShd = ShdClass->getVertexShader();
    video::Shader* FragShd = ShdClass->getPixelShader();
    
    VertShd->setConstant(
        "WorldViewProjectionMatrix",
        __spVideoDriver->getProjectionMatrix()
    );
    
}


DeferredRenderer::DeferredRenderer() :
    GBufferShader_  (0),
    DeferredShader_ (0),
    Flags_          (0)
{
    if (!gSharedObjects.CgContext)
        __spDevice->createCgShaderContext();
}
DeferredRenderer::~DeferredRenderer()
{
    deleteShaders();
    GBuffer_.deleteGBuffer();
}

bool DeferredRenderer::generateResources(s32 Flags)
{
    /* Setup shader compilation options */
    Flags_ = Flags;
    
    std::vector<const c8*> GBufferCompilerOp, DeferredCompilerOp;
    
    if (Flags_ & DEFERREDFLAG_USE_TEXTURE_MATRIX)
        GBufferCompilerOp.push_back("-DUSE_TEXTURE_MATRIX");
    if (Flags_ & DEFERREDFLAG_NORMAL_MAPPING)
    {
        GBufferCompilerOp.push_back("-DNORMAL_MAPPING");
        if (Flags_ & DEFERREDFLAG_PARALLAX_MAPPING)
            GBufferCompilerOp.push_back("-DPARALLAX_MAPPING");
    }
    if (Flags_ & DEFERREDFLAG_HAS_SPECULAR_MAP)
        GBufferCompilerOp.push_back("-DHAS_SPECULAR_MAP");
    
    GBufferCompilerOp.push_back(0);
    DeferredCompilerOp.push_back(0);
    
    /* Create new vertex formats and delete old shaders */
    createVertexFormats();
    deleteShaders();
    
    /* Get shader buffers */
    std::vector<io::stringc> GBufferShdBuf(1), DeferredShdBuf(1);
    
    GBufferShdBuf[0] = (
        #include "RenderSystem/DeferredRenderer/spGBufferShaderStr.h"
    );
    
    DeferredShdBuf[0] = (
        #include "RenderSystem/DeferredRenderer/spDeferredShaderStr.h"
    );
    
    /* Generate g-buffer shader */
    GBufferShader_ = __spVideoDriver->createCgShaderClass(&VertexFormat_);
    
    if (!GBufferShader_)
        return false;
    
    __spVideoDriver->createCgShader(
        GBufferShader_, video::SHADER_VERTEX, video::CG_VERSION_2_0, GBufferShdBuf, "VertexMain", &GBufferCompilerOp[0]
    );
    __spVideoDriver->createCgShader(
        GBufferShader_, video::SHADER_PIXEL, video::CG_VERSION_2_0, GBufferShdBuf, "PixelMain", &GBufferCompilerOp[0]
    );
    
    if (!GBufferShader_->link())
    {
        io::Log::error("Compiling g-buffer shader failed");
        deleteShaders();
        return false;
    }
    
    GBufferShader_->setObjectCallback(GBufferShaderCallback);
    
    /* Generate deferred shader */
    DeferredShader_ = __spVideoDriver->createCgShaderClass(&ImageVertexFormat_);
    
    if (!DeferredShader_)
        return false;
    
    __spVideoDriver->createCgShader(
        DeferredShader_, video::SHADER_VERTEX, video::CG_VERSION_2_0, DeferredShdBuf, "VertexMain", &DeferredCompilerOp[0]
    );
    __spVideoDriver->createCgShader(
        DeferredShader_, video::SHADER_PIXEL, video::CG_VERSION_2_0, DeferredShdBuf, "PixelMain", &DeferredCompilerOp[0]
    );
    
    if (!DeferredShader_->link())
    {
        io::Log::error("Compiling deferred shader failed");
        deleteShaders();
        return false;
    }
    
    DeferredShader_->setObjectCallback(DeferredShaderCallback);
    
    /* Build g-buffer */
    return GBuffer_.createGBuffer(
        dim::size2di(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight)
    );
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
    else if ( !Graph || ( RenderTarget && !RenderTarget->getRenderTarget() ) )
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
    __spVideoDriver->beginDrawing2D();
    {
        DeferredShader_->bind();
        {
            GBuffer_.draw2DImage();
        }
        DeferredShader_->unbind();
    }
    __spVideoDriver->endDrawing2D();
    __spVideoDriver->setRenderTarget(0);
}

void DeferredRenderer::deleteShaders()
{
    __spVideoDriver->deleteShaderClass(GBufferShader_,  true);
    __spVideoDriver->deleteShaderClass(DeferredShader_, true);
    
    GBufferShader_ = 0;
    DeferredShader_ = 0;
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
