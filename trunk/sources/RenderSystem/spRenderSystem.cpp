/*
 * Render system file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spRenderSystem.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "Framework/Cg/spCgShaderClass.hpp"
#include "Framework/Tools/spToolXMLParser.hpp"

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace video
{


RenderSystem::RenderSystem(const ERenderSystems Type) :
    RendererType_           (Type               ),
    
    #if defined(SP_PLATFORM_WINDOWS)
    DeviceContext_          (0                  ),
    PixelFormat_            (0                  ),
    #elif defined(SP_PLATFORM_LINUX)
    Display_                (0                  ),
    Window_                 (0                  ),
    #endif
    
    MaxClippingPlanes_      (0                  ),
    isFrontFace_            (true               ),
    isSolidMode_            (true               ),
    RenderTarget_           (0                  ),
    CurShaderClass_         (0                  ),
    GlobalShaderClass_      (0                  ),
    ShaderSurfaceCallback_  (0                  ),
    LastMaterial_           (0                  ),
    VertexFormatDefault_    (0                  ),
    VertexFormatReduced_    (0                  ),
    VertexFormatExtended_   (0                  ),
    VertexFormatFull_       (0                  )
{
    /* General settings */
    __spVideoDriver = this;
    
    for (s32 i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)
        scene::spTextureMatrix[i].reset();
    
    setFillColor(255);
    
    memset(RenderQuery_, 0, sizeof(RenderQuery_));
}
RenderSystem::~RenderSystem()
{
    /* Delete all extendet lists */
    MemoryManager::deleteList(MovieList_            );
    MemoryManager::deleteList(FontList_             );
    MemoryManager::deleteList(VertexFormatList_     );
    MemoryManager::deleteList(ShaderList_           );
    MemoryManager::deleteList(ShaderClassList_      );
    MemoryManager::deleteList(ComputeShaderIOList_  );
}


/*
 * ======= Renderer information =======
 */

io::stringc RenderSystem::getVendorNameByID(const u32 VendorID)
{
    switch (VendorID)
    {
        case VENDOR_ATI:
            return "ATI Technologies Inc.";
        case VENDOR_NVIDIA:
            return "NVIDIA Corporation";
        case VENDOR_MATROX:
            return "Matrox Electronic Systems Ltd.";
        case VENDOR_3DFX:
            return "3dfx Interactive Inc.";
        case VENDOR_S3GC:
            return "S3 Graphics Co., Ltd.";
        case VENDOR_INTEL:
            return "Intel Corporation";
        default:
            return "Unknown VendorId: " + io::stringc(VendorID);
    }
    return "";
}

bool RenderSystem::queryExtensionSupport(const io::stringc &TargetExtension) const
{
    return false;
}

dim::EMatrixCoordinateSystmes RenderSystem::getProjectionMatrixType() const
{
    return dim::MATRIX_LEFTHANDED;
}

void RenderSystem::printWarning() { }


/*
 * ======= Configuration functions =======
 */

void RenderSystem::setShadeMode(const EShadeModeTypes ShadeMode) { }
void RenderSystem::setFrontFace(bool isFrontFace)
{
    isFrontFace_ = isFrontFace;
}

void RenderSystem::setClearColor(const color &Color) { }
void RenderSystem::setColorMask(bool isRed, bool isGreen, bool isBlue, bool isAlpha) { }
void RenderSystem::setDepthMask(bool isDepth) { }

void RenderSystem::setVsync(bool isVsync) { }

void RenderSystem::setAntiAlias(bool isAntiAlias) { }


/*
 * ======= Rendering 3D scenes =======
 */

void RenderSystem::setupShaderClass(const scene::MaterialNode* Object, ShaderClass* ShaderObject)
{
    if (RenderQuery_[RENDERQUERY_SHADER])
    {
        if (GlobalShaderClass_)
        {
            GlobalShaderClass_->bind(Object);
            CurShaderClass_ = GlobalShaderClass_;
        }
        else if (ShaderObject)
        {
            ShaderObject->bind(Object);
            CurShaderClass_ = ShaderObject;
        }
        else
            CurShaderClass_ = 0;
    }
}

void RenderSystem::updateMaterialStates(MaterialStates* Material, bool isClear) { }

void RenderSystem::drawPrimitiveList(
    const ERenderPrimitives Type,
    const scene::SMeshVertex3D* Vertices, u32 VertexCount, const void* Indices, u32 IndexCount,
    std::vector<SMeshSurfaceTexture>* TextureList)
{
}

void RenderSystem::updateLight(
    u32 LightID, const scene::ELightModels LightType, bool isVolumetric,
    const dim::vector3df &Direction, f32 SpotInnerConeAngle, f32 SpotOuterConeAngle,
    f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic)
{
}

void RenderSystem::disableTriangleListStates() { }
void RenderSystem::disable3DRenderStates() { }
void RenderSystem::disableTexturing() { }

void RenderSystem::setDefaultAlphaBlending() { }

void RenderSystem::enableBlending() { }
void RenderSystem::disableBlending() { }

void RenderSystem::updateWireframeMode(s32 &ModeFront, s32 &ModeBack) { }

void RenderSystem::addDynamicLightSource(
    u32 LightID, scene::ELightModels Type,
    video::color &Diffuse, video::color &Ambient, video::color &Specular,
    f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic)
{
}
void RenderSystem::setLightStatus(u32 LightID, bool isEnable) { }
void RenderSystem::setLightColor(
    u32 LightID, const video::color &Diffuse, const video::color &Ambient, const video::color &Specular)
{
}


/* === Hardware mesh buffers === */

void RenderSystem::createVertexBuffer(void* &BufferID) { }
void RenderSystem::createIndexBuffer(void* &BufferID) { }

void RenderSystem::deleteVertexBuffer(void* &BufferID) { }
void RenderSystem::deleteIndexBuffer(void* &BufferID) { }

void RenderSystem::updateVertexBuffer(
    void* BufferID, const dim::UniversalBuffer &BufferData, const VertexFormat* Format, const EMeshBufferUsage Usage)
{
}
void RenderSystem::updateIndexBuffer(
    void* BufferID, const dim::UniversalBuffer &BufferData, const IndexFormat* Format, const EMeshBufferUsage Usage)
{
}

void RenderSystem::updateVertexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index) { }
void RenderSystem::updateIndexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index) { }

void RenderSystem::drawMeshBuffer(const MeshBuffer* MeshBuffer) { }

void RenderSystem::drawMeshBufferPlain(const MeshBuffer* MeshBuffer, bool useFirstTextureLayer)
{
    drawMeshBuffer(MeshBuffer);
}


/* === Fog effect === */

void RenderSystem::setFog(const EFogTypes Type) { }
EFogTypes RenderSystem::getFog() const
{
    return Fog_.Type;
}

void RenderSystem::setFogColor(const video::color &Color) { }
video::color RenderSystem::getFogColor() const
{
    return Fog_.Color;
}

void RenderSystem::setFogRange(f32 Range, f32 NearPlane, f32 FarPlane, const EFogModes Mode) { }
void RenderSystem::getFogRange(f32 &Range, f32 &NearPlane, f32 &FarPlane, EFogModes &Mode)
{
    Range       = Fog_.Range;
    NearPlane   = Fog_.Near;
    FarPlane    = Fog_.Far;
    Mode        = Fog_.Mode;
}


/* === Stencil buffer === */

void RenderSystem::clearStencilBuffer() { }

void RenderSystem::drawStencilShadowVolume(
    const dim::vector3df* pTriangleList, s32 Count, bool ZFailMethod, bool VolumetricShadow
) { }
void RenderSystem::drawStencilShadow(const video::color &Color) { }

