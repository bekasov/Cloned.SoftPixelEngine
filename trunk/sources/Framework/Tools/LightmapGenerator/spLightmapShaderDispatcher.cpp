/*
 * Lightmap shader dispatcher file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/LightmapGenerator/spLightmapShaderDispatcher.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spMath.hpp"
#include "Base/spMathRandomizer.hpp"
#include "Base/spDimensionVector2D.hpp"
#include "Base/spDimensionVector4D.hpp"
#include "Base/spDimensionMatrix4.hpp"


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

static const u32 MAX_NUM_RADIOSITY_RAYS = 4096;

struct SLightmapMainCB
{
    float4x4 InvWorldMatrix;
    float4 AmbientColor;
    u32 NumLights;
    uint2 LightmapSize;
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
    float4 RadiosityDirections[MAX_NUM_RADIOSITY_RAYS];
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
    RadiosityEnabled_       (false  ),
    NumLights_              (0      )
{
}
ShaderDispatcher::~ShaderDispatcher()
{
    deleteResources();
}

bool ShaderDispatcher::createResources(bool EnableRadiosity, u32 LMGridSize)
{
    /* Initialization */
    io::Log::message("Create resources for lightmap generation shader dispatcher");
    io::Log::ScopedTab Unused;
    
    if (!GlbRenderSys->queryVideoSupport(video::QUERY_COMPUTE_SHADER))
    {
        io::Log::error("Compute shaders are not available");
        return false;
    }
    
    RadiosityEnabled_ = EnableRadiosity;
    
    /* Create all shader resources */
    if (!createAllShaderResources())
    {
        io::Log::error("Creating shader resources failed");
        return false;
    }
    
    /* Initialize lightmap grid */
    LMGridSize_ = LMGridSize;
    LightmapGridSR_->setupBuffer<SLightmapTexelLoc>(math::pow2(LMGridSize_));
    
    /* Create shader classes */
    if (!createAllComputeShaders())
        return false;
    
    /* Create lightmap textures */
    if (!createTextures())
        return false;
    
    /* Setup constant buffers */
    generateRadiosityRays();
    
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
    LightListSR_->setupBuffer<SLightSourceSR>(NumLights_, &BufferLightList[0]);
    
    return true;
}

bool ShaderDispatcher::setupLightmapGrid(const SLightmap &Lightmap)
{
    /* Copy texel location buffer to shader resource */
    if (LightmapGridSR_ && Lightmap.TexelLocBuffer)
    {
        LightmapGridSR_->writeBuffer(Lightmap.TexelLocBuffer);
        return true;
    }
    return false;
}

void ShaderDispatcher::dispatchDirectIllumination()
{
    //todo...
}

void ShaderDispatcher::dispatchIndirectIllumination()
{
    //InputLightmap_->bind(5);
    
    //todo...
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

bool ShaderDispatcher::createComputeShader(video::ShaderClass* &ShdClass)
{
    return ( ShdClass = GlbRenderSys->createShaderClass() ) != 0;
}

bool ShaderDispatcher::createAllComputeShaders()
{
    /* Load shader source code */
    std::list<io::stringc> ShdBuf;
    
    video::Shader::addOption(ShdBuf, "MAX_NUM_RADIOSITY_RAYS " + io::stringc(MAX_NUM_RADIOSITY_RAYS));
    
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
    
    video::Shader* CompShd = GlbRenderSys->createShader(
        DirectIlluminationSC_, video::SHADER_COMPUTE, video::HLSL_COMPUTE_5_0, ShdBuf, "ComputeDirectIllumination"
    );
    
    if (!DirectIlluminationSC_->link())
    {
        io::Log::error("Compiling direct illumination compute shader failed");
        return false;
    }
    
    /* Build direct illumination compute shader */
    if (RadiosityEnabled_)
    {
        if (!createComputeShader(IndirectIlluminationSC_))
            return false;
        
        CompShd = GlbRenderSys->createShader(
            IndirectIlluminationSC_, video::SHADER_COMPUTE, video::HLSL_COMPUTE_5_0, ShdBuf, "ComputeIndirectIllumination"
        );
        
        if (!IndirectIlluminationSC_->link())
        {
            io::Log::error("Compiling indirect illumination compute shader failed");
            return false;
        }
    }
    
    /* Append resources to shaders */
    video::ShaderClass* ShdClass[2] = { DirectIlluminationSC_, IndirectIlluminationSC_ };
    
    for (u32 i = 0; i < 2; ++i)
    {
        ShdClass[i]->addShaderResource(LightListSR_     );
        ShdClass[i]->addShaderResource(LightmapGridSR_  );
        ShdClass[i]->addShaderResource(TriangleListSR_  );
        ShdClass[i]->addShaderResource(TriangleIdListSR_);
        ShdClass[i]->addShaderResource(NodeListSR_      );
    }
    
    return true;
}

bool ShaderDispatcher::createTextures()
{
    /* Create input and output lightmap textures */
    video::STextureCreationFlags CreationFlags;
    {
        CreationFlags.Type              = video::TEXTURE_2D;
        CreationFlags.Size              = LMGridSize_;
        CreationFlags.Format            = video::PIXELFORMAT_RGBA;
        CreationFlags.HWFormat          = video::HWTEXFORMAT_FLOAT32;
        CreationFlags.Filter.HasMIPMaps = false;
    }
    InputLightmap_ = GlbRenderSys->createTexture(CreationFlags);
    {
        CreationFlags.Type              = video::TEXTURE_2D_RW;
    }
    OutputLightmap_ = GlbRenderSys->createTexture(CreationFlags);
    
    /* Append textures to  */
    DirectIlluminationSC_->addRWTexture(OutputLightmap_);
    IndirectIlluminationSC_->addRWTexture(OutputLightmap_);
    
    return true;
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

void ShaderDispatcher::generateRadiosityRays()
{
    if (IndirectIlluminationSC_)
    {
        SRadiosityRaysCB RadiosityRays;
        
        for (u32 i = 0; i < MAX_NUM_RADIOSITY_RAYS; ++i)
            RadiosityRays.RadiosityDirections[i] = getRandomRadiosityRay();
        
        IndirectIlluminationSC_->getComputeShader()->setConstantBuffer(2, &RadiosityRays);
    }
}

} // /namespace LightmapGen


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
