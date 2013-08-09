/*
 * Render system file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spTextureLayer.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/spSceneManager.hpp"
#include "Framework/Cg/spCgShaderClass.hpp"
#include "Framework/Tools/spToolXMLParser.hpp"
#include "Base/spMathRasterizer.hpp"
#include "Base/spSharedObjects.hpp"

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;
extern scene::SceneGraph* GlbSceneGraph;

namespace video
{


#ifdef SP_DEBUGMODE
u32 RenderSystem::NumDrawCalls_             = 0;
u32 RenderSystem::NumMeshBufferBindings_    = 0;
u32 RenderSystem::NumTexLayerBindings_      = 0;
u32 RenderSystem::NumMaterialUpdates_       = 0;
#endif

RenderSystem::RenderSystem(const ERenderSystems Type) :
    RendererType_           (Type           ),
    
    #if defined(SP_PLATFORM_WINDOWS)
    DeviceContext_          (0              ),
    PixelFormat_            (0              ),
    #elif defined(SP_PLATFORM_LINUX)
    Display_                (0              ),
    Window_                 (0              ),
    #endif
    
    RenderMode_             (RENDERMODE_NONE),
    MaxClippingPlanes_      (0              ),
    isFrontFace_            (true           ),
    TexLayerVisibleMask_    (~0             ),
    RenderTarget_           (0              ),
    CurShaderClass_         (0              ),
    GlobalShaderClass_      (0              ),
    ShaderSurfaceCallback_  (0              ),
    PrevMaterial_           (0              ),
    PrevTextureLayers_      (0              ),
    Material2DDrawing_      (0              ),
    Material3DDrawing_      (0              ),
    
    VertexFormatDefault_    (0              ),
    VertexFormatReduced_    (0              ),
    VertexFormatExtended_   (0              ),
    VertexFormatFull_       (0              )
{
    /* General settings */
    GlbRenderSys = this;
    
    setFillColor(255);
    
    for (u32 i = 0; i < RENDERQUERY_COUNT; ++i)
        RenderQuery_[i] = false;
    memset(DefaultTextures_, 0, sizeof(DefaultTextures_));
}
RenderSystem::~RenderSystem()
{
    /* Delete all extendet lists */
    MemoryManager::deleteList(MovieList_            );
    MemoryManager::deleteList(FontList_             );
    MemoryManager::deleteList(VertexFormatList_     );
    MemoryManager::deleteList(ShaderList_           );
    MemoryManager::deleteList(ShaderClassList_      );
    MemoryManager::deleteList(ShaderResourceList_   );
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
void RenderSystem::setDepthMask(bool Enable) { }

void RenderSystem::setAntiAlias(bool isAntiAlias) { }

void RenderSystem::setDepthRange(f32 Near, f32 Far)
{
    DepthRange_.Near = Near;
    DepthRange_.Far = Far;
}
void RenderSystem::getDepthRange(f32 &Near, f32 &Far) const
{
    Near = DepthRange_.Near;
    Far = DepthRange_.Far;
}

void RenderSystem::setDepthClip(bool Enable)
{
    DepthRange_.Enabled = Enable;
}

void RenderSystem::setStencilMask(u32 BitMask)
{
    // dummy
}
void RenderSystem::setStencilMethod(const ESizeComparisionTypes Method, s32 Reference, u32 BitMask)
{
    // dummy
}
void RenderSystem::setStencilOperation(const EStencilOperations FailOp, const EStencilOperations ZFailOp, const EStencilOperations ZPassOp)
{
    // dummy
}
void RenderSystem::setClearStencil(s32 Stencil)
{
    // dummy
}


/*
 * ======= Rendering 3D scenes =======
 */

void RenderSystem::setupTextureLayer(
    u8 LayerIndex, const dim::matrix4f &TexMatrix, const ETextureEnvTypes EnvType,
    const EMappingGenTypes GenType, s32 MappingCoordsFlags)
{
    // do nothing
}

void RenderSystem::bindTextureLayers(const TextureLayerListType &TexLayers)
{
    /* Check if this texture layer list is already bound */
    if (PrevTextureLayers_ == (&TexLayers))
        return;
    
    /* Unbind previously bounded texture layers */
    unbindPrevTextureLayers();
    
    PrevTextureLayers_ = (&TexLayers);
    
    /* Check for multi-texture support */
    if (RenderQuery_[RENDERQUERY_MULTI_TEXTURE])
    {
        /* Bind all texture layers */
        foreach (TextureLayer* TexLayer, TexLayers)
            TexLayer->bind();
    }
    else if (!TexLayers.empty())
    {
        /* Bind first texture layer only */
        TexLayers.front()->bind();
    }
    
    #ifdef SP_DEBUGMODE
    ++RenderSystem::NumTexLayerBindings_;
    #endif
}

void RenderSystem::unbindTextureLayers(const TextureLayerListType &TexLayers)
{
    /* Check for multi-texture support */
    if (RenderQuery_[RENDERQUERY_MULTI_TEXTURE])
    {
        /* Unbind all texture layers */
        foreach (TextureLayer* TexLayer, TexLayers)
            TexLayer->unbind();
    }
    else if (!TexLayers.empty())
    {
        /* Unbind first texture layer only */
        TexLayers.front()->unbind();
    }
}

void RenderSystem::setupShaderClass(const scene::MaterialNode* Object, ShaderClass* ShaderObject)
{
    if (RenderQuery_[RENDERQUERY_SHADER])
    {
        if (GlobalShaderClass_)
        {
            //if (!CurShaderClass_)
            //    PrevMaterial_ = 0;
            
            GlobalShaderClass_->bind(Object);
            CurShaderClass_ = GlobalShaderClass_;
        }
        else if (ShaderObject)
        {
            //if (!CurShaderClass_)
            //    PrevMaterial_ = 0;
            
            ShaderObject->bind(Object);
            CurShaderClass_ = ShaderObject;
        }
        else
        {
            //if (CurShaderClass_)
            //    PrevMaterial_ = 0;
            
            CurShaderClass_ = 0;
        }
    }
}

void RenderSystem::updateMaterialStates(MaterialStates* Material, bool isClear)
{
    if (PrevMaterial_ == Material)
        PrevMaterial_ = 0;
}

void RenderSystem::updateLight(
    u32 LightID, const scene::ELightModels LightType, bool isVolumetric,
    const dim::vector3df &Direction, f32 SpotInnerConeAngle, f32 SpotOuterConeAngle,
    f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic)
{
}

void RenderSystem::addDynamicLightSource(
    u32 LightID, scene::ELightModels Type,
    video::color &Diffuse, video::color &Ambient, video::color &Specular,
    f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic)
{
    // dummy
}
void RenderSystem::setLightStatus(u32 LightID, bool isEnable, bool UseAllRCs)
{
    // dummy
}
void RenderSystem::setLightColor(
    u32 LightID, const video::color &Diffuse, const video::color &Ambient, const video::color &Specular, bool UseAllRCs)
{
    // dummy
}


/* === Hardware mesh buffers === */

void RenderSystem::drawMeshBufferPlain(const MeshBuffer* MeshBuffer, bool useFirstTextureLayer)
{
    drawMeshBuffer(MeshBuffer);
}

void RenderSystem::setRenderMode(const ERenderModes Mode)
{
    if (RenderMode_ != Mode)
    {
        /* End previous render mode */
        switch (RenderMode_)
        {
            case RENDERMODE_DRAWING_2D:
                endDrawing2D();
                break;
            case RENDERMODE_DRAWING_3D:
                endDrawing3D();
                break;
            case RENDERMODE_SCENE:
                endSceneRendering();
                break;
            default:
                break;
        }
        
        /* Store new active mode */
        RenderMode_ = Mode;
        
        /* Begin with new render mode */
        switch (RenderMode_)
        {
            case RENDERMODE_DRAWING_2D:
                beginDrawing2D();
                break;
            case RENDERMODE_DRAWING_3D:
                beginDrawing3D();
                break;
            case RENDERMODE_SCENE:
                beginSceneRendering();
                break;
            default:
                break;
        }
    }
}


/* === Fog effect === */

void RenderSystem::setFog(const EFogTypes Type)
{
    Fog_.Type = Type;
}
EFogTypes RenderSystem::getFog() const
{
    return Fog_.Type;
}

void RenderSystem::setFogColor(const video::color &Color)
{
    Fog_.Color = Color;
}
video::color RenderSystem::getFogColor() const
{
    return Fog_.Color;
}

void RenderSystem::setFogRange(f32 Range, f32 NearPlane, f32 FarPlane, const EFogModes Mode)
{
    Fog_.Range  = Range;
    Fog_.Near   = NearPlane;
    Fog_.Far    = FarPlane;
    Fog_.Mode   = Mode;
}
void RenderSystem::getFogRange(f32 &Range, f32 &NearPlane, f32 &FarPlane, EFogModes &Mode)
{
    Range       = Fog_.Range;
    NearPlane   = Fog_.Near;
    FarPlane    = Fog_.Far;
    Mode        = Fog_.Mode;
}

void RenderSystem::setClipPlane(u32 Index, const dim::plane3df &Plane, bool Enable)
{
    // dummy
}


/*
 * ======= Shader programs =======
 */

ShaderClass* RenderSystem::createShaderClass(const VertexFormat* VertexInputLayout)
{
    return 0;
}

void RenderSystem::deleteShaderClass(ShaderClass* ShaderClassObj, bool DeleteAppendantShaders)
{
    if (ShaderClassObj)
    {
        if (DeleteAppendantShaders)
        {
            deleteShader(ShaderClassObj->getVertexShader());
            deleteShader(ShaderClassObj->getPixelShader());
            deleteShader(ShaderClassObj->getGeometryShader());
            deleteShader(ShaderClassObj->getHullShader());
            deleteShader(ShaderClassObj->getDomainShader());
        }
        
        MemoryManager::removeElement(ShaderClassList_, ShaderClassObj, true);
    }
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
    const io::stringc &Filename, io::stringc EntryPoint, s32 Flags, const std::list<io::stringc> &PreShaderCode)
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
            default:
                break;
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
        default:
            break;
    }
    
    if (EntryPoint.empty() && Version >= HLSL_VERTEX_1_0 && Version <= CG_VERSION_2_0)
    {
        switch (Type)
        {
            case SHADER_VERTEX:
                EntryPoint = "VertexMain"; break;
            case SHADER_PIXEL:
                EntryPoint = "PixelMain"; break;
            case SHADER_GEOMETRY:
                EntryPoint = "GeometryMain"; break;
            case SHADER_HULL:
                EntryPoint = "HullMain"; break;
            case SHADER_DOMAIN:
                EntryPoint = "DomainMain"; break;
            case SHADER_COMPUTE:
                EntryPoint = "ComputeMain"; break;
            default:
                break;
        }
    }
    
    io::Log::message("Load " + ShaderName + ": \"" + Filename + "\"");
    io::Log::upperTab();
    
    /* Read the shader file and copy the program lines into the shader buffer */
    std::list<io::stringc> ShaderBuffer(PreShaderCode);
    
    io::FileSystem FileSys;
    
    if (!FileSys.findFile(Filename))
        return createEmptyShaderWithError("Could not find shader file", ShaderClassObj, Type, Version);
    
    if (Flags & SHADERFLAG_ALLOW_INCLUDES)
    {
        if (!ShaderClass::loadShaderResourceFile(FileSys, Filename, ShaderBuffer))
            return createEmptyShaderWithError("Could not read shader file", ShaderClassObj, Type, Version);
    }
    else
    {
        io::File* ShaderFile = FileSys.readResourceFile(Filename);
        
        if (ShaderFile)
        {
            io::stringc Line, SubFilename;
            
            while (!ShaderFile->isEOF())
                ShaderBuffer.push_back(ShaderFile->readString() + "\n");
            
            FileSys.closeFile(ShaderFile);
        }
        else
            return createEmptyShaderWithError("Could not read shader file", ShaderClassObj, Type, Version);
    }
    
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
    const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint)
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
    if (RenderQuery_[RENDERQUERY_SHADER] && gSharedObjects.CgContext)
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
    const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint,
    const c8** CompilerOptions)
{
    return 0;
}