void RenderSystem::setClipPlane(u32 Index, const dim::plane3df &Plane, bool Enable) { }


/*
 * ======= Shader programs =======
 */

ShaderClass* RenderSystem::createShaderClass(VertexFormat* VertexInputLayout)
{
    return 0;
}
void RenderSystem::deleteShaderClass(ShaderClass* ShaderClassObj)
{
    MemoryManager::removeElement(ShaderClassList_, ShaderClassObj, true);
}

Shader* RenderSystem::createEmptyShaderWithError(
    const io::stringc &Message, ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version)
{
    io::Log::error(Message);
    io::Log::lowerTab();
    Shader* NewShader = new Shader(ShaderClassObj, Type, Version);
    ShaderList_.push_back(NewShader);
    return NewShader;
}

Shader* RenderSystem::loadShader(
    ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
    const io::stringc &Filename, const io::stringc &EntryPoint)
{
    /* Print the information message */
    io::stringc ShaderName;
    
    if (Version == CG_VERSION_2_0)
        ShaderName = "Cg ";
    else
    {
        switch (RendererType_)
        {
            case RENDERER_OPENGL:
                ShaderName = "GLSL "; break;
            case RENDERER_OPENGLES2:
                ShaderName = "GLSL ES "; break;
            case RENDERER_DIRECT3D9:
            case RENDERER_DIRECT3D11:
                ShaderName = "HLSL "; break;
        }
    }
    
    switch (Type)
    {
        case SHADER_VERTEX_PROGRAM:
            ShaderName += "vertex program"; break;
        case SHADER_PIXEL_PROGRAM:
            ShaderName += "pixel program"; break;
        case SHADER_VERTEX:
            ShaderName += "vertex shader"; break;
        case SHADER_PIXEL:
            ShaderName += "pixel shader"; break;
        case SHADER_GEOMETRY:
            ShaderName += "geometry shader"; break;
        case SHADER_HULL:
            ShaderName += "hull shader"; break;
        case SHADER_DOMAIN:
            ShaderName += "domain shader"; break;
        case SHADER_COMPUTE:
            ShaderName += "compute shader"; break;
    }
    
    io::Log::message("Load " + ShaderName + ": \"" + Filename + "\"");
    io::Log::upperTab();
    
    /* Read the shader file and copy the program lines into the shader buffer */
    std::vector<io::stringc> ShaderBuffer;
    
    io::FileSystem FileSys;
    
    if (!FileSys.findFile(Filename))
        return createEmptyShaderWithError("Could not found shader file", ShaderClassObj, Type, Version);
    
    io::File* ShaderFile = FileSys.readResourceFile(Filename);
    
    if (ShaderFile)
    {
        io::stringc Line;
        
        while (!ShaderFile->isEOF())
            ShaderBuffer.push_back(ShaderFile->readString());
        
        FileSys.closeFile(ShaderFile);
    }
    else
        return createEmptyShaderWithError("Could not read shader file", ShaderClassObj, Type, Version);
    
    /* Create the shader program */
    Shader* NewShader = 0;
    
    if (Version == CG_VERSION_2_0)
        NewShader = createCgShader(ShaderClassObj, Type, Version, ShaderBuffer, EntryPoint);
    else
        NewShader = createShader(ShaderClassObj, Type, Version, ShaderBuffer, EntryPoint);
    
    io::Log::lowerTab();
    
    return NewShader;
}

Shader* RenderSystem::createShader(
    ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
    const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint)
{
    return 0;
}

ShaderClass* RenderSystem::loadShaderClass(
    const io::stringc &FilenameVertex, const io::stringc &FilenamePixel,
    const EShaderVersions VersionVertex, const EShaderVersions VersionPixel,
    const io::stringc &EntryPointVertex, const io::stringc &EntryPointPixel)
{
    ShaderClass* NewShaderClass = createShaderClass();
    
    loadShader(NewShaderClass, SHADER_VERTEX, VersionVertex, FilenameVertex, EntryPointVertex);
    loadShader(NewShaderClass, SHADER_PIXEL, VersionPixel, FilenamePixel, EntryPointPixel);
    
    NewShaderClass->link();
    
    return NewShaderClass;
}

ShaderClass* RenderSystem::createCgShaderClass(VertexFormat* VertexInputLayout)
{
    #ifdef SP_COMPILE_WITH_CG
    if (RenderQuery_[RENDERQUERY_SHADER])
    {
        ShaderClass* NewShaderClass = new CgShaderClass(VertexInputLayout);
        ShaderClassList_.push_back(NewShaderClass);
        return NewShaderClass;
    }
    return 0;
    #else
    io::Log::error("This engine was not compiled with the Cg toolkit");
    return 0;
    #endif
}

Shader* RenderSystem::createCgShader(
    ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
    const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint)
{
    return 0;
}

void RenderSystem::unbindShaders() { }
void RenderSystem::deleteShader(Shader* ShaderObj)
{
    MemoryManager::removeElement(ShaderList_, ShaderObj, true);
}

bool RenderSystem::runComputeShader(
    Shader* ShaderObj, ComputeShaderIO* IOInterface, const dim::vector3di &GroupSize)
{
    return false;
}

ComputeShaderIO* RenderSystem::createComputeShaderIO()
{
    ComputeShaderIO* NewIOInterface = MemoryManager::createMemory<ComputeShaderIO>();
    ComputeShaderIOList_.push_back(NewIOInterface);
    return NewIOInterface;
}

void RenderSystem::deleteComputeShaderIO(ComputeShaderIO* &IOInterface)
{
    MemoryManager::removeElement(ComputeShaderIOList_, IOInterface, true);
}


/*
 * ========== Simple drawing functions ==========
 */

void RenderSystem::beginDrawing2D() { }
void RenderSystem::endDrawing2D() { }

void RenderSystem::beginDrawing3D() { }
void RenderSystem::endDrawing3D() { }

void RenderSystem::setBlending(const EBlendingTypes SourceBlend, const EBlendingTypes DestBlend) { }
void RenderSystem::setClipping(bool Enable, const dim::point2di &Position, const dim::size2di &Size) { }
void RenderSystem::setViewport(const dim::point2di &Position, const dim::size2di &Size) { }

bool RenderSystem::setRenderTarget(Texture* Target)
{
    return false;
}
Texture* RenderSystem::getRenderTarget() const
{
    return RenderTarget_;
}

void RenderSystem::setLineSize(s32 Size) { }
void RenderSystem::setPointSize(s32 Size) { }


/*
 * ======= Image drawing =======
 */

void RenderSystem::draw2DImage(
    Texture* Tex, const dim::point2di &Position, const color &Color)
{
}
void RenderSystem::draw2DImage(
    Texture* Tex, const dim::rect2di &Position, const dim::rect2df &Clipping, const color &Color)
{
}
void RenderSystem::draw2DImage(
    Texture* Tex, const dim::point2di &Position, f32 Rotation, f32 Radius, const color &Color)
{
}

