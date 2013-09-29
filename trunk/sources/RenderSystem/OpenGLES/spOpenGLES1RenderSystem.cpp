/*
 * OpenGL|ES 1 render system file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGLES/spOpenGLES1RenderSystem.hpp"

#if defined(SP_COMPILE_WITH_OPENGLES1)


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
 * ======= OpenGLES1Driver class =======
 */

OpenGLES1RenderSystem::OpenGLES1RenderSystem() :
    RenderSystem            (RENDERER_OPENGLES1 ),
    GLFixedFunctionPipeline (                   )
{
}
OpenGLES1RenderSystem::~OpenGLES1RenderSystem()
{
}


/*
 * ========== Render system information ==========
 */

io::stringc OpenGLES1RenderSystem::getVersion() const
{
    return glGetString(GL_VERSION);
}

bool OpenGLES1RenderSystem::queryVideoSupport(const EVideoFeatureSupport Query) const
{
    switch (Query)
    {
        case QUERY_MULTI_TEXTURE:
            return queryExtensionSupport("GL_ARB_multitexture");
        case QUERY_HARDWARE_MESHBUFFER:
        case QUERY_STENCIL_BUFFER:
            return true;
            
        case QUERY_BILINEAR_FILTER:
        case QUERY_TRILINEAR_FILTER:
            return true; // (todo)
        case QUERY_ANISOTROPY_FILTER:
            return queryExtensionSupport("GL_EXT_texture_filter_anisotropic");
        case QUERY_MIPMAPS:
            return true;
        default:
            break;
    }
    
    return false;
}


/*
 * ======= Context functions (for initializing the engine) =======
 */

void OpenGLES1RenderSystem::setupConfiguration()
{
    /* Get OpenGL major and minor version */
    GLMajorVersion_ = 1;
    GLMinorVersion_ = 5;
    
    initExtensionInfo();
    
    /* Default queries */
    RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER]   = true;
    RenderQuery_[RENDERQUERY_MULTI_TEXTURE]         = true;
    
    /* Default settings */
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_NORMALIZE);
    
    /* Hints for the renderer */
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    
    /* Get multi texture count information */
    MultiTextureCount_ = getMultitexCount();
    
    /* Clear video buffer */
    clearBuffers();
}


/*
 * ======= Rendering functions =======
 */

void OpenGLES1RenderSystem::setupMaterialStates(const MaterialStates* Material)
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
    
    /* Fog effect */
    #ifndef SP_PLATFORM_IOS // !!!
    setGlRenderState(GL_FOG, __isFog && Material->getFog());
    #endif
    
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

void OpenGLES1RenderSystem::drawPrimitiveList(
    const ERenderPrimitives Type,
    const scene::SMeshVertex3D* Vertices, u32 VertexCount, const void* Indices, u32 IndexCount,
    const TextureLayerListType* TextureLayers)
{
    if (!Vertices || !VertexCount || Type < PRIMITIVE_POINTS || Type > PRIMITIVE_POLYGON)
        return;
    
    /* Get primtivie type and check if primitive type is supported for OpenGL|ES */
    const GLenum Mode = GLPrimitiveModes[Type];

    if (Mode == ~0)
        return;
    
    /* Enable all client states */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
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
        glDrawElements(Mode, IndexCount, GL_UNSIGNED_SHORT, Indices);
    
    /* Disbale all client states */
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
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

void OpenGLES1RenderSystem::drawMeshBuffer(const MeshBuffer* MeshBuffer)
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
    
    /* Bind textures */
    if (__isTexturing)
        bindTextureList(OrigMeshBuffer->getSurfaceTextureList());
    
    /* Draw the primitives */
    if (MeshBuffer->getIndexBufferEnable())
    {
        glDrawElements(
            Mode,
            MeshBuffer->getIndexCount(),
            GLBasicDataTypes[MeshBuffer->getIndexFormat()->getDataType()],
            (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] ? 0 : MeshBuffer->getIndexBuffer().getArray())
        );
    }
    else
    {
        glDrawArrays(
            Mode, 0, MeshBuffer->getVertexCount()
        );
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
    
    /* Unbind vertex- and index buffer */
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER])
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
}


/*
 * ======= Image drawing =======
 */

void OpenGLES1RenderSystem::draw2DImage(
    Texture* Tex, const dim::point2di &Position, const color &Color)
{
    if (!Tex)
        return;
    
    /* Binding the texture */
    Tex->bind(0);
    
    /* Load 2dimensional matrix */
    setup2DDrawing();
    
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
    
    /* Back settings */
    Tex->unbind(0);
}

