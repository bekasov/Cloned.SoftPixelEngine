/*
 * OpenGL render system file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLRenderSystem.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "SceneGraph/spSceneCamera.hpp"
#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Base/spSharedObjects.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Framework/Cg/spCgShaderProgramGL.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern io::InputControl* __spInputControl;
extern scene::SceneGraph* __spSceneManager;

namespace video
{


/*
 * ======= Internal members =======
 */

f32 __spLightPosition[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

extern s32 GLCompareList[];
extern GLenum GLPrimitiveModes[];
extern GLenum GLBasicDataTypes[];
extern s32 GLBlendingList[];


/*
 * ======= OpenGLRenderSystem class =======
 */

OpenGLRenderSystem::OpenGLRenderSystem() :
    RenderSystem                    (RENDERER_OPENGL),
    GLFixedFunctionPipeline         (               ),
    GLProgrammableFunctionPipeline  (               )
{
}
OpenGLRenderSystem::~OpenGLRenderSystem()
{
    deleteFontObjects();
}


/*
 * ======= Initialization functions =======
 */

void OpenGLRenderSystem::setupConfiguration()
{
    /* Get OpenGL major and minor version */
    io::stringc GLVersion = glGetString(GL_VERSION);
    
    GLMajorVersion_ = GLVersion.left(1).val<s32>();
    GLMinorVersion_ = GLVersion.section(2, 3).val<s32>();
    
    /* Default queries */
    RenderQuery_[RENDERQUERY_SHADER                     ] = queryVideoSupport(QUERY_SHADER                  );
    RenderQuery_[RENDERQUERY_GEOMETRY_SHADER            ] = queryVideoSupport(QUERY_GEOMETRY_SHADER         );
    RenderQuery_[RENDERQUERY_TESSELLATION_SHADER        ] = queryVideoSupport(QUERY_TESSELLATION_SHADER     );
    
    RenderQuery_[RENDERQUERY_RENDERTARGET               ] = queryVideoSupport(QUERY_RENDERTARGET            );
    RenderQuery_[RENDERQUERY_MULTI_TEXTURE              ] = queryVideoSupport(QUERY_MULTI_TEXTURE           );
    RenderQuery_[RENDERQUERY_MULTISAMPLE_RENDERTARGET   ] = queryVideoSupport(QUERY_MULTISAMPLE_RENDERTARGET);
    RenderQuery_[RENDERQUERY_CUBEMAP_ARRAY              ] = queryVideoSupport(QUERY_CUBEMAP_ARRAY           );
    
    RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER        ] = queryVideoSupport(QUERY_HARDWARE_MESHBUFFER     );
    RenderQuery_[RENDERQUERY_HARDWARE_INSTANCING        ] = queryVideoSupport(QUERY_HARDWARE_INSTANCING     );
    
    RenderQuery_[RENDERQUERY_FOG_COORD                  ] = queryVideoSupport(QUERY_VOLUMETRIC_FOG          );
    
    /* Load all OpenGL extensions */
    loadExtensions();
    
    /* Get multi texture count information */
    MultiTextureCount_ = getMultitexCount();
    
    defaultTextureGenMode();
}


/*
 * ======= Render system information =======
 */

bool OpenGLRenderSystem::queryVideoSupport(const EVideoFeatureQueries Query) const
{
    switch (Query)
    {
        case QUERY_ANTIALIASING:
            #if defined(SP_PLATFORM_WINDOWS)
            return queryExtensionSupport("WGL_ARB_multisample");
            #elif defined(SP_PLATFORM_LINUX)
            return queryExtensionSupport("GL_ARB_multisample");
            #endif
            
        case QUERY_RENDERTARGET:
            return queryExtensionSupport("GL_ARB_framebuffer_object") || queryExtensionSupport("GL_EXT_framebuffer_object");
        case QUERY_MULTI_TEXTURE:
            return queryExtensionSupport("GL_ARB_multitexture");
        case QUERY_MULTISAMPLE_TEXTURE:
            return queryExtensionSupport("GL_ARB_texture_multisample");
        case QUERY_MULTISAMPLE_RENDERTARGET:
            return queryExtensionSupport("GL_EXT_framebuffer_multisample");
        case QUERY_CUBEMAP_ARRAY:
            return queryExtensionSupport("GL_ARB_texture_cube_map_array");
            
        case QUERY_HARDWARE_MESHBUFFER:
            return queryExtensionSupport("GL_ARB_vertex_buffer_object");
        case QUERY_HARDWARE_INSTANCING:
            return queryExtensionSupport("GL_ARB_draw_instanced");
        case QUERY_STENCIL_BUFFER:
            return true;
        case QUERY_VOLUMETRIC_FOG:
            return queryExtensionSupport("GL_EXT_fog_coord");
            
        case QUERY_BILINEAR_FILTER:
        case QUERY_TRILINEAR_FILTER:
            return true; // (todo)
        case QUERY_ANISOTROPY_FILTER:
            return queryExtensionSupport("GL_EXT_texture_filter_anisotropic");
        case QUERY_MIPMAPS:
            return true;
        case QUERY_VOLUMETRIC_TEXTURE:
            return queryExtensionSupport("GL_EXT_texture3D");
            
        case QUERY_VETEX_PROGRAM:
            return queryExtensionSupport("GL_ARB_vertex_program");
        case QUERY_FRAGMENT_PROGRAM:
            return queryExtensionSupport("GL_ARB_fragment_program");
        case QUERY_SHADER:
        case QUERY_GLSL:
            return queryExtensionSupport("GL_ARB_shader_objects");
        case QUERY_GEOMETRY_SHADER:
            return queryExtensionSupport("GL_EXT_geometry_shader4") || queryExtensionSupport("GL_ARB_geometry_shader4");
        case QUERY_TESSELLATION_SHADER:
            return queryExtensionSupport("GL_ARB_tessellation_shader");
    }
    
    return false;
}


/*
 * ======= Configuration functions =======
 */

void OpenGLRenderSystem::setAntiAlias(bool isAntiAlias)
{
    if (isAntiAlias)
        glEnable(GL_MULTISAMPLE_ARB);
    else
        glDisable(GL_MULTISAMPLE_ARB);
}

void OpenGLRenderSystem::setDepthClip(bool Enable)
{
    setGlRenderState(GL_DEPTH_CLAMP, !Enable);
}
bool OpenGLRenderSystem::getDepthClip() const
{
    return !getGlRenderState(GL_DEPTH_CLAMP);
}


/*
 * ======= Rendering functions =======
 */

void OpenGLRenderSystem::setupMaterialStates(const MaterialStates* Material)
{
    /* Check for equality to optimize render path */
    if (!Material || Material->compare(LastMaterial_))
        return;
    else
        LastMaterial_ = Material;
    
    /* Face culling & polygon mode */
    switch (Material->getRenderFace())
    {
        case video::FACE_FRONT:
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            glPolygonMode(GL_BACK, GL_POINT + Material->getWireframeFront());
        }
        break;
        
        case video::FACE_BACK:
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glPolygonMode(GL_FRONT, GL_POINT + Material->getWireframeBack());
        }
        break;
        
        case video::FACE_BOTH:
        {
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_BACK, GL_POINT + Material->getWireframeFront());
            glPolygonMode(GL_FRONT, GL_POINT + Material->getWireframeBack());
        }
        break;
    }
    
    /* Fog effect */
    setGlRenderState(GL_FOG, __isFog && Material->getFog());
    
    /* Color material */
    setGlRenderState(GL_COLOR_MATERIAL, Material->getColorMaterial());
    
    /* Lighting material */
    if (__isLighting && Material->getLighting())
    {
        glEnable(GL_LIGHTING);
        
        /* Shininess */
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, Material->getShininessFactor());
        
        /* Diffuse color */
        Material->getDiffuseColor().getFloatArray(TempColor_);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, TempColor_);
        
        /* Ambient color */
        Material->getAmbientColor().getFloatArray(TempColor_);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, TempColor_);
        
        /* Specular color */
        Material->getSpecularColor().getFloatArray(TempColor_);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, TempColor_);
        
        /* Emission color */
        Material->getEmissionColor().getFloatArray(TempColor_);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, TempColor_);
    }
    else
        glDisable(GL_LIGHTING);
    
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
    
    /* Alpha function */
    glAlphaFunc(GLCompareList[Material->getAlphaMethod()], Material->getAlphaReference());
}