// !TODO! -> make a rotation matrix here !!!
void RenderSystem::draw2DImage(
    Texture* Tex, dim::rect2di Position, const dim::rect2df &Clipping, f32 Rotation, const dim::point2df &RotationPoint,
    const color &lefttopColor, const color &righttopColor, const color &rightbottomColor, const color &leftbottomColor)
{
    #define DIST(X1, Y1, X2, Y2) \
        math::getDistance(dim::point2di(X1, Y1).cast<f32>(), dim::point2di(X2, Y2).cast<f32>())
    
    /* Translation of location */
    Position.Right   += Position.Left;
    Position.Bottom  += Position.Top;
    
    /* Temporary variables */
    dim::point2df lefttopPos, righttopPos;
    dim::point2df rightbottomPos, leftbottomPos;
    dim::point2di tmpRotPoint;
    f32 tmpDegree, tmpRotDist;
    f32 tmpExtLine, tmpExpPosY;
    
    /* Compute the rotation */
    tmpRotPoint.X       = static_cast<s32>((Position.Right - Position.Left) * RotationPoint.X);
    tmpRotPoint.Y       = static_cast<s32>((Position.Bottom - Position.Top) * RotationPoint.Y);
    tmpExpPosY          = (Position.Bottom - Position.Top) * RotationPoint.Y;
    
    tmpExtLine          = (Position.Right - Position.Left) * RotationPoint.X;
    tmpRotDist          = DIST(Position.Left, Position.Top, Position.Left + tmpRotPoint.X, Position.Top + tmpRotPoint.Y);
    tmpDegree           = (tmpRotDist == 0 ? 0 : math::ASin( tmpExtLine / tmpRotDist ));
    lefttopPos.X        = Position.Left + tmpRotPoint.X - math::Sin(tmpDegree - Rotation) * tmpRotDist;
    lefttopPos.Y        = Position.Top  + tmpExpPosY    - math::Cos(tmpDegree - Rotation) * tmpRotDist;
    
    tmpRotDist          = DIST(Position.Left, Position.Bottom, Position.Left + tmpRotPoint.X, Position.Top + tmpRotPoint.Y);
    tmpDegree           = (tmpRotDist == 0 ? 0 : math::ASin( tmpExtLine / tmpRotDist ));
    leftbottomPos.X     = Position.Left + tmpRotPoint.X - math::Sin(tmpDegree + Rotation) * tmpRotDist;
    leftbottomPos.Y     = Position.Top  + tmpExpPosY    + math::Cos(tmpDegree + Rotation) * tmpRotDist;
    
    tmpExtLine          = (Position.Right - Position.Left) - (Position.Right - Position.Left) * RotationPoint.X;
    tmpRotDist          = DIST(Position.Right, Position.Top, Position.Left + tmpRotPoint.X, Position.Top + tmpRotPoint.Y);
    tmpDegree           = (tmpRotDist == 0 ? 0 : math::ASin( tmpExtLine / tmpRotDist ));
    righttopPos.X       = Position.Left + tmpRotPoint.X + math::Sin(tmpDegree + Rotation) * tmpRotDist;
    righttopPos.Y       = Position.Top  + tmpExpPosY    - math::Cos(tmpDegree + Rotation) * tmpRotDist;
    
    tmpRotDist          = DIST(Position.Right, Position.Bottom, Position.Left + tmpRotPoint.X, Position.Top + tmpRotPoint.Y);
    tmpDegree           = (tmpRotDist == 0 ? 0 : math::ASin( tmpExtLine / tmpRotDist ));
    rightbottomPos.X    = Position.Left + tmpRotPoint.X + math::Sin(tmpDegree - Rotation) * tmpRotDist;
    rightbottomPos.Y    = Position.Top  + tmpExpPosY    + math::Cos(tmpDegree - Rotation) * tmpRotDist;
    
    /* Draw the image using the next draw2DImage function */
    draw2DImage(
        Tex,
        lefttopPos.cast<s32>(),
        righttopPos.cast<s32>(),
        rightbottomPos.cast<s32>(),
        leftbottomPos.cast<s32>(),
        dim::point2df(Clipping.Left, Clipping.Bottom),
        dim::point2df(Clipping.Right, Clipping.Bottom),
        dim::point2df(Clipping.Right, Clipping.Top),
        dim::point2df(Clipping.Left, Clipping.Top),
        lefttopColor, righttopColor, rightbottomColor, leftbottomColor
    );
    
    #undef DIST
}

void RenderSystem::draw2DImage(
    Texture* Tex,
    const dim::point2di &lefttopPosition, const dim::point2di &righttopPosition,
    const dim::point2di &rightbottomPosition, const dim::point2di &leftbottomPosition,
    const dim::point2df &lefttopClipping, const dim::point2df &righttopClipping,
    const dim::point2df &rightbottomClipping, const dim::point2df &leftbottomClipping,
    const color &lefttopColor, const color &righttopColor,
    const color &rightbottomColor, const color &leftbottomColor)
{
}

/*
 * ======= Primitive drawing =======
 */

color RenderSystem::getPixelColor(const dim::point2di &Position) const { return color(); }
f32 RenderSystem::getPixelDepth(const dim::point2di &Position) const { return 0.0f; }

void RenderSystem::draw2DPoint(const dim::point2di &Position, const color &Color) { }

void RenderSystem::draw2DLine(const dim::point2di &PositionA, const dim::point2di &PositionB, const color &Color) { }
void RenderSystem::draw2DLine(const dim::point2di &PositionA, const dim::point2di &PositionB, const color &ColorA, const color &ColorB) { }

void RenderSystem::draw2DRectangle(const dim::rect2di &Rect, const color &Color, bool isSolid) { }
void RenderSystem::draw2DRectangle(
    const dim::rect2di &Rect, const color &lefttopColor, const color &righttopColor,
    const color &rightbottomColor, const color &leftbottomColor, bool isSolid)
{
}

void RenderSystem::draw2DBox(
    const dim::point2di &Position, const dim::size2di &Size, const color &Color, bool isSolid)
{
    draw2DRectangle(
        dim::rect2di(
            Position.X - Size.Width/2, Position.Y - Size.Height/2,
            Position.X + Size.Width/2, Position.Y + Size.Height/2
        ),
        Color, isSolid
    );
}

void RenderSystem::draw2DCircle(const dim::point2di &Position, s32 Radius, const color &Color)
{
    /* Temporary variables */
    Radius  = math::Abs(Radius);
    
    s32 f   = 1 - Radius;
    s32 dx  = 0;
    s32 dy  = -2 * Radius;
    s32 x   = 0;
    s32 y   = Radius;
    
    /* Draw the circle using the "Brensenham Algorithm" */
    draw2DPoint(dim::point2di(Position.X, Position.Y + Radius), Color);
    draw2DPoint(dim::point2di(Position.X, Position.Y - Radius), Color);
    draw2DPoint(dim::point2di(Position.X + Radius, Position.Y), Color);
    draw2DPoint(dim::point2di(Position.X - Radius, Position.Y), Color);
    
    while (x < y)
    {
        if (f >= 0)
        {
            --y;
            dy += 2;
            f += dy;
        }
        
        ++x;
        dx += 2;
        f += dx + 1;
        
        draw2DPoint(dim::point2di(Position.X + x, Position.Y + y), Color);
        draw2DPoint(dim::point2di(Position.X - x, Position.Y + y), Color);
        draw2DPoint(dim::point2di(Position.X + x, Position.Y - y), Color);
        draw2DPoint(dim::point2di(Position.X - x, Position.Y - y), Color);
        draw2DPoint(dim::point2di(Position.X + y, Position.Y + x), Color);
        draw2DPoint(dim::point2di(Position.X - y, Position.Y + x), Color);
        draw2DPoint(dim::point2di(Position.X + y, Position.Y - x), Color);
        draw2DPoint(dim::point2di(Position.X - y, Position.Y - x), Color);
    }
}