void OpenGLES1RenderSystem::draw2DImage(
    Texture* Tex, const dim::rect2di &Position, const dim::rect2df &Clipping, const color &Color)
{
    if (!Tex)
        return;
    
    /* Binding the texture */
    Tex->bind(0);
    
    /* Load 2dimensional matrix */
    setup2DDrawing();
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(static_cast<f32>(Position.Left   ), static_cast<f32>(Position.Top    ), Clipping.Left , Clipping.Top      , Color);
    Vertices2D_[1] = scene::SPrimitiveVertex2D(static_cast<f32>(Position.Right  ), static_cast<f32>(Position.Top    ), Clipping.Right, Clipping.Top      , Color);
    Vertices2D_[2] = scene::SPrimitiveVertex2D(static_cast<f32>(Position.Right  ), static_cast<f32>(Position.Bottom ), Clipping.Right, Clipping.Bottom   , Color);
    Vertices2D_[3] = scene::SPrimitiveVertex2D(static_cast<f32>(Position.Left   ), static_cast<f32>(Position.Bottom ), Clipping.Left , Clipping.Bottom   , Color);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    /* Back settings */
    Tex->unbind(0);
}

void OpenGLES1RenderSystem::draw2DImage(
    Texture* Tex, const dim::point2di &Position, f32 Rotation, f32 Radius, const color &Color)
{
    if (!Tex)
        return;
    
    /* Binding the texture */
    Tex->bind(0);
    
    /* Load 2dimensional matrix */
    setup2DDrawing();
    
    glTranslatef(static_cast<f32>(Position.X), static_cast<f32>(Position.Y), 0.0f);
    glRotatef(Rotation, 0.0f, 0.0f, 1.0f);
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(-Radius, -Radius, 0.0f, 0.0f, Color);
    Vertices2D_[1] = scene::SPrimitiveVertex2D( Radius, -Radius, 1.0f, 0.0f, Color);
    Vertices2D_[2] = scene::SPrimitiveVertex2D( Radius,  Radius, 1.0f, 1.0f, Color);
    Vertices2D_[3] = scene::SPrimitiveVertex2D(-Radius,  Radius, 0.0f, 1.0f, Color);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    /* Back settings */
    Tex->unbind(0);
}

void OpenGLES1RenderSystem::draw2DImage(
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
    setup2DDrawing();
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(static_cast<f32>(lefttopPosition.X       ), static_cast<f32>(lefttopPosition.Y       ), lefttopClipping.X    , lefttopClipping.Y     , lefttopColor      );
    Vertices2D_[1] = scene::SPrimitiveVertex2D(static_cast<f32>(righttopPosition.X      ), static_cast<f32>(righttopPosition.Y      ), righttopClipping.X   , righttopClipping.Y    , righttopColor     );
    Vertices2D_[2] = scene::SPrimitiveVertex2D(static_cast<f32>(rightbottomPosition.X   ), static_cast<f32>(rightbottomPosition.Y   ), rightbottomClipping.X, rightbottomClipping.Y , rightbottomColor  );
    Vertices2D_[3] = scene::SPrimitiveVertex2D(static_cast<f32>(leftbottomPosition.X    ), static_cast<f32>(leftbottomPosition.Y    ), leftbottomClipping.X , leftbottomClipping.Y  , leftbottomColor   );
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    /* Back settings */
    Tex->unbind(0);
}


/*
 * ======= Primitive drawing =======
 */

void OpenGLES1RenderSystem::draw2DLine(
    const dim::point2di &PositionA, const dim::point2di &PositionB, const color &Color)
{
    setup2DDrawing();
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(static_cast<f32>(PositionA.X), static_cast<f32>(PositionA.Y), 0.0f, 0.0f, Color);
    Vertices2D_[1] = scene::SPrimitiveVertex2D(static_cast<f32>(PositionB.X), static_cast<f32>(PositionB.Y), 0.0f, 0.0f, Color);
    
    glDrawArrays(GL_LINES, 0, 2);
}

void OpenGLES1RenderSystem::draw2DLine(
    const dim::point2di &PositionA, const dim::point2di &PositionB, const color &ColorA, const color &ColorB)
{
    setup2DDrawing();
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(static_cast<f32>(PositionA.X), static_cast<f32>(PositionA.Y), 0.0f, 0.0f, ColorA);
    Vertices2D_[1] = scene::SPrimitiveVertex2D(static_cast<f32>(PositionB.X), static_cast<f32>(PositionB.Y), 0.0f, 0.0f, ColorB);
    
    glDrawArrays(GL_LINES, 0, 2);
}

