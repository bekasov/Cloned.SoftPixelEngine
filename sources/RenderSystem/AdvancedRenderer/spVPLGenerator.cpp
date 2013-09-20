/*
 * VPL generator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/AdvancedRenderer/spVPLGenerator.hpp"

#if defined(SP_COMPILE_WITH_ADVANCEDRENDERER)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "SceneGraph/spSceneLight.hpp"
#include "Base/spSharedObjects.hpp"


//!!!
#if 1
#   include "spAdvancedRenderer.hpp"
#endif


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


/*
 * Constant buffer structures
 */

using dim::uint3;
using dim::float3;

#if defined(_MSC_VER)
#   pragma pack(push, packing)
#   pragma pack(1)
#   define SP_PACK_STRUCT
#elif defined(__GNUC__)
#   define SP_PACK_STRUCT __attribute__((packed))
#else
#   define SP_PACK_STRUCT
#endif

struct SVPLGeneratorMainCB
{
    u32 NumVPLsPerLight;
    uint3 NumThreadGroups;
}
SP_PACK_STRUCT;

#ifdef _MSC_VER
#   pragma pack(pop, packing)
#endif

#undef SP_PACK_STRUCT


/*
 * VPLGenerator class
 */

VPLGenerator::VPLGenerator() :
    ShdClass_                   (0),
    VPLListShaderResourceOut_   (0),
    VPLListShaderResourceIn_    (0),
    NumLights_                  (0),
    NumVPLsPerLight_            (0)
{
}
VPLGenerator::~VPLGenerator()
{
    releaseResources();
}

bool VPLGenerator::generateResources(u32 NumLights, u32 NumVPLsPerLight)
{
    /* Setup configuration */
    NumLights_          = NumLights;
    NumVPLsPerLight_    = NumVPLsPerLight;
    
    if (NumLights_ == 0 || NumVPLsPerLight_ == 0)
    {
        io::Log::error("Invalid number of VPLs for VPL generator");
        return false;
    }
    
    /* Release previous resources */
    releaseResources();
    
    /* Create compute shader and shader resources */
    return createShaderResources() && createComputeShader();
}

void VPLGenerator::releaseResources()
{
    /* Delete shader resources */
    GlbRenderSys->deleteShaderResource(VPLListShaderResourceOut_);
    GlbRenderSys->deleteShaderResource(VPLListShaderResourceIn_);
    
    /* Delete shader */
    GlbRenderSys->deleteShaderClass(ShdClass_);
    ShdClass_ = 0;
}

void VPLGenerator::generateVPLs(ShadowMapper &ShadowMapGen)
{
    /* Validate parameters */
    if (!ShdClass_)
        return;
    
    if (!ShadowMapGen.useRSM())
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("VPLGenerator::generateVPLs", "Shadow mapper has not RSM (reflective shadow maps) enabled", io::LOG_UNIQUE);
        #endif
        return;
    }
    
    /* Run compute shader */
    ShadowMapGen.bind(0);
    {
        GlbRenderSys->dispatch(ShdClass_, getNumThreads());
    }
    ShadowMapGen.unbind(0);
    
    /* Copy input buffer to output buffer */
    VPLListShaderResourceOut_->copyBuffer(VPLListShaderResourceIn_);
}

void VPLGenerator::setNumLights(u32 NumLights, u32 NumVPLsPerLight)
{
    if ( NumLights > 0 && ( NumLights_ != NumLights || ( NumVPLsPerLight > 0 && NumVPLsPerLight_ != NumVPLsPerLight ) ) )
    {
        /* Set new configuration and update shader resources */
        NumLights_ = NumLights;
        
        if (NumVPLsPerLight > 0)
            NumVPLsPerLight_ = NumVPLsPerLight;
        
        setupShaderResources();
    }
}


/*
 * ======= Private: =======
 */

bool VPLGenerator::createShaderResources()
{
    /* Create new shader resource */
    VPLListShaderResourceOut_   = GlbRenderSys->createShaderResource();
    VPLListShaderResourceIn_    = GlbRenderSys->createShaderResource();
    
    if (!VPLListShaderResourceOut_ || !VPLListShaderResourceIn_)
    {
        io::Log::error("Could not create shader resources for VPL generator");
        return false;
    }
    
    /* Setup dynamic shader resources (they can change when number of lights is resized) */
    return setupShaderResources();
}

bool VPLGenerator::setupShaderResources()
{
    /* Setup VPL generator shader resources */
    const u32 NumVPLs = NumLights_ * NumVPLsPerLight_;
    
    struct SVPL
    {
        float3 Position;
        float3 Color;
    };
    
    if ( !VPLListShaderResourceOut_->setupBuffer<SVPL>(NumVPLs) ||
         !VPLListShaderResourceIn_->setupBufferRW<SVPL>(NumVPLs) )
    {
        return false;
    }
    
    /* Setup main constant buffer */
    SVPLGeneratorMainCB BufferMain;
    {
        BufferMain.NumVPLsPerLight  = NumVPLsPerLight_;
        BufferMain.NumThreadGroups  = getNumThreads();
    }
    ShdClass_->getComputeShader()->setConstantBuffer(0, &BufferMain);
    
    return true;
}

bool VPLGenerator::createComputeShader()
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
            ShaderClass::loadShaderResourceFile(fsys, _DEB_SHADER_PATH_ + "spVPLGeneratorShader.hlsl", ShdBuf);
            #endif
        }
        break;
        
        default:
            io::Log::error("No VPL generator compute shader support for this render system");
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
    
    /* Setup final compute shader */
    ShdClass_->addShaderResource(VPLListShaderResourceIn_);
    
    return true;
}

dim::vector3d<u32> VPLGenerator::getNumThreads() const
{
    const f32 Fraction = static_cast<f32>(NumVPLsPerLight_) / static_cast<f32>(VPLGenerator::THREAD_GROUP_SIZE);
    
    return dim::vector3d<u32>(
        static_cast<u32>(std::ceil(Fraction)),
        NumLights_,
        1
    );
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
