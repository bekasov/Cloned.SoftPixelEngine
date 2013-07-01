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


LightGrid::LightGrid() :
    TLITexture_             (0),
    TLIShaderResourceOut_   (0),
    TLIShaderResourceIn_    (0),
    //LGShaderResourceOut_    (0),
    //LGShaderResourceIn_     (0),
    ShdClass_               (0),
    GroupSize_              (1)
{
}
LightGrid::~LightGrid()
{
    deleteGrid();
}

bool LightGrid::createGrid(const dim::size2di &Resolution, const dim::size2di &GridSize)
{
    /* Delete old grid */
    deleteGrid();

    switch (__spVideoDriver->getRendererType())
    {
        case video::RENDERER_OPENGL:
            return createTLITexture();
        case video::RENDERER_DIRECT3D11:
            GroupSize_ = getGroupSize(Resolution);
            return createTLIShaderResources() && createTLIComputeShader();
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
}

void LightGrid::build()
{
    if (ShdClass_)
        buildOnGPU();
    else
        buildOnCPU();
}

void LightGrid::fillLightIntoGrid(scene::Light* Obj)
{

    //todo

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

bool LightGrid::createTLIShaderResources()
{
    /* Create new shader resource */
    TLIShaderResourceOut_   = __spVideoDriver->createShaderResource();
    TLIShaderResourceIn_    = __spVideoDriver->createShaderResource();

    if (!TLIShaderResourceOut_ || !TLIShaderResourceIn_)
    {
        io::Log::error("Could not create shader resources for light-grid");
        return false;
    }

    #if 1//!!!
    dim::point2di rawbuf[50];

    for (u32 i = 0; i < 50; ++i)
    {
        if (i > 25)
            rawbuf[i].X = 100;
        else
            rawbuf[i].X = i;
        rawbuf[i].Y = 0;
    }

    TLIShaderResourceOut_->setupBuffer<dim::point2di>(50, &rawbuf[0].X);
    TLIShaderResourceIn_->setupBufferRW<dim::point2di>(50, 0, SHADERBUFFERFLAG_COUNTER);
    #endif

    return true;
}

bool LightGrid::createTLIComputeShader()
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

    __spVideoDriver->createShader(ShdClass_, SHADER_COMPUTE, HLSL_COMPUTE_5_0, ShdBuf, "ComputeMain");

    if (!ShdClass_->link())
    {
        io::Log::error("Compiling light-grid shader failed");
        return false;
    }

    /* Setup final compute shader */
    ShdClass_->addShaderResource(TLIShaderResourceIn_);

    return true;
}

void LightGrid::buildOnGPU()
{
    /* Execute compute shader and copy input buffer to output buffer */
    __spVideoDriver->runComputeShader(ShdClass_, GroupSize_);
    TLIShaderResourceOut_->copyBuffer(TLIShaderResourceIn_);
}

void LightGrid::buildOnCPU()
{
    // todo ...
}

dim::vector3di LightGrid::getGroupSize(const dim::size2di &Resolution) const
{
    //!TODO! -> determine light grid size!!!
    return dim::vector3di(32, 32, 1);
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