void OpenGLES1RenderSystem::draw2DRectangle(const dim::rect2di &Rect, const color &Color, bool isSolid)
{
    setup2DDrawing();
    
    const dim::rect2df RectF(Rect.cast<f32>());
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(RectF.Left   , RectF.Top     , 0.0f, 0.0f, Color);
    Vertices2D_[1] = scene::SPrimitiveVertex2D(RectF.Right  , RectF.Top     , 0.0f, 0.0f, Color);
    Vertices2D_[2] = scene::SPrimitiveVertex2D(RectF.Right  , RectF.Bottom  , 0.0f, 0.0f, Color);
    Vertices2D_[3] = scene::SPrimitiveVertex2D(RectF.Left   , RectF.Bottom  , 0.0f, 0.0f, Color);
    
    glDrawArrays(isSolid ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, 4);
}

void OpenGLES1RenderSystem::draw2DRectangle(
    const dim::rect2di &Rect, const color &lefttopColor, const color &righttopColor,
    const color &rightbottomColor, const color &leftbottomColor, bool isSolid)
{
    setup2DDrawing();
    
    const dim::rect2df RectF(Rect.cast<f32>());
    
    Vertices2D_[0] = scene::SPrimitiveVertex2D(RectF.Left   , RectF.Top     , 0.0f, 0.0f, lefttopColor      );
    Vertices2D_[1] = scene::SPrimitiveVertex2D(RectF.Right  , RectF.Top     , 0.0f, 0.0f, righttopColor     );
    Vertices2D_[2] = scene::SPrimitiveVertex2D(RectF.Right  , RectF.Bottom  , 0.0f, 0.0f, rightbottomColor  );
    Vertices2D_[3] = scene::SPrimitiveVertex2D(RectF.Left   , RectF.Bottom  , 0.0f, 0.0f, leftbottomColor   );
    
    glDrawArrays(isSolid ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, 4);
}


/*
 * ======= Extra drawing functions =======
 */

void OpenGLES1RenderSystem::draw2DPolygon(
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
 
void OpenGLES1RenderSystem::draw3DPoint(const dim::vector3df &Position, const color &Color)
{
    setup3DDrawing();
    
    scene::SMeshVertex3D Vertices[1] = {
        scene::SMeshVertex3D(Position.X, Position.Y, Position.Z, Color)
    };
    
    drawPrimitiveList(PRIMITIVE_POINTS, Vertices, 1, 0, 0, 0);
}

void OpenGLES1RenderSystem::draw3DLine(
    const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &Color)
{
    setup3DDrawing();
    
    scene::SMeshVertex3D Vertices[2] = {
        scene::SMeshVertex3D(PositionA.X, PositionA.Y, PositionA.Z, Color),
        scene::SMeshVertex3D(PositionB.X, PositionB.Y, PositionB.Z, Color)
    };
    
    drawPrimitiveList(PRIMITIVE_LINES, Vertices, 2, 0, 0, 0);
}

void OpenGLES1RenderSystem::draw3DLine(
    const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &ColorA, const color &ColorB)
{
    setup3DDrawing();
    
    scene::SMeshVertex3D Vertices[2] = {
        scene::SMeshVertex3D(PositionA.X, PositionA.Y, PositionA.Z, ColorA),
        scene::SMeshVertex3D(PositionB.X, PositionB.Y, PositionB.Z, ColorB)
    };
    
    drawPrimitiveList(PRIMITIVE_LINES, Vertices, 2, 0, 0, 0);
}

void OpenGLES1RenderSystem::draw3DTriangle(
    Texture* Tex, const dim::triangle3df &Triangle, const color &Color)
{
    setup3DDrawing();
    
    if (Tex)
        Tex->bind(0);
    
    scene::SMeshVertex3D Vertices[3] = {
        scene::SMeshVertex3D(Triangle.PointA.X, Triangle.PointA.Y, Triangle.PointA.Z, Color),
        scene::SMeshVertex3D(Triangle.PointB.X, Triangle.PointB.Y, Triangle.PointB.Z, Color),
        scene::SMeshVertex3D(Triangle.PointC.X, Triangle.PointC.Y, Triangle.PointC.Z, Color)
    };
    
    drawPrimitiveList(PRIMITIVE_TRIANGLES, Vertices, 3, 0, 0, 0);
    
    if (Tex)
        Tex->unbind(0);
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
