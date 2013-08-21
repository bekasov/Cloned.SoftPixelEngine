/*
 * Light grid file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spLightGrid.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "SceneGraph/spSceneLight.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Base/spSharedObjects.hpp"


//!!!
#if defined(SP_DEBUGMODE) || 1
#   define _DEB_LOAD_SHADERS_FROM_FILES_
#endif


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


/*
 * Constant buffer structures
 */

#if defined(_MSC_VER)
#   pragma pack(push, packing)
#   pragma pack(1)
#   define SP_PACK_STRUCT
#elif defined(__GNUC__)
#   define SP_PACK_STRUCT __attribute__((packed))
#else
#   define SP_PACK_STRUCT
#endif

struct SLightGridMainCB
{
    dim::uint2 NumTiles;
    dim::float2 InvNumTiles;
    dim::float2 InvResolution;
    dim::float2 Pad0;
}
SP_PACK_STRUCT;

struct SLightGridFrameCB
{
    dim::float4x4 InvViewProjection;
    dim::float3 ViewPosition;
    u32 NumLights;
    dim::plane3df NearPlane;
    dim::plane3df FarPlane;
}
SP_PACK_STRUCT;

#ifdef _MSC_VER
#   pragma pack(pop, packing)
#endif

#undef SP_PACK_STRUCT


/*
 * LightGrid class
 */

const dim::size2di LightGrid::GRID_SIZE(32);

LightGrid::LightGrid() :
    TLITexture_             (0),
    LGShaderResourceOut_    (0),
    LGShaderResourceIn_     (0),
    TLIShaderResourceOut_   (0),
    TLIShaderResourceIn_    (0),
    #ifdef _DEB_USE_LIGHT_TEXBUFFER_
    PointLightsShaderResource_(0),
    #endif
    ShdClass_               (0),
    ShdClassInit_           (0),
    NumTiles_               (1),
    NumLights_              (0)
{
}
LightGrid::~LightGrid()
{
    deleteGrid();
}

bool LightGrid::createGrid(const dim::size2di &Resolution, u32 MaxNumLights)
{
    /* Validate parameters */
    if (Resolution.Width <= 0 || Resolution.Height <= 0)
    {
        io::Log::error("Resolution for light-grid must be greater than zero");
        return false;
    }
    if (MaxNumLights == 0)
    {
        io::Log::error("Number of maximal lights for light-grid must be greater than zero");
        return false;
    }

    /* Delete old grid */
    deleteGrid();

    NumTiles_ = LightGrid::computeNumTiles(Resolution);

    switch (GlbRenderSys->getRendererType())
    {
        case RENDERER_OPENGL:
            return createTLITexture();
        case RENDERER_DIRECT3D11:
            return createShaderResources(MaxNumLights) && createComputeShaders(Resolution);
        default:
            io::Log::error("LightGrid is not supported for this render system");
            break;
    }

    return false;
}

void LightGrid::deleteGrid()
{
    /* Delete textures */
    if (TLITexture_)
        GlbRenderSys->deleteTexture(TLITexture_);
    
    /* Delete shader resources */
    GlbRenderSys->deleteShaderResource(LGShaderResourceOut_);
    GlbRenderSys->deleteShaderResource(LGShaderResourceIn_);

    GlbRenderSys->deleteShaderResource(TLIShaderResourceOut_);
    GlbRenderSys->deleteShaderResource(TLIShaderResourceIn_);

    #ifdef _DEB_USE_LIGHT_TEXBUFFER_
    GlbRenderSys->deleteShaderResource(PointLightsShaderResource_);
    #endif
    
    /* Delete shaders */
    GlbRenderSys->deleteShaderClass(ShdClass_);
    ShdClass_ = 0;
    
    GlbRenderSys->deleteShaderClass(ShdClassInit_);
    ShdClassInit_ = 0;
}

void LightGrid::updateLights(const std::vector<dim::vector4df> &PointLights, u32 NumLights)
{
    if (ShdClass_ && ShdClass_->getComputeShader())
    {
        /* Setup point light data */
        #ifdef _DEB_USE_LIGHT_TEXBUFFER_
        static bool _a;
        if (!_a)
        {
        _a = true;
        PointLightsShaderResource_->writeBuffer(&PointLights[0].X);
        }
        #else
        ShdClass_->getComputeShader()->setConstantBuffer(2, &PointLights[0].X);
        #endif
        NumLights_ = math::Min(NumLights, PointLights.size());
    }
}