void RenderSystem::draw2DEllipse(
    const dim::point2di &Position, const dim::size2di &Radius, const color &Color)
{
    /* Temporary variables */
    s32 a = math::Abs(Radius.Width), b = math::Abs(Radius.Height);
    s32 xc = Position.X, yc = Position.Y;
    s32 a2 = a*a, b2 = b*b, fa2 = 4*a2, fb2 = 4*b2;
    s32 x, y, sigma;
    
    /* Draw the ellipse using the "Brensenham Algorithm" */
    for (x = 0, y = b, sigma = 2*b2 + a2*(1-2*b); b2*x <= a2*y; ++x)
    {
        draw2DPoint(dim::point2di(xc + x, yc + y), Color);
        draw2DPoint(dim::point2di(xc - x, yc + y), Color);
        draw2DPoint(dim::point2di(xc + x, yc - y), Color);
        draw2DPoint(dim::point2di(xc - x, yc - y), Color);
        
        if (sigma >= 0)
        {
            sigma += fa2*(1-y);
            --y;
        }
        
        sigma += b2*(4*x+6);
    }
    
    for (x = a, y = 0, sigma = 2*a2 + b2*(1-2*a); a2*y <= b2*x; ++y)
    {
        draw2DPoint(dim::point2di(xc + x, yc + y), Color);
        draw2DPoint(dim::point2di(xc - x, yc + y), Color);
        draw2DPoint(dim::point2di(xc + x, yc - y), Color);
        draw2DPoint(dim::point2di(xc - x, yc - y), Color);
        
        if (sigma >= 0)
        {
            sigma += fb2*(1-x);
            --x;
        }
        
        sigma += a2*(4*y+6);
    }
}

void RenderSystem::draw2DPolygon(
    const ERenderPrimitives Type, const scene::SPrimitiveVertex2D* VerticesList, u32 Count)
{
}
void RenderSystem::draw2DPolygonImage(
    const ERenderPrimitives Type, Texture* Tex, const scene::SPrimitiveVertex2D* VerticesList, u32 Count)
{
}

/*
 * ======= 3D drawing functions =======
 */

void RenderSystem::draw3DPoint(const dim::vector3df &Position, const color &Color) { }
void RenderSystem::draw3DLine(
    const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &Color)
{
}
void RenderSystem::draw3DLine(
    const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &ColorA, const color &ColorB)
{
}
void RenderSystem::draw3DEllipse(
    const dim::vector3df &Position, const dim::vector3df &Rotation, const dim::size2df &Radius, const color &Color)
{
}
void RenderSystem::draw3DTriangle(
    Texture* Tex, const dim::triangle3df &Triangle, const color &Color)
{
}

void RenderSystem::draw3DBox(
    const dim::aabbox3df &BoundBox, const dim::matrix4f &Transformation, const color &Color)
{
    /* Get all transformed corners */
    const dim::vector3df LeftBottomFront    (BoundBox.Min.X, BoundBox.Min.Y, BoundBox.Min.Z);
    const dim::vector3df LeftBottomBack     (BoundBox.Min.X, BoundBox.Min.Y, BoundBox.Max.Z);
    const dim::vector3df LeftTopFront       (BoundBox.Min.X, BoundBox.Max.Y, BoundBox.Min.Z);
    const dim::vector3df LeftTopBack        (BoundBox.Min.X, BoundBox.Max.Y, BoundBox.Max.Z);
    
    const dim::vector3df RightBottomFront   (BoundBox.Max.X, BoundBox.Min.Y, BoundBox.Min.Z);
    const dim::vector3df RightBottomBack    (BoundBox.Max.X, BoundBox.Min.Y, BoundBox.Max.Z);
    const dim::vector3df RightTopFront      (BoundBox.Max.X, BoundBox.Max.Y, BoundBox.Min.Z);
    const dim::vector3df RightTopBack       (BoundBox.Max.X, BoundBox.Max.Y, BoundBox.Max.Z);
    
    /* Draw top */
    draw3DLine(Transformation * LeftTopFront,       Transformation * RightTopFront,     Color);
    draw3DLine(Transformation * RightTopFront,      Transformation * RightTopBack,      Color);
    draw3DLine(Transformation * RightTopBack,       Transformation * LeftTopBack,       Color);
    draw3DLine(Transformation * LeftTopBack,        Transformation * LeftTopFront,      Color);

    /* Draw bottom */
    draw3DLine(Transformation * LeftBottomFront,    Transformation * RightBottomFront,  Color);
    draw3DLine(Transformation * RightBottomFront,   Transformation * RightBottomBack,   Color);
    draw3DLine(Transformation * RightBottomBack,    Transformation * LeftBottomBack,    Color);
    draw3DLine(Transformation * LeftBottomBack,     Transformation * LeftBottomFront,   Color);

    /* Draw coat */
    draw3DLine(Transformation * LeftTopFront,       Transformation * LeftBottomFront,   Color);
    draw3DLine(Transformation * RightTopFront,      Transformation * RightBottomFront,  Color);
    draw3DLine(Transformation * RightTopBack,       Transformation * RightBottomBack,   Color);
    draw3DLine(Transformation * LeftTopBack,        Transformation * LeftBottomBack,    Color);
}

void RenderSystem::draw3DBox(const dim::obbox3df &BoundBox, const color &Color)
{
    draw3DBox(dim::aabbox3df(-1.0f, 1.0f), dim::matrix4f(BoundBox), Color);
}


/*
 * ======= Texture loading and creating =======
 */

Texture* RenderSystem::loadTexture(const io::stringc &Filename)
{
    /* Initialization */
    Texture* NewTexture = 0;
    
    io::Log::message("Load texture: \"" + Filename + "\"");
    io::Log::upperTab();
    
    /* Open file for reading */
    io::FileSystem FileSys;
    io::File* TexFile = FileSys.readResourceFile(Filename);
    
    if (!TexFile)
    {
        /* Create empty texture */
        NewTexture = createTexture(DEF_TEXTURE_SIZE);
        io::Log::lowerTab();
        return NewTexture;
    }
    
    /* Get a suitable image loader */
    const EImageFileFormats FileFormat = getImageFileFormat(TexFile);
    
    boost::shared_ptr<ImageLoader> Loader;
    
    switch (FileFormat)
    {
        #ifdef SP_COMPILE_WITH_TEXLOADER_BMP
        case IMAGEFORMAT_BMP:
            Loader = boost::shared_ptr<ImageLoader>(new ImageLoaderBMP(TexFile)); break;
        #endif
        #ifdef SP_COMPILE_WITH_TEXLOADER_JPG
        case IMAGEFORMAT_JPG:
            Loader = boost::shared_ptr<ImageLoader>(new ImageLoaderJPG(TexFile)); break;
        #endif
        #ifdef SP_COMPILE_WITH_TEXLOADER_TGA
        case IMAGEFORMAT_TGA:
            Loader = boost::shared_ptr<ImageLoader>(new ImageLoaderTGA(TexFile)); break;
        #endif
        #ifdef SP_COMPILE_WITH_TEXLOADER_PNG
        case IMAGEFORMAT_PNG:
            Loader = boost::shared_ptr<ImageLoader>(new ImageLoaderPNG(TexFile)); break;
        #endif
        #ifdef SP_COMPILE_WITH_TEXLOADER_PCX
        case IMAGEFORMAT_PCX:
            Loader = boost::shared_ptr<ImageLoader>(new ImageLoaderPCX(TexFile)); break;
        #endif
        #ifdef SP_COMPILE_WITH_TEXLOADER_DDS
        case IMAGEFORMAT_DDS:
            Loader = boost::shared_ptr<ImageLoader>(new ImageLoaderDDS(TexFile)); break;
        #endif
        
        default:
        {
            /* Print error message and create an empty texture */
            if (FileFormat == IMAGEFORMAT_WAD)
                io::Log::error("Texture file format WAD must be loaded as a texture list");
            else
                io::Log::error("Texture has unsupported file format");
            
            NewTexture = createTexture(DEF_TEXTURE_SIZE);
            
            io::Log::lowerTab();
            return NewTexture;
        }
    }
    
    /* Load the texture and create the renderer texture */
    NewTexture = loadTexture(Loader.get());
    
    io::Log::lowerTab();
    
    return NewTexture;
}

