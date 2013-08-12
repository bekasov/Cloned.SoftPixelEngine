/*
 * Sparse octree voxelizer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spSparseOctreeVoxelizer.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "SceneGraph/spSceneGraph.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


//!!!
#if defined(SP_DEBUGMODE) || 1
#   define _DEB_LOAD_SHADERS_FROM_FILES_
#endif

/*
 * Constant buffer structures
 */

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

struct SSOVBufferMainCB
{
    float4x4 ProjectionMatrix;
    float4x4 DominantAxisMatrices[3];
    float4 BoundBoxMin;
    float4 BoundBoxMax;
    float4 VolumeSize;
}
SP_PACK_STRUCT;

#ifdef _MSC_VER
#   pragma pack(pop, packing)
#endif

#undef SP_PACK_STRUCT


/*
 * Sparse octree voxelizer class
 */

SparseOctreeVoxelizer::SparseOctreeVoxelizer() :
    ShdClass_(0)
{
    ViewCam_.setOrtho(true);
}
SparseOctreeVoxelizer::~SparseOctreeVoxelizer()
{
}

bool SparseOctreeVoxelizer::createShaders()
{
    deleteShaders();
    
    /* Load shader source code */
    const bool IsGL = (GlbRenderSys->getRendererType() == RENDERER_OPENGL);
    
    std::list<io::stringc> ShdBuf;
    
    switch (GlbRenderSys->getRendererType())
    {
        case RENDERER_DIRECT3D11:
        {
            #ifndef _DEB_LOAD_SHADERS_FROM_FILES_//!!!
            ShdBuf.push_back(
                #include "Resources/spSparseVoxelOctreeShaderStr.hlsl"
            );
            #else
            io::FileSystem fsys;
            ShaderClass::loadShaderResourceFile(fsys, "../../sources/RenderSystem/DeferredRenderer/spSparseVoxelOctreeShader.hlsl", ShdBuf);
            #endif
        }
        break;
        
        default:
            io::Log::error("No octree voxelizer shader support for this render system");
            return false;
    }
    
    /* Create shader */
    ShdClass_ = GlbRenderSys->createShaderClass();
    
    GlbRenderSys->createShader(
        ShdClass_, SHADER_VERTEX, (IsGL ? video::GLSL_VERSION_4_00_8 : HLSL_VERTEX_5_0), ShdBuf, "VertexMain"
    );
    GlbRenderSys->createShader(
        ShdClass_, SHADER_GEOMETRY, (IsGL ? video::GLSL_VERSION_4_00_8 : HLSL_GEOMETRY_5_0), ShdBuf, "GeometryMain"
    );
    GlbRenderSys->createShader(
        ShdClass_, SHADER_PIXEL, (IsGL ? video::GLSL_VERSION_4_00_8 : video::HLSL_PIXEL_5_0), ShdBuf, "PixelMain"
    );
    
    if (!ShdClass_->link())
    {
        io::Log::error("Compiling octree voxelizer shader failed");
        return false;
    }
    
    return true;
}

void SparseOctreeVoxelizer::deleteShaders()
{
    if (ShdClass_)
    {
        GlbRenderSys->deleteShaderClass(ShdClass_, true);
        ShdClass_ = 0;
    }
}

bool SparseOctreeVoxelizer::generateSparseOctree(Texture* VolumeTexture, scene::SceneGraph* Graph, const dim::aabbox3df &BoundVolume)
{
    /* Validate argument list */
    if (!VolumeTexture || !Graph)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("SparseOctreeVoxelizer::generateSparseOctree", "Null pointers passed");
        #endif
        return false;
    }
    if (!ShdClass_)
    {
        io::Log::error("Shader was not loaded successful before generating sparse voxel octree");
        return false;
    }
    if (VolumeTexture->getType() != TEXTURE_3D_RW)
    {
        io::Log::error("Cannot generate sparse voxel octree when texture is not an 'RW 3D texture' (TEXTURE_3D_RW)");
        return false;
    }
    if (BoundVolume.getVolume() <= math::ROUNDING_ERROR)
    {
        io::Log::error("Cannot generate sparse voxel octree when bounding volume is nearly zero");
        return false;
    }
    
    const dim::vector3di VolumeSize(VolumeTexture->getImageBuffer()->getSizeVector());
    
    /* Setup view camera settings */
    ViewCam_.setViewport(dim::rect2di(0, 0, VolumeSize.X, VolumeSize.Y));
    
    /* Setup constant buffers */
    SSOVBufferMainCB BufferMain;
    {
        BufferMain.ProjectionMatrix = ViewCam_.getProjectionMatrix();
        
        /* Setup projection matrices for dominant axes (X+/-, Y+/-, Z+/-) */
        (BufferMain.DominantAxisMatrices[0])[ 0] = 0.0f;
        (BufferMain.DominantAxisMatrices[0])[ 8] = 1.0f;
        (BufferMain.DominantAxisMatrices[0])[10] = 0.0f;
        
        (BufferMain.DominantAxisMatrices[1])[ 5] = 0.0f;
        (BufferMain.DominantAxisMatrices[1])[ 9] = 1.0f;
        (BufferMain.DominantAxisMatrices[1])[10] = 0.0f;
        
        (BufferMain.DominantAxisMatrices[2])[10] = 0.0f;
        
        BufferMain.BoundBoxMin = BoundVolume.Min;
        BufferMain.BoundBoxMax = BoundVolume.Max;
        
        BufferMain.VolumeSize = VolumeSize.cast<f32>();
    }
    ShdClass_->getGeometryShader()->setConstantBuffer("BufferMain", &BufferMain);
    
    /* Render scene with diffuse map only and setup render states */
    const s32 PrevTexLayerMask = GlbRenderSys->getTexLayerVisibleMask();
    GlbRenderSys->setTexLayerVisibleMask(TEXLAYERFLAG_DIFFUSE);
    
    GlbRenderSys->setColorMask(false, false, false, false);
    GlbRenderSys->setDepthMask(false);

    /* Setup shader class for voxelization pipeline */
    GlbRenderSys->setGlobalShaderClass(ShdClass_);
    
    /* Render scene */
    GlbRenderSys->setRenderTarget(0, ShdClass_);
    {
        Graph->renderScene(&ViewCam_);
    }
    GlbRenderSys->setRenderTarget(0, 0);
    
    /* Restore previous render states */
    GlbRenderSys->setColorMask(true, true, true, true);
    GlbRenderSys->setDepthMask(true);

    GlbRenderSys->setGlobalShaderClass(0);
    GlbRenderSys->setTexLayerVisibleMask(PrevTexLayerMask);
    
    return true;
}


/*
 * ======= Private: =======
 */

bool SparseOctreeVoxelizer::loadShader()
{
    
    //todo ...
    
    return false;
}


} // /namespace video

} // /namespace sp



// ================================================================================