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


//!!!
//#define __DRAW2DARRAYS__

namespace sp
{

extern SoftPixelDevice* GlbEngineDev;
extern io::InputControl* GlbInputCtrl;
extern scene::SceneGraph* GlbSceneGraph;

namespace video
{


/*
 * ======= Internal members =======
 */

extern s32 GLCompareList[];
extern GLenum GLPrimitiveModes[];
extern GLenum GLBasicDataTypes[];
extern s32 GLBlendingList[];
extern GLenum GLStencilOperationList[];


#if 0//!!!

// DEBUG FUNCTION
static void PrintGLErr(const io::stringc &PointDesc, bool Always = true)
{
    const GLenum Err = glGetError();
    if (Err != GL_NO_ERROR)
    {
        switch (Err)
        {
            case GL_INVALID_ENUM:
                io::Log::error("GL ERR [ \"" + PointDesc + "\" ]: Invalid Enumeration");
                break;
            case GL_INVALID_VALUE:
                io::Log::error("GL ERR [ \"" + PointDesc + "\" ]: Invalid Value");
                break;
            case GL_INVALID_OPERATION:
                io::Log::error("GL ERR [ \"" + PointDesc + "\" ]: Invalid Operation");
                break;
            case GL_STACK_OVERFLOW:
                io::Log::error("GL ERR [ \"" + PointDesc + "\" ]: Stack Overflow");
                break;
            case GL_STACK_UNDERFLOW:
                io::Log::error("GL ERR [ \"" + PointDesc + "\" ]: Stack Underflow");
                break;
            case GL_OUT_OF_MEMORY:
                io::Log::error("GL ERR [ \"" + PointDesc + "\" ]: Out Of Memory");
                break;
            default:
                io::Log::error("GL ERR [ \"" + PointDesc + "\" ]: Unknown (Code = " + io::stringc(Err) + ")");
                break;
        }
    }
    else if (Always)
        io::Log::message("Successful [ \"" + PointDesc + "\" ]");
}

#endif


/*
 * ======= OpenGLRenderSystem class =======
 */

OpenGLRenderSystem::OpenGLRenderSystem() :
    RenderSystem                    (RENDERER_OPENGL),
    GLFixedFunctionPipeline         (               ),
    GLProgrammableFunctionPipeline  (               ),
    PrevBoundMeshBuffer_            (0              )
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
    glGetIntegerv(GL_MAJOR_VERSION, &GLMajorVersion_);
    glGetIntegerv(GL_MINOR_VERSION, &GLMinorVersion_);
    
    /* Default queries */
    RenderQuery_[RENDERQUERY_SHADER                     ] = queryVideoSupport(QUERY_SHADER                  );
    RenderQuery_[RENDERQUERY_GEOMETRY_SHADER            ] = queryVideoSupport(QUERY_GEOMETRY_SHADER         );
    RenderQuery_[RENDERQUERY_TESSELLATION_SHADER        ] = queryVideoSupport(QUERY_TESSELLATION_SHADER     );
    RenderQuery_[RENDERQUERY_COMPUTE_SHADER             ] = queryVideoSupport(QUERY_COMPUTE_SHADER          );
    RenderQuery_[RENDERQUERY_CONSTANT_BUFFER            ] = queryVideoSupport(QUERY_CONSTANT_BUFFER         );
    
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
    
    /**
    This is a very important GL function call:
    It sets the pixel-storage configuration to byte-alignment (the default setting is word-alignment).
    This is required to load textures with unusual sizes.
    */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
        case QUERY_CONSTANT_BUFFER:
            return queryExtensionSupport("GL_ARB_uniform_buffer_object");
        case QUERY_COMPUTE_SHADER:
            return queryExtensionSupport("GL_ARB_compute_shader");
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
    RenderSystem::setDepthClip(Enable);
    setGlRenderState(GL_DEPTH_CLAMP, !Enable);
}


/*
 * ======= Stencil buffer =======
 */

void OpenGLRenderSystem::setStencilMask(u32 BitMask)
{
    glStencilMask(BitMask);
}
void OpenGLRenderSystem::setStencilMethod(const ESizeComparisionTypes Method, s32 Reference, u32 BitMask)
{
    glStencilFunc(GLCompareList[Method], Reference, BitMask);
}
void OpenGLRenderSystem::setStencilOperation(const EStencilOperations FailOp, const EStencilOperations ZFailOp, const EStencilOperations ZPassOp)
{
    glStencilOp(GLStencilOperationList[FailOp], GLStencilOperationList[ZFailOp], GLStencilOperationList[ZPassOp]);
}
void OpenGLRenderSystem::setClearStencil(s32 Stencil)
{
    glClearStencil(Stencil);
}


/*
 * ======= Rendering functions =======
 */

bool OpenGLRenderSystem::setupMaterialStates(const MaterialStates* Material, bool Forced)
{
    /* Check for equality to optimize render path */
    if ( !Material || ( !Forced && ( PrevMaterial_ == Material || Material->compare(PrevMaterial_) ) ) )
        return false;
    
    PrevMaterial_ = Material;
    
    /* Face culling & polygon mode */
    switch (Material->getRenderFace())
    {
        case FACE_FRONT:
        {
            /* Cull back face */
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            
            /* Setup wireframe for front face */
            glPolygonMode(GL_FRONT, GL_POINT + Material->getWireframeFront());
        }
        break;
        
        case FACE_BACK:
        {
            /* Cull front face */
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            
            /* Setup wireframe for back face */
            glPolygonMode(GL_BACK, GL_POINT + Material->getWireframeBack());
        }
        break;
        
        case FACE_BOTH:
        {
            /* Disable face culling */
            glDisable(GL_CULL_FACE);
            
            /* Setup wireframe for front and back face */
            if (Material->getWireframeFront() != Material->getWireframeBack())
            {
                glPolygonMode(GL_BACK, GL_POINT + Material->getWireframeFront());
                glPolygonMode(GL_FRONT, GL_POINT + Material->getWireframeBack());
            }
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT + Material->getWireframeFront());
        }
        break;
    }
    
    //if (!CurShaderClass_)
    {
        /* Fog effect */
        setGlRenderState(GL_FOG, __isFog && Material->getFog());
        
        /* Color material */
        setGlRenderState(GL_COLOR_MATERIAL, Material->getColorMaterial());
        
        /* Lighting material */
        if (__isLighting && Material->getLighting())
        {
            glEnable(GL_LIGHTING);
            
            /* Light model */
            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, Material->getRenderFace() == FACE_BOTH ? 1 : 0);
            
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
        
        /* Alpha function */
        glAlphaFunc(GLCompareList[Material->getAlphaMethod()], Material->getAlphaReference());
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
    
    #ifdef SP_DEBUGMODE
    ++RenderSystem::NumMaterialUpdates_;
    #endif
    
    return true;
}

void OpenGLRenderSystem::endSceneRendering()
{
    RenderSystem::endSceneRendering();
    
    /* Unbind previously bounded mesh buffer and vertex format */
    unbindPrevBoundMeshBuffer();
    
    PrevMaterial_ = 0;
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
        ShaderSurfaceCallback_(CurShaderClass_, MeshBuffer->getTextureLayerList());
    
    /* Bind mesh buffer and vertex format */
    bindMeshBuffer(MeshBuffer);
    
    /* Bind textures */
    if (__isTexturing)
        bindTextureLayers(OrigMeshBuffer->getTextureLayerList());
    else
        unbindPrevTextureLayers();
    
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
    
    #ifdef SP_DEBUGMODE
    ++RenderSystem::NumDrawCalls_;
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
    
    //!TODO! -> also optimize the render path for this function!!!
    /* Unbind previously bounding mesh buffer and vertex format */
    unbindPrevBoundMeshBuffer();
    
    /* Bind hardware vertex- and index buffers */
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER])
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, *(u32*)MeshBuffer->getVertexBufferID());
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, *(u32*)MeshBuffer->getIndexBufferID());
    }
    
    /* Vertex data pointers */
    const VertexFormat* Format  = MeshBuffer->getVertexFormat();
    const s32 FormatSize        = Format->getFormatSize();
    
    const c8* VBOPointerOffset  = (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] ? 0 : (const c8*)MeshBuffer->getVertexBuffer().getArray());
    
    /* Setup vertex coordinates */
    if (Format->getFlags() & VERTEXFORMAT_COORD)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(Format->getCoord().Size, GLBasicDataTypes[Format->getCoord().Type], FormatSize, VBOPointerOffset + Format->getCoord().Offset);
    }
    
    /* Setup vertex texture coordinates */
    const bool isTexCoordBind = ( useFirstTextureLayer && (Format->getFlags() & VERTEXFORMAT_TEXCOORDS) && !Format->getTexCoords().empty() );
    
    if (isTexCoordBind)
    {
        std::vector<SVertexAttribute>::const_iterator it = Format->getTexCoords().begin();
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(it->Size, GLBasicDataTypes[it->Type], FormatSize, VBOPointerOffset + it->Offset);
    }
    
    /* Bind textures */
    const bool isTextureBind = ( __isTexturing && useFirstTextureLayer && !OrigMeshBuffer->getTextureLayerList().empty() );
    
    TextureLayer* FirstTexLayer = 0;
    
    if (isTextureBind)
    {
        FirstTexLayer = OrigMeshBuffer->getTextureLayerList().front();
        FirstTexLayer->bind();
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
    if (isTextureBind && FirstTexLayer)
        FirstTexLayer->unbind();
    
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
    
    #ifdef SP_DEBUGMODE
    ++RenderSystem::NumDrawCalls_;
    ++RenderSystem::NumMeshBufferBindings_;
    #endif
}


/*
 * ======= Shader programs =======
 */

Shader* OpenGLRenderSystem::createCgShader(
    ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
    const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint,
    const c8** CompilerOptions)
{
    #ifdef SP_COMPILE_WITH_CG
    if (!gSharedObjects.CgContext)
    {
        io::Log::error("Missing Cg context to create shader");
        return 0;
    }
    #endif
    
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
    const Texture* Tex, const dim::point2di &Position, const color &Color)
{
    if (!Tex)
        return;
    
    #ifndef __DRAW2DARRAYS__
    /* Coloring */
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    #endif
    
    setup2DDrawing();
    
    /* Binding the texture */
    Tex->bind(0);
    
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
    const Texture* Tex, const dim::rect2di &Position, const dim::rect2df &Clipping, const color &Color)
{
    if (!Tex)
        return;
    
    #ifndef __DRAW2DARRAYS__
    /* Coloring */
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    #endif
    
    setup2DDrawing();
    
    /* Binding the texture */
    Tex->bind(0);
    
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
    const Texture* Tex, const dim::point2di &Position, f32 Rotation, f32 Radius, const color &Color)
{
    if (!Tex)
        return;
    
    #if 0
    #ifndef __DRAW2DARRAYS__
    /* Temporary variables */
    Radius *= math::SQRT2F;
    
    const dim::point2df lefttopPos      (math::Sin(Rotation -  45)*Radius + Position.X, -math::Cos(Rotation -  45)*Radius + Position.Y);
    const dim::point2df righttopPos     (math::Sin(Rotation +  45)*Radius + Position.X, -math::Cos(Rotation +  45)*Radius + Position.Y);
    const dim::point2df rightbottomPos  (math::Sin(Rotation + 135)*Radius + Position.X, -math::Cos(Rotation + 135)*Radius + Position.Y);
    const dim::point2df leftbottomPos   (math::Sin(Rotation - 135)*Radius + Position.X, -math::Cos(Rotation - 135)*Radius + Position.Y);
    #endif
    #endif
    
    #ifndef __DRAW2DARRAYS__
    /* Coloring */
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    #endif
    
    setup2DDrawing();
    
    /* Setup transformation */
    glTranslatef(static_cast<f32>(Position.X), static_cast<f32>(Position.Y), 0.0f);
    glRotatef(Rotation, 0.0f, 0.0f, 1.0f);
    
    /* Binding the texture */
    Tex->bind(0);
    
    #ifdef __DRAW2DARRAYS__
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(-Radius, -Radius, 0.0f, 0.0f, Color);
    Vertices2D_[1] = scene::SPrimitiveVertex2D( Radius, -Radius, 1.0f, 0.0f, Color);
    Vertices2D_[2] = scene::SPrimitiveVertex2D( Radius,  Radius, 1.0f, 1.0f, Color);
    Vertices2D_[3] = scene::SPrimitiveVertex2D(-Radius,  Radius, 0.0f, 1.0f, Color);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    #else
    
    /* Drawing the quad */
    glBegin(GL_QUADS);
    {
        #if 0
        glTexCoord2i(0, 0);
        glVertex2f(lefttopPos.X, lefttopPos.Y);
        
        glTexCoord2i(1, 0);
        glVertex2f(righttopPos.X, righttopPos.Y);
        
        glTexCoord2i(1, 1);
        glVertex2f(rightbottomPos.X, rightbottomPos.Y);
        
        glTexCoord2i(0, 1);
        glVertex2f(leftbottomPos.X, leftbottomPos.Y);
        #else
        glTexCoord2i(0, 0);
        glVertex2f(-Radius, -Radius);
        
        glTexCoord2i(1, 0);
        glVertex2f(Radius, -Radius);
        
        glTexCoord2i(1, 1);
        glVertex2f(Radius, Radius);
        
        glTexCoord2i(0, 1);
        glVertex2f(-Radius, Radius);
        #endif
    }
    glEnd();
    
    #endif
    
    glLoadIdentity();
    
    /* Back settings */
    Tex->unbind(0);
}

