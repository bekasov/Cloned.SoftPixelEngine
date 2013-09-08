/*
 * Lightmap shader dispatcher header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_LIGHTMAP_SHADER_DISPATCHER_H__
#define __SP_LIGHTMAP_SHADER_DISPATCHER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "Base/spDimensionVector3D.hpp"
#include "Framework/Tools/LightmapGenerator/spLightmapBase.hpp"
#include "Framework/Tools/LightmapGenerator/spLightmapGeneratorStructs.hpp"


namespace sp
{
namespace video
{
    class ShaderClass;
    class ShaderResource;
    class Texture;
}
namespace scene
{
    class CollisionMesh;
}
namespace tool
{


namespace LightmapGen
{

/**
This 'shader dispatcher' is used for hardware accelerated lightmap generation.
\since Version 3.3
*/
class SP_EXPORT ShaderDispatcher
{
    
    public:
        
        ShaderDispatcher();
        ~ShaderDispatcher();
        
        /* === Macros === */
        
        static const u32 MAX_NUM_RADIOSITY_RAYS = 4096;
        
        /* === Functions === */
        
        bool createResources(
            const scene::CollisionMesh* SceneCollMdl, bool EnableRadiosity,
            u32 LMGridSize, u32 NumRadiosityRays = MAX_NUM_RADIOSITY_RAYS
        );
        void deleteResources();
        
        bool setupLightSources(const std::vector<SLightmapLight> &LightList);
        bool setupLightmapGrid(const SLightmap &Lightmap);
        
        void dispatchDirectIllumination(const dim::matrix4f &InvWorldMatrix, const video::color &AmbientColor);
        void dispatchIndirectIllumination(const dim::matrix4f &InvWorldMatrix);
        
    private:
        
        /* === Functions === */
        
        bool createShaderResource(video::ShaderResource* &ShdResource);
        bool createAllShaderResources();
        void appendShaderResources(video::ShaderClass* ShdClass);
        
        bool createComputeShader(video::ShaderClass* &ShdClass);
        bool createAllComputeShaders();
        
        bool createTextures();
        
        bool setupCollisionModel(const scene::CollisionMesh* SceneCollMdl);
        
        void setupMainConstBuffer(
            video::ShaderClass* ShdClass, const dim::matrix4f &InvWorldMatrix, const video::color &AmbientColor
        );
        
        dim::vector3df getRandomRadiosityRay() const;
        void generateRadiosityRays(u32 NumRays);
        
        /* === Inline functions === */
        
        inline dim::vector3d<u32> getNumWorkGroup() const
        {
            return dim::vector3d<u32>(LMGridSize_, LMGridSize_, 1);
        }
        
        /* === Members === */
        
        video::ShaderClass* DirectIlluminationSC_;
        video::ShaderClass* IndirectIlluminationSC_;
        
        video::ShaderResource* LightListSR_;
        video::ShaderResource* LightmapGridSR_;
        video::ShaderResource* NodeListSR_;
        video::ShaderResource* TriangleIdListSR_;
        video::ShaderResource* TriangleListSR_;
        
        video::Texture* InputLightmap_;
        video::Texture* OutputLightmap_;
        
        bool RadiosityEnabled_;
        
        u32 NumLights_;
        u32 LMGridSize_;
        
};

} // /namespace LightmapGen


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
