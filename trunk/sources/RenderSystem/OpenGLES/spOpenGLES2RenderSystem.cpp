/*
 * OpenGL|ES 2 render system file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGLES/spOpenGLES2RenderSystem.hpp"

#if defined(SP_COMPILE_WITH_OPENGLES2)


#include "SceneGraph/spSceneCamera.hpp"
#include "RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern io::InputControl* GlbInputCtrl;
extern scene::SceneGraph* GlbSceneGraph;
extern SoftPixelDevice* GlbEngineDev;

namespace video
{


/*
 * ======= Internal members =======
 */

extern s32 GLCompareList[];
extern GLenum GLPrimitiveModes[];
extern GLenum GLTexFormatList[];
extern GLenum GLBasicDataTypes[];
extern s32 GLBlendingList[];


/*
 * ======= OpenGLES2Driver class =======
 */

OpenGLES2RenderSystem::OpenGLES2RenderSystem() :
    RenderSystem                    (RENDERER_OPENGLES2 ),
    GLProgrammableFunctionPipeline  (                   )
{
}
OpenGLES2RenderSystem::~OpenGLES2RenderSystem()
{
}


/*
 * ======= Render system information =======
 */

io::stringc OpenGLES2RenderSystem::getVersion() const
{
    return glGetString(GL_VERSION);
}

bool OpenGLES2RenderSystem::queryVideoSupport(const EVideoFeatureSupport Query) const
{
    switch (Query)
    {
        case QUERY_HARDWARE_MESHBUFFER:
        case QUERY_STENCIL_BUFFER:
        case QUERY_SHADER:
        case QUERY_GLSL:
        case QUERY_MIPMAPS:
        case QUERY_MULTI_TEXTURE:
            return true;
        case QUERY_BILINEAR_FILTER:
        case QUERY_TRILINEAR_FILTER:
            return true; // (todo)
        case QUERY_ANISOTROPY_FILTER:
            return queryExtensionSupport("GL_EXT_texture_filter_anisotropic");
        default:
            break;
    }
    
    return false;
}

s32 OpenGLES2RenderSystem::getMultitexCount() const
{
    s32 MultiTextureCount;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MultiTextureCount);
    return MultiTextureCount;
}
s32 OpenGLES2RenderSystem::getMaxLightCount() const
{
    return 8;
}


/*
 * ======= Context functions (for initializing the engine) =======
 */

void OpenGLES2RenderSystem::setupConfiguration()
{
    /* Get OpenGL major and minor version */
    GLMajorVersion_ = 2;
    GLMinorVersion_ = 0;
    
    initExtensionInfo();
    
    /* Default queries */
    RenderQuery_[RENDERQUERY_SHADER]                = true;
    RenderQuery_[RENDERQUERY_MULTI_TEXTURE]         = true;
    RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER]   = true;
    RenderQuery_[RENDERQUERY_RENDERTARGET]          = true;
    RenderQuery_[RENDERQUERY_FOG_COORD]             = true;
    
    /* Default settings */
    glDepthFunc(GL_LEQUAL);
    
    /* Hints for the renderer */
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    
    /* Get multi texture count information */
    MultiTextureCount_ = getMultitexCount();
    
    /* Clear video buffer */
    clearBuffers();
}


/*
 * ======= Render states =======
 */

void OpenGLES2RenderSystem::setRenderState(const video::ERenderStates Type, s32 State)
{
    switch (Type)
    {
        case RENDER_BLEND:
            setGlRenderState(GL_BLEND, State); break;
        case RENDER_CULLFACE:
            setGlRenderState(GL_CULL_FACE, State); break;
        case RENDER_DEPTH:
            setGlRenderState(GL_DEPTH_TEST, State); break;
        case RENDER_DITHER:
            setGlRenderState(GL_DITHER, State); break;
        case RENDER_SCISSOR:
            setGlRenderState(GL_SCISSOR_TEST, State); break;
        case RENDER_STENCIL:
            setGlRenderState(GL_STENCIL_TEST, State); break;
        case RENDER_TEXTURE:
            __isTexturing = (State != 0); break;
        default:
            break;
    }
}

s32 OpenGLES2RenderSystem::getRenderState(const video::ERenderStates Type) const
{
    switch (Type)
    {
        case RENDER_BLEND:
            return (s32)getGlRenderState(GL_BLEND);
        case RENDER_CULLFACE:
            return (s32)getGlRenderState(GL_CULL_FACE);
        case RENDER_DEPTH:
            return (s32)getGlRenderState(GL_DEPTH_TEST);
        case RENDER_DITHER:
            return (s32)getGlRenderState(GL_DITHER);
        case RENDER_SCISSOR:
            return (s32)getGlRenderState(GL_SCISSOR_TEST);
        case RENDER_STENCIL:
            return (s32)getGlRenderState(GL_STENCIL_TEST);
        case RENDER_TEXTURE:
            return (s32)__isTexturing;
        default:
            break;
    }
    
    return 0;
}

void OpenGLES2RenderSystem::disableTriangleListStates()
{
    /* Back settings - texture */
    glActiveTextureARB(GL_TEXTURE0);
    
    /* Back settings - mesh buffer */
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    
    /* Back settings - polygons */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    /* Default render functions */
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    
    LastMaterial_ = 0;
}