void OpenGLRenderSystem::draw2DImage(
    const Texture* Tex,
    const dim::point2di &lefttopPosition, const dim::point2di &righttopPosition,
    const dim::point2di &rightbottomPosition, const dim::point2di &leftbottomPosition,
    const dim::point2df &lefttopClipping, const dim::point2df &righttopClipping,
    const dim::point2df &rightbottomClipping, const dim::point2df &leftbottomClipping,
    const color &lefttopColor, const color &righttopColor,
    const color &rightbottomColor, const color &leftbottomColor)
{
    if (!Tex)
        return;
    
    setup2DDrawing();
    
    /* Texture binding */
    Tex->bind(0);
    
    #ifdef __DRAW2DARRAYS__
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(static_cast<f32>(lefttopPosition.X       ), static_cast<f32>(lefttopPosition.Y       ), lefttopClipping.X    , lefttopClipping.Y     , lefttopColor      );
    Vertices2D_[1] = scene::SPrimitiveVertex2D(static_cast<f32>(righttopPosition.X      ), static_cast<f32>(righttopPosition.Y      ), righttopClipping.X   , righttopClipping.Y    , righttopColor     );
    Vertices2D_[2] = scene::SPrimitiveVertex2D(static_cast<f32>(rightbottomPosition.X   ), static_cast<f32>(rightbottomPosition.Y   ), rightbottomClipping.X, rightbottomClipping.Y , rightbottomColor  );
    Vertices2D_[3] = scene::SPrimitiveVertex2D(static_cast<f32>(leftbottomPosition.X    ), static_cast<f32>(leftbottomPosition.Y    ), leftbottomClipping.X , leftbottomClipping.Y  , leftbottomColor   );
    
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
    setup2DDrawing();
    
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
    setup2DDrawing();
    
    #ifdef __DRAW2DARRAYS__
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(static_cast<f32>(PositionA.X), static_cast<f32>(PositionA.Y), 0.0f, 0.0f, Color);
    Vertices2D_[1] = scene::SPrimitiveVertex2D(static_cast<f32>(PositionB.X), static_cast<f32>(PositionB.Y), 0.0f, 0.0f, Color);
    
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
    setup2DDrawing();
    
    #ifdef __DRAW2DARRAYS__
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(static_cast<f32>(PositionA.X), static_cast<f32>(PositionA.Y), 0.0f, 0.0f, ColorA);
    Vertices2D_[1] = scene::SPrimitiveVertex2D(static_cast<f32>(PositionB.X), static_cast<f32>(PositionB.Y), 0.0f, 0.0f, ColorB);
    
    glDrawArrays(GL_LINES, 0, 2);
    
    #else
    
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
    setup2DDrawing();
    
    #ifdef __DRAW2DARRAYS__
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(static_cast<f32>(Rect.Left   ), static_cast<f32>(Rect.Top    ), 0.0f, 0.0f, Color);
    Vertices2D_[1] = scene::SPrimitiveVertex2D(static_cast<f32>(Rect.Right  ), static_cast<f32>(Rect.Top    ), 0.0f, 0.0f, Color);
    Vertices2D_[2] = scene::SPrimitiveVertex2D(static_cast<f32>(Rect.Right  ), static_cast<f32>(Rect.Bottom ), 0.0f, 0.0f, Color);
    Vertices2D_[3] = scene::SPrimitiveVertex2D(static_cast<f32>(Rect.Left   ), static_cast<f32>(Rect.Bottom ), 0.0f, 0.0f, Color);
    
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
    setup2DDrawing();
    
    #ifdef __DRAW2DARRAYS__
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(static_cast<f32>(Rect.Left   ), static_cast<f32>(Rect.Top    ), 0.0f, 0.0f, lefttopColor      );
    Vertices2D_[1] = scene::SPrimitiveVertex2D(static_cast<f32>(Rect.Right  ), static_cast<f32>(Rect.Top    ), 0.0f, 0.0f, righttopColor     );
    Vertices2D_[2] = scene::SPrimitiveVertex2D(static_cast<f32>(Rect.Right  ), static_cast<f32>(Rect.Bottom ), 0.0f, 0.0f, rightbottomColor  );
    Vertices2D_[3] = scene::SPrimitiveVertex2D(static_cast<f32>(Rect.Left   ), static_cast<f32>(Rect.Bottom ), 0.0f, 0.0f, leftbottomColor   );
    
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
    
    setup2DDrawing();
    
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
    setup3DDrawing();
    
    #ifdef __DRAW2DARRAYS__
    
    scene::SMeshVertex3D Vertex(Position.X, Position.Y, Position.Z, Color);
    drawPrimitiveList(PRIMITIVE_POINTS, &Vertex, 1, 0, 0, 0);
    
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
    setup3DDrawing();
    
    #ifdef __DRAW2DARRAYS__
    
    scene::SMeshVertex3D Vertices[2] =
    {
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
    setup3DDrawing();
    
    #ifdef __DRAW2DARRAYS__
    
    scene::SMeshVertex3D Vertices[2] =
    {
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
    setup3DDrawing();
    
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
    
    glLoadIdentity();
}

void OpenGLRenderSystem::draw3DTriangle(
    Texture* Tex, const dim::triangle3df &Triangle, const color &Color)
{
    setup3DDrawing();
    
    if (Tex)
        Tex->bind(0);
    
    #ifdef __DRAW2DARRAYS__
    
    scene::SMeshVertex3D Vertices[3] =
    {
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
        static_cast<SoftPixelDeviceLinux*>(GlbEngineDev)->Display_, X11FontName.c_str()
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
    const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color)
{
    const dim::size2di FontSize(FontObj->getSize());
    
    setRenderMode(RENDERMODE_NONE);
    
    /* Reset projection and material states */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_POLYGON_OFFSET_FILL);
    
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
    if (!FontObject || !FontObject->getBufferRawData() || FontObject->getTexture())
        return;
    
    glLoadIdentity();
    
    /* Coloring (before loacting raster position) */
    glColor4ub(Color.Red, Color.Green, Color.Blue, Color.Alpha);
    
    /* Check if a camera is used */
    if (GlbSceneGraph && GlbSceneGraph->getActiveCamera())
    {
        /* Update the modelview matrix of the camera */
        GlbSceneGraph->getActiveCamera()->updateTransformation();
        
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
    if (!FontObj || !FontObj->getBufferRawData())
        return;
    
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
            static_cast<SoftPixelDeviceLinux*>(GlbEngineDev)->Display_, FontPackage->FontID
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

void OpenGLRenderSystem::loadExtensions()
{
    /* Load swap-interval procs */
    if (!GLExtensionLoader::loadSwapIntervalProcs())
        io::Log::message("Swap interval is not supported");
    
    /* Load multi-texture procs */
    if (!RenderQuery_[RENDERQUERY_MULTI_TEXTURE] || !GLExtensionLoader::loadMultiTextureProcs())
    {
        RenderQuery_[RENDERQUERY_MULTI_TEXTURE] = false;
        io::Log::message("Multi-texturing is not supported");
    }
    
    /* Load VBO procs */
    if (!RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] || !GLExtensionLoader::loadVBOProcs())
    {
        RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] = false;
        io::Log::message("VertexBufferObjects (VBO) are not supported");
    }
    
    /* Load FBO procs */
    if (RenderQuery_[RENDERQUERY_RENDERTARGET] && GLExtensionLoader::loadFBOProcs())
    {
        if (!RenderQuery_[RENDERQUERY_MULTISAMPLE_RENDERTARGET] || !GLExtensionLoader::loadFBOMultiSampledProcs())
            RenderQuery_[RENDERQUERY_MULTISAMPLE_RENDERTARGET] = false;
    }
    else
    {
        RenderQuery_[RENDERQUERY_RENDERTARGET] = false;
        RenderQuery_[RENDERQUERY_MULTISAMPLE_RENDERTARGET] = false;
        io::Log::message("FrameBufferObjects (FBO) are not supported");
    }
    
    /* Load draw instanced procs */
    if (!RenderQuery_[RENDERQUERY_HARDWARE_INSTANCING] || !GLExtensionLoader::loadDrawInstancedProcs())
        RenderQuery_[RENDERQUERY_HARDWARE_INSTANCING] = false;
    
    /* Load shader procs */
    if (RenderQuery_[RENDERQUERY_SHADER] && GLExtensionLoader::loadShaderProcs())
    {
        if (!RenderQuery_[RENDERQUERY_CONSTANT_BUFFER] || !GLExtensionLoader::loadShaderConstBufferProcs())
            RenderQuery_[RENDERQUERY_CONSTANT_BUFFER] = false;
        if (!RenderQuery_[RENDERQUERY_GEOMETRY_SHADER] || !GLExtensionLoader::loadGeometryShaderProcs())
            RenderQuery_[RENDERQUERY_GEOMETRY_SHADER] = false;
        if (!RenderQuery_[RENDERQUERY_TESSELLATION_SHADER] || !GLExtensionLoader::loadTessellationShaderProcs())
            RenderQuery_[RENDERQUERY_TESSELLATION_SHADER] = false;
        if (!RenderQuery_[RENDERQUERY_COMPUTE_SHADER] || !GLExtensionLoader::loadComputeShaderProcs())
            RenderQuery_[RENDERQUERY_COMPUTE_SHADER] = false;
    }
    else
    {
        for (u32 i = RENDERQUERY_SHADER; i <= RENDERQUERY_CONSTANT_BUFFER; ++i)
            RenderQuery_[i] = false;
        io::Log::message("OpenGL Shaders (GLSL) are not supported");
    }
    
    /* Load fog-coordinate procs */
    if (!RenderQuery_[RENDERQUERY_FOG_COORD] || !GLExtensionLoader::loadFogCoordProcs())
        io::Log::message("Fog coordinates are not supported");
    
    /* Load image 3D procs */
    if (!queryVideoSupport(QUERY_VOLUMETRIC_TEXTURE) || !GLExtensionLoader::loadTex3DProcs())
        io::Log::message("Volumetric textures are not supported");
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

void OpenGLRenderSystem::bindMeshBuffer(const MeshBuffer* MeshBuffer)
{
    /* Check if this mesh buffer is already bound */
    if (PrevBoundMeshBuffer_ == MeshBuffer)
        return;
    
    /* Unbind previously bounded mesh buffer */
    unbindPrevBoundMeshBuffer();
    
    PrevBoundMeshBuffer_ = MeshBuffer;
    
    /* Bind hardware vertex- and index buffers */
    const c8* VBOPointerOffset = 0;
    
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER])
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, *(u32*)MeshBuffer->getVertexBufferID());
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, *(u32*)MeshBuffer->getIndexBufferID());
    }
    else
        VBOPointerOffset = (const c8*)MeshBuffer->getVertexBuffer().getArray();
    
    /* Vertex data pointers */
    const VertexFormat* Format = MeshBuffer->getVertexFormat();
    const s32 FormatSize = Format->getFormatSize();
    
    /* Setup vertex coordinates */
    if ((Format->getFlags() & VERTEXFORMAT_COORD) != 0 && !Format->getCoord().isReference)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(Format->getCoord().Size, GLBasicDataTypes[Format->getCoord().Type], FormatSize, VBOPointerOffset + Format->getCoord().Offset);
    }
    
    /* Setup vertex normals */
    if (Format->getFlags() & VERTEXFORMAT_NORMAL)
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GLBasicDataTypes[Format->getNormal().Type], FormatSize, VBOPointerOffset + Format->getNormal().Offset);
    }
    
    /* Setup vertex colors */
    if (Format->getFlags() & VERTEXFORMAT_COLOR)
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(Format->getColor().Size, GLBasicDataTypes[Format->getColor().Type], FormatSize, VBOPointerOffset + Format->getColor().Offset);
    }
    
    /* Setup vertex fog coordinates */
    if ((Format->getFlags() & VERTEXFORMAT_FOGCOORD) && RenderQuery_[RENDERQUERY_FOG_COORD])
    {
        glEnableClientState(GL_FOG_COORDINATE_ARRAY);
        glFogCoordPointer(GLBasicDataTypes[Format->getFogCoord().Type], FormatSize, VBOPointerOffset + Format->getFogCoord().Offset);
    }
    
    /* Setup vertex texture coordinates */
    if (Format->getFlags() & VERTEXFORMAT_TEXCOORDS)
    {
        if (RenderQuery_[RENDERQUERY_MULTI_TEXTURE])
        {
            u32 i = 0;
            for (std::vector<SVertexAttribute>::const_iterator it = Format->getTexCoords().begin(); it != Format->getTexCoords().end(); ++it, ++i)
            {
                if (!it->isReference)
                {
                    glClientActiveTextureARB(GL_TEXTURE0 + i);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glTexCoordPointer(it->Size, GLBasicDataTypes[it->Type], FormatSize, VBOPointerOffset + it->Offset);
                }
            }
        }
        else if (!Format->getTexCoords().empty())
        {
            std::vector<SVertexAttribute>::const_iterator it = Format->getTexCoords().begin();
            if (!it->isReference)
            {
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(it->Size, GLBasicDataTypes[it->Type], FormatSize, VBOPointerOffset + it->Offset);
            }
        }
    }
    
    /* Setup universal vertex attributes */
    if ((Format->getFlags() & VERTEXFORMAT_UNIVERSAL) && RenderQuery_[RENDERQUERY_SHADER])
    {
        u32 i = 0;
        foreach (const SVertexAttribute &Attrib, Format->getUniversals())
        {
            glEnableVertexAttribArrayARB(i);
            glVertexAttribPointerARB(i, Attrib.Size, GLBasicDataTypes[Attrib.Type], Attrib.Normalize, FormatSize, VBOPointerOffset + Attrib.Offset);
            ++i;
        }
    }
    
    #ifdef SP_DEBUGMODE
    ++RenderSystem::NumMeshBufferBindings_;
    #endif
}