void RenderSystem::unbindShaders()
{
    // do nothing
}
void RenderSystem::deleteShader(Shader* ShaderObj)
{
    MemoryManager::removeElement(ShaderList_, ShaderObj, true);
}

ShaderResource* RenderSystem::createShaderResource()
{
    io::Log::warning("Shader resources are not supported by this render system");
    return 0;
}
void RenderSystem::deleteShaderResource(ShaderResource* &Resource)
{
    MemoryManager::removeElement(ShaderResourceList_, Resource, true);
}

bool RenderSystem::runComputeShader(ShaderClass* ShdClass, const dim::vector3di &GroupSize)
{
    io::Log::warning("Compute shaders are not supported within this render system");
    return false;
}


/*
 * ========== Simple drawing functions ==========
 */

void RenderSystem::beginSceneRendering()
{
    RenderMode_ = RENDERMODE_SCENE;
}
void RenderSystem::endSceneRendering()
{
    /* Unbind previously bounded texture layers */
    unbindPrevTextureLayers();
    
    /* Unbind previously bounding shader class */
    if (CurShaderClass_)
    {
        CurShaderClass_->unbind();
        CurShaderClass_ = 0;
    }
    
    RenderMode_ = RENDERMODE_NONE;
}

void RenderSystem::beginDrawing2D()
{
    /* Setup material states for 2D drawing */
    setupMaterialStates(Material2DDrawing_);
    
    RenderMode_ = RENDERMODE_DRAWING_2D;
}
void RenderSystem::endDrawing2D()
{
    RenderMode_ = RENDERMODE_NONE;
}

