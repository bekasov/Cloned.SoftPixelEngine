/*
 * Lightmap shader dispatcher file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/LightmapGenerator/spLightmapShaderDispatcher.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "Framework/Tools/LightmapGenerator/spKDTreeBufferMapper.hpp"
#include "SceneGraph/Collision/spCollisionMesh.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spMath.hpp"
#include "Base/spMathRandomizer.hpp"
#include "Base/spDimensionVector2D.hpp"
#include "Base/spDimensionVector4D.hpp"
#include "Base/spDimensionMatrix4.hpp"
#include "Base/spBaseExceptions.hpp"
#include "Base/spTimer.hpp"


#define _DEB_LOAD_SHADERS_FROM_FILES_//!!!


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace tool
{


namespace LightmapGen
{


/*
 * Constant buffer structures
 */

using dim::uint2;
using dim::float2;
using dim::float3;
using dim::float4;
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

struct SLightmapMainCB
{
    float4x4 InvWorldMatrix;
    float4 AmbientColor;
    u32 NumLights;
    u32 LightmapSize;
    u32 NumTriangles;
}
SP_PACK_STRUCT;

struct SRadiositySetupCB
{
    u32 NumRadiosityRays;
    f32 RadiosityFactor;    // (1.0 / NumRadiosityRays) * Factor
}
SP_PACK_STRUCT;

struct SRadiosityRaysCB
{
    float4 RadiosityDirections[ShaderDispatcher::MAX_NUM_RADIOSITY_RAYS];
}
SP_PACK_STRUCT;

struct SLightSourceSR
{
    s32 Type;
    float4 Sphere;  // Position (XYZ) and inverse radius (W).
    float3 Color;
    float3 Direction;
    f32 SpotTheta;
    f32 SpotPhiMinusTheta;
}
SP_PACK_STRUCT;

#ifdef _MSC_VER
#   pragma pack(pop, packing)
#endif

#undef SP_PACK_STRUCT


/*
 * LightmapGenerator class
 */

ShaderDispatcher::ShaderDispatcher() :
    DirectIlluminationSC_   (0      ),
    IndirectIlluminationSC_ (0      ),
    LightListSR_            (0      ),
    LightmapGridSR_         (0      ),
    TriangleListSR_         (0      ),
    TriangleIdListSR_       (0      ),
    NodeListSR_             (0      ),
    InputLightmap_          (0      ),
    OutputLightmap_         (0      ),
    ActiveLightmap_         (0      ),
    RadiosityEnabled_       (false  ),
    UseTreeHierarchy_       (false  ),
    NumLights_              (0      ),
    LMGridSize_             (0      )
{
}
ShaderDispatcher::~ShaderDispatcher()
{
    deleteResources();
}

bool ShaderDispatcher::createResources(
    const scene::CollisionMesh* SceneCollMdl, bool EnableRadiosity,
    bool UseTreeHierarchy, u32 LMGridSize, u32 NumRadiosityRays)
{
    /* Initialization */
    io::Log::message("Create resources for lightmap generation shader dispatcher");
    io::Log::ScopedTab Unused;
    
    try
    {
        if (!SceneCollMdl)
            throw io::DefaultException("Invalid collision model for shader dispatcher");
        
        if (!GlbRenderSys->queryVideoSupport(video::QUERY_COMPUTE_SHADER))
            throw io::DefaultException("Compute shaders are not available");
        
        RadiosityEnabled_ = EnableRadiosity;
        UseTreeHierarchy_ = UseTreeHierarchy;
        
        /* Create all shader resources */
        if (!createAllShaderResources())
            throw io::DefaultException("Creating shader resources failed");
        
        /* Initialize lightmap grid */
        LMGridSize_ = LMGridSize;
        LightmapGridSR_->setupBuffer<SLightmapTexelLoc>(math::pow2(LMGridSize_));
        
        /* Create shader classes, lightmap textures and setup collision model */
        if ( !createAllComputeShaders() ||
             !createTextures() ||
             !setupCollisionModel(SceneCollMdl) )
        {
            throw io::DefaultException("");
        }
    }
    catch (const std::exception &e)
    {
        if (*e.what())
            io::Log::error(e.what());
        deleteResources();
        return false;
    }
    
    /* Setup constant buffers */
    generateRadiosityRays(NumRadiosityRays);
    
    return true;
}