Texture* RenderSystem::loadTexture(ImageLoader* Loader)
{
    if (!Loader)
        return RenderSystem::createTexture(DEF_TEXTURE_SIZE);
    
    /* Load image data */
    SImageDataRead* ImageData = Loader->loadImageData();
    
    if (!ImageData)
        return RenderSystem::createTexture(DEF_TEXTURE_SIZE);
    
    /* Setup flags and create texture */
    STextureCreationFlags CreationFlags(TexGenFlags_);
    {
        CreationFlags.Filename      = Loader->getFilename();
        CreationFlags.Size          = dim::size2di(ImageData->Width, ImageData->Height);
        CreationFlags.ImageBuffer   = ImageData->ImageBuffer;
        CreationFlags.Format        = ImageData->Format;
    }
    Texture* NewTexture = createTexture(CreationFlags);
    
    /* Delete image data */
    MemoryManager::deleteMemory(ImageData);
    
    return NewTexture;
}

Texture* RenderSystem::getTexture(const io::stringc &Filename)
{
    std::map<std::string, Texture*>::iterator it = TextureMap_.find(Filename.str());
    
    if (it == TextureMap_.end())
    {
        Texture* NewTexture = loadTexture(Filename);
        TextureMap_[Filename.str()] = NewTexture;
        return NewTexture;
    }
    
    io::Log::message("Get preloaded texture: \"" + Filename + "\"");
    
    return it->second;
}

void RenderSystem::setTextureGenFlags(const ETextureGenFlags Flag, const s32 Value)
{
    switch (Flag)
    {
        case TEXGEN_FILTER:
            TexGenFlags_.MagFilter = static_cast<ETextureFilters>(Value);
            TexGenFlags_.MinFilter = static_cast<ETextureFilters>(Value);
            break;
        case TEXGEN_MAGFILTER:
            TexGenFlags_.MagFilter = static_cast<ETextureFilters>(Value); break;
        case TEXGEN_MINFILTER:
            TexGenFlags_.MinFilter = static_cast<ETextureFilters>(Value); break;
            
        case TEXGEN_MIPMAPFILTER:
            TexGenFlags_.MipMapFilter = static_cast<ETextureMipMapFilters>(Value); break;
        case TEXGEN_MIPMAPS:
            TexGenFlags_.MipMaps = (Value != 0); break;
            
        case TEXGEN_WRAP:
            TexGenFlags_.WrapMode.X = static_cast<ETextureWrapModes>(Value);
            TexGenFlags_.WrapMode.Y = static_cast<ETextureWrapModes>(Value);
            TexGenFlags_.WrapMode.Z = static_cast<ETextureWrapModes>(Value);
            break;
        case TEXGEN_WRAP_U:
            TexGenFlags_.WrapMode.X = static_cast<ETextureWrapModes>(Value); break;
        case TEXGEN_WRAP_V:
            TexGenFlags_.WrapMode.Y = static_cast<ETextureWrapModes>(Value); break;
        case TEXGEN_WRAP_W:
            TexGenFlags_.WrapMode.Z = static_cast<ETextureWrapModes>(Value); break;
            
        case TEXGEN_ANISOTROPY:
            TexGenFlags_.Anisotropy = Value; break;
    }
}

void RenderSystem::deleteTexture(Texture* &Tex)
{
    TextureListSemaphore_.lock();
    MemoryManager::removeElement(TextureList_, Tex, true);
    TextureListSemaphore_.unlock();
}

bool RenderSystem::isTexture(const Texture* Tex) const
{
    return Tex && Tex->valid();
}

Texture* RenderSystem::copyTexture(const Texture* Tex)
{
    if (!Tex)
        return RenderSystem::createTexture(DEF_TEXTURE_SIZE);
    
    /* Setup texture creation flags */
    STextureCreationFlags CreationFlags;
    {
        CreationFlags.Filename      = Tex->getFilename();
        CreationFlags.Size          = Tex->getSize();
        CreationFlags.ImageBuffer   = Tex->getImageBuffer();
        CreationFlags.MagFilter     = Tex->getMagFilter();
        CreationFlags.MinFilter     = Tex->getMinFilter();
        CreationFlags.MipMapFilter  = Tex->getMipMapFilter();
        CreationFlags.Format        = Tex->getFormat();
        CreationFlags.Anisotropy    = Tex->getAnisotropicSamples();
        CreationFlags.MipMaps       = Tex->getMipMapping();
        CreationFlags.WrapMode      = Tex->getWrapMode();
    }
    return createTexture(CreationFlags);
}

Texture* RenderSystem::createTexture(
    const dim::size2di &Size, const EPixelFormats Format, const u8* ImageBuffer)
{
    /* Create default image buffer if needed */
    u8* DefImageBuffer = 0;
    
    if (!ImageBuffer)
    {
        /* Temporary vairables */
        const s32 FormatSize        = ImageBuffer::getFormatSize(Format);
        const s32 ImageBufferSize   = Size.getArea() * FormatSize;
        
        s32 x, y, i, j = 0;
        
        /* Allocate new image memory */
        DefImageBuffer = MemoryManager::createBuffer<u8>(ImageBufferSize);
        
        /* Fill the image buffer */
        for (y = 0; y < Size.Height; ++y)
        {
            for (x = 0; x < Size.Width; ++x)
            {
                for (i = 0; i < FormatSize; ++i, ++j)
                    DefImageBuffer[j] = StdFillColor_[i];
            }
        }
        
        ImageBuffer = DefImageBuffer;
    }
    
    /* Setup texture creation flags */
    STextureCreationFlags CreationFlags(TexGenFlags_);
    {
        CreationFlags.Size          = Size;
        CreationFlags.Format        = Format;
        CreationFlags.ImageBuffer   = ImageBuffer;
    }
    Texture* NewTexture = createTexture(CreationFlags);
    
    /* Delete default image buffer if it was used */
    MemoryManager::deleteBuffer(DefImageBuffer);
    
    return NewTexture;
}

Texture* RenderSystem::createTexture(
    const dim::size2di &Size, const EPixelFormats Format, const f32* ImageBuffer)
{
    if (ImageBuffer)
    {
        /* Setup texture creation flags */
        STextureCreationFlags CreationFlags(TexGenFlags_);
        {
            CreationFlags.Size          = Size;
            CreationFlags.Format        = Format;
            CreationFlags.HWFormat      = HWTEXFORMAT_FLOAT32;
            CreationFlags.BufferType    = IMAGEBUFFER_FLOAT;
            CreationFlags.ImageBuffer   = ImageBuffer;
        }
        return createTexture(CreationFlags);
    }
    return createTexture(Size, Format);
}

EImageFileFormats RenderSystem::getImageFileFormat(const io::stringc &Filename) const
{
    return getImageFileFormat(io::FileSystem().readResourceFile(Filename));
}