void OpenGLRenderSystem::drawPrimitiveList(
    const ERenderPrimitives Type,
    const scene::SMeshVertex3D* Vertices, u32 VertexCount, const void* Indices, u32 IndexCount,
    std::vector<SMeshSurfaceTexture>* TextureList)
{
    if (!Vertices || !VertexCount || Type < PRIMITIVE_POINTS || Type > PRIMITIVE_POLYGON)
        return;
    
    /* Select the primitive type */
    const GLenum Mode = GLPrimitiveModes[Type];
    
    /* Enable all client states */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    if (RenderQuery_[RENDERQUERY_FOG_COORD])
        glEnableClientState(GL_FOG_COORDINATE_ARRAY);
    
    /* Hardware buffer configuration */
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER])
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    
    /* Vertex data pointers */
    glVertexPointer(3, GL_FLOAT, sizeof(scene::SMeshVertex3D), (s8*)Vertices + VBO_OFFSET_VERTEX);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(scene::SMeshVertex3D), (s8*)Vertices + VBO_OFFSET_COLOR);
    glNormalPointer(GL_FLOAT, sizeof(scene::SMeshVertex3D), (s8*)Vertices + VBO_OFFSET_NORMAL);
    if (RenderQuery_[RENDERQUERY_FOG_COORD])
        glFogCoordPointer(GL_FLOAT, sizeof(scene::SMeshVertex3D), (s8*)Vertices + VBO_OFFSET_FOG);
    
    /* Texture coordinate pointers */
    const s32 MaxTexCoords = (RenderQuery_[RENDERQUERY_MULTI_TEXTURE] ? 1 : MAX_COUNT_OF_TEXTURES);
    
    for (s32 i = 0; i < MaxTexCoords; ++i)
    {
        glClientActiveTextureARB(GL_TEXTURE0 + i);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(3, GL_FLOAT, sizeof(scene::SMeshVertex3D), (s8*)Vertices + VBO_OFFSET_TEXCOORD(i));
    }
    
    /* Bind texture layers */
    if (TextureList && __isTexturing)
        bindTextureList(*TextureList);
    
    /* Render primitives */
    if (!Indices || !IndexCount)
        glDrawArrays(Mode, 0, VertexCount);
    else
        glDrawElements(Mode, IndexCount, GL_UNSIGNED_INT, Indices);
    
    /* Disbale all client states */
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    if (RenderQuery_[RENDERQUERY_FOG_COORD])
        glDisableClientState(GL_FOG_COORDINATE_ARRAY);
    
    /* Disable texture coordinate pointers */
    for (s32 i = 0; i < MaxTexCoords; ++i)
    {
        glClientActiveTextureARB(GL_TEXTURE0 + i);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    
    /* Unbind texture layers */
    if (TextureList && __isTexturing)
        unbindTextureList(*TextureList);
}


/*
 * ======= Hardware mesh buffers =======
 */

void OpenGLRenderSystem::drawMeshBuffer(const MeshBuffer* MeshBuffer)
{
    /* Get reference mesh buffer */
    if (!MeshBuffer)
        return;
    
    const video::MeshBuffer* OrigMeshBuffer = MeshBuffer;
    MeshBuffer = MeshBuffer->getReference();
    
    if (!MeshBuffer->renderable())
        return;
    
    /* Surface shader callback */
    if (CurShaderClass_ && ShaderSurfaceCallback_)
        ShaderSurfaceCallback_(CurShaderClass_, &MeshBuffer->getSurfaceTextureList());
    
    /* Bind hardware vertex- and index buffers */
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER])
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, *(u32*)MeshBuffer->getVertexBufferID());
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, *(u32*)MeshBuffer->getIndexBufferID());
    }
    
    /* Vertex data pointers */
    const VertexFormat* Format  = MeshBuffer->getVertexFormat();
    const s32 FormatSize        = Format->getFormatSize();
    
    const c8* vboPointerOffset  = (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] ? 0 : (const c8*)MeshBuffer->getVertexBuffer().getArray());
    
    /* Setup vertex coordinates */
    if (Format->getFlags() & VERTEXFORMAT_COORD)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(Format->getCoord().Size, GLBasicDataTypes[Format->getCoord().Type], FormatSize, vboPointerOffset + Format->getCoord().Offset);
    }
    
    /* Setup vertex normals */
    if (Format->getFlags() & VERTEXFORMAT_NORMAL)
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GLBasicDataTypes[Format->getNormal().Type], FormatSize, vboPointerOffset + Format->getNormal().Offset);
    }
    
    /* Setup vertex colors */
    if (Format->getFlags() & VERTEXFORMAT_COLOR)
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(Format->getColor().Size, GLBasicDataTypes[Format->getColor().Type], FormatSize, vboPointerOffset + Format->getColor().Offset);
    }
    
    /* Setup vertex fog coordinates */
    if ((Format->getFlags() & VERTEXFORMAT_FOGCOORD) && RenderQuery_[RENDERQUERY_FOG_COORD])
    {
        glEnableClientState(GL_FOG_COORDINATE_ARRAY);
        glFogCoordPointer(GLBasicDataTypes[Format->getFogCoord().Type], FormatSize, vboPointerOffset + Format->getFogCoord().Offset);
    }
    
    /* Setup vertex texture coordinates */
    if (Format->getFlags() & VERTEXFORMAT_TEXCOORDS)
    {
        if (RenderQuery_[RENDERQUERY_MULTI_TEXTURE])
        {
            u32 i = 0;
            for (std::vector<SVertexAttribute>::const_iterator it = Format->getTexCoords().begin(); it != Format->getTexCoords().end(); ++it, ++i)
            {
                glClientActiveTextureARB(GL_TEXTURE0 + i);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(it->Size, GLBasicDataTypes[it->Type], FormatSize, vboPointerOffset + it->Offset);
            }
        }
        else if (!Format->getTexCoords().empty())
        {
            std::vector<SVertexAttribute>::const_iterator it = Format->getTexCoords().begin();
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(it->Size, GLBasicDataTypes[it->Type], FormatSize, vboPointerOffset + it->Offset);
        }
    }
    
    /* Setup universal vertex attributes */
    if ((Format->getFlags() & VERTEXFORMAT_UNIVERSAL) && RenderQuery_[RENDERQUERY_SHADER])
    {
        u32 i = 0;
        foreach (const SVertexAttribute &Attrib, Format->getUniversals())
        {
            glEnableVertexAttribArrayARB(i);
            glVertexAttribPointerARB(i, Attrib.Size, GLBasicDataTypes[Attrib.Type], Attrib.Normalize, FormatSize, vboPointerOffset + Attrib.Offset);
            ++i;
        }
    }
    
    /* Bind textures */
    if (__isTexturing)
        bindTextureList(OrigMeshBuffer->getSurfaceTextureList());
    
    if (MeshBuffer->getHardwareInstancing() > 1 && RenderQuery_[RENDERQUERY_HARDWARE_INSTANCING])
    {
        /* Draw the primitives instanced */
        if (MeshBuffer->getIndexBufferEnable())
        {
            glDrawElementsInstancedARB(
                GLPrimitiveModes[MeshBuffer->getPrimitiveType()],
                MeshBuffer->getIndexCount(),
                GLBasicDataTypes[MeshBuffer->getIndexFormat()->getDataType()],
                (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] ? 0 : MeshBuffer->getIndexBuffer().getArray()),
                MeshBuffer->getHardwareInstancing()
            );
        }
        else
        {
            glDrawArraysInstancedARB(
                GLPrimitiveModes[MeshBuffer->getPrimitiveType()],
                0,
                MeshBuffer->getVertexCount(),
                MeshBuffer->getHardwareInstancing()
            );
        }
    }
    else
    {
        /* Draw the primitives */
        if (MeshBuffer->getIndexBufferEnable())
        {
            glDrawElements(
                GLPrimitiveModes[MeshBuffer->getPrimitiveType()],
                MeshBuffer->getIndexCount(),
                GLBasicDataTypes[MeshBuffer->getIndexFormat()->getDataType()],
                (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] ? 0 : MeshBuffer->getIndexBuffer().getArray())
            );
        }
        else
        {
            glDrawArrays(
                GLPrimitiveModes[MeshBuffer->getPrimitiveType()],
                0,
                MeshBuffer->getVertexCount()
            );
        }
    }
    
    /* Unbind textures */
    if (__isTexturing)
        unbindTextureList(OrigMeshBuffer->getSurfaceTextureList());
    
    /* Unbind vertex format */
    if (Format->getFlags() & VERTEXFORMAT_COORD)
        glDisableClientState(GL_VERTEX_ARRAY);
    if (Format->getFlags() & VERTEXFORMAT_NORMAL)
        glDisableClientState(GL_NORMAL_ARRAY);
    if (Format->getFlags() & VERTEXFORMAT_COLOR)
        glDisableClientState(GL_COLOR_ARRAY);
    if ((Format->getFlags() & VERTEXFORMAT_FOGCOORD) && RenderQuery_[RENDERQUERY_FOG_COORD])
        glDisableClientState(GL_FOG_COORDINATE_ARRAY);
    if (Format->getFlags() & VERTEXFORMAT_TEXCOORDS)
    {
        if (RenderQuery_[RENDERQUERY_MULTI_TEXTURE])
        {
            u32 i = 0;
            for (std::vector<SVertexAttribute>::const_iterator it = Format->getTexCoords().begin(); it != Format->getTexCoords().end(); ++it, ++i)
            {
                glClientActiveTextureARB(GL_TEXTURE0 + i);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
        }
        else if (!Format->getTexCoords().empty())
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if ((Format->getFlags() & VERTEXFORMAT_UNIVERSAL) && RenderQuery_[RENDERQUERY_SHADER])
    {
        for (u32 i = 0, c = Format->getUniversals().size(); i < c; ++i)
            glDisableVertexAttribArrayARB(i);
    }
    
    /* Unbind vertex- and index buffer */
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER])
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    
    #ifdef SP_DEBUGMODE
    ++RenderSystem::DrawCallCounter_;
    #endif
}