void ShaderDispatcher::deleteResources()
{
    /* Delete all shader resources and textures */
    GlbRenderSys->deleteShaderClass(DirectIlluminationSC_   );
    GlbRenderSys->deleteShaderClass(IndirectIlluminationSC_ );
    
    GlbRenderSys->deleteShaderResource(LightListSR_     );
    GlbRenderSys->deleteShaderResource(LightmapGridSR_  );
    GlbRenderSys->deleteShaderResource(TriangleListSR_  );
    GlbRenderSys->deleteShaderResource(TriangleIdListSR_);
    GlbRenderSys->deleteShaderResource(NodeListSR_      );
    
    GlbRenderSys->deleteTexture(InputLightmap_  );
    GlbRenderSys->deleteTexture(OutputLightmap_ );
    
    DirectIlluminationSC_   = 0;
    IndirectIlluminationSC_ = 0;
}

bool ShaderDispatcher::setupLightSources(const std::vector<SLightmapLight> &LightList)
{
    if (!LightListSR_)
        return false;
    
    /* Initialize buffer light list and get iterators for easy access */
    std::vector<SLightSourceSR> BufferLightList(LightList.size());
    
    std::vector<SLightSourceSR>::iterator it = BufferLightList.begin();
    std::vector<SLightmapLight>::const_iterator itRef = LightList.begin();
    
    NumLights_ = LightList.size();
    
    /* Fill each light source entry */
    for (; it != BufferLightList.end(); ++it, ++itRef)
    {
        it->Type                = static_cast<s32>(itRef->Type);
        it->Sphere              = dim::vector4df(itRef->Matrix.getPosition(), itRef->Attn1);
        it->Color               = itRef->Color.getVector(true);
        it->Direction           = (itRef->Matrix * dim::vector3df(0, 0, 1));
        it->SpotTheta           = itRef->InnerConeAngle;
        it->SpotPhiMinusTheta   = itRef->OuterConeAngle - itRef->InnerConeAngle;
    }
    
    /* Copy to shader resource */
    return LightListSR_->setupBuffer<SLightSourceSR>(NumLights_, &BufferLightList[0]);
}

bool ShaderDispatcher::setupLightmapGrid(SLightmap* Lightmap)
{
    /* Copy texel location buffer to shader resource */
    if (Lightmap && LightmapGridSR_ && Lightmap->TexelLocBuffer && Lightmap->TexelBuffer)
    {
        ActiveLightmap_ = Lightmap;
        LightmapGridSR_->writeBuffer(Lightmap->TexelLocBuffer);
        return true;
    }
    return false;
}

void ShaderDispatcher::dispatchDirectIllumination(const dim::matrix4f &InvWorldMatrix, const video::color &AmbientColor)
{
    if (!DirectIlluminationSC_ || !ActiveLightmap_ || NumLights_ == 0)
        return;
    
    /* Setup constant buffers */
    setupMainConstBuffer(DirectIlluminationSC_, InvWorldMatrix, AmbientColor);
    
    /* Run compute shader to generate lightmap texels */
    if (GlbRenderSys->dispatch(DirectIlluminationSC_, getNumWorkGroup()))
        extractLightmapTexels();
}

void ShaderDispatcher::dispatchIndirectIllumination(const dim::matrix4f &InvWorldMatrix)
{
    if (!IndirectIlluminationSC_ || !InputLightmap_ || !OutputLightmap_)
        return;
    
    /* Setup constant buffers */
    setupMainConstBuffer(IndirectIlluminationSC_, InvWorldMatrix, 0);
    
    /* Copy input texture from previous output texture and bind it */
    //InputLightmap_->copyTexture(OutputLightmap_);
    InputLightmap_->bind(5);
    
    /* Run compute shader to generate lightmap texels */
    GlbRenderSys->dispatch(IndirectIlluminationSC_, getNumWorkGroup());
}


/*
 * ======= Private: =======
 */

bool ShaderDispatcher::createShaderResource(video::ShaderResource* &ShdResource)
{
    return ( ShdResource = GlbRenderSys->createShaderResource() ) != 0;
}

bool ShaderDispatcher::createAllShaderResources()
{
    return
        createShaderResource(LightListSR_       ) &&
        createShaderResource(LightmapGridSR_    ) &&
        createShaderResource(TriangleListSR_    ) &&
        createShaderResource(TriangleIdListSR_  ) &&
        createShaderResource(NodeListSR_        );
}

void ShaderDispatcher::appendShaderResources(video::ShaderClass* ShdClass)
{
    if (ShdClass)
    {
        ShdClass->addShaderResource(LightListSR_        );
        ShdClass->addShaderResource(LightmapGridSR_     );
        ShdClass->addShaderResource(TriangleListSR_     );
        ShdClass->addShaderResource(TriangleIdListSR_   );
        ShdClass->addShaderResource(NodeListSR_         );
    }
}