void OpenGLRenderSystem::unbindMeshBuffer(const MeshBuffer* MeshBuffer)
{
    /* Unbind vertex- and index buffer */
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER])
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    
    /* Unbind vertex format */
    const VertexFormat* Format = MeshBuffer->getVertexFormat();
    
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
}

void OpenGLRenderSystem::unbindPrevBoundMeshBuffer()
{
    if (PrevBoundMeshBuffer_)
    {
        unbindMeshBuffer(PrevBoundMeshBuffer_);
        PrevBoundMeshBuffer_ = 0;
    }
}

//!!!DEPRECATED!!! -> remove
void OpenGLRenderSystem::drawPrimitiveList(
    const ERenderPrimitives Type,
    const scene::SMeshVertex3D* Vertices, u32 VertexCount, const void* Indices, u32 IndexCount,
    const TextureLayerListType* TextureLayers)
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
    if (__isTexturing && TextureLayers)
        bindTextureLayers(*TextureLayers);
    
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
}

GLenum OpenGLRenderSystem::getGL3TexFormat(const EHWTextureFormats HWTexFormat, const EPixelFormats PixelFormat)
{
    switch (HWTexFormat)
    {
        case HWTEXFORMAT_UBYTE8:
        {
            switch (PixelFormat)
            {
                case PIXELFORMAT_ALPHA:
                case PIXELFORMAT_GRAY:
                    return GL_R8UI;
                case PIXELFORMAT_GRAYALPHA:
                    return GL_RG8UI;
                case PIXELFORMAT_RGB:
                case PIXELFORMAT_BGR:
                    return 0;
                case PIXELFORMAT_RGBA:
                case PIXELFORMAT_BGRA:
                    return GL_RGBA8UI;
                default:
                    return 0;
            }
        }
        break;
        
        case HWTEXFORMAT_FLOAT16:
        {
            switch (PixelFormat)
            {
                case PIXELFORMAT_ALPHA:
                case PIXELFORMAT_GRAY:
                    return GL_R16F;
                case PIXELFORMAT_GRAYALPHA:
                    return GL_RG16F;
                case PIXELFORMAT_RGB:
                case PIXELFORMAT_BGR:
                    return GL_RGB16F;
                case PIXELFORMAT_RGBA:
                case PIXELFORMAT_BGRA:
                    return GL_RGBA16F;
                default:
                    return 0;
            }
        }
        break;
        
        case HWTEXFORMAT_FLOAT32:
        {
            switch (PixelFormat)
            {
                case PIXELFORMAT_ALPHA:
                case PIXELFORMAT_GRAY:
                    return GL_R32F;
                case PIXELFORMAT_GRAYALPHA:
                    return GL_RG32F;
                case PIXELFORMAT_RGB:
                case PIXELFORMAT_BGR:
                    return GL_RGB32F;
                case PIXELFORMAT_RGBA:
                case PIXELFORMAT_BGRA:
                    return GL_RGBA32F;
                default:
                    return 0;
            }
        }
        break;
        
        case HWTEXFORMAT_INT32:
        {
            switch (PixelFormat)
            {
                case PIXELFORMAT_ALPHA:
                case PIXELFORMAT_GRAY:
                    return GL_R32I;
                case PIXELFORMAT_GRAYALPHA:
                    return GL_RG32I;
                case PIXELFORMAT_RGB:
                case PIXELFORMAT_BGR:
                    return 0;
                case PIXELFORMAT_RGBA:
                case PIXELFORMAT_BGRA:
                    return GL_RGBA32I;
                default:
                    return 0;
            }
        }
        break;
        
        case HWTEXFORMAT_UINT32:
        {
            switch (PixelFormat)
            {
                case PIXELFORMAT_ALPHA:
                case PIXELFORMAT_GRAY:
                    return GL_R32UI;
                case PIXELFORMAT_GRAYALPHA:
                    return GL_RG32UI;
                case PIXELFORMAT_RGB:
                case PIXELFORMAT_BGR:
                    return 0;
                case PIXELFORMAT_RGBA:
                case PIXELFORMAT_BGRA:
                    return GL_RGBA32UI;
                default:
                    return 0;
            }
        }
        break;
    }
    return 0;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
