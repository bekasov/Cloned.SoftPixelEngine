/*
 * VPL generator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_VPL_GENERATOR_H__
#define __SP_VPL_GENERATOR_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_ADVANCEDRENDERER)


#include "RenderSystem/AdvancedRenderer/spShadowMapper.hpp"


namespace sp
{

namespace scene
{
    class Light;
}

namespace video
{


class ShaderResource;
class ShaderClass;
class Shader;


/**
VPL (virtual point light) generator class. This is used for real-time global illumination.
\since Version 3.3
*/
class SP_EXPORT VPLGenerator
{
    
    public:
        
        VPLGenerator();
        ~VPLGenerator();
        
        /* === Functions === */
        
        bool generateResources(u32 NumLights, u32 NumVPLsPerLight = 128);
        void releaseResources();
        
        void generateVPLs(ShadowMapper &ShadowMapGen);
        
        /**
        Sets the new number of lights and optionally the number of VPLs per light.
        \param[in] NumLights Specifies the number of lights which have global-illumination enabled.
        This must be greater than zero.
        \param[in] NumVPLsPerLight Specifies the new number of VPLs fore ach light source.
        Set this to zero if this number is not to be changed. By default zero.
        \see setNumVPLsPerLight
        */
        void setNumLights(u32 NumLights, u32 NumVPLsPerLight = 0);
        
        /* === Inline functions === */
        
        /**
         * Returns the VPL list shader resource object. This is a 'structured shader buffer'.
         * \code
         * // For HLSL and GLSL:
         * struct SVPL
         * {
         *     float3 WorldPos;
         *     float3 Normal;
         *     float3 Color;
         * };
         * 
         * // HLSL Example:
         * StructuredBuffer<SVPL> VPLList : register(t1);
         * 
         * // GLSL Example:
         * layout(std430, binding = 1) buffer BufferVPLList
         * {
         *     SVPL VPLList[];
         * };
         * \endcode
         * 
         */
        inline ShaderResource* getVPLListShaderResource() const
        {
            return VPLListShaderResourceOut_;
        }
        
        /**
        Sets the new number of VPLs per light source.
        \param[in] NumVPLsPerLight Specifies the number of VPLs for each light source.
        This must be greater than zero. By default 128.
        */
        inline void setNumVPLsPerLight(u32 NumVPLsPerLight)
        {
            setNumLights(NumLights_, NumVPLsPerLight);
        }
        
        //! Returns the number of VPLs per light source. By default 128.
        inline u32 getNumVPLsPerLight() const
        {
            return NumVPLsPerLight_;
        }
        
        //! Returns the number of lights.
        inline u32 getNumLights() const
        {
            return NumLights_;
        }
        
    private:
        
        /* === Macros === */
        
        /**
        Thread group size (On NVIDIA GPUs a so called "Warp" consists of 32 threads and
        on AMD GPUs a so called "Wavefront" consists of 64 threads).
        */
        static const u32 THREAD_GROUP_SIZE = 64;
        
        /* === Functions === */
        
        bool createShaderResources();
        bool setupShaderResources();
        bool createComputeShader();
        
        dim::vector3d<u32> getNumThreads() const;
        
        /* === Members === */
        
        ShaderClass* ShdClass_;
        
        ShaderResource* VPLListShaderResourceOut_;
        ShaderResource* VPLListShaderResourceIn_;
        
        u32 NumLights_;
        u32 NumVPLsPerLight_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
