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

struct SRadiosityRaysCB
{
    float4 RadiosityDirections[MAX_NUM_RADIOSITY_RAYS];
}
SP_PACK_STRUCT;


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
    RadiosityEnabled_       (false  )
{
}
ShaderDispatcher::~ShaderDispatcher()
{
    deleteResources();
}

bool ShaderDispatcher::createResources(bool EnableRadiosity)
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
    
    /* Create shader classes */
    if (!createAllComputeShaders())
        return false;
    
    /* Setup constant buffers */
    generateRadiosityRays();
    
    return true;
}

void ShaderDispatcher::deleteResources()
{
    GlbRenderSys->deleteShaderClass(DirectIlluminationSC_   );
    GlbRenderSys->deleteShaderClass(IndirectIlluminationSC_ );
    
    GlbRenderSys->deleteShaderResource(LightListSR_     );
    GlbRenderSys->deleteShaderResource(LightmapGridSR_  );
    GlbRenderSys->deleteShaderResource(TriangleListSR_  );
    GlbRenderSys->deleteShaderResource(TriangleIdListSR_);
    GlbRenderSys->deleteShaderResource(NodeListSR_      );
    
    DirectIlluminationSC_   = 0;
    IndirectIlluminationSC_ = 0;
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
    
    return true;
}

dim::vector3df ShaderDispatcher::getRandomRadiosityRay() const
{
    /* Get transformed random angle */
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