void RenderSystem::beginDrawing3D()
{
    /* Setup camera view */
    if (GlbSceneGraph && GlbSceneGraph->getActiveCamera())
        GlbSceneGraph->getActiveCamera()->setupRenderView();
    
    /* Setup material states for 3D drawing */
    setupMaterialStates(Material3DDrawing_);
    
    RenderMode_ = RENDERMODE_DRAWING_3D;
}
void RenderSystem::endDrawing3D()
{
    RenderMode_ = RENDERMODE_NONE;
}

void RenderSystem::setBlending(const EBlendingTypes SourceBlend, const EBlendingTypes DestBlend) { }
void RenderSystem::setClipping(bool Enable, const dim::point2di &Position, const dim::size2di &Size) { }
void RenderSystem::setViewport(const dim::point2di &Position, const dim::size2di &Size) { }

bool RenderSystem::setRenderTarget(Texture* Target)
{
    return false;
}
bool RenderSystem::setRenderTarget(Texture* Target, ShaderClass* ShdClass)
{
    return setRenderTarget(Target);
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
    const Texture* Tex, const dim::point2di &Position, const color &Color)
{
    // do nothing
}
void RenderSystem::draw2DImage(
    const Texture* Tex, const dim::rect2di &Position, const dim::rect2df &Clipping, const color &Color)
{
    // do nothing
}
void RenderSystem::draw2DImage(
    const Texture* Tex, const dim::point2di &Position, f32 Rotation, f32 Radius, const color &Color)
{
    // do nothing
}