void OpenGLRenderSystem::drawMeshBufferPlain(const MeshBuffer* MeshBuffer, bool useFirstTextureLayer)
{
    /* Get reference mesh buffer */
    if (!MeshBuffer)
        return;
    
    const video::MeshBuffer* OrigMeshBuffer = MeshBuffer;
    MeshBuffer = MeshBuffer->getReference();
    
    if (!MeshBuffer->renderable())
        return;
    
    /* Bind hardware vertex- and index buffers */
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER])
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, *(u32*)MeshBuffer->getVertexBufferID());
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, *(u32*)MeshBuffer->getIndexBufferID());
    }
    
    /* Vertex data pointers */
    const VertexFormat* Format  = MeshBuffer->getVertexFormat();
    const s32 FormatSize        = Format->getFormatSize();
    
    const c8* vboPointerOffset  = (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] ? 0 : (const c8*)MeshBuffer->getVertexBuffer().getArray());
    
    /* Setup vertex coordinates */
    if (Format->getFlags() & VERTEXFORMAT_COORD)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(Format->getCoord().Size, GLBasicDataTypes[Format->getCoord().Type], FormatSize, vboPointerOffset + Format->getCoord().Offset);
    }
    
    /* Setup vertex texture coordinates */
    const bool isTexCoordBind = ( useFirstTextureLayer && (Format->getFlags() & VERTEXFORMAT_TEXCOORDS) && !Format->getTexCoords().empty() );
    
    if (isTexCoordBind)
    {
        std::vector<SVertexAttribute>::const_iterator it = Format->getTexCoords().begin();
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(it->Size, GLBasicDataTypes[it->Type], FormatSize, vboPointerOffset + it->Offset);
    }
    
    /* Bind textures */
    const bool isTextureBind = ( __isTexturing && useFirstTextureLayer && !OrigMeshBuffer->getSurfaceTextureList().empty() );
    
    Texture* FirstTexture = 0;
    
    if (isTextureBind)
    {
        FirstTexture = OrigMeshBuffer->getSurfaceTextureList().begin()->TextureObject;
        if (FirstTexture)
            FirstTexture->bind(0);
    }
    
    if (MeshBuffer->getHardwareInstancing() > 1 && RenderQuery_[RENDERQUERY_HARDWARE_INSTANCING])
    {
        /* Draw the primitives instanced */
        if (MeshBuffer->getIndexBufferEnable())
        {
            glDrawElementsInstancedARB(
                GLPrimitiveModes[MeshBuffer->getPrimitiveType()],
                MeshBuffer->getIndexCount(),
                GLBasicDataTypes[MeshBuffer->getIndexFormat()->getDataType()],
                (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] ? 0 : MeshBuffer->getIndexBuffer().getArray()),
                MeshBuffer->getHardwareInstancing()
            );
        }
        else
        {
            glDrawArraysInstancedARB(
                GLPrimitiveModes[MeshBuffer->getPrimitiveType()],
                0,
                MeshBuffer->getVertexCount(),
                MeshBuffer->getHardwareInstancing()
            );
        }
    }
    else
    {
        /* Draw the primitives */
        if (MeshBuffer->getIndexBufferEnable())
        {
            glDrawElements(
                GLPrimitiveModes[MeshBuffer->getPrimitiveType()],
                MeshBuffer->getIndexCount(),
                GLBasicDataTypes[MeshBuffer->getIndexFormat()->getDataType()],
                (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] ? 0 : MeshBuffer->getIndexBuffer().getArray())
            );
        }
        else
        {
            glDrawArrays(
                GLPrimitiveModes[MeshBuffer->getPrimitiveType()],
                0,
                MeshBuffer->getVertexCount()
            );
        }
    }
    
    /* Unbind textures */
    if (isTextureBind && FirstTexture)
        FirstTexture->unbind(0);
    
    /* Unbind vertex format */
    if (Format->getFlags() & VERTEXFORMAT_COORD)
        glDisableClientState(GL_VERTEX_ARRAY);
    if (isTexCoordBind)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    /* Unbind vertex- and index buffer */
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER])
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
}


/*
 * ======= Stencil buffer =======
 */

void OpenGLRenderSystem::clearStencilBuffer()
{
    glClearStencil(0);
}

void OpenGLRenderSystem::drawStencilShadowVolume(
    const dim::vector3df* pTriangleList, s32 Count, bool ZFailMethod, bool VolumetricShadow)
{
    if (!pTriangleList || !Count)
        return;
    
    /* Store the current OpenGL states */
    glPushAttrib(
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT
    );
    
    /* Configure the stencil states */
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    
    glDepthFunc(GL_LEQUAL);
    
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_CLAMP_NV);
    //glEnable(GL_POLYGON_OFFSET_FILL);
    //glPolygonOffset(0.0f, 1.0f);
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(dim::vector3df), pTriangleList);
    
    glStencilMask(~0);
    glStencilFunc(GL_ALWAYS, 0, ~0);
    
    /* Draw the stencil shadow */
    if (ZFailMethod)
    {
        glCullFace(GL_FRONT);
        glStencilOp(GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
        glDrawArrays(GL_TRIANGLES, 0, Count);
        
        if (!VolumetricShadow)
        {
            glCullFace(GL_BACK);
            glStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
            glDrawArrays(GL_TRIANGLES, 0, Count);
        }
    }
    else
    {
        glCullFace(GL_BACK);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
        glDrawArrays(GL_TRIANGLES, 0, Count);
        
        if (!VolumetricShadow)
        {
            glCullFace(GL_FRONT);
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
            glDrawArrays(GL_TRIANGLES, 0, Count);
        }
    }
    
    /* Reset the OpenGL states */
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopAttrib();
}

void OpenGLRenderSystem::drawStencilShadow(const video::color &Color)
{
    /* Store the current OpenGL states */
    glPushAttrib(
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT
    );
    glPushMatrix();
    
    /* Configure the stencil states */
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_FALSE);
    
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 0, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    
    /* Draw the rectangle */
    beginDrawing2D();
    draw2DRectangle(dim::rect2di(0, 0, gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight), Color);
    endDrawing2D();
    
    /* Clear the stencil buffer */
    glClear(GL_STENCIL_BUFFER_BIT);
    
    /* Reset the OpenGL states */
    glPopMatrix();
    glPopAttrib();
}


/*
 * ======= Shader programs =======
 */

Shader* OpenGLRenderSystem::createCgShader(
    ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
    const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint,
    const c8** CompilerOptions)
{
    Shader* NewShader = 0;
    
    #ifndef SP_COMPILE_WITH_CG
    io::Log::error("This engine was not compiled with the Cg toolkit");
    #else
    if (RenderQuery_[RENDERQUERY_SHADER])
        NewShader = new CgShaderProgramGL(ShaderClassObj, Type, Version);
    else
    #endif
        NewShader = new Shader(ShaderClassObj, Type, Version);
    
    NewShader->compile(ShaderBuffer, EntryPoint, CompilerOptions);
    
    if (!ShaderClassObj)
        NewShader->getShaderClass()->link();
    
    ShaderList_.push_back(NewShader);
    
    return NewShader;
}


/*
 * ======= Image drawing =======
 */

