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
#ifdef SP_DEBUGMODE
#   define _DEB_LOAD_SHADERS_FROM_FILES_
#endif


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

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
    u32 Pad0[2];
    dim::size2df GridSize;
}
SP_PACK_STRUCT;

struct SLightGridFrameCB
{
    dim::matrix4f CameraMatrix;
    dim::matrix4f InvViewProjection;
    u32 LightCount;
    u32 Pad[3];
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

    switch (__spVideoDriver->getRendererType())
    {
        case video::RENDERER_OPENGL:
            return createTLITexture();
        case video::RENDERER_DIRECT3D11:
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
        __spVideoDriver->deleteTexture(TLITexture_);
    
    /* Delete shader resources */
    if (LGShaderResourceOut_)
        __spVideoDriver->deleteShaderResource(LGShaderResourceOut_);
    if (LGShaderResourceIn_)
        __spVideoDriver->deleteShaderResource(LGShaderResourceIn_);

    if (TLIShaderResourceOut_)
        __spVideoDriver->deleteShaderResource(TLIShaderResourceOut_);
    if (TLIShaderResourceIn_)
        __spVideoDriver->deleteShaderResource(TLIShaderResourceIn_);

    /* Delete shaders */
    if (ShdClass_)
    {
        __spVideoDriver->deleteShaderClass(ShdClass_);
        ShdClass_ = 0;
    }
    if (ShdClassInit_)
    {
        __spVideoDriver->deleteShaderClass(ShdClassInit_);
        ShdClassInit_ = 0;
    }
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

#if 0

void LightGrid::fillLightIntoGrid(scene::Light* Obj)
{

    //todo

}

#endif

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
    video::STextureCreationFlags CreationFlags;
    {
        CreationFlags.Size          = 128;//1;
        CreationFlags.Format        = video::PIXELFORMAT_GRAYALPHA;
        CreationFlags.HWFormat      = video::HWTEXFORMAT_INT32;
        CreationFlags.BufferType    = video::IMAGEBUFFER_UBYTE;//!!!IMAGEBUFFER_INT
        CreationFlags.Dimension     = video::TEXTURE_BUFFER;
    }
    TLITexture_ = __spVideoDriver->createTexture(CreationFlags);

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
    TLIShaderResourceOut_   = __spVideoDriver->createShaderResource();
    TLIShaderResourceIn_    = __spVideoDriver->createShaderResource();

    LGShaderResourceOut_    = __spVideoDriver->createShaderResource();
    LGShaderResourceIn_     = __spVideoDriver->createShaderResource();

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

    #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
    ShdBuf.push_back(
        #include "Resources/spLightGridShaderStr.glvert"
    );
    #else
    io::FileSystem fsys;
    ShaderClass::loadShaderResourceFile(fsys, "../../sources/RenderSystem/DeferredRenderer/spLightGridShader.hlsl", ShdBuf);
    #endif

    /* Build compute shader */
    ShdClass_ = __spVideoDriver->createShaderClass();

    video::Shader* CompShd = __spVideoDriver->createShader(
        ShdClass_, SHADER_COMPUTE, HLSL_COMPUTE_5_0, ShdBuf, "ComputeMain"
    );

    if (!ShdClass_->link())
    {
        io::Log::error("Compiling light-grid compute shader failed");
        return false;
    }

    /* Build initialization compute shader */
    ShdClassInit_ = __spVideoDriver->createShaderClass();

    video::Shader* CompShdInit = __spVideoDriver->createShader(
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
        /* Setup camera matrix */
        BufferFrame.CameraMatrix = Cam->getTransformMatrix(true);

        /* Setup inverse-view-projection matrix */
        dim::matrix4f ViewMatrix(BufferFrame.CameraMatrix);
        ViewMatrix.setPosition(0.0f);
        ViewMatrix.setInverse();

        BufferFrame.InvViewProjection = Cam->getProjection().getMatrixLH();
        BufferFrame.InvViewProjection *= ViewMatrix;
        BufferFrame.InvViewProjection.setInverse();

        /* Setup light count */
        BufferFrame.LightCount = LightCount_;
    }
    ShdClass_->getComputeShader()->setConstantBuffer(1, &BufferFrame);

    /* Execute compute shaders and copy input buffers to output buffers */
    const dim::vector3di ThreadCount(TileCount_.Width, TileCount_.Height, 1);

    __spVideoDriver->runComputeShader(ShdClassInit_, ThreadCount);
    __spVideoDriver->runComputeShader(ShdClass_, ThreadCount);

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