// !TODO! -> make a rotation matrix here !!!
void RenderSystem::draw2DImage(
    const Texture* Tex, dim::rect2di Position, const dim::rect2df &Clipping, f32 Rotation, const dim::point2df &RotationPoint,
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
    const Texture* Tex,
    const dim::point2di &lefttopPosition, const dim::point2di &righttopPosition,
    const dim::point2di &rightbottomPosition, const dim::point2di &leftbottomPosition,
    const dim::point2df &lefttopClipping, const dim::point2df &righttopClipping,
    const dim::point2df &rightbottomClipping, const dim::point2df &leftbottomClipping,
    const color &lefttopColor, const color &righttopColor,
    const color &rightbottomColor, const color &leftbottomColor)
{
    // do nothing
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

static void Draw2DPointCallback(s32 x, s32 y, void* UserData)
{
    GlbRenderSys->draw2DPoint(dim::point2di(x, y), *static_cast<const video::color*>(UserData));
}

void RenderSystem::draw2DCircle(const dim::point2di &Position, s32 Radius, const color &Color)
{
    math::Rasterizer::rasterizeCircle(Draw2DPointCallback, Position, Radius, const_cast<color*>(&Color));
}
void RenderSystem::draw2DEllipse(const dim::point2di &Position, const dim::size2di &Radius, const color &Color)
{
    math::Rasterizer::rasterizeEllipse(Draw2DPointCallback, Position, Radius, const_cast<color*>(&Color));
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

void RenderSystem::draw3DPoint(const dim::vector3df &Position, const color &Color)
{
    // do nothing
}
void RenderSystem::draw3DLine(
    const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &Color)
{
    // do nothing
}
void RenderSystem::draw3DLine(
    const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &ColorA, const color &ColorB)
{
    // do nothing
}
void RenderSystem::draw3DEllipse(
    const dim::vector3df &Position, const dim::vector3df &Rotation, const dim::size2df &Radius, const color &Color)
{
    // do nothing
}
void RenderSystem::draw3DTriangle(
    Texture* Tex, const dim::triangle3df &Triangle, const color &Color)
{
    // do nothing
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
    
    //io::Log::message("Get preloaded texture: \"" + Filename + "\"");
    
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
    if (Tex)
    {
        TextureListSemaphore_.lock();
        MemoryManager::removeElement(TextureList_, Tex, true);
        TextureListSemaphore_.unlock();
    }
}

bool RenderSystem::isTexture(const Texture* Tex) const
{
    return Tex && Tex->valid();
}

Texture* RenderSystem::copyTexture(const Texture* Tex)
{
    if (!Tex)
    {
        io::Log::warning("Invalid object for texture copy");
        return RenderSystem::createTexture(DEF_TEXTURE_SIZE);
    }
    
    /* Setup texture creation flags */
    STextureCreationFlags CreationFlags;
    {
        CreationFlags.Filename      = Tex->getFilename();
        CreationFlags.Size          = Tex->getSize();
        CreationFlags.ImageBuffer   = Tex->getImageBuffer()->getBuffer();
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

Texture* RenderSystem::createCubeMap(const dim::size2di &Size, bool isRenderTarget)
{
    /* Setup texture creation flags */
    STextureCreationFlags CreationFlags(TexGenFlags_);
    {
        CreationFlags.Size.Width    = Size.Width;
        CreationFlags.Size.Height   = Size.Height;
        CreationFlags.Depth         = 6;
        CreationFlags.Dimension     = TEXTURE_CUBEMAP;
        CreationFlags.WrapMode      = TEXWRAP_CLAMP;
    }
    Texture* CubeMap = createTexture(CreationFlags);
    
    if (isRenderTarget)
        CubeMap->setRenderTarget(true);
    
    return CubeMap;
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
    const s32 MagicNumber = TexFile->readValue<s32>();
    
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
    if ( *((const s32*)&MagicNumber) == *((const s32*)"8BPS") )
        return IMAGEFORMAT_PSD;
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
    
    if (!Tex->getImageBuffer()->getBuffer())
    {
        io::Log::warning("Can not save texture with no buffer");
        return false;
    }
    
    if (Tex->getImageBuffer()->getType() != IMAGEBUFFER_UBYTE)
    {
        io::Log::warning("Currently only UBYTE image buffers can be saved");
        return false;
    }
    
    /* Information message */
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

Texture* RenderSystem::createTextureFromDeviceBitmap(void* BitmapDC, void* Bitmap)
{
    #if defined(SP_PLATFORM_WINDOWS)
    
    /* Get bitmap information */
    if (!BitmapDC || !Bitmap)
        return 0;
    
    HDC dc = *((HDC*)BitmapDC);
    HBITMAP bmp = *((HBITMAP*)Bitmap);
    
    BITMAP bmInfo;
    GetObject(bmp, sizeof(bmInfo), &bmInfo);
    
    /* Get the bitmap's image buffer */
    BITMAPINFOHEADER bi = { 0 };
    bi.biSize           = sizeof(BITMAPINFOHEADER);
    bi.biWidth          = bmInfo.bmWidth;
    bi.biHeight         = bmInfo.bmHeight;
    bi.biPlanes         = 1;
    bi.biBitCount       = 24;
    bi.biCompression    = BI_RGB;
    
    /* Get the size of the image data */
    if (!GetDIBits(dc, bmp, 0, bmInfo.bmHeight, 0, (BITMAPINFO*)&bi, DIB_RGB_COLORS))
    {
        io::Log::error("Getting device bitmap information failed");
        return 0;
    }
    
    /* Retrieve the image data */
    u8* ImageBuffer = new u8[bi.biSizeImage];
    
    if (!GetDIBits(dc, bmp, 0, bmInfo.bmHeight, ImageBuffer, (BITMAPINFO*)&bi, DIB_RGB_COLORS))
    {
        delete [] ImageBuffer;
        io::Log::error("Getting device bitmap's image buffer failed");
        return 0;
    }
    
    /* Create final font texture */
    video::Texture* Tex = createTexture(
        dim::size2di(bmInfo.bmWidth, bmInfo.bmHeight), PIXELFORMAT_RGB, ImageBuffer
    );
    
    delete [] ImageBuffer;
    
    Tex->getImageBuffer()->flipImageVertical();
    Tex->updateImageBuffer();
    
    return Tex;
    
    #else
    
    #   ifdef SP_DEBUGMODE
    io::Log::debug("RenderSystem::createTextureFromDeviceBitmap", "Not supported for unix systems yet");
    #   endif
    
    return 0;
    
    #endif
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

Font* RenderSystem::createFont(const io::stringc &FontName, s32 FontSize, s32 Flags)
{
    Font* NewFontObj = 0;
    
    if (Flags & FONT_BITMAP)
    {
        io::Log::message("Create bitmap font: \"" + FontName + "\" with size of " + io::stringc(FontSize));
        io::Log::upperTab();
        
        NewFontObj = createBitmapFont(FontName, FontSize, Flags);
    }
    else
    {
        io::Log::message("Create texture font: \"" + FontName + "\" with size of " + io::stringc(FontSize));
        io::Log::upperTab();
        
        NewFontObj = createTexturedFont(FontName, FontSize, Flags);
    }
    
    io::Log::lowerTab();
    
    return NewFontObj;
}

Font* RenderSystem::createTexturedFont(const io::stringc &FontName, s32 FontSize, s32 Flags)
{
    std::vector<video::SFontGlyph> GlyphList;
    
    video::Texture* Tex = createFontTexture(GlyphList, FontName, FontSize, Flags);
    Tex->setFilename(FontName + "|" + io::stringc(FontSize));
    
    return createFont(Tex, GlyphList, FontSize);
}

Font* RenderSystem::createBitmapFont(const io::stringc &FontName, s32 FontSize, s32 Flags)
{
    Font* NewFont = MemoryManager::createMemory<Font>("Font");
    FontList_.push_back(NewFont);
    return NewFont;
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
    
    std::vector<SFontGlyph> GlyphList;
    
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
                
                GlyphList.push_back(dim::rect2di(Start.X, Start.Y + 2, Pos.X, Start.Y + FontHeight));
                isSearchEnd = false;
            }
        }
    }
    
    FontTexture->updateImageBuffer();
    
    /* Check for errors after analyzing font texture */
    if (GlyphList.empty())
    {
        io::Log::error("Font texture does not contain any characters");
        return 0;
    }
    
    /* Create font out of the given texture */
    return createFont(FontTexture, GlyphList, FontHeight);
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
    std::vector<SFontGlyph> GlyphList(256);
    u8 i = 0;
    //u32 Count = 0;
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
                GlyphList[i].Rect = dim::rect2di(
                    Attrib.Value.section( 0,  3).val<s32>(),
                    Attrib.Value.section( 4,  7).val<s32>(),
                    Attrib.Value.section( 8, 11).val<s32>(),
                    Attrib.Value.section(12, 15).val<s32>()
                );
                
                s32 Height = GlyphList[i].Rect.Bottom - GlyphList[i].Rect.Top;
                
                if (FontHeight < Height)
                    FontHeight = Height;
            }
        }
    }
    
    /* Create font out of the given texture */
    return createFont(FontTexture, GlyphList, FontHeight);
    
    #else
    
    io::Log::error("XML parser is required but was not compiled in this engine");
    io::Log::lowerTab();
    return 0;
    
    #endif
}