void OpenGLRenderSystem::draw2DImage(
    Texture* Tex, const dim::point2di &Position, const color &Color)
{
    if (!Tex)
        return;
    
    #ifndef __DRAW2DARRAYS__
    /* Coloring */
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    #endif
    
    /* Binding the texture */
    Tex->bind(0);
    
    /* Load 2dimensional matrix */
    setDrawingMatrix2D();
    
    #ifdef __DRAW2DARRAYS__
    
    const dim::rect2df Rect(
        static_cast<f32>(Position.X),
        static_cast<f32>(Position.Y),
        static_cast<f32>(Position.X + Tex->getSize().Width),
        static_cast<f32>(Position.Y + Tex->getSize().Height)
    );
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(Rect.Left    , Rect.Top      , 0.0f, 0.0f, Color);
    Vertices2D_[1] = scene::SPrimitiveVertex2D(Rect.Right   , Rect.Top      , 1.0f, 0.0f, Color);
    Vertices2D_[2] = scene::SPrimitiveVertex2D(Rect.Right   , Rect.Bottom   , 1.0f, 1.0f, Color);
    Vertices2D_[3] = scene::SPrimitiveVertex2D(Rect.Left    , Rect.Bottom   , 0.0f, 1.0f, Color);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    #else
    
    /* Drawing the quad */
    glBegin(GL_QUADS);
    {
        glTexCoord2i(0, 0);
        glVertex2i(Position.X, Position.Y);
        
        glTexCoord2i(1, 0);
        glVertex2i(Position.X + Tex->getSize().Width, Position.Y);
        
        glTexCoord2i(1, 1);
        glVertex2i(Position.X + Tex->getSize().Width, Position.Y + Tex->getSize().Height);
        
        glTexCoord2i(0, 1);
        glVertex2i(Position.X, Position.Y + Tex->getSize().Height);
    }
    glEnd();
    
    #endif
    
    /* Back settings */
    Tex->unbind(0);
}

void OpenGLRenderSystem::draw2DImage(
    Texture* Tex, const dim::rect2di &Position, const dim::rect2df &Clipping, const color &Color)
{
    if (!Tex)
        return;
    
    #ifndef __DRAW2DARRAYS__
    /* Coloring */
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    #endif
    
    /* Binding the texture */
    Tex->bind(0);
    
    /* Load 2dimensional matrix */
    setDrawingMatrix2D();
    
    #ifdef __DRAW2DARRAYS__
    
    dim::rect2df Rect(Position.cast<f32>());
    
    Rect.Right += Rect.Left;
    Rect.Bottom += Rect.Top;
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(Rect.Left,   Rect.Top,       Clipping.Left,  Clipping.Top,       Color);
    Vertices2D_[1] = scene::SPrimitiveVertex2D(Rect.Right,  Rect.Top,       Clipping.Right, Clipping.Top,       Color);
    Vertices2D_[2] = scene::SPrimitiveVertex2D(Rect.Right,  Rect.Bottom,    Clipping.Right, Clipping.Bottom,    Color);
    Vertices2D_[3] = scene::SPrimitiveVertex2D(Rect.Left,   Rect.Bottom,    Clipping.Left,  Clipping.Bottom,    Color);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    #else
    
    /* Drawing the quad */
    glBegin(GL_QUADS);
    {
        glTexCoord2f(Clipping.Left, Clipping.Top);
        glVertex2i(Position.Left, Position.Top);
        
        glTexCoord2f(Clipping.Right, Clipping.Top);
        glVertex2i(Position.Left + Position.Right, Position.Top);
        
        glTexCoord2f(Clipping.Right, Clipping.Bottom);
        glVertex2i(Position.Left + Position.Right, Position.Top + Position.Bottom);
        
        glTexCoord2f(Clipping.Left, Clipping.Bottom);
        glVertex2i(Position.Left, Position.Top + Position.Bottom);
    }
    glEnd();
    
    #endif
    
    /* Back settings */
    Tex->unbind(0);
}

void OpenGLRenderSystem::draw2DImage(
    Texture* Tex, const dim::point2di &Position, f32 Rotation, f32 Radius, const color &Color)
{
    if (!Tex)
        return;
    
    #ifndef __DRAW2DARRAYS__
    /* Temporary variables */
    Radius *= math::SQRT2F;
    
    const dim::point2df lefttopPos      (math::Sin(Rotation -  45)*Radius + Position.X, -math::Cos(Rotation -  45)*Radius + Position.Y);
    const dim::point2df righttopPos     (math::Sin(Rotation +  45)*Radius + Position.X, -math::Cos(Rotation +  45)*Radius + Position.Y);
    const dim::point2df rightbottomPos  (math::Sin(Rotation + 135)*Radius + Position.X, -math::Cos(Rotation + 135)*Radius + Position.Y);
    const dim::point2df leftbottomPos   (math::Sin(Rotation - 135)*Radius + Position.X, -math::Cos(Rotation - 135)*Radius + Position.Y);
    #endif
    
    #ifndef __DRAW2DARRAYS__
    /* Coloring */
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    #endif
    
    /* Binding the texture */
    Tex->bind(0);
    
    /* Load 2dimensional matrix */
    setDrawingMatrix2D();
    
    #ifdef __DRAW2DARRAYS__
    
    glTranslatef((f32)Position.X, (f32)Position.Y, 0.0f);
    glRotatef(Rotation, 0.0f, 0.0f, 1.0f);
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(-Radius, -Radius, 0.0f, 0.0f, Color);
    Vertices2D_[1] = scene::SPrimitiveVertex2D( Radius, -Radius, 1.0f, 0.0f, Color);
    Vertices2D_[2] = scene::SPrimitiveVertex2D( Radius,  Radius, 1.0f, 1.0f, Color);
    Vertices2D_[3] = scene::SPrimitiveVertex2D(-Radius,  Radius, 0.0f, 1.0f, Color);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    #else
    
    /* Drawing the quad */
    glBegin(GL_QUADS);
    {
        glTexCoord2i(0, 0);
        glVertex2f(lefttopPos.X, lefttopPos.Y);
        
        glTexCoord2i(1, 0);
        glVertex2f(righttopPos.X, righttopPos.Y);
        
        glTexCoord2i(1, 1);
        glVertex2f(rightbottomPos.X, rightbottomPos.Y);
        
        glTexCoord2i(0, 1);
        glVertex2f(leftbottomPos.X, leftbottomPos.Y);
    }
    glEnd();
    
    #endif
    
    /* Back settings */
    Tex->unbind(0);
}

void OpenGLRenderSystem::draw2DImage(
    Texture* Tex,
    const dim::point2di &lefttopPosition, const dim::point2di &righttopPosition,
    const dim::point2di &rightbottomPosition, const dim::point2di &leftbottomPosition,
    const dim::point2df &lefttopClipping, const dim::point2df &righttopClipping,
    const dim::point2df &rightbottomClipping, const dim::point2df &leftbottomClipping,
    const color &lefttopColor, const color &righttopColor,
    const color &rightbottomColor, const color &leftbottomColor)
{
    if (!Tex)
        return;
    
    /* Texture binding */
    Tex->bind(0);
    
    /* Load 2dimensional matrix */
    setDrawingMatrix2D();
    
    #ifdef __DRAW2DARRAYS__
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D((f32)lefttopPosition.X       , (f32)lefttopPosition.Y    , lefttopClipping.X     , lefttopClipping.Y     , lefttopColor      );
    Vertices2D_[1] = scene::SPrimitiveVertex2D((f32)righttopPosition.X      , (f32)righttopPosition.Y   , righttopClipping.X    , righttopClipping.Y    , righttopColor     );
    Vertices2D_[2] = scene::SPrimitiveVertex2D((f32)rightbottomPosition.X   , (f32)rightbottomPosition.Y, rightbottomClipping.X , rightbottomClipping.Y , rightbottomColor  );
    Vertices2D_[3] = scene::SPrimitiveVertex2D((f32)leftbottomPosition.X    , (f32)leftbottomPosition.Y , leftbottomClipping.X  , leftbottomClipping.Y  , leftbottomColor   );
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    #else
    
    /* Drawing the quad */
    glBegin(GL_QUADS);
    {
        glColor4ub(lefttopColor.Red, lefttopColor.Green, lefttopColor.Blue, lefttopColor.Alpha);
        glTexCoord2f(lefttopClipping.X, lefttopClipping.Y);
        glVertex2i(lefttopPosition.X, lefttopPosition.Y);
        
        glColor4ub(righttopColor.Red, righttopColor.Green, righttopColor.Blue, righttopColor.Alpha);
        glTexCoord2f(righttopClipping.X, righttopClipping.Y);
        glVertex2i(righttopPosition.X, righttopPosition.Y);
        
        glColor4ub(rightbottomColor.Red, rightbottomColor.Green, rightbottomColor.Blue, rightbottomColor.Alpha);
        glTexCoord2f(rightbottomClipping.X, rightbottomClipping.Y);
        glVertex2i(rightbottomPosition.X, rightbottomPosition.Y);
        
        glColor4ub(leftbottomColor.Red, leftbottomColor.Green, leftbottomColor.Blue, leftbottomColor.Alpha);
        glTexCoord2f(leftbottomClipping.X, leftbottomClipping.Y);
        glVertex2i(leftbottomPosition.X, leftbottomPosition.Y);
    }
    glEnd();
    
    #endif
    
    /* Back settings */
    Tex->unbind(0);
}


/*
 * ======= Primitive drawing =======
 */

f32 OpenGLRenderSystem::getPixelDepth(const dim::point2di &Position) const
{
    f32 Depth;
    glReadPixels(Position.X, gSharedObjects.ScreenHeight - Position.Y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Depth);
    return Depth;
}

void OpenGLRenderSystem::draw2DPoint(const dim::point2di &Position, const color &Color)
{
    setDrawingMatrix2D();
    
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    
    glBegin(GL_POINTS);
    {
        glVertex2i(Position.X, Position.Y);
    }
    glEnd();
}

void OpenGLRenderSystem::draw2DLine(
    const dim::point2di &PositionA, const dim::point2di &PositionB, const color &Color)
{
    setDrawingMatrix2D();
    
    #ifdef __DRAW2DARRAYS__
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D((f32)PositionA.X, (f32)PositionA.Y, 0.0f, 0.0f, Color);
    Vertices2D_[1] = scene::SPrimitiveVertex2D((f32)PositionB.X, (f32)PositionB.Y, 0.0f, 0.0f, Color);
    
    glDrawArrays(GL_LINES, 0, 2);
    
    #else
    
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    
    glBegin(GL_LINES);
    {
        glVertex2i(PositionA.X, PositionA.Y);
        glVertex2i(PositionB.X, PositionB.Y);
    }
    glEnd();
    
    #endif
}

void OpenGLRenderSystem::draw2DLine(
    const dim::point2di &PositionA, const dim::point2di &PositionB, const color &ColorA, const color &ColorB)
{
    #ifdef __DRAW2DARRAYS__
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D((f32)PositionA.X, (f32)PositionA.Y, 0.0f, 0.0f, ColorA);
    Vertices2D_[1] = scene::SPrimitiveVertex2D((f32)PositionB.X, (f32)PositionB.Y, 0.0f, 0.0f, ColorB);
    
    glDrawArrays(GL_LINES, 0, 2);
    
    #else
    
    setDrawingMatrix2D();
    
    glBegin(GL_LINES);
    {
        glColor4ub(ColorA.Red, ColorA.Green, ColorA.Blue, ColorA.Alpha);
        glVertex2i(PositionA.X, PositionA.Y);
        
        glColor4ub(ColorB.Red, ColorB.Green, ColorB.Blue, ColorB.Alpha);
        glVertex2i(PositionB.X, PositionB.Y);
    }
    glEnd();
    
    #endif
}

void OpenGLRenderSystem::draw2DRectangle(const dim::rect2di &Rect, const color &Color, bool isSolid)
{
    setDrawingMatrix2D();
    
    #ifdef __DRAW2DARRAYS__
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D((f32)Rect.Left   , (f32)Rect.Top     , 0.0f, 0.0f, Color);
    Vertices2D_[1] = scene::SPrimitiveVertex2D((f32)Rect.Right  , (f32)Rect.Top     , 0.0f, 0.0f, Color);
    Vertices2D_[2] = scene::SPrimitiveVertex2D((f32)Rect.Right  , (f32)Rect.Bottom  , 0.0f, 0.0f, Color);
    Vertices2D_[3] = scene::SPrimitiveVertex2D((f32)Rect.Left   , (f32)Rect.Bottom  , 0.0f, 0.0f, Color);
    
    glDrawArrays(isSolid ? GL_QUADS : GL_LINE_LOOP, 0, 4);
    
    #else
    
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    
    glBegin(isSolid ? GL_QUADS : GL_LINE_LOOP);
    {
        glVertex2i(Rect.Left, Rect.Top);
        glVertex2i(Rect.Right, Rect.Top);
        glVertex2i(Rect.Right, Rect.Bottom);
        glVertex2i(Rect.Left, Rect.Bottom);
    }
    glEnd();
    
    #endif
}

void OpenGLRenderSystem::draw2DRectangle(
    const dim::rect2di &Rect, const color &lefttopColor, const color &righttopColor,
    const color &rightbottomColor, const color &leftbottomColor, bool isSolid)
{
    setDrawingMatrix2D();
    
    #ifdef __DRAW2DARRAYS__
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D((f32)Rect.Left   , (f32)Rect.Top     , 0.0f, 0.0f, lefttopColor      );
    Vertices2D_[1] = scene::SPrimitiveVertex2D((f32)Rect.Right  , (f32)Rect.Top     , 0.0f, 0.0f, righttopColor     );
    Vertices2D_[2] = scene::SPrimitiveVertex2D((f32)Rect.Right  , (f32)Rect.Bottom  , 0.0f, 0.0f, rightbottomColor  );
    Vertices2D_[3] = scene::SPrimitiveVertex2D((f32)Rect.Left   , (f32)Rect.Bottom  , 0.0f, 0.0f, leftbottomColor   );
    
    glDrawArrays(isSolid ? GL_QUADS : GL_LINE_LOOP, 0, 4);
    
    #else
    
    glBegin(isSolid ? GL_QUADS : GL_LINE_LOOP);
    {
        glColor4ub(lefttopColor.Red, lefttopColor.Green, lefttopColor.Blue, lefttopColor.Alpha);
        glVertex2i(Rect.Left, Rect.Top);
        
        glColor4ub(righttopColor.Red, righttopColor.Green, righttopColor.Blue, righttopColor.Alpha);
        glVertex2i(Rect.Right, Rect.Top);
        
        glColor4ub(rightbottomColor.Red, rightbottomColor.Green, rightbottomColor.Blue, rightbottomColor.Alpha);
        glVertex2i(Rect.Right, Rect.Bottom);
        
        glColor4ub(leftbottomColor.Red, leftbottomColor.Green, leftbottomColor.Blue, leftbottomColor.Alpha);
        glVertex2i(Rect.Left, Rect.Bottom);
    }
    glEnd();
    
    #endif
}


/*
 * ======= Extra drawing functions =======
 */

void OpenGLRenderSystem::draw2DPolygon(
    const ERenderPrimitives Type, const scene::SPrimitiveVertex2D* VerticesList, u32 Count)
{
    if (!VerticesList || !Count)
        return;
    
    setDrawingMatrix2D();
    
    /* Set the vertex pointers */
    glVertexPointer(4, GL_FLOAT, sizeof(scene::SPrimitiveVertex2D), (c8*)VerticesList + ARY_OFFSET_VERTEX);
    glTexCoordPointer(2, GL_FLOAT, sizeof(scene::SPrimitiveVertex2D), (c8*)VerticesList + ARY_OFFSET_TEXCOORD);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(scene::SPrimitiveVertex2D), (c8*)VerticesList + ARY_OFFSET_COLOR);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    /* Draw the vertex array */
    glDrawArrays(GLPrimitiveModes[Type], 0, Count);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}


/*
 * ======= 3D drawing functions =======
 */
 
void OpenGLRenderSystem::draw3DPoint(const dim::vector3df &Position, const color &Color)
{
    setDrawingMatrix3D();
    
    #ifdef __DRAW2DARRAYS__
    
    scene::SMeshVertex3D Vertices[1] = {
        scene::SMeshVertex3D(Position.X, Position.Y, Position.Z, Color)
    };
    
    drawPrimitiveList(PRIMITIVE_POINTS, Vertices, 1, 0, 0, 0);
    
    #else
    
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    
    glBegin(GL_POINTS);
    {
        glVertex3f(Position.X, Position.Y, Position.Z);
    }
    glEnd();
    
    #endif
}

void OpenGLRenderSystem::draw3DLine(
    const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &Color)
{
    setDrawingMatrix3D();
    
    #ifdef __DRAW2DARRAYS__
    
    scene::SMeshVertex3D Vertices[2] = {
        scene::SMeshVertex3D(PositionA.X, PositionA.Y, PositionA.Z, Color),
        scene::SMeshVertex3D(PositionB.X, PositionB.Y, PositionB.Z, Color)
    };
    
    drawPrimitiveList(PRIMITIVE_LINES, Vertices, 2, 0, 0, 0);
    
    #else
    
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    
    glBegin(GL_LINES);
    {
        glVertex3f(PositionA.X, PositionA.Y, PositionA.Z);
        glVertex3f(PositionB.X, PositionB.Y, PositionB.Z);
    }
    glEnd();
    
    #endif
}

void OpenGLRenderSystem::draw3DLine(
    const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &ColorA, const color &ColorB)
{
    setDrawingMatrix3D();
    
    #ifdef __DRAW2DARRAYS__
    
    scene::SMeshVertex3D Vertices[2] = {
        scene::SMeshVertex3D(PositionA.X, PositionA.Y, PositionA.Z, ColorA),
        scene::SMeshVertex3D(PositionB.X, PositionB.Y, PositionB.Z, ColorB)
    };
    
    drawPrimitiveList(PRIMITIVE_LINES, Vertices, 2, 0, 0, 0);
    
    #else
    
    glBegin(GL_LINES);
    {
        glColor4ub(ColorA.Red, ColorA.Green, ColorA.Blue, ColorA.Alpha);
        glVertex3f(PositionA.X, PositionA.Y, PositionA.Z);
        
        glColor4ub(ColorB.Red, ColorB.Green, ColorB.Blue, ColorB.Alpha);
        glVertex3f(PositionB.X, PositionB.Y, PositionB.Z);
    }
    glEnd();
    
    #endif
}

void OpenGLRenderSystem::draw3DEllipse(
    const dim::vector3df &Position, const dim::vector3df &Rotation, const dim::size2df &Radius, const color &Color)
{
    setDrawingMatrix3D();
    
    glTranslatef(Position.X, Position.Y, Position.Z);
    glRotatef(Rotation.Y, 0.0f, 1.0f, 0.0f);
    glRotatef(Rotation.X, 1.0f, 0.0f, 0.0f);
    glRotatef(Rotation.Z, 0.0f, 0.0f, 1.0f);
    
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    glBegin(GL_LINE_STRIP);
    {
        for (s32 i = 0; i <= 36; ++i)
            glVertex2f(math::Sin(i*10)*Radius.Width, math::Cos(i*10)*Radius.Height);
    }
    glEnd();
}

void OpenGLRenderSystem::draw3DTriangle(
    Texture* Tex, const dim::triangle3df &Triangle, const color &Color)
{
    setDrawingMatrix3D();
    
    if (Tex)
        Tex->bind(0);
    
    #ifdef __DRAW2DARRAYS__
    
    scene::SMeshVertex3D Vertices[3] = {
        scene::SMeshVertex3D(Triangle.PointA.X, Triangle.PointA.Y, Triangle.PointA.Z, Color),
        scene::SMeshVertex3D(Triangle.PointB.X, Triangle.PointB.Y, Triangle.PointB.Z, Color),
        scene::SMeshVertex3D(Triangle.PointC.X, Triangle.PointC.Y, Triangle.PointC.Z, Color)
    };
    
    drawPrimitiveList(PRIMITIVE_TRIANGLES, Vertices, 3, 0, 0, 0);
    
    #else
    
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    
    glBegin(GL_TRIANGLES);
    {
        glVertex3f(Triangle.PointA.X, Triangle.PointA.Y, Triangle.PointA.Z);
        glVertex3f(Triangle.PointB.X, Triangle.PointB.Y, Triangle.PointB.Z);
        glVertex3f(Triangle.PointC.X, Triangle.PointC.Y, Triangle.PointC.Z);
    }
    glEnd();
    
    #endif
    
    if (Tex)
        Tex->unbind(0);
}


/*
 * ======= Font loading and text drawing =======
 */

#if defined(SP_PLATFORM_WINDOWS)

Font* OpenGLRenderSystem::createBitmapFont(const io::stringc &FontName, s32 FontSize, s32 Flags)
{
    /* Register 256 new OpenGL lists */
    GLuint* DisplayListsID = new GLuint;
    *DisplayListsID = glGenLists(256);
    
    if (FontSize <= 0)
        FontSize = DEF_FONT_SIZE;
    
    /* Create device font */
    HFONT FontObject = 0;
    createDeviceFont(&FontObject, FontName, dim::size2di(0, FontSize), Flags);
    
    /* Create the bitmap font display lists */
    HANDLE LastObject = SelectObject(DeviceContext_, FontObject);
    wglUseFontBitmaps(DeviceContext_, 0, 256, *DisplayListsID);
    SelectObject(DeviceContext_, LastObject);
    
    /* Create new font */
    Font* NewFont = new Font(DisplayListsID, FontName, dim::size2di(0, FontSize), getCharWidths(&FontObject));
    FontList_.push_back(NewFont);
    
    /* Delete device font object */
    DeleteObject(FontObject);
    
    return NewFont;
}

#elif defined(SP_PLATFORM_LINUX)

Font* OpenGLRenderSystem::createBitmapFont(const io::stringc &FontName, s32 FontSize, s32 Flags)
{
    /* Generate X11 font name string */
    const s32 Height    = (FontSize ? math::MinMax(FontSize, 6, 20) : 18);
    const s32 Width     = FontSize/2;
    
    io::stringc X11FontName("*normal--" + io::stringc(FontSize) + "*");
    
    SX11FontPackage* FontPackage = new SX11FontPackage;
    
    /* Register 256 new OpenGL lists */
    FontPackage->DisplayListsID = glGenLists(256);
    
    /* Load X11 font */
    FontPackage->FontID = XLoadFont(
        static_cast<SoftPixelDeviceLinux*>(__spDevice)->Display_, X11FontName.c_str()
    );
    
    if (FontPackage->FontID)
    {
        /* Create the bitmap font display lists */
        glXUseXFont(FontPackage->FontID, 0, 256, FontPackage->DisplayListsID);
    }
    else
        io::Log::error("Could not load X11 font");
    
    /* Create new font */
    Font* NewFont = new Font(FontPackage, FontName, dim::size2di(Width, Height), getCharWidths(0));
    FontList_.push_back(NewFont);
    
    return NewFont;
}

#endif

void OpenGLRenderSystem::drawBitmapFont(
    Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color)
{
    const dim::size2di FontSize(FontObj->getSize());
    
    glLoadIdentity();
    
    #if 1
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_POLYGON_OFFSET_FILL);
    #endif
    
    /* Coloring (before loacting raster position) */
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    
    /* Locate raster position */
    glRasterPos2i(0, 0);
    
    /* Additional move for the raster position */
    glBitmap(
        0, 0, 0.0f, 0.0f,
        static_cast<f32>(Position.X - gSharedObjects.ScreenWidth/2),
        isInvertScreen_ ?
            static_cast<f32>(Position.Y - gSharedObjects.ScreenHeight/2) :
            static_cast<f32>(-Position.Y + gSharedObjects.ScreenHeight/2 - FontSize.Height),
        0
    );
    
    /* Draw the text */
    glPushAttrib(GL_LIST_BIT);
    {
        glListBase(*reinterpret_cast<u32*>(FontObj->getBufferRawData()));
        glCallLists(Text.size(), GL_UNSIGNED_BYTE, Text.c_str());
    }
    glPopAttrib();
}

void OpenGLRenderSystem::draw3DText(
    Font* FontObject, const dim::matrix4f &Transformation, const io::stringc &Text, const color &Color)
{
    if (!FontObject || !FontObject->getBufferRawData())
        return;
    
    glLoadIdentity();
    
    /* Coloring (before loacting raster position) */
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    
    /* Check if a camera is used */
    if (__spSceneManager && __spSceneManager->getActiveCamera())
    {
        /* Update the modelview matrix of the camera */
        __spSceneManager->getActiveCamera()->updateTransformation();
        
        /* Update the matrix */
        updateModelviewMatrix();
    }
    
    /* Locate raster position */
    const dim::vector3df Position(Transformation.getPosition());
    const dim::vector3df Rotation(Transformation.getRotation());

    glRasterPos3f(Position.X, Position.Y, Position.Z);
    
    glRotatef(Rotation.Y, 0.0f, 1.0f, 0.0f);
    glRotatef(Rotation.X, 1.0f, 0.0f, 0.0f);
    glRotatef(Rotation.Z, 0.0f, 0.0f, 1.0f);
    
    /* Draw the text */
    glPushAttrib(GL_LIST_BIT);
    {
        glListBase(*reinterpret_cast<GLuint*>(FontObject->getBufferRawData()));
        glCallLists(Text.size(), GL_UNSIGNED_BYTE, Text.c_str());
    }
    glPopAttrib();
}


