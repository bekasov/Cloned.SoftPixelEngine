/*
 * Light grid file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/AdvancedRenderer/spLightGrid.hpp"

#if defined(SP_COMPILE_WITH_ADVANCEDRENDERER)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "SceneGraph/spSceneLight.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Base/spSharedObjects.hpp"


//!!!
#if ( defined(SP_DEBUGMODE) || 1 ) && 1
#   define _DEB_LOAD_SHADERS_FROM_FILES_
#endif

//!!!
//#define _DEB_USE_LIGHT_TEXBUFFER_


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


#ifdef _DEB_DEPTH_EXTENT_
video::ShaderResource* _debDepthExt_Out_ = 0;
video::ShaderResource* _debDepthExt_In_ = 0;
#endif

/*
 * Constant buffer structures
 */

using dim::uint2;
using dim::float2;
using dim::float3;
using dim::float4x4;

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
    uint2 NumTiles;
    float2 InvNumTiles;
    float2 InvResolution;
    float2 Pad0;
}
SP_PACK_STRUCT;

struct SLightGridFrameCB
{
    float4x4 InvViewProjection;
    float4x4 ViewMatrix;
    dim::plane3df NearPlane;
    dim::plane3df FarPlane;
    float3 ViewPosition;
    u32 NumLights;
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
    SRGlobalCounter_        (0),
    #ifdef _DEB_USE_LIGHT_TEXBUFFER_
    SRPointLights_          (0),
    #endif
    ShdClass_               (0),
    ShdClassInit_           (0),
    NumTiles_               (1),
    NumLights_              (0),
    MaxNumLights_           (1)
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
    MaxNumLights_ = MaxNumLights;

    switch (GlbRenderSys->getRendererType())
    {
        case RENDERER_OPENGL:
            return createTLITexture();
        case RENDERER_DIRECT3D11:
            return createShaderResources() && createComputeShaders(Resolution);
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
    
    GlbRenderSys->deleteShaderResource(SRGlobalCounter_);
    
    #ifdef _DEB_USE_LIGHT_TEXBUFFER_
    GlbRenderSys->deleteShaderResource(SRPointLights_);
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
        SRPointLights_->writeBuffer(&PointLights[0].X, sizeof(dim::float4)*NumLights);
        #else
        ShdClass_->getComputeShader()->setConstantBuffer(2, &PointLights[0].X);
        #endif
        NumLights_ = math::Min(NumLights, PointLights.size());
    }
}