Font* RenderSystem::createFont(
    video::Texture* FontTexture, const std::vector<SFontGlyph> &GlyphList, s32 FontHeight)
{
    if (!FontTexture)
    {
        io::Log::error("Can not create textured-font without texture");
        return 0;
    }
    
    /* Create vertex buffer */
    dim::rect2df Mapping;
    dim::UniversalBuffer VertexBuffer;
    VertexFormatUniversal VertFormat;
    
    createTexturedFontVertexBuffer(VertexBuffer, VertFormat);
    
    VertexBuffer.setCount(4*256);
    void* RawVertexData = VertexBuffer.getArray();
    
    const dim::size2di TexSize(FontTexture->getSize());
    
    /* Create each character for texture font */
    foreach (const SFontGlyph &Glyph, GlyphList)
    {
        /* Calculate texture mapping */
        Mapping.Left    = static_cast<f32>(Glyph.Rect.Left    ) / TexSize.Width;
        Mapping.Top     = static_cast<f32>(Glyph.Rect.Top     ) / TexSize.Height;
        Mapping.Right   = static_cast<f32>(Glyph.Rect.Right   ) / TexSize.Width;
        Mapping.Bottom  = static_cast<f32>(Glyph.Rect.Bottom  ) / TexSize.Height;
        
        /* Setup vertex data */
        setupTexturedFontGlyph(RawVertexData, Glyph, Mapping);
    }
    
    /* Create new vertex buffer for character */
    void* BufferID = 0;
    
    createVertexBuffer(BufferID);
    updateVertexBuffer(BufferID, VertexBuffer, &VertFormat, HWBUFFER_STATIC);
    
    /* Create final font object */
    Font* NewFont = new Font(
        BufferID, FontTexture->getFilename(),
        dim::size2di(FontHeight/2, FontHeight), GlyphList, FontTexture
    );
    
    FontList_.push_back(NewFont);
    
    return NewFont;
}


#if defined(SP_PLATFORM_WINDOWS)
//!TODO! -> rename this when it's a global structure!!!
/* Declare glyph structure */
struct SGlyph;

typedef scene::ImageTreeNode<SGlyph> TGlyphNode;

struct SGlyph : public SFontGlyph
{
    SGlyph(HDC dc, u32 GlyphChar) : SFontGlyph()
    {
        c8 CharUTF8 = static_cast<c8>(GlyphChar);
        
        /* Query glyph metrics */
        SIZE sz;
        GetTextExtentPoint32A(dc, &CharUTF8, 1, &sz);
        
        ABC abc;
        if (!GetCharABCWidths(dc, GlyphChar, GlyphChar, &abc))
            throw io::stringc("Getting font glyph metrics failed");
        
        /* Setup glyph metrics */
        StartOffset = abc.abcA;
        DrawnWidth  = abc.abcB;
        WhiteSpace  = abc.abcC;
        
        Size.Width  = DrawnWidth + 2;
        Size.Height = sz.cy + 2;
    }
    ~SGlyph()
    {
    }
    
    /* Inline functions */
    inline dim::size2di getSize() const
    {
        return Size;
    }
    inline void setupTreeNode(TGlyphNode* Node)
    {
        // do nothing (template interface function)
    }
    
    /* Members */
    dim::size2di Size;
};

#endif