EImageFileFormats RenderSystem::getImageFileFormat(io::File* TexFile) const
{
    if (!TexFile)
        return IMAGEFORMAT_UNKNOWN;
    
    /* Store file position */
    const s32 PrevPos = TexFile->getSeek();
    TexFile->setSeek(0);
    
    /* Read the magic number out of the specified file */
    s32 MagicNumber = TexFile->readValue<s32>();
    
    /* Reset file position */
    TexFile->setSeek(PrevPos);
    
    /* Check the magic number of each known image file format */
    if ( *((s16*)&MagicNumber) == (s16)0x4D42 )
        return IMAGEFORMAT_BMP;
    if ( *((s16*)&MagicNumber) == (s16)0xD8FF )
        return IMAGEFORMAT_JPG;
    if ( *((s32*)&MagicNumber) == 0x474E5089 )
        return IMAGEFORMAT_PNG;
    if ( *((s8*)&MagicNumber) == (s8)0x0A )
        return IMAGEFORMAT_PCX;
    if ( *((s32*)&MagicNumber) == 0x20534444 )
        return IMAGEFORMAT_DDS;
    if ( *((s32*)&MagicNumber) == 0x38464947 )
        return IMAGEFORMAT_GIF;
    if ( *((const s32*)&MagicNumber) == *((const s32*)"WAD2") || *((const s32*)&MagicNumber) == *((const s32*)"WAD3") )
        return IMAGEFORMAT_WAD;
    if (TexFile->getFilename().right(4).lower() == ".tga" || TexFile->getFilename().right(5).lower() == ".tpic")
        return IMAGEFORMAT_TGA;
    
    /* Unknown file format */
    return IMAGEFORMAT_UNKNOWN;
}

std::list<Texture*> RenderSystem::loadTextureList(const io::stringc &Filename)
{
    #ifdef SP_COMPILE_WITH_TEXLOADER_WAD
    return ImageLoaderWAD().loadTextureList(Filename);
    #else
    io::Log::error("Cannot load textures because the engine is compiled without WAD texture loader");
    std::list<Texture*> EmptyList;
    return EmptyList;
    #endif
}

/*
 * Saves the specified texture allways as a bitmap image
 */

bool RenderSystem::saveTexture(const Texture* Tex, io::stringc Filename, const EImageFileFormats FileFormat)
{
    /* Check parameter validity */
    if (!Tex || !Filename.size())
        return false;
    
    if (Tex->getImageBuffer()->getType() != IMAGEBUFFER_UBYTE)
    {
        io::Log::warning("Currently only UBYTE image buffers can be saved");
        return false;
    }
    
    /* Information message */
    Filename.adjustPath();
    io::Log::message("Save texture: \"" + Filename + "\"");
    io::Log::upperTab();
    
    boost::shared_ptr<ImageSaver> Saver;
    
    io::FileSystem FileSys;
    io::File* TexFile = FileSys.openFile(Filename, io::FILE_WRITE);
    
    /* Get suitable file format */
    switch (FileFormat)
    {
        case IMAGEFORMAT_BMP:
            Saver = boost::shared_ptr<ImageSaver>(new ImageSaverBMP(TexFile)); break;
        
        default:
        {
            io::Log::error("Texture file format " + Filename.getExtensionPart().upper() + " is not supported");
            io::Log::lowerTab();
            return false;
        }
    }
    
    /* === Fill the texture raw structure === */
    
    SImageDataWrite ImageData;
    
    // !TODO! -> pass "ImageBuffer" object only
    
    const ImageBuffer* ImgBuffer = Tex->getImageBuffer();
    
    ImageData.Width        = ImgBuffer->getSize().Width;
    ImageData.Height       = ImgBuffer->getSize().Height;
    ImageData.FormatSize   = ImgBuffer->getFormatSize();
    ImageData.ImageBuffer  = static_cast<const u8*>(ImgBuffer->getBuffer());
    
    /* Save the image */
    Saver->saveImageData(&ImageData);
    
    /* Clear the image buffer to avoid that the buffer will be deleted with the raw data */
    ImageData.ImageBuffer = 0;
    
    io::Log::lowerTab();
    
    return true;
}

Texture* RenderSystem::createScreenShot(const dim::point2di &Position, dim::size2di Size)
{
    return createTexture(dim::size2di(1));
}
void RenderSystem::createScreenShot(Texture* Tex, const dim::point2di &Position)
{
}


/* === Cube map auto generation === */

void RenderSystem::updateCubeMapDirection(
    scene::Camera* Cam, dim::matrix4f CamDir, const ECubeMapDirections Direction, Texture* CubeMapTexture)
{
    switch (Direction)
    {
        case CUBEMAP_POSITIVE_X:
            CamDir.rotateY(90);     break;
        case CUBEMAP_NEGATIVE_X:
            CamDir.rotateY(-90);    break;
        case CUBEMAP_POSITIVE_Y:
            CamDir.rotateX(-90);    break;
        case CUBEMAP_NEGATIVE_Y:
            CamDir.rotateX(90);     break;
        case CUBEMAP_POSITIVE_Z:
                                    break;
        case CUBEMAP_NEGATIVE_Z:
            CamDir.rotateY(180);    break;
    }
    
    Cam->setRotationMatrix(CamDir, true);
    
    CubeMapTexture->setCubeMapFace(Direction);
    
    setRenderTarget(CubeMapTexture);
    {
        clearBuffers();
        __spSceneManager->renderScene(Cam);
    }
    setRenderTarget(0);
    
    CubeMapTexture->setArrayLayer(CubeMapTexture->getArrayLayer() + 1);
}

void RenderSystem::updateCubeMap(Texture* Tex, const dim::vector3df &GlobalLocation)
{
    /* Check if the cube map can be updated */
    if (!Tex || ( Tex->getDimension() != TEXTURE_CUBEMAP && Tex->getDimension() != TEXTURE_CUBEMAP_ARRAY ) ||
        !__spSceneManager || !__spSceneManager->getActiveCamera())
    {
        return;
    }
    
    /* Store camera configuration */
    scene::Camera* Cam = __spSceneManager->getActiveCamera();
    
    const dim::matrix4f LastPos     = Cam->getPositionMatrix(true);
    const dim::matrix4f LastRot     = Cam->getRotationMatrix(true);
    const dim::matrix4f LastScl     = Cam->getScaleMatrix(true);
    const dim::rect2di LastViewport = Cam->getViewport();
    const f32 LastFOV               = Cam->getFOV();
    
    Cam->setViewport(dim::rect2di(0, 0, Tex->getSize().Width, Tex->getSize().Height));
    Cam->setFOV(90);
    
    /* Transform the camera direction matrix */
    Cam->setPosition(GlobalLocation, true);
    const dim::matrix4f CamDir(Cam->getRotationMatrix(true));
    
    /* Render the scene for all 6 directions */
    updateCubeMapDirection(Cam, CamDir, CUBEMAP_POSITIVE_X, Tex);
    updateCubeMapDirection(Cam, CamDir, CUBEMAP_NEGATIVE_X, Tex);
    updateCubeMapDirection(Cam, CamDir, CUBEMAP_POSITIVE_Y, Tex);
    updateCubeMapDirection(Cam, CamDir, CUBEMAP_NEGATIVE_Y, Tex);
    updateCubeMapDirection(Cam, CamDir, CUBEMAP_POSITIVE_Z, Tex);
    updateCubeMapDirection(Cam, CamDir, CUBEMAP_NEGATIVE_Z, Tex);
    
    /* Reset camera configuration */
    Cam->setPositionMatrix(LastPos, true);
    Cam->setRotationMatrix(LastRot, true);
    Cam->setScaleMatrix(LastScl, true);
    Cam->setViewport(LastViewport);
    Cam->setFOV(LastFOV);
}


/* Normal map generation */

