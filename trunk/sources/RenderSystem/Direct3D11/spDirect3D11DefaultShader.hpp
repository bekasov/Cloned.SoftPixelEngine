/*
 * Direct3D11 default shader header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_D3D11_DEFAULT_SHADER_H__
#define __SP_D3D11_DEFAULT_SHADER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionMatrix4.hpp"
#include "Base/spGeometryStructures.hpp"
#include "Base/spMaterialConfigTypes.hpp"
#include "RenderSystem/spRenderSystem.hpp"


namespace sp
{
namespace scene
{
    class Light;
    class Mesh;
}
namespace video
{


class Shader;
class ShaderClass;

//! Direct3D11 render system. This renderer supports Direct3D 11.0.
class D3D11DefaultShader
{
    
    private:
        
        friend class Direct3D11RenderSystem;
        
        /* === Structures === */
        
        struct SConstantBufferLights
        {
            struct SLight
            {
                SLight();
                ~SLight();
                
                /* Members */
                s32 Model;                              // Light model (Directionl, Point, Spot)
                s32 Enabled;                            // Enabled/ disabled
                f32 Unused1[2];
                dim::float4 Position;                   // Position for Point- and Spot light and Direction for Directional light
                dim::float4 Diffuse, Ambient, Specular; // Light colors
                dim::float4 SpotDir;                    // Spot light direction
                f32 Attn0, Attn1, Attn2;                // Attunation values
                f32 Unused2;
                f32 Theta, Phi, Falloff, Range;         // Spot light attributes
            };
            
            /* Members */
            SLight Lights[8];
        };
        
        struct SConstantBufferObject
        {
            struct SMaterial
            {
                SMaterial();
                ~SMaterial();
                
                /* Members */
                dim::float4 Diffuse, Ambient, Specular, Emission;   // Material colors
                s32 Shading;                                        // Shading (flat, gouraud, phong, perpixel)
                s32 LightingEnabled;                                // Global lighting enabled/ disabled
                s32 FogEnabled;                                     // Global fog enabled/ disabled
                f32 Shininess;                                      // Specular shininess
                s32 AlphaMethod;                                    // Alpha test function
                f32 AlphaReference;                                 // Alpha test reference value
                s32 Unused[2];
            };
            
            /* Members */
            dim::float4x4 WorldMatrix, ViewMatrix, ProjectionMatrix;
            SMaterial Material;
        };
        
        struct SConstantBufferSurface
        {
            SConstantBufferSurface();
            ~SConstantBufferSurface();
            
            struct STextureLayer
            {
                STextureLayer();
                ~STextureLayer();
                
                /* Members */
                dim::int3 MapGenType;   // Texture coordinate generation
                s32 TexEnvType;         // Texture environment
                dim::float4x4 Matrix;   // Texture coordiante transformation
            };
            
            /* Members */
            s32 NumTextureLayers;
            s32 Unused[3];
            STextureLayer TextureLayers[4];
        };
        
        struct SConstantBufferExtension
        {
            struct SClipPlane
            {
                SClipPlane();
                ~SClipPlane();
                
                /* Members */
                s32 Enabled;            // Enabled/ disabled
                s32 Unused[3];
                dim::plane3df Plane;    // Clipping plane
            };
            
            struct SFogStates
            {
                SFogStates();
                ~SFogStates();
                
                enum EConstBufferFogModes
                {
                    FOGMODE_STATIC_PALE = 0,
                    FOGMODE_STATIC_THICK,
                    FOGMODE_VOLUMETRIC,
                };
                
                /* Members */
                s32 Mode;           // Fog mode (Plane, Thick etc.)
                f32 Density;        // Density/ thikness
                f32 Near, Far;      // Near/ far planes
                dim::float4 Color;  // Fog color
            };
            
            SClipPlane Planes[8];   // Clipping planes;
            SFogStates Fog;         // Fog effect states
        };
        
        /* === Functions === */
        
        D3D11DefaultShader();
        ~D3D11DefaultShader();
        
        bool createShader();
        
        void bind();
        void unbind();
        
        void setupLight(
            u32 LightID, const scene::ELightModels LightType, bool isVolumetric,
            const dim::vector3df &Direction, f32 SpotInnerConeAngle, f32 SpotOuterConeAngle,
            f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic
        );
        void setupLightStatus(u32 LightID, bool Enable);
        void setupLightColor(
            u32 LightID, const video::color &Diffuse, const video::color &Ambient, const video::color &Specular
        );
        
        void setupFog(const EFogTypes Type, const EFogModes Mode, f32 Range, f32 Near, f32 Far);
        void setupFogColor(const video::color &Color);
        
        void setupClipPlane(u32 Index, const dim::plane3df &Plane, bool Enable);
        
        void updateObject(scene::Mesh* MeshObj);
        void updateTextureLayers(const TextureLayerListType &TextureLayers);
        void updateExtensions();
        
        /* === Members === */
        
        ShaderClass* ShaderClass_;
        
        Shader* VertexShader_;
        Shader* PixelShader_;
        
        bool Valid_;
        
        SConstantBufferLights ConstBufferLights_;
        SConstantBufferObject ConstBufferObject_;
        SConstantBufferSurface ConstBufferSurface_;
        SConstantBufferExtension ConstBufferExtension_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