/*
 * ======= Matrix controll =======
 */

void OpenGLRenderSystem::setColorMatrix(const dim::matrix4f &Matrix)
{
    scene::spColorMatrix = Matrix;
    
    glMatrixMode(GL_COLOR);
    glLoadMatrixf(Matrix.getArray());
}


/*
 * ======= Private: =======
 */

void OpenGLRenderSystem::deleteFontObjects()
{
    foreach (Font* FontObj, FontList_)
        releaseFontObject(FontObj);
}

void OpenGLRenderSystem::releaseFontObject(Font* FontObj)
{
    if (FontObj && FontObj->getBufferRawData())
    {
        if (FontObj->getTexture())
        {
            void* BufferID = FontObj->getBufferRawData();
            deleteVertexBuffer(BufferID);
        }
        else
        {
            #if defined(SP_PLATFORM_LINUX)
            
            SX11FontPackage* FontPackage = reinterpret_cast<SX11FontPackage*>(FontObj->getBufferRawData());
            
            /* Delete OpenGL display lists */
            glDeleteLists(FontPackage->DisplayListsID, 256);
            
            /* Release X11 font object */
            XUnloadFont(
                static_cast<SoftPixelDeviceLinux*>(__spDevice)->Display_, FontPackage->FontID
            );
            
            delete FontPackage;
            
            #else
            
            GLuint* DisplayListsID = reinterpret_cast<GLuint*>(FontObj->getBufferRawData());
            
            /* Delete OpenGL display lists */
            glDeleteLists(*DisplayListsID, 256);
            
            delete DisplayListsID;
            
            #endif
        }
    }
}