void LightGrid::build(scene::SceneGraph* Graph, scene::Camera* ActiveCamera)
{
    if (Graph && ActiveCamera)
    {
        if (ShdClass_)
            buildOnGPU(Graph, ActiveCamera);
        else
            buildOnCPU(Graph, ActiveCamera);
    }
}

s32 LightGrid::bind(s32 TexLayerBase)
{
    if (TLITexture_)
        TLITexture_->bind(TexLayerBase++);
    return TexLayerBase;
}
s32 LightGrid::unbind(s32 TexLayerBase)
{
    if (TLITexture_)
        TLITexture_->unbind(TexLayerBase++);
    return TexLayerBase;
}

dim::size2di LightGrid::computeNumTiles(const dim::size2di &Resolution)
{
    return dim::size2di(
        static_cast<s32>(std::ceil(static_cast<f32>(Resolution.Width) / LightGrid::GRID_SIZE.Width)),
        static_cast<s32>(std::ceil(static_cast<f32>(Resolution.Height) / LightGrid::GRID_SIZE.Height))
    );
}


/*
 * ======= Private: =======
 */

bool LightGrid::createTLITexture()
{
    /* Create new buffer texture */
    STextureCreationFlags CreationFlags;
    {
        CreationFlags.Size          = 128;//1;
        CreationFlags.Format        = PIXELFORMAT_GRAYALPHA;
        CreationFlags.HWFormat      = HWTEXFORMAT_INT32;
        CreationFlags.BufferType    = IMAGEBUFFER_UBYTE;//!!!IMAGEBUFFER_INT
        CreationFlags.Type          = TEXTURE_BUFFER;
    }
    TLITexture_ = GlbRenderSys->createTexture(CreationFlags);

    #if 1//!!!
    ImageBuffer* buf = TLITexture_->getImageBuffer();
    u32* rawbuf = reinterpret_cast<u32*>(buf->getBuffer());

    for (u32 i = 0; i < 50; ++i)
    {
        if (i > 25)
            *(rawbuf++) = 100;
        else
            *(rawbuf++) = i;
        *(rawbuf++) = 0;
    }

    TLITexture_->updateImageBuffer();
    #endif

    //...

    return true;
}

bool LightGrid::createShaderResources(u32 MaxNumLights)
{
    /* Create new shader resource */
    TLIShaderResourceOut_   = GlbRenderSys->createShaderResource();
    TLIShaderResourceIn_    = GlbRenderSys->createShaderResource();

    LGShaderResourceOut_    = GlbRenderSys->createShaderResource();
    LGShaderResourceIn_     = GlbRenderSys->createShaderResource();

    if (!TLIShaderResourceOut_ || !TLIShaderResourceIn_ || !LGShaderResourceOut_ || !LGShaderResourceIn_)
    {
        io::Log::error("Could not create shader resources for light-grid");
        return false;
    }

    /* Setup light-grid shader resources */
    const u32 NumLightGridElements = NumTiles_.getArea();

    LGShaderResourceOut_->setupBuffer<u32>(NumLightGridElements);
    LGShaderResourceIn_->setupBufferRW<u32>(NumLightGridElements);

    /* Setup tile-light-index list shader resources */
    struct SLightNode
    {
        u32 LightID;
        u32 Next;
    };
    
    const u32 MaxTileLinks = NumLightGridElements * MaxNumLights;

    TLIShaderResourceOut_->setupBuffer<SLightNode>(MaxTileLinks);
    TLIShaderResourceIn_->setupBufferRW<SLightNode>(MaxTileLinks, 0, SHADERBUFFERFLAG_COUNTER);
    
    #ifdef _DEB_USE_LIGHT_TEXBUFFER_
    
    /* Setup point light shader resource */
    PointLightsShaderResource_ = GlbRenderSys->createShaderResource();
    PointLightsShaderResource_->setupBuffer<dim::float4>(MaxNumLights);
    
    #endif

    return true;
}