bool ShaderDispatcher::createComputeShader(video::ShaderClass* &ShdClass)
{
    return ( ShdClass = GlbRenderSys->createShaderClass() ) != 0;
}

bool ShaderDispatcher::createAllComputeShaders()
{
    /* Load shader source code */
    std::list<io::stringc> ShdBuf;
    
    video::Shader::addOption(ShdBuf, "MAX_NUM_RADIOSITY_RAYS " + io::stringc(ShaderDispatcher::MAX_NUM_RADIOSITY_RAYS));
    
    if (UseTreeHierarchy_)
        video::Shader::addOption(ShdBuf, "USE_TREE_HIERARCHY");
    
    switch (GlbRenderSys->getRendererType())
    {
        case video::RENDERER_DIRECT3D11:
        {
            #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
            ShdBuf.push_back(
                #include "Resources/spLightmapGenerationShaderStr.hlsl"
            );
            #else
            io::FileSystem fsys;
            video::ShaderClass::loadShaderResourceFile(fsys, "../../sources/Framework/Tools/LightmapGenerator/spLightmapGenerationShader.hlsl", ShdBuf);
            #endif
        }
        break;
        
        default:
            io::Log::error("No direct illumination compute shader support for this render system");
            return false;
    }
    
    /* Build direct illumination compute shader */
    if (!createComputeShader(DirectIlluminationSC_))
        return false;
    
    const u64 StartTime = io::Timer::millisecs();
    
    video::Shader* CompShd = GlbRenderSys->createShader(
        DirectIlluminationSC_, video::SHADER_COMPUTE, video::HLSL_COMPUTE_5_0,
        ShdBuf, "ComputeDirectIllumination", video::SHADERFLAG_NO_OPTIMIZATION//!!!
    );
    
    if (!DirectIlluminationSC_->compile())
    {
        io::Log::error("Compiling direct illumination compute shader failed");
        return false;
    }
    
    /* Print information about shader compilation time */
    io::Log::message(
        "Shader compilation time: " + io::stringc(io::Timer::millisecs() - StartTime) + " ms."
    );
    
    /* Build direct illumination compute shader */
    if (RadiosityEnabled_)
    {
        if (!createComputeShader(IndirectIlluminationSC_))
            return false;
        
        CompShd = GlbRenderSys->createShader(
            IndirectIlluminationSC_, video::SHADER_COMPUTE, video::HLSL_COMPUTE_5_0, ShdBuf, "ComputeIndirectIllumination"
        );
        
        if (!IndirectIlluminationSC_->compile())
        {
            io::Log::error("Compiling indirect illumination compute shader failed");
            return false;
        }
    }
    
    /* Append resources to shaders */
    appendShaderResources(DirectIlluminationSC_);
    appendShaderResources(IndirectIlluminationSC_);
    
    return true;
}

bool ShaderDispatcher::createTextures()
{
    /* Create input and output lightmap textures */
    video::STextureCreationFlags CreationFlags;
    {
        CreationFlags.Filename          = "Input Lightmap";
        CreationFlags.Type              = video::TEXTURE_2D;
        CreationFlags.Size              = LMGridSize_;
        CreationFlags.Format            = video::PIXELFORMAT_RGBA;
        CreationFlags.HWFormat          = video::HWTEXFORMAT_FLOAT32;
        CreationFlags.BufferType        = video::IMAGEBUFFER_FLOAT;
        CreationFlags.Filter.HasMIPMaps = false;
    }
    InputLightmap_ = GlbRenderSys->createTexture(CreationFlags);
    {
        CreationFlags.Filename          = "Output Lightmap";
        CreationFlags.Type              = video::TEXTURE_2D_RW;
    }
    OutputLightmap_ = GlbRenderSys->createTexture(CreationFlags);
    
    /* Append textures to  */
    if (DirectIlluminationSC_)
        DirectIlluminationSC_->addRWTexture(OutputLightmap_);
    if (IndirectIlluminationSC_)
        IndirectIlluminationSC_->addRWTexture(OutputLightmap_);
    
    return true;
}

