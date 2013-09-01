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
namespace tool
{


namespace LightmapGen
{

//! The lightmap generator class is a utility actually only used in a world editor.
class SP_EXPORT ShaderDispatcher
{
    
    public:
        
        ShaderDispatcher();
        ~ShaderDispatcher();
        
        /* === Functions === */
        
        bool createResources(bool EnableRadiosity, u32 LMGridSize);
        void deleteResources();
        
        bool setupLightSources(const std::vector<SLightmapLight> &LightList);
        bool setupLightmapGrid(const SLightmap &Lightmap);
        
        void dispatchDirectIllumination();
        void dispatchIndirectIllumination();
        
        /* === Inline functions === */
        
        
        
    private:
        
        /* === Functions === */
        
        bool createShaderResource(video::ShaderResource* &ShdResource);
        bool createAllShaderResources();
        
        bool createComputeShader(video::ShaderClass* &ShdClass);
        bool createAllComputeShaders();
        
        bool createTextures();
        
        dim::vector3df getRandomRadiosityRay() const;
        void generateRadiosityRays();
        
        /* === Members === */
        
        video::ShaderClass* DirectIlluminationSC_;
        video::ShaderClass* IndirectIlluminationSC_;
        
        video::ShaderResource* LightListSR_;
        video::ShaderResource* LightmapGridSR_;
        video::ShaderResource* TriangleListSR_;
        video::ShaderResource* TriangleIdListSR_;
        video::ShaderResource* NodeListSR_;
        
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