void OpenGLRenderSystem::loadExtensions()
{
    #if defined(SP_PLATFORM_WINDOWS)
    
    #   define LOADOPENGLPROC(o, t, n)                                                          \
            o = (t)wglGetProcAddress(n);                                                        \
            if (!o)                                                                             \
                io::Log::error("Could not load OpenGL function \"" + io::stringc(n) + "\"");
    
    #elif defined(SP_PLATFORM_LINUX)
    
    #   define LOADOPENGLPROC(o, t, n)                                                          \
            o = (t)glXGetProcAddress(reinterpret_cast<const GLubyte*>(n));                      \
            if (!o)                                                                             \
                io::Log::error("Could not load OpenGL function \"" + io::stringc(n) + "\"");
    
    #endif
    
    /* Load "GL_ARB_multitexture" extension */
    
    if (RenderQuery_[RENDERQUERY_MULTI_TEXTURE])
    {
        LOADOPENGLPROC(glActiveTextureARB,          PFNGLACTIVETEXTUREARBPROC,          "glActiveTextureARB"        )
        LOADOPENGLPROC(glMultiTexCoord2fARB,        PFNGLMULTITEXCOORD2FARBPROC,        "glMultiTexCoord2fARB"      )
        LOADOPENGLPROC(glClientActiveTextureARB,    PFNGLCLIENTACTIVETEXTUREARBPROC,    "glClientActiveTextureARB"  )
    }
    else
        io::Log::message("Multi-texturing is not supported");
    
    /* Load "GL_ARB_vertex_buffer_object" extension */
    
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER])
    {
        LOADOPENGLPROC(glGenBuffersARB,     PFNGLGENBUFFERSARBPROC,     "glGenBuffersARB"   )
        LOADOPENGLPROC(glBindBufferARB,     PFNGLBINDBUFFERARBPROC,     "glBindBufferARB"   )
        LOADOPENGLPROC(glBufferDataARB,     PFNGLBUFFERDATAARBPROC,     "glBufferDataARB"   )
        LOADOPENGLPROC(glBufferSubDataARB,  PFNGLBUFFERSUBDATAARBPROC,  "glBufferSubDataARB")
        LOADOPENGLPROC(glDeleteBuffersARB,  PFNGLDELETEBUFFERSARBPROC,  "glDeleteBuffersARB")
    }
    else
        io::Log::message("VertexBufferObjects (VBO) are not supported");
    
    /* Load "GL_ARB_draw_instanced" extension */
    
    if (RenderQuery_[RENDERQUERY_HARDWARE_INSTANCING])
    {
        LOADOPENGLPROC(glDrawElementsInstancedARB,  PFNGLDRAWELEMENTSINSTANCEDARBPROC,  "glDrawElementsInstancedARB")
        LOADOPENGLPROC(glDrawArraysInstancedARB,    PFNGLDRAWARRAYSINSTANCEDARBPROC,    "glDrawArraysInstancedARB"  )
    }
    /*else
        io::Log::message("Hardware instancing is not supported");*/
    
    /* Load "GL_ARB_framebuffer_object" extension */
    
    if (RenderQuery_[RENDERQUERY_RENDERTARGET])
    {
        LOADOPENGLPROC(glGenFramebuffersEXT,            PFNGLGENFRAMEBUFFERSEXTPROC,            "glGenFramebuffersEXT"          )
        LOADOPENGLPROC(glGenRenderbuffersEXT,           PFNGLGENRENDERBUFFERSEXTPROC,           "glGenRenderbuffersEXT"         )
        LOADOPENGLPROC(glDeleteFramebuffersEXT,         PFNGLDELETEFRAMEBUFFERSEXTPROC,         "glDeleteFramebuffersEXT"       )
        LOADOPENGLPROC(glDeleteRenderbuffersEXT,        PFNGLDELETERENDERBUFFERSEXTPROC,        "glDeleteRenderbuffersEXT"      )
        LOADOPENGLPROC(glBindFramebufferEXT,            PFNGLBINDFRAMEBUFFEREXTPROC,            "glBindFramebufferEXT"          )
        LOADOPENGLPROC(glBindRenderbufferEXT,           PFNGLBINDRENDERBUFFEREXTPROC,           "glBindRenderbufferEXT"         )
        LOADOPENGLPROC(glFramebufferRenderbufferEXT,    PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC,    "glFramebufferRenderbufferEXT"  )
        LOADOPENGLPROC(glFramebufferTexture1DEXT,       PFNGLFRAMEBUFFERTEXTURE1DEXTPROC,       "glFramebufferTexture1DEXT"     )
        LOADOPENGLPROC(glFramebufferTexture2DEXT,       PFNGLFRAMEBUFFERTEXTURE2DEXTPROC,       "glFramebufferTexture2DEXT"     )
        LOADOPENGLPROC(glFramebufferTextureLayerEXT,    PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC,    "glFramebufferTextureLayerEXT"  )
        LOADOPENGLPROC(glRenderbufferStorageEXT,        PFNGLRENDERBUFFERSTORAGEEXTPROC,        "glRenderbufferStorageEXT"      )
        LOADOPENGLPROC(glCheckFramebufferStatusEXT,     PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC,     "glCheckFramebufferStatusEXT"   )
        LOADOPENGLPROC(glGenerateMipmapEXT,             PFNGLGENERATEMIPMAPEXTPROC,             "glGenerateMipmapEXT"           )
        
        if (RenderQuery_[RENDERQUERY_MULTISAMPLE_RENDERTARGET])
        {
            LOADOPENGLPROC(glRenderbufferStorageMultisampleEXT, PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC, "glRenderbufferStorageMultisampleEXT"   )
            LOADOPENGLPROC(glBlitFramebufferEXT,                PFNGLBLITFRAMEBUFFEREXTPROC,                "glBlitFramebufferEXT"                  )
        }
    }
    else
        io::Log::message("FrameBufferObjects (FBO) are not supported");
    
    /* Load "GL_ARB_shader_objects" extension */
    
    if (RenderQuery_[RENDERQUERY_SHADER])
    {
        LOADOPENGLPROC(glBindProgramARB,                PFNGLBINDPROGRAMARBPROC,                "glBindProgramARB"              )
        LOADOPENGLPROC(glProgramStringARB,              PFNGLPROGRAMSTRINGARBPROC,              "glProgramStringARB"            )
        LOADOPENGLPROC(glGenProgramsARB,                PFNGLGENPROGRAMSARBPROC,                "glGenProgramsARB"              )
        LOADOPENGLPROC(glDeleteProgramsARB,             PFNGLDELETEPROGRAMSARBPROC,             "glDeleteProgramsARB"           )
        LOADOPENGLPROC(glProgramLocalParameter4fvARB,   PFNGLPROGRAMLOCALPARAMETER4FVARBPROC,   "glProgramLocalParameter4fvARB" )
        LOADOPENGLPROC(glDrawBuffersARB,                PFNGLDRAWBUFFERSARBPROC,                "glDrawBuffersARB"              )
        
        LOADOPENGLPROC(glDeleteProgram,                 PFNGLDELETEPROGRAMPROC,                 "glDeleteProgram"               )
        LOADOPENGLPROC(glDeleteShader,                  PFNGLDELETESHADERPROC,                  "glDeleteShader"                )
        LOADOPENGLPROC(glCreateProgramObjectARB,        PFNGLCREATEPROGRAMOBJECTARBPROC,        "glCreateProgramObjectARB"      )
        LOADOPENGLPROC(glCreateShaderObjectARB,         PFNGLCREATESHADEROBJECTARBPROC,         "glCreateShaderObjectARB"       )
        LOADOPENGLPROC(glShaderSourceARB,               PFNGLSHADERSOURCEARBPROC,               "glShaderSourceARB"             )
        LOADOPENGLPROC(glCompileShaderARB,              PFNGLCOMPILESHADERARBPROC,              "glCompileShaderARB"            )
        LOADOPENGLPROC(glAttachObjectARB,               PFNGLATTACHOBJECTARBPROC,               "glAttachObjectARB"             )
        LOADOPENGLPROC(glDeleteObjectARB,               PFNGLDELETEOBJECTARBPROC,               "glDeleteObjectARB"             )
        LOADOPENGLPROC(glLinkProgramARB,                PFNGLLINKPROGRAMARBPROC,                "glLinkProgramARB"              )
        LOADOPENGLPROC(glUseProgramObjectARB,           PFNGLUSEPROGRAMOBJECTARBPROC,           "glUseProgramObjectARB"         )
        LOADOPENGLPROC(glGetObjectParameterivARB,       PFNGLGETOBJECTPARAMETERIVARBPROC,       "glGetObjectParameterivARB"     )
        LOADOPENGLPROC(glGetInfoLogARB,                 PFNGLGETINFOLOGARBPROC,                 "glGetInfoLogARB"               )
        LOADOPENGLPROC(glDetachObjectARB,               PFNGLDETACHOBJECTARBPROC,               "glDetachObjectARB"             )
        LOADOPENGLPROC(glGetActiveUniformARB,           PFNGLGETACTIVEUNIFORMARBPROC,           "glGetActiveUniformARB"         )
        LOADOPENGLPROC(glGetUniformLocationARB,         PFNGLGETUNIFORMLOCATIONARBPROC,         "glGetUniformLocationARB"       )
        LOADOPENGLPROC(glUniform1fARB,                  PFNGLUNIFORM1FARBPROC,                  "glUniform1fARB"                )
        LOADOPENGLPROC(glUniform3fARB,                  PFNGLUNIFORM3FARBPROC,                  "glUniform3fARB"                )
        LOADOPENGLPROC(glUniform4fARB,                  PFNGLUNIFORM4FARBPROC,                  "glUniform4fARB"                )
        LOADOPENGLPROC(glUniform1iARB,                  PFNGLUNIFORM1IARBPROC,                  "glUniform1iARB"                )
        LOADOPENGLPROC(glUniform1ivARB,                 PFNGLUNIFORM1IVARBPROC,                 "glUniform1ivARB"               )
        LOADOPENGLPROC(glUniform1fvARB,                 PFNGLUNIFORM1FVARBPROC,                 "glUniform1fvARB"               )
        LOADOPENGLPROC(glUniform2fvARB,                 PFNGLUNIFORM2FVARBPROC,                 "glUniform2fvARB"               )
        LOADOPENGLPROC(glUniform3fvARB,                 PFNGLUNIFORM3FVARBPROC,                 "glUniform3fvARB"               )
        LOADOPENGLPROC(glUniform4fvARB,                 PFNGLUNIFORM4FVARBPROC,                 "glUniform4fvARB"               )
        LOADOPENGLPROC(glUniformMatrix2fvARB,           PFNGLUNIFORMMATRIX2FVARBPROC,           "glUniformMatrix2fvARB"         )
        LOADOPENGLPROC(glUniformMatrix3fvARB,           PFNGLUNIFORMMATRIX3FVARBPROC,           "glUniformMatrix3fvARB"         )
        LOADOPENGLPROC(glUniformMatrix4fvARB,           PFNGLUNIFORMMATRIX4FVARBPROC,           "glUniformMatrix4fvARB"         )
        LOADOPENGLPROC(glEnableVertexAttribArrayARB,    PFNGLENABLEVERTEXATTRIBARRAYARBPROC,    "glEnableVertexAttribArrayARB"  )
        LOADOPENGLPROC(glDisableVertexAttribArrayARB,   PFNGLDISABLEVERTEXATTRIBARRAYARBPROC,   "glDisableVertexAttribArrayARB" )
        LOADOPENGLPROC(glVertexAttribPointerARB,        PFNGLVERTEXATTRIBPOINTERARBPROC,        "glVertexAttribPointerARB"      )
        LOADOPENGLPROC(glBindAttribLocationARB,         PFNGLBINDATTRIBLOCATIONARBPROC,         "glBindAttribLocationARB"       )
        LOADOPENGLPROC(glBindFragDataLocationEXT,       PFNGLBINDFRAGDATALOCATIONEXTPROC,       "glBindFragDataLocationEXT"     )
    }
    else
        io::Log::message("OpenGL Shaders (GLSL) are not supported");
    
    /* Load "GL_ARB_geometry_shader4" extension */
    
    if (RenderQuery_[RENDERQUERY_GEOMETRY_SHADER])
    {
        LOADOPENGLPROC(glProgramParameteriEXT,  PFNGLPROGRAMPARAMETERIEXTPROC,  "glProgramParameteriEXT")
    }
    /*else
        io::Log::message("GeometryShaders are not supported");*/
    
    /* Load "GL_ARB_tessellation_shader" extension */
    
    #ifdef GL_ARB_tessellation_shader
    if (RenderQuery_[RENDERQUERY_TESSELLATION_SHADER])
    {
        LOADOPENGLPROC(glPatchParameteriARB,    PFNGLPATCHPARAMETERIPROC,       "glPatchParameteri"     )
        LOADOPENGLPROC(glPatchParameterfvARB,   PFNGLPATCHPARAMETERFVPROC,      "glPatchParameterfv"    )
    }
    /*else
        io::Log::message("TessellationShaders are not supported");*/
    #endif
    
    /* Load "GL_EXT_fog_coord" extension */
    
    if (RenderQuery_[RENDERQUERY_FOG_COORD])
    {
        LOADOPENGLPROC(glFogCoordPointer,       PFNGLFOGCOORDPOINTERPROC,       "glFogCoordPointer"     )
    }
    else
        io::Log::message("Fog coordinates are not supported");
    
    //LOADOPENGLPROC(glActiveStencilFaceEXT,  PFNGLACTIVESTENCILFACEEXTPROC,  "glActiveStencilFaceEXT")
    
    /* Load "GL_EXT_texture3D" extension */
    
    if (queryVideoSupport(QUERY_VOLUMETRIC_TEXTURE))
    {
        LOADOPENGLPROC(glTexImage3DEXT,         PFNGLTEXIMAGE3DEXTPROC,         "glTexImage3DEXT"       )
        LOADOPENGLPROC(glTexSubImage3DEXT,      PFNGLTEXSUBIMAGE3DEXTPROC,      "glTexSubImage3DEXT"    )
    }
    else
        io::Log::message("Volumetric textures are not supported");
    
    #if defined(SP_PLATFORM_WINDOWS)
    LOADOPENGLPROC(wglSwapIntervalEXT,          PFNWGLSWAPINTERVALFARPROC,      "wglSwapIntervalEXT"    )
    #elif defined(SP_PLATFORM_LINUX)
    LOADOPENGLPROC(glXSwapIntervalSGI,          PFNGLXSWAPINTERVALSGIPROC,      "glXSwapIntervalSGI"    )
    #endif
    
    #undef LOADOPENGLPROC
}

void OpenGLRenderSystem::defaultTextureGenMode()
{
    for (s32 i = 0; i < MultiTextureCount_; ++i)
    {
        if (RenderQuery_[RENDERQUERY_MULTI_TEXTURE])
        {
            /* Active current texture layer */
            glActiveTextureARB(GL_TEXTURE0 + i);
        }
        
        /* Default sphete mapping generation */
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    }
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