bool ShaderDispatcher::setupCollisionModel(const scene::CollisionMesh* SceneCollMdl)
{
    const u64 StartTime = io::Timer::millisecs();
    
    /* Setup collision model shader resources with the kd-tree buffer mapper */
    bool Result = KDTreeBufferMapper::copyTreeHierarchy(
        SceneCollMdl,
        (UseTreeHierarchy_ ? NodeListSR_ : 0),
        (UseTreeHierarchy_ ? TriangleIdListSR_ : 0),
        TriangleListSR_
    );
    
    if (Result)
    {
        /* Print information about collision model creation time */
        io::Log::message(
            "Collision model creation time: " + io::stringc(io::Timer::millisecs() - StartTime) + " ms."
        );
    }
    
    return Result;
}

void ShaderDispatcher::setupMainConstBuffer(
    video::ShaderClass* ShdClass, const dim::matrix4f &InvWorldMatrix, const video::color &AmbientColor)
{
    /* Setup main constant buffer */
    video::Shader* CompShd = ShdClass->getComputeShader();
    
    SLightmapMainCB BufferMain;
    {
        BufferMain.InvWorldMatrix   = InvWorldMatrix;
        BufferMain.AmbientColor     = AmbientColor.getVector4(true);
        BufferMain.NumLights        = NumLights_;
        BufferMain.LightmapSize     = LMGridSize_;
        BufferMain.NumTriangles     = TriangleListSR_->getCount();
    }
    CompShd->setConstantBuffer(0, &BufferMain);
}

dim::vector3df ShaderDispatcher::getRandomRadiosityRay() const
{
    /*
    Get transformed random angle.
    We need more rays in the surface's normal direction.
    */
    const f32 Theta = 90.0f * std::pow(math::Randomizer::randFloat(-1.0f, 1.0f), 5);
    const f32 Phi = math::Randomizer::randFloat(360.0f);
    
    /* Convert spherical coordinate into cartesian coordinate */
    dim::vector3df Vec;
    math::convertToCartesianCoordiantes(Vec, Theta, Phi, 1.0f);
    return Vec;
}

void ShaderDispatcher::generateRadiosityRays(u32 NumRays)
{
    if (!IndirectIlluminationSC_)
        return;
    
    video::Shader* CompShd = IndirectIlluminationSC_->getComputeShader();
    
    /* Clamp number of rays */
    if (NumRays > ShaderDispatcher::MAX_NUM_RADIOSITY_RAYS)
    {
        NumRays = ShaderDispatcher::MAX_NUM_RADIOSITY_RAYS;
        io::Log::warning("Maximal number of radiosity rays is " + io::stringc(ShaderDispatcher::MAX_NUM_RADIOSITY_RAYS));
    }
    
    /* Setup radiosity configuration */
    SRadiositySetupCB RadiositySetup;
    {
        RadiositySetup.NumRadiosityRays = NumRays;
    }
    CompShd->setConstantBuffer(1, &RadiositySetup);
    
    /* Setup radiosity ray directions */
    SRadiosityRaysCB RadiosityRays;
    {
        for (u32 i = 0; i < ShaderDispatcher::MAX_NUM_RADIOSITY_RAYS; ++i)
            RadiosityRays.RadiosityDirections[i] = getRandomRadiosityRay();
    }
    CompShd->setConstantBuffer(2, &RadiosityRays);
}

bool ShaderDispatcher::extractLightmapTexels()
{
    /* Load texel buffer from GPU */
    if (!ActiveLightmap_ || !OutputLightmap_->shareImageBuffer())
        return false;
    
    /* Copy texel data into active lightmap texel buffer */
    video::ImageBuffer* ImgBuffer = OutputLightmap_->getImageBuffer();
    
    dim::point2di Pos;
    const dim::size2di Size = ImgBuffer->getSize();
    
    for (Pos.Y = 0; Pos.Y < Size.Height; ++Pos.Y)
    {
        for (Pos.X = 0; Pos.X < Size.Width; ++Pos.X)
        {
            /* Get current texel color and clamp to range [0, 255] */
            dim::vector4df TexelColor = ImgBuffer->getPixelVector(Pos);
            
            for (u32 i = 0; i < 4; ++i)
                math::clamp(TexelColor[i], 0.0f, 1.0f);
            
            ActiveLightmap_->getTexel(Pos.X, Pos.Y).Color = video::color(TexelColor);
        }
    }
    
    return true;
}

dim::vector3d<u32> ShaderDispatcher::getNumWorkGroup() const
{
    return UseTreeHierarchy_ ?
        dim::vector3d<u32>(LMGridSize_, LMGridSize_, 1) :
        dim::vector3d<u32>(LMGridSize_ / 8, LMGridSize_ / 8, 1);
}

} // /namespace LightmapGen


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