void OpenGLES2RenderSystem::disable3DRenderStates()
{
    /* Disable all used states */
    glDisable(GL_BLEND);
}


/*
 * ======= Rendering functions =======
 */

bool OpenGLES2RenderSystem::setupMaterialStates(const MaterialStates* Material, bool Forced)
{
    /* Check for equality to optimize render path */
    if ( GlobalMaterialStates_ != 0 || !Material || ( !Forced && ( PrevMaterial_ == Material || Material->compare(PrevMaterial_) ) ) )
        return false;
    
    PrevMaterial_ = Material;
    
    /* Face culling & polygon mode */
    switch (Material->getRenderFace())
    {
        case video::FACE_FRONT:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
            
        case video::FACE_BACK:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
            
        case video::FACE_BOTH:
            glDisable(GL_CULL_FACE);
            break;
    }
    
    /* Depth function */
    if (Material->getDepthBuffer())
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GLCompareList[Material->getDepthMethod()]);
    }
    else
        glDisable(GL_DEPTH_TEST);
    
    /* Blending function */
    if (Material->getBlending())
    {
        glEnable(GL_BLEND);
        glBlendFunc(GLBlendingList[Material->getBlendSource()], GLBlendingList[Material->getBlendTarget()]);
    }
    else
        glDisable(GL_BLEND);
    
    /* Polygon offset */
    if (Material->getPolygonOffset())
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(Material->getPolygonOffsetFactor(), Material->getPolygonOffsetUnits());
    }
    else
        glDisable(GL_POLYGON_OFFSET_FILL);
}


/*
 * ======= Hardware mesh buffers =======
 */

bool OpenGLES2RenderSystem::bindMeshBuffer(const MeshBuffer* Buffer)
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("OpenGLES2RenderSystem::bindMeshBuffer", "Not implemented yet", io::LOG_UNIQUE);
    #endif
    return false;
}

void OpenGLES2RenderSystem::unbindMeshBuffer()
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("OpenGLES2RenderSystem::unbindMeshBuffer", "Not implemented yet", io::LOG_UNIQUE);
    #endif
}

void OpenGLES2RenderSystem::drawMeshBufferPart(const MeshBuffer* Buffer, u32 StartOffset, u32 NumVertices)
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("OpenGLES2RenderSystem::drawMeshBufferPart", "Not implemented yet", io::LOG_UNIQUE);
    #endif
}

void OpenGLES2RenderSystem::drawMeshBuffer(const MeshBuffer* MeshBuffer)
{
    /* Get reference mesh buffer */
    if (!MeshBuffer)
        return;
    
    const video::MeshBuffer* OrigMeshBuffer = MeshBuffer;
    MeshBuffer = MeshBuffer->getReference();
    
    if (!MeshBuffer->renderable())
        return;
    
    /* Get primtivie type and check if primitive type is supported for OpenGL|ES */
    GLenum Mode = GLPrimitiveModes[MeshBuffer->getPrimitiveType()];
    
    if (Mode == ~0)
        return;
    
    /* Surface shader callback */
    if (CurShaderClass_ && ShaderSurfaceCallback_)
        ShaderSurfaceCallback_(CurShaderClass_, &MeshBuffer->getSurfaceTextureList());
    
    /* Bind hardware vertex- and index buffers */
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, *(u32*)MeshBuffer->getVertexBufferID());
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, *(u32*)MeshBuffer->getIndexBufferID());
    
    /* Vertex data pointers */
    const VertexFormat* Format  = MeshBuffer->getVertexFormat();
    const s32 FormatSize        = Format->getFormatSize();
    
    const c8* vboPointerOffset  = 0;
    
    /* Setup universal vertex attributes */
    u32 i = 0;
    for (std::vector<SVertexAttribute>::const_iterator it = Format->getUniversals().begin(); it != Format->getUniversals().end(); ++it, ++i)
    {
        glEnableVertexAttribArrayARB(i);
        glVertexAttribPointerARB(i, it->Size, GLBasicDataTypes[it->Type], it->Normalize, FormatSize, vboPointerOffset + it->Offset);
    }
    
    /* Bind textures */
    if (__isTexturing)
        bindTextureLayer(OrigMeshBuffer->getTextureLayerList());
    
    /* Draw the primitives */
    if (MeshBuffer->getIndexBufferEnable())
    {
        glDrawElements(
            GLPrimitiveModes[MeshBuffer->getPrimitiveType()],
            MeshBuffer->getIndexCount(),
            GLBasicDataTypes[MeshBuffer->getIndexFormat()->getDataType()],
            0
        );
    }
    else
    {
        glDrawArrays(
            GLPrimitiveModes[MeshBuffer->getPrimitiveType()], 0, MeshBuffer->getVertexCount()
        );
    }
    
    /* Unbind vertex format */
    for (u32 i = 0, c = Format->getUniversals().size(); i < c; ++i)
        glDisableVertexAttribArrayARB(i);
    
    /* Unbind vertex- and index buffer */
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}


/*
 * ======= Image drawing =======
 */

// !!!!!!!!! TODO !!!!!!!!!


/*
 * ======= Matrix controll =======
 */

void OpenGLES2RenderSystem::updateModelviewMatrix()
{
    // todo
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