Texture* RenderSystem::createFontTexture(
    std::vector<SFontGlyph> &GlyphList, const io::stringc &FontName, s32 FontSize, s32 Flags)
{
    #if defined(SP_PLATFORM_WINDOWS)
    
    Texture* Tex = 0;
    
    /* Create device font */
    HFONT FontHandle = 0;
    createDeviceFont(&FontHandle, FontName, dim::size2di(0, FontSize), Flags);
    
    HGDIOBJ PrevFont = SelectObject(DeviceContext_, FontHandle);
    
    /* Create all glyphs */
    SGlyph* Glyphs[256] = { 0 };
    SGlyph* Glyph = 0;
    s32 Area = 0;
    
    try
    {
        /* Create all character glyphs */
        for (s32 i = 32; i < 256; ++i)
        {
            if (!IsDBCSLeadByte(static_cast<CHAR>(i)))
            {
                Glyphs[i] = new SGlyph(DeviceContext_, i);
                Area += Glyphs[i]->getSize().getArea();
            }
        }
    }
    catch (const io::stringc &ErrMessage)
    {
        /* Print error message */
        io::Log::error(ErrMessage);
        
        /* Release allocated memory */
        SelectObject(DeviceContext_, PrevFont);
        DeleteObject(FontHandle);
        
        for (s32 i = 32; i < 256; ++i)
            delete Glyphs[i];
        
        /* Create default texture */
        return createTexture(1);
    }
    
    /* Compute texture size */
    Area = static_cast<s32>(sqrt(static_cast<f32>(Area)));
    dim::size2di TexSize = dim::size2di(math::roundPow2(Area));
    
    if (TexSize.Width < Area)
        TexSize.Width *= 2;
    
    /* Build glyph tree */
    TGlyphNode RootNode(TexSize);
    
    while (1)
    {
        bool FillTreeFailed = false;
        
        for (s32 i = 32; i < 256; ++i)
        {
            Glyph = Glyphs[i];
            
            if (!Glyph)
                continue;
            
            TGlyphNode* Node = RootNode.insert(Glyph);
            
            if (!Node)
            {
                FillTreeFailed = true;
                break;
            }
            
            Glyph->Rect = Node->getRect();
        }
        
        if (FillTreeFailed)
        {
            /* Increase texture size and try tree-building again */
            if (TexSize.Width < TexSize.Height)
                TexSize.Width *= 2;
            else
                TexSize.Height *= 2;
            
            RootNode.deleteChildren();
            RootNode.setRect(dim::rect2di(0, 0, TexSize.Width, TexSize.Height));
        }
        else
            break;
    }
    
    /* Create font bitmap */
    HBITMAP Bitmap          = CreateCompatibleBitmap(DeviceContext_, TexSize.Width, TexSize.Height);
    HDC BitmapDC            = CreateCompatibleDC(DeviceContext_);
    
    LOGBRUSH LogBrush;
    LogBrush.lbStyle        = BS_SOLID;
    LogBrush.lbColor        = RGB(0, 0, 0);
    LogBrush.lbHatch        = 0;
    
    HBRUSH Brush            = CreateBrushIndirect(&LogBrush);
    HPEN Pen                = CreatePen(PS_NULL, 0, 0);
    
    HGDIOBJ PrevBitmap      = SelectObject(BitmapDC, Bitmap);
    HGDIOBJ PrevBmpPen      = SelectObject(BitmapDC, Pen);
    HGDIOBJ PrevBmpBrush    = SelectObject(BitmapDC, Brush);
    HGDIOBJ PrevBmpFont     = SelectObject(BitmapDC, FontHandle);
    
    SetTextColor(BitmapDC, RGB(255, 255, 255));
    
    Rectangle(BitmapDC, 0, 0, TexSize.Width, TexSize.Height);
    SetBkMode(BitmapDC, TRANSPARENT);
    
    GlyphList.resize(256);
    
    /* Draw font characters */
    for (s32 i = 32; i < 256; ++i)
    {
        Glyph = Glyphs[i];
        
        if (!Glyph)
            continue;
        
        c8 Char = static_cast<c8>(i);
        
        /* Draw glyph to bitmap */
        TextOut(
            BitmapDC,
            Glyph->Rect.Left - Glyph->StartOffset + 1,
            Glyph->Rect.Top + 1,
            &Char, 1
        );
        
        /* Copy glyph information */
        GlyphList[i] = *Glyph;
        GlyphList[i].Rect += dim::rect2di(1, 1, -1, -1);
    }
    
    Tex = createTextureFromDeviceBitmap(&BitmapDC, &Bitmap);
    
    /* Convert gray values to alpha channel */
    Tex->getImageBuffer()->grayToAlpha();
    Tex->updateImageBuffer();
    
    /* Clean up */
    SelectObject(BitmapDC, PrevBitmap);
    SelectObject(BitmapDC, PrevBmpPen);
    SelectObject(BitmapDC, PrevBmpBrush);
    SelectObject(BitmapDC, PrevBmpFont);
    
    SelectObject(DeviceContext_, PrevFont);
    DeleteObject(FontHandle);
    
    DeleteDC(BitmapDC);
    DeleteObject(Brush);
    DeleteObject(Pen);
    DeleteObject(Bitmap);
    
    for (s32 i = 32; i < 256; ++i)
        delete Glyphs[i];
    
    return Tex;
    
    #else
    
    io::Log::error("Dynamic font texture creation is only supported under MS/Windows");
    return createTexture(1);
    
    #endif
}

void RenderSystem::deleteFont(Font* FontObj)
{
    if (FontObj)
    {
        releaseFontObject(FontObj);
        MemoryManager::removeElement(FontList_, FontObj, true);
    }
}

void RenderSystem::draw2DText(
    const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color, s32 Flags)
{
    if (!FontObj || !FontObj->getBufferRawData() || FontObj->getGlyphList().size() < 256)
        return;
    
    /* Check for drawing flags */
    dim::matrix4f PrevFontTransform;
    
    if (Flags)
    {
        dim::vector3df Offset;
        
        if (Flags & TEXT_RIGHT_ALIGN)
        {
            const s32 TextWidth = FontObj->getStringWidth(Text);
            Offset.X = static_cast<f32>(-TextWidth);
        }
        else if (Flags & TEXT_CENTER_HORZ)
        {
            const s32 TextWidth = FontObj->getStringWidth(Text);
            Offset.X = static_cast<f32>(-TextWidth/2);
        }
        
        if (Flags & TEXT_CENTER_VERT)
        {
            const s32 TextHeight = FontObj->getSize().Height;
            Offset.Y = static_cast<f32>(-TextHeight/2);
        }
        
        PrevFontTransform = FontTransform_;
        FontTransform_.translate(Offset);
    }
    
    /* Draw 2d textured- or bitmap font */
    const dim::size2di FontSize(FontObj->getSize());
    
    if (Position.X < gSharedObjects.ScreenWidth && Position.Y < gSharedObjects.ScreenHeight && Position.Y > -FontSize.Height)
    {
        if (FontObj->getTexture())
            drawTexturedFont(FontObj, Position, Text, Color);
        else
            drawBitmapFont(FontObj, Position, Text, Color);
    }
    
    /* Reset previous settings */
    if (Flags)
        FontTransform_ = PrevFontTransform;
}