void RenderSystem::makeNormalMap(Texture* HeightMap, f32 Amplitude)
{
    /* Check if the texture it not empty */
    if (!HeightMap || !HeightMap->getImageBuffer())
        return;
    
    /* Make sure the texture has at least 3 color components */
    HeightMap->setFormat(PIXELFORMAT_RGB);
    
    /* Temporary variables */
    dim::vector3df p1, p2, p3;
    dim::vector3df Normal;
    
    s32 Width   = HeightMap->getSize().Width;
    s32 Height  = HeightMap->getSize().Height;
    
    const s32 ImageBufferSize = Width * Height * 3;
    
    /* Copy the image data temporary */
    u8* ImageBuffer = MemoryManager::createBuffer<u8>(ImageBufferSize);
    memcpy(ImageBuffer, HeightMap->getImageBuffer(), ImageBufferSize);
    
    /* Loop for each texel */
    for (s32 y = 0, x; y < Height; ++y)
    {
        for (x = 0; x < Width; ++x)
        {
            /* Get the heights */
            p1.X = static_cast<f32>(x);
            p1.Y = static_cast<f32>(y);
            p1.Z = Amplitude * static_cast<f32>(ImageBuffer[(y * Width + x)*3]) / 255;
            
            p2.X = static_cast<f32>(x + 1);
            p2.Y = static_cast<f32>(y);
            
            if (x < Width - 1)
                p2.Z = Amplitude * static_cast<f32>(ImageBuffer[(y * Width + x + 1)*3]) / 255;
            else
                p2.Z = Amplitude * static_cast<f32>(ImageBuffer[(y * Width)*3]) / 255;
            
            p3.X = static_cast<f32>(x);
            p3.Y = static_cast<f32>(y + 1);
            
            if (y < Height - 1)
                p3.Z = Amplitude * static_cast<f32>(ImageBuffer[((y + 1) * Width + x)*3]) / 255;
            else
                p3.Z = Amplitude * static_cast<f32>(ImageBuffer[x*3]) / 255;
            
            /* Compute the normal */
            Normal = math::getNormalVector(p1, p2, p3);
            
            Normal *= 0.5f;
            Normal += 0.5f;
            Normal *= 255.0f;
            
            /* Set the new texel */
            HeightMap->getImageBuffer()->setPixelColor(
                dim::point2di(x, y), video::color(Normal, false)
            );
        }
    }
    
    delete [] ImageBuffer;
    
    /* Update the image data */
    HeightMap->updateImageBuffer();
}


/*
 * ======= Texture list: reloading & clearing =======
 */

void RenderSystem::setFillColor(const video::color &Color)
{
    StdFillColor_[0] = Color.Red;
    StdFillColor_[1] = Color.Green;
    StdFillColor_[2] = Color.Blue;
    StdFillColor_[3] = Color.Alpha;
}

void RenderSystem::clearTextureList()
{
    TextureListSemaphore_.lock();
    MemoryManager::deleteList(TextureList_);
    TextureListSemaphore_.unlock();
}

void RenderSystem::reloadTextureList()
{
    TextureListSemaphore_.lock();
    
    foreach (Texture* Tex, TextureList_)
    {
        if (isTexture(Tex))
            Tex->updateImageBuffer();
    }
    
    TextureListSemaphore_.unlock();
}

void RenderSystem::deleteVertexFormat(VertexFormat* Format)
{
    std::list<VertexFormat*>::iterator it = std::find(VertexFormatList_.begin(), VertexFormatList_.end(), Format);
    
    if (it != VertexFormatList_.end())
    {
        MemoryManager::deleteMemory(Format);
        VertexFormatList_.erase(it);
    }
}


/*
 * ======= Movie loading =======
 */

Movie* RenderSystem::loadMovie(const io::stringc &Filename, s32 Resolution)
{
    Movie* NewMovie = new Movie(Filename, Resolution);
    MovieList_.push_back(NewMovie);
    return NewMovie;
}
void RenderSystem::deleteMovie(Movie* &MovieObject)
{
    MemoryManager::removeElement(MovieList_, MovieObject, true);
}


/*
 * ======= Font loading and text drawing =======
 */

Font* RenderSystem::createFont(const io::stringc &FontName, dim::size2di FontSize, s32 Flags)
{
    Font* NewFont = MemoryManager::createMemory<Font>();
    FontList_.push_back(NewFont);
    return NewFont;
}
Font* RenderSystem::createFont(const io::stringc &FontName, s32 FontSize, s32 Flags)
{
    return createFont(FontName, dim::size2di(0, FontSize), Flags);
}

Font* RenderSystem::createFont(video::Texture* FontTexture)
{
    /* Check parameter validity */
    if (!FontTexture)
    {
        io::Log::error("Cannot create texture font without texture");
        return 0;
    }
    if (!FontTexture->getImageBuffer())
    {
        io::Log::error("Cannot create texture font without texture image buffer");
        return 0;
    }
    
    ImageBuffer* ImgBuffer = FontTexture->getImageBuffer();
    
    if (ImgBuffer->getSize().getArea() <= 0)
    {
        io::Log::error("Texture size is too small to be used for texture font");
        return 0;
    }
    
    /* Analyze texture image buffer */
    static const video::color MarkStart(255, 255, 0), MarkEnd(255, 0, 255);
    static const video::color MarkMapStart(255, 0, 0), MarkMapEnd(0, 0, 255);
    static const video::color MarkHeight(0, 255, 255);
    
    const dim::size2di TexSize(FontTexture->getSize());
    
    video::color Texel;
    dim::point2di Start, End;
    s32 FontHeight = -1;
    
    bool isSearchEnd = false;
    
    std::vector<dim::rect2di> ClipList;
    
    /* Search font height */
    for (dim::point2di Pos; Pos.Y < TexSize.Height; ++Pos.Y)
    {
        /* Get texel color without alpha channel */
        Texel = ImgBuffer->getPixelColor(Pos);
        
        if (Texel.equal(MarkHeight, false))
        {
            FontHeight = Pos.Y;
            break;
        }
    }
    
    if (FontHeight == -1)
    {
        io::Log::error("Analyzing font texture for font height failed");
        return 0;
    }
    
    isSearchEnd = false;
    
    /* Get all character mapping coordinates */
    for (dim::point2di Pos; Pos.Y < TexSize.Height; Pos.Y += (FontHeight + 1))
    {
        for (Pos.X = 0; Pos.X < TexSize.Width; ++Pos.X)
        {
            /* Get texel color without alpha channel */
            Texel = ImgBuffer->getPixelColor(Pos);
            
            /* Search start mark */
            if (!isSearchEnd && Texel.equal(MarkStart, false))
            {
                ImgBuffer->setPixelColor(Pos, video::color(0, 0, 0, 0));
                
                isSearchEnd = true;
                Start = Pos;
            }
            else if (isSearchEnd && Texel.equal(MarkEnd, false))
            {
                ImgBuffer->setPixelColor(Pos, video::color(0, 0, 0, 0));
                
                ClipList.push_back(dim::rect2di(Start.X, Start.Y + 2, Pos.X, Start.Y + FontHeight));
                isSearchEnd = false;
            }
        }
    }
    
    FontTexture->updateImageBuffer();
    
    /* Check for errors after analyzing font texture */
    if (ClipList.empty())
    {
        io::Log::error("Font texture does not contain any characters");
        return 0;
    }
    
    /* Create font out of the given texture */
    return createFont(FontTexture, ClipList, FontHeight);
}