void LightGrid::build(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, video::Texture* DepthTexture)
{
    if (Graph && ActiveCamera && DepthTexture)
    {
        if (ShdClass_)
            buildOnGPU(Graph, ActiveCamera, DepthTexture);
        else
            buildOnCPU(Graph, ActiveCamera, DepthTexture);
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

void LightGrid::setResolution(const dim::size2di &Resolution)
{
    if (useGPU())
    {
        /* Compute new number of tiles */
        NumTiles_ = LightGrid::computeNumTiles(Resolution);
        
        /* Setup main constant buffer and setup shader resources again */
        setupMainConstBuffer(
            ShdClass_->getComputeShader(), ShdClassInit_->getComputeShader(), Resolution
        );
        setupShaderResources();
    }
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

bool LightGrid::createShaderResources()
{
    /* Create new shader resource */
    TLIShaderResourceOut_   = GlbRenderSys->createShaderResource();
    TLIShaderResourceIn_    = GlbRenderSys->createShaderResource();

    LGShaderResourceOut_    = GlbRenderSys->createShaderResource();
    LGShaderResourceIn_     = GlbRenderSys->createShaderResource();
    
    SRGlobalCounter_        = GlbRenderSys->createShaderResource();

    if (!TLIShaderResourceOut_ || !TLIShaderResourceIn_ || !LGShaderResourceOut_ || !LGShaderResourceIn_ || !SRGlobalCounter_)
    {
        io::Log::error("Could not create shader resources for light-grid");
        return false;
    }
    
    /* Setup dynamic shader resources (they can change when resolution is resized) */
    if (!setupShaderResources())
        return false;
    
    /* Setup global counter shader resoruce */
    SRGlobalCounter_->setupBufferRW<u32>(1);
    
    #ifdef _DEB_USE_LIGHT_TEXBUFFER_
    
    /* Setup point light shader resource */
    SRPointLights_ = GlbRenderSys->createShaderResource();
    SRPointLights_->setupBuffer<dim::float4>(MaxNumLights_);
    
    #endif

    return true;
}

bool LightGrid::setupShaderResources()
{
    /* Setup light-grid shader resources */
    const u32 NumLightGridElements = NumTiles_.getArea();
    
    if ( !LGShaderResourceOut_->setupBuffer<u32>(NumLightGridElements) ||
         !LGShaderResourceIn_->setupBufferRW<u32>(NumLightGridElements) )
    {
        return false;
    }
    
    /* Setup tile-light-index list shader resources */
    //#define _DEB_USE_GROUP_SHARED_OPT_
    #ifdef _DEB_USE_GROUP_SHARED_OPT_
    const u32 MaxTileLinks = NumLightGridElements * (MaxNumLights_ + 1);
    
    if ( !TLIShaderResourceOut_->setupBuffer<u32>(MaxTileLinks) ||
         !TLIShaderResourceIn_->setupBufferRW<u32>(MaxTileLinks) )
    {
        return false;
    }
    #else
    struct SLightNode
    {
        u32 LightID;
        u32 Next;
    };
    
    const u32 MaxTileLinks = NumLightGridElements * MaxNumLights_;
    
    if ( !TLIShaderResourceOut_->setupBuffer<SLightNode>(MaxTileLinks) ||
         !TLIShaderResourceIn_->setupBufferRW<SLightNode>(MaxTileLinks, 0, SHADERBUFFERFLAG_COUNTER) )
    {
        return false;
    }
    #endif
    
    return true;
}

bool LightGrid::createComputeShaders(const dim::size2di &Resolution)
{
    /* Load shader source code */
    std::list<io::stringc> ShdBuf;
    
    Shader::addOption(ShdBuf, "MAX_LIGHTS " + io::stringc(MaxNumLights_));
    
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
            ShaderClass::loadShaderResourceFile(fsys, "../../sources/RenderSystem/AdvancedRenderer/spLightGridShader.hlsl", ShdBuf);
            #endif
        }
        break;
        
        default:
            io::Log::error("No light-grid compute shader support for this render system");
            return false;
    }
    
    /* Build compute shader */
    ShdClass_ = GlbRenderSys->createShaderClass();

    if (!ShdClass_)
        return false;

    Shader* CompShd = GlbRenderSys->createShader(
        ShdClass_, SHADER_COMPUTE, HLSL_COMPUTE_5_0, ShdBuf, "ComputeMain"
    );

    if (!ShdClass_->compile())
    {
        io::Log::error("Compiling light-grid compute shader failed");
        return false;
    }

    /* Build initialization compute shader */
    ShdClassInit_ = GlbRenderSys->createShaderClass();
    
    if (!ShdClassInit_)
        return false;

    Shader* CompShdInit = GlbRenderSys->createShader(
        ShdClassInit_, SHADER_COMPUTE, HLSL_COMPUTE_5_0, ShdBuf, "ComputeInitMain"
    );

    if (!ShdClassInit_->compile())
    {
        io::Log::error("Compiling light-grid initialization compute shader failed");
        return false;
    }
    
    /* Setup main constant buffer */
    setupMainConstBuffer(CompShd, CompShdInit, Resolution);
    
    /* Setup final compute shader */
    ShdClass_->addShaderResource(LGShaderResourceIn_);
    ShdClass_->addShaderResource(TLIShaderResourceIn_);
    ShdClass_->addShaderResource(SRGlobalCounter_);
    
    #ifdef _DEB_USE_LIGHT_TEXBUFFER_
    ShdClass_->addShaderResource(SRPointLights_);
    #endif

    ShdClassInit_->addShaderResource(LGShaderResourceIn_);
    ShdClassInit_->addShaderResource(TLIShaderResourceIn_);
    ShdClassInit_->addShaderResource(SRGlobalCounter_);
    
    #ifdef _DEB_DEPTH_EXTENT_
    if (!_debDepthExt_Out_)
    {
        _debDepthExt_Out_ = GlbRenderSys->createShaderResource();
        _debDepthExt_In_ = GlbRenderSys->createShaderResource();
        
        s32 w = static_cast<s32>(std::ceil(static_cast<f32>(Resolution.Width) / 32.0f));
        s32 h = static_cast<s32>(std::ceil(static_cast<f32>(Resolution.Height) / 32.0f));
        const u32 num = w*h;
        
        _debDepthExt_Out_->setupBuffer<dim::float2>(num);
        _debDepthExt_In_->setupBufferRW<dim::float2>(num);
        
        ShdClass_->addShaderResource(_debDepthExt_In_);
    }
    #endif
    
    return true;
}

void LightGrid::setupMainConstBuffer(
    Shader* CompShd, Shader* CompShdInit, const dim::size2di &Resolution)
{
    /* Initialize constant buffers */
    SLightGridMainCB BufferMain;
    {
        BufferMain.NumTiles.X       = static_cast<u32>(NumTiles_.Width);
        BufferMain.NumTiles.Y       = static_cast<u32>(NumTiles_.Height);
        BufferMain.InvNumTiles.X    = 1.0f / static_cast<f32>(NumTiles_.Width);
        BufferMain.InvNumTiles.Y    = 1.0f / static_cast<f32>(NumTiles_.Height);
        BufferMain.InvResolution.X  = 1.0f / static_cast<f32>(Resolution.Width);
        BufferMain.InvResolution.Y  = 1.0f / static_cast<f32>(Resolution.Height);
    }
    CompShd->setConstantBuffer(0, &BufferMain);
    CompShdInit->setConstantBuffer(0, &BufferMain);
}

void LightGrid::buildOnGPU(
    scene::SceneGraph* Graph, scene::Camera* Cam, video::Texture* DepthTexture)
{
    /* Update frame constant buffer */
    SLightGridFrameCB BufferFrame;
    {
        /* Setup view and inverse view-projection matrix */
        dim::matrix4f ViewMatrix = Cam->getTransformMatrix(true);
        
        BufferFrame.ViewPosition = ViewMatrix.getPosition();
        
        BufferFrame.ViewMatrix = ViewMatrix;
        BufferFrame.ViewMatrix.setInverse();
        
        ViewMatrix.setPosition(0.0f);
        ViewMatrix.setInverse();
        
        BufferFrame.InvViewProjection = Cam->getProjection().getMatrixLH();
        BufferFrame.InvViewProjection *= ViewMatrix;
        BufferFrame.InvViewProjection.setInverse();
        
        /* Setup clipping planes */
        BufferFrame.NearPlane   = Cam->getViewFrustum().getPlane(scene::VIEWFRUSTUM_NEAR);
        BufferFrame.FarPlane    = Cam->getViewFrustum().getPlane(scene::VIEWFRUSTUM_FAR);
        
        /* Setup light count */
        BufferFrame.NumLights = NumLights_;
    }
    ShdClass_->getComputeShader()->setConstantBuffer(1, &BufferFrame);

    /* Execute compute shaders */
    const dim::vector3d<u32> NumThreads(NumTiles_.Width, NumTiles_.Height, 1);
    
    GlbRenderSys->dispatch(ShdClassInit_, NumThreads);
    
    DepthTexture->bind(0);
    {
        GlbRenderSys->dispatch(ShdClass_, NumThreads);
    }
    DepthTexture->unbind(0);
    
    /* Copy input buffers to output buffers */
    TLIShaderResourceOut_->copyBuffer(TLIShaderResourceIn_);
    LGShaderResourceOut_->copyBuffer(LGShaderResourceIn_);
    
    #ifdef _DEB_DEPTH_EXTENT_
    _debDepthExt_Out_->copyBuffer(_debDepthExt_In_);
    #endif
}

void LightGrid::buildOnCPU(
    scene::SceneGraph* Graph, scene::Camera* Cam, video::Texture* DepthTexture)
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
