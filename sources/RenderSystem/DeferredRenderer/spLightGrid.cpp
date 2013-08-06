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
    dim::size2d<u32> TileCount;
    dim::size2df GridSize;
}
SP_PACK_STRUCT;

struct SLightGridFrameCB
{
    dim::matrix4f InvViewProjection;
    dim::vector3df ViewPosition;
    u32 LightCount;
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

LightGrid::LightGrid() :
    TLITexture_             (0),
    LGShaderResourceOut_    (0),
    LGShaderResourceIn_     (0),
    TLIShaderResourceOut_   (0),
    TLIShaderResourceIn_    (0),
    ShdClass_               (0),
    ShdClassInit_           (0),
    TileCount_              (1),
    LightCount_             (0)
{
}
LightGrid::~LightGrid()
{
    deleteGrid();
}

bool LightGrid::createGrid(const dim::size2di &Resolution, const dim::size2di &TileCount)
{
    /* Validate parameters */
    if (TileCount.Width <= 0 || TileCount.Height <= 0)
    {
        io::Log::error("Count of tiles for Light-grid must be greater than zero");
        return false;
    }

    /* Delete old grid */
    deleteGrid();

    TileCount_  = TileCount;
    GridSize_   = getGridSize(Resolution, TileCount_);

    switch (GlbRenderSys->getRendererType())
    {
        case RENDERER_OPENGL:
            return createTLITexture();
        case RENDERER_DIRECT3D11:
            return createShaderResources() && createComputeShaders();
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

    /* Delete shaders */
    GlbRenderSys->deleteShaderClass(ShdClass_);
    ShdClass_ = 0;
    
    GlbRenderSys->deleteShaderClass(ShdClassInit_);
    ShdClassInit_ = 0;
}

void LightGrid::updateLights(const std::vector<dim::vector4df> &PointLights, u32 LightCount)
{
    if (ShdClass_ && ShdClass_->getComputeShader())
    {
        /* Setup point light data */
        ShdClass_->getComputeShader()->setConstantBuffer(2, &PointLights[0].X);
        LightCount_ = math::Min(LightCount, PointLights.size());
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
        CreationFlags.Dimension     = TEXTURE_BUFFER;
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

    if (!TLIShaderResourceOut_ || !TLIShaderResourceIn_ || !LGShaderResourceOut_ || !LGShaderResourceIn_)
    {
        io::Log::error("Could not create shader resources for light-grid");
        return false;
    }

    /* Setup light-grid shader resources */
    const u32 NumLightGridElements = TileCount_.getArea();

    LGShaderResourceOut_->setupBuffer<u32>(NumLightGridElements);
    LGShaderResourceIn_->setupBufferRW<u32>(NumLightGridElements);

    /* Setup tile-light-index list shader resources */
    struct SLightNode
    {
        u32 LightID;
        u32 Next;
    };
    
    const u32 MaxTileLinks = NumLightGridElements * 50;//!!!50 -> max number of lights must be variable

    TLIShaderResourceOut_->setupBuffer<SLightNode>(MaxTileLinks);
    TLIShaderResourceIn_->setupBufferRW<SLightNode>(MaxTileLinks, 0, SHADERBUFFERFLAG_COUNTER);

    return true;
}

bool LightGrid::createComputeShaders()
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
        BufferMain.TileCount = TileCount_.cast<u32>();
        BufferMain.GridSize = GridSize_.cast<f32>();
    }
    CompShd->setConstantBuffer(0, &BufferMain);
    CompShdInit->setConstantBuffer(0, &BufferMain);

    /* Setup final compute shader */
    ShdClass_->addShaderResource(LGShaderResourceIn_);
    ShdClass_->addShaderResource(TLIShaderResourceIn_);

    ShdClassInit_->addShaderResource(LGShaderResourceIn_);

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
        BufferFrame.LightCount = LightCount_;

        /* Setup clipping planes */
        BufferFrame.NearPlane   = Cam->getViewFrustum().getPlane(scene::VIEWFRUSTUM_NEAR);
        BufferFrame.FarPlane    = Cam->getViewFrustum().getPlane(scene::VIEWFRUSTUM_FAR);
    }
    ShdClass_->getComputeShader()->setConstantBuffer(1, &BufferFrame);

    /* Execute compute shaders and copy input buffers to output buffers */
    const dim::vector3di ThreadCount(TileCount_.Width, TileCount_.Height, 1);

    GlbRenderSys->runComputeShader(ShdClassInit_, ThreadCount);
    GlbRenderSys->runComputeShader(ShdClass_, ThreadCount);

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

dim::size2di LightGrid::getGridSize(const dim::size2di &Resolution, const dim::size2di &TileCount) const
{
    return dim::size2di(
        (Resolution.Width + TileCount.Width - 1) / TileCount.Width,
        (Resolution.Height + TileCount.Height - 1) / TileCount.Height
    );
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