Font* RenderSystem::createFont(video::Texture* FontTexture, const io::stringc &FontXMLFile)
{
    io::Log::message("Create texture font: \"" + FontXMLFile.getFilePart() + "\"");
    io::Log::upperTab();
    
    #ifdef SP_COMPILE_WITH_XMLPARSER
    
    if (!FontTexture)
    {
        io::Log::error("Invalid texture object");
        io::Log::lowerTab();
        return RenderSystem::createFont();
    }
    
    /* Load XML file */
    tool::XMLParser Parser;
    
    if (!Parser.loadFile(FontXMLFile))
    {
        io::Log::lowerTab();
        return RenderSystem::createFont();
    }
    
    /* Examine XML tags */
    std::vector<dim::rect2di> ClipList(256);
    u8 i = 0;
    u32 Count = 0;
    s32 FontHeight = 0;
    
    const dim::size2di TexSize(FontTexture->getSize());
    
    foreach (const tool::SXMLTag &Tag, Parser.getRootTag().Tags)
    {
        if (Tag.Name != "c")
        {
            io::Log::warning("Unknown tag in font XML file");
            continue;
        }
        
        foreach (const tool::SXMLAttribute &Attrib, Tag.Attributes)
        {
            if (Attrib.Name[0] == 'c')
            {
                if (Attrib.Value.size() == 1)
                    i = Attrib.Value[0];
                else if (Attrib.Value.size() > 1 && Attrib.Value[0] == '&')
                {
                    if (Attrib.Value == "&quot;")
                        i = '\"';
                    else if (Attrib.Value == "&amp;")
                        i = '&';
                    else if (Attrib.Value == "&lt;")
                        i = '&';
                    else if (Attrib.Value == "&rt;")
                        i = '<';
                    else if (Attrib.Value == "&amp;")
                        i = '>';
                }
                
                if (i > ' ')
                    i -= ' ';
            }
            else if (Attrib.Name[0] == 'r' && Attrib.Value.size() == 15)
            {
                ClipList[i] = dim::rect2di(
                    Attrib.Value.section( 0,  3).val<s32>(),
                    Attrib.Value.section( 4,  7).val<s32>(),
                    Attrib.Value.section( 8, 11).val<s32>(),
                    Attrib.Value.section(12, 15).val<s32>()
                );
                
                s32 Height = ClipList[i].Bottom - ClipList[i].Top;
                
                if (FontHeight < Height)
                    FontHeight = Height;
            }
        }
    }
    
    /* Create font out of the given texture */
    return createFont(FontTexture, ClipList, FontHeight);
    
    #else
    
    io::Log::error("XML parser is required but was not compiled in this engine");
    io::Log::lowerTab();
    return 0;
    
    #endif
}

Font* RenderSystem::createFont(
    video::Texture* FontTexture, const std::vector<dim::rect2di> &ClipList, s32 FontHeight)
{
    return createFont("", 0, 0);
}

void RenderSystem::deleteFont(Font* FontObject)
{
    MemoryManager::removeElement(FontList_, FontObject, true);
}

void RenderSystem::draw2DText(
    Font* FontObject, const dim::point2di &Position, const io::stringc &Text, const color &Color)
{
}
void RenderSystem::draw3DText(
    Font* FontObject, const dim::matrix4f &Transformation, const io::stringc &Text, const color &Color)
{
}


/*
 * === Matrix controll ===
 */

void RenderSystem::matrixWorldViewReset()
{
    scene::spViewMatrix.reset();
    scene::spWorldMatrix.reset();
}

void RenderSystem::setProjectionMatrix(const dim::matrix4f &Matrix)
{
    scene::spProjectionMatrix = Matrix;
}
void RenderSystem::setViewMatrix(const dim::matrix4f &Matrix)
{
    scene::spViewMatrix = Matrix;
    scene::spViewInvMatrix = Matrix.getInverse();
}
void RenderSystem::setWorldMatrix(const dim::matrix4f &Matrix)
{
    scene::spWorldMatrix = Matrix;
}
void RenderSystem::setTextureMatrix(const dim::matrix4f &Matrix, u8 TextureLayer)
{
    scene::spTextureMatrix[TextureLayer] = Matrix;
}
void RenderSystem::setColorMatrix(const dim::matrix4f &Matrix)
{
    scene::spColorMatrix = Matrix;
}

dim::matrix4f RenderSystem::getProjectionMatrix() const
{
    return scene::spProjectionMatrix;
}
dim::matrix4f RenderSystem::getViewMatrix() const
{
    return scene::spViewMatrix;
}
dim::matrix4f RenderSystem::getWorldMatrix() const
{
    return scene::spWorldMatrix;
}
dim::matrix4f RenderSystem::getTextureMatrix(u8 TextureLayer) const
{
    return scene::spTextureMatrix[TextureLayer];
}
dim::matrix4f RenderSystem::getColorMatrix() const
{
    return scene::spColorMatrix;
}


/*
 * === Other renderer option functions ===
 */

void RenderSystem::setClippingRange(f32 Near, f32 Far)
{
    RangeNear_  = Near;
    RangeFar_   = Far;
}

void RenderSystem::setDrawingMatrix2D() { }

void RenderSystem::setDrawingMatrix3D()
{
    matrixWorldViewReset();
    
    if (__spSceneManager && __spSceneManager->getActiveCamera())
        __spSceneManager->getActiveCamera()->updateTransformation();
    
    updateModelviewMatrix();
}

void RenderSystem::createDeviceFont(
    void* FontObject, const io::stringc &FontName, const dim::size2di &FontSize,
    bool isBold,  bool isItalic, bool isUnderlined, bool isStrikeout, bool isSymbols) const
{
    if (!FontObject)
        return;
    
    #if defined(SP_PLATFORM_WINDOWS)
    *(HFONT*)FontObject = CreateFont(
        FontSize.Height,                            // Height of font
        FontSize.Width,                             // Width of font
        0,                                          // Angle of escapement
        0,                                          // Orientation angle
        isBold ? FW_BOLD : FW_NORMAL,               // Font weight
        isItalic ? TRUE : FALSE,                    // Italic
        isUnderlined ? TRUE : FALSE,                // Underline
        isStrikeout ? TRUE : FALSE,                 // Strikeout
        isSymbols ? SYMBOL_CHARSET : ANSI_CHARSET,  // Character set identifier
        OUT_TT_PRECIS,                              // Output precision
        CLIP_DEFAULT_PRECIS,                        // Clipping precision
        ANTIALIASED_QUALITY,                        // Output quality
        FF_DONTCARE | DEFAULT_PITCH,                // Family and pitch
        TEXT(FontName.c_str())                      // Font name
    );
    #endif
}

std::vector<s32> RenderSystem::getCharWidths(void* FontObject) const
{
    std::vector<s32> CharWidths;
    
    if (!FontObject)
        return CharWidths;
    
    #if defined(SP_PLATFORM_WINDOWS)
    CharWidths.resize(256);
    
    HFONT FontHandle = *(HFONT*)FontObject;
    
    /* Store the width of each ASCII character in the specified integer buffer */
    HANDLE LastObject = SelectObject(DeviceContext_, FontHandle);
    GetCharWidth(DeviceContext_, 0, 255, &CharWidths[0]);
    SelectObject(DeviceContext_, LastObject);
    #endif
    
    return CharWidths;
}

void RenderSystem::updateVertexInputLayout(VertexFormat* Format, bool isCreate) { }

void RenderSystem::createDefaultVertexFormats()
{
    /* Create default vertex formats */
    VertexFormatDefault_    = createVertexFormat<VertexFormatDefault>();
    VertexFormatReduced_    = createVertexFormat<VertexFormatReduced>();
    VertexFormatExtended_   = createVertexFormat<VertexFormatExtended>();
    VertexFormatFull_       = createVertexFormat<VertexFormatFull>();
    
    scene::SceneGraph::setDefaultVertexFormat(VertexFormatDefault_);
}


} // /namespace video

} // /namespace sp



// ================================================================================