bool LightGrid::createComputeShaders(const dim::size2di &Resolution)
{
    /* Load shader source code */
    std::list<io::stringc> ShdBuf;

    switch (GlbRenderSys->getRendererType())
    {
        case RENDERER_DIRECT3D11:
        {
            #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
            ShdBuf.push_back(
                #include "Resources/spLightGridShaderStr.hlsl"
            );
            #else
            io::FileSystem fsys;
            ShaderClass::loadShaderResourceFile(fsys, "../../sources/RenderSystem/DeferredRenderer/spLightGridShader.hlsl", ShdBuf);
            #endif
        }
        break;
        
        default:
            io::Log::error("No light-grid compute shader support for this render system");
            return false;
    }
    
    /* Build compute shader */
    ShdClass_ = GlbRenderSys->createShaderClass();

    Shader* CompShd = GlbRenderSys->createShader(
        ShdClass_, SHADER_COMPUTE, HLSL_COMPUTE_5_0, ShdBuf, "ComputeMain"
    );

    if (!ShdClass_->link())
    {
        io::Log::error("Compiling light-grid compute shader failed");
        return false;
    }

    /* Build initialization compute shader */
    ShdClassInit_ = GlbRenderSys->createShaderClass();

    Shader* CompShdInit = GlbRenderSys->createShader(
        ShdClassInit_, SHADER_COMPUTE, HLSL_COMPUTE_5_0, ShdBuf, "ComputeInitMain"
    );

    if (!ShdClassInit_->link())
    {
        io::Log::error("Compiling light-grid initialization compute shader failed");
        return false;
    }

    /* Initialize constant buffers */
    SLightGridMainCB BufferMain;
    {
        BufferMain.NumTiles.X       = static_cast<u32>(NumTiles_.Width);
        BufferMain.NumTiles.Y       = static_cast<u32>(NumTiles_.Height);
        BufferMain.InvNumTiles.X    = 1.0f / static_cast<f32>(BufferMain.NumTiles.X);
        BufferMain.InvNumTiles.Y    = 1.0f / static_cast<f32>(BufferMain.NumTiles.Y);
        BufferMain.InvResolution.X  = 1.0f / static_cast<f32>(Resolution.Width);
        BufferMain.InvResolution.Y  = 1.0f / static_cast<f32>(Resolution.Width);
    }
    CompShd->setConstantBuffer(0, &BufferMain);
    CompShdInit->setConstantBuffer(0, &BufferMain);

    /* Setup final compute shader */
    ShdClass_->addShaderResource(LGShaderResourceIn_);
    ShdClass_->addShaderResource(TLIShaderResourceIn_);

    ShdClassInit_->addShaderResource(LGShaderResourceIn_);
    
    #ifdef _DEB_USE_LIGHT_TEXBUFFER_
    ShdClass_->addShaderResource(PointLightsShaderResource_);
    #endif

    return true;
}

void LightGrid::buildOnGPU(scene::SceneGraph* Graph, scene::Camera* Cam)
{
    /* Update frame constant buffer */
    SLightGridFrameCB BufferFrame;
    {
        /* Setup inverse-view-projection matrix */
        dim::matrix4f ViewMatrix = Cam->getTransformMatrix(true);
        
        BufferFrame.ViewPosition = ViewMatrix.getPosition();
        
        ViewMatrix.setPosition(0.0f);
        ViewMatrix.setInverse();
        
        BufferFrame.InvViewProjection = Cam->getProjection().getMatrixLH();
        BufferFrame.InvViewProjection *= ViewMatrix;
        BufferFrame.InvViewProjection.setInverse();
        
        /* Setup light count */
        BufferFrame.NumLights = NumLights_;
        
        /* Setup clipping planes */
        BufferFrame.NearPlane   = Cam->getViewFrustum().getPlane(scene::VIEWFRUSTUM_NEAR);
        BufferFrame.FarPlane    = Cam->getViewFrustum().getPlane(scene::VIEWFRUSTUM_FAR);
    }
    ShdClass_->getComputeShader()->setConstantBuffer(1, &BufferFrame);

    /* Execute compute shaders and copy input buffers to output buffers */
    const dim::vector3di NumThreads(NumTiles_.Width, NumTiles_.Height, 1);

    GlbRenderSys->runComputeShader(ShdClassInit_, NumThreads);
    GlbRenderSys->runComputeShader(ShdClass_, NumThreads);

    TLIShaderResourceOut_->copyBuffer(TLIShaderResourceIn_);
    LGShaderResourceOut_->copyBuffer(LGShaderResourceIn_);
}

void LightGrid::buildOnCPU(scene::SceneGraph* Graph, scene::Camera* Cam)
{
    // todo ...
    #ifdef SP_DEBUGMODE
    io::Log::debug("LightGrid::buildOnCPU", "Not yet implemented");
    #endif
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