void RenderSystem::draw3DText(
    const Font* FontObject, const dim::matrix4f &Transformation, const io::stringc &Text, const color &Color)
{
    // dummy
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

void RenderSystem::setupWVPMatrix(dim::matrix4f &Mat) const
{
    Mat = getProjectionMatrix();
    Mat *= getViewMatrix();
    Mat *= getWorldMatrix();
}
void RenderSystem::setupVPMatrix(dim::matrix4f &Mat) const
{
    Mat = getViewMatrix();
    Mat *= getWorldMatrix();
}
void RenderSystem::setupWVMatrix(dim::matrix4f &Mat) const
{
    Mat = getViewMatrix();
    Mat *= getWorldMatrix();
}
void RenderSystem::setupInvVPMatrix(dim::matrix4f &Mat) const
{
    Mat = getProjectionMatrix();
    
    dim::matrix4f ViewMatrix(getViewMatrix());
    ViewMatrix.setPosition(0.0f);
    
    Mat *= ViewMatrix;
    
    Mat.setInverse();
}

dim::matrix4f RenderSystem::getWVPMatrix() const
{
    dim::matrix4f Mat;
    setupWVPMatrix(Mat);
    return Mat;
}
dim::matrix4f RenderSystem::getVPMatrix() const
{
    dim::matrix4f Mat;
    setupVPMatrix(Mat);
    return Mat;
}
dim::matrix4f RenderSystem::getWVMatrix() const
{
    dim::matrix4f Mat;
    setupWVMatrix(Mat);
    return Mat;
}
dim::matrix4f RenderSystem::getInvVPMatrix() const
{
    dim::matrix4f Mat;
    setupInvVPMatrix(Mat);
    return Mat;
}

u32 RenderSystem::queryDrawCalls()
{
    #ifdef SP_DEBUGMODE
    return NumDrawCalls_;
    #else
    return 0;
    #endif
}
u32 RenderSystem::queryMeshBufferBindings()
{
    #ifdef SP_DEBUGMODE
    return NumMeshBufferBindings_;
    #else
    return 0;
    #endif
}
u32 RenderSystem::queryTextureLayerBindings()
{
    #ifdef SP_DEBUGMODE
    return NumTexLayerBindings_;
    #else
    return 0;
    #endif
}
u32 RenderSystem::queryMaterialUpdates()
{
    #ifdef SP_DEBUGMODE
    return NumMaterialUpdates_;
    #else
    return 0;
    #endif
}


/*
 * === Other renderer option functions ===
 */

void RenderSystem::setup2DDrawing()
{
    setRenderMode(RENDERMODE_DRAWING_2D);
}

void RenderSystem::setup3DDrawing()
{
    setRenderMode(RENDERMODE_DRAWING_3D);
    
    matrixWorldViewReset();
    
    if (GlbSceneGraph && GlbSceneGraph->getActiveCamera())
        GlbSceneGraph->getActiveCamera()->updateTransformation();
    
    updateModelviewMatrix();
}

void RenderSystem::createDeviceFont(
    void* FontObject, const io::stringc &FontName, const dim::size2di &FontSize, s32 Flags, s32 CharSet) const
{
    if (!FontObject)
        return;
    
    #if defined(SP_PLATFORM_WINDOWS)
    *(HFONT*)FontObject = CreateFont(
        FontSize.Height,                                        // Height of font
        FontSize.Width,                                         // Width of font
        0,                                                      // Angle of escapement
        0,                                                      // Orientation angle
        (Flags & FONT_BOLD      ) != 0 ? FW_BOLD : FW_NORMAL,   // Font weight
        (Flags & FONT_ITALIC    ) != 0 ? TRUE : FALSE,          // Italic
        (Flags & FONT_UNDERLINED) != 0 ? TRUE : FALSE,          // Underline
        (Flags & FONT_STRIKEOUT ) != 0 ? TRUE : FALSE,          // Strikeout
        ANSI_CHARSET,                                           // Character set identifier
        OUT_TT_PRECIS,                                          // Output precision
        CLIP_DEFAULT_PRECIS,                                    // Clipping precision
        ANTIALIASED_QUALITY,                                    // Output quality
        FF_DONTCARE | DEFAULT_PITCH,                            // Family and pitch
        TEXT(FontName.c_str())                                  // Font name
    );
    #endif
}

std::vector<SFontGlyph> RenderSystem::getCharWidths(void* FontObject) const
{
    std::vector<SFontGlyph> GlyphList;
    
    if (!FontObject)
        return GlyphList;
    
    #if defined(SP_PLATFORM_WINDOWS)
    
    HFONT FontHandle = *(HFONT*)FontObject;
    
    /* Store the width of each ASCII character in the specified integer buffer */
    HANDLE PrevObject = SelectObject(DeviceContext_, FontHandle);
    
    ABC CharWidths[256];
    if (!GetCharABCWidths(DeviceContext_, 0, 255, CharWidths))
    {
        io::Log::error("Getting device font character widths failed");
        return GlyphList;
    }
    
    SelectObject(DeviceContext_, PrevObject);
    
    /* Convert to glyph list */
    GlyphList.resize(256);
    
    for (u32 i = 0; i < 256; ++i)
    {
        GlyphList[i].StartOffset    = CharWidths[i].abcA;
        GlyphList[i].DrawnWidth     = CharWidths[i].abcB;
        GlyphList[i].WhiteSpace     = CharWidths[i].abcC;
    }
    
    #endif
    
    return GlyphList;
}

void RenderSystem::updateVertexInputLayout(VertexFormat* Format, bool isCreate) { }

void RenderSystem::createDefaultResources()
{
    createDefaultVertexFormats();
    createDefaultTextures();
    createDrawingMaterials();
}
void RenderSystem::deleteDefaultResources()
{
    /* Delete default textures */
    for (u32 i = 0; i < DEFAULT_TEXTURE_COUNT; ++i)
        MemoryManager::deleteMemory(DefaultTextures_[i]);
    
    /* Delete drawing material states */
    delete Material2DDrawing_;
    delete Material3DDrawing_;
}

void RenderSystem::releaseFontObject(Font* FontObj)
{
    // dummy
}

void RenderSystem::drawTexturedFont(
    const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color)
{
    // dummy
}

void RenderSystem::drawBitmapFont(
    const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color)
{
    // dummy
}

// Default font glyph vertex format (OpenGL format)
struct SFontGlyphVertexGL
{
    dim::point2di Position;
    dim::point2df TexCoord;
};

void RenderSystem::createTexturedFontVertexBuffer(dim::UniversalBuffer &VertexBuffer, VertexFormatUniversal &VertFormat)
{
    /* Default vertex buffer for textured font glyphs (OpenGL format) */
    VertexBuffer.setStride(sizeof(SFontGlyphVertexGL));
    
    VertFormat.addCoord(DATATYPE_INT, 2);
    VertFormat.addTexCoord();
}

void RenderSystem::setupTexturedFontGlyph(
    void* &RawVertexData, const SFontGlyph &Glyph, const dim::rect2df &Mapping)
{
    SFontGlyphVertexGL* VertexData = reinterpret_cast<SFontGlyphVertexGL*>(RawVertexData);
    
    VertexData[0].Position = dim::point2di(0, 0);
    VertexData[1].Position = dim::point2di(Glyph.Rect.Right - Glyph.Rect.Left, 0);
    VertexData[2].Position = dim::point2di(0, Glyph.Rect.Bottom - Glyph.Rect.Top);
    VertexData[3].Position = dim::point2di(Glyph.Rect.Right - Glyph.Rect.Left, Glyph.Rect.Bottom - Glyph.Rect.Top);
    
    VertexData[0].TexCoord = dim::point2df(Mapping.Left, Mapping.Top);
    VertexData[1].TexCoord = dim::point2df(Mapping.Right, Mapping.Top);
    VertexData[2].TexCoord = dim::point2df(Mapping.Left, Mapping.Bottom);
    VertexData[3].TexCoord = dim::point2df(Mapping.Right, Mapping.Bottom);
    
    VertexData += 4;
    
    RawVertexData = VertexData;
}

void RenderSystem::unbindPrevTextureLayers()
{
    if (PrevTextureLayers_)
    {
        unbindTextureLayers(*PrevTextureLayers_);
        PrevTextureLayers_ = 0;
    }
}

void RenderSystem::noticeTextureLayerChanged(const TextureLayer* TexLayer)
{
    if (PrevTextureLayers_)
    {
        foreach (TextureLayer* SubTexLayer, *PrevTextureLayers_)
        {
            /* Check if given texture layer is stored in the previous texture layer list */
            if (SubTexLayer == TexLayer)
            {
                unbindPrevTextureLayers();
                break;
            }
        }
    }
}

void RenderSystem::resetQueryCounters()
{
    #ifdef SP_DEBUGMODE
    /* Reset draw call counter */
    NumDrawCalls_           = 0;
    NumMeshBufferBindings_  = 0;
    NumTexLayerBindings_    = 0;
    NumMaterialUpdates_     = 0;
    #endif
}


/*
 * ======= Private: =======
 */

void RenderSystem::createDefaultVertexFormats()
{
    /* Create default vertex formats */
    VertexFormatDefault_    = createVertexFormat<VertexFormatDefault>();
    VertexFormatReduced_    = createVertexFormat<VertexFormatReduced>();
    VertexFormatExtended_   = createVertexFormat<VertexFormatExtended>();
    VertexFormatFull_       = createVertexFormat<VertexFormatFull>();
    
    scene::SceneManager::setDefaultVertexFormat(VertexFormatDefault_);
}

void RenderSystem::createDefaultTextures()
{
    /* Create default textures */
    video::Texture* Tex = DefaultTextures_[DEFAULT_TEXTURE_TILES] = createTexture(2, video::PIXELFORMAT_RGBA);
    Tex->setFilter(video::FILTER_LINEAR);
    
    const video::color ImgBuffer[4] =
    {
        video::color(100),
        video::color( 75),
        video::color( 75),
        video::color(100)
    };
    Tex->setupImageBuffer(&ImgBuffer[0].Red);
    
    /* Remove texture from original list */
    //!TODO! -> don't add the texture to the list and then remove it (change API for GL, D3D9 etc.)
    TextureList_.pop_back();
}

void RenderSystem::createDrawingMaterials()
{
    /* Create default 2D drawing material states */
    Material2DDrawing_ = new MaterialStates();
    
    Material2DDrawing_->setRenderFace(video::FACE_BOTH);
    Material2DDrawing_->setLighting(false);
    Material2DDrawing_->setDepthBuffer(false);
    Material2DDrawing_->setFog(false);
    
    /* Create default 3D drawing material states */
    Material3DDrawing_ = new MaterialStates();
    
    Material2DDrawing_->setRenderFace(video::FACE_BOTH);
    Material3DDrawing_->setLighting(false);
    Material3DDrawing_->setFog(false);
}


} // /namespace video

} // /namespace sp



// ================================================================================
