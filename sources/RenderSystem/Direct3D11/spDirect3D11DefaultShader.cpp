/*
 * Direct3D11 default shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11DefaultShader.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "Base/spInternalDeclarations.hpp"
#include "SceneGraph/spSceneLight.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spTextureLayerStandard.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace video
{


D3D11DefaultShader::D3D11DefaultShader() :
    ShaderClass_    (0      ),
    VertexShader_   (0      ),
    PixelShader_    (0      ),
    Valid_          (false  )
{
}
D3D11DefaultShader::~D3D11DefaultShader()
{
}

bool D3D11DefaultShader::createShader()
{
    /* Create default shaders */
    ShaderClass_ = __spVideoDriver->createShaderClass();
    
    if (__spVideoDriver->queryVideoSupport(QUERY_VERTEX_SHADER_4_0))
    {
        std::list<io::stringc> ShaderBuffer;
        ShaderBuffer.push_back(
            #include "Resources/spDefaultShaderStr.hlsl"
        );
        
        VertexShader_ = __spVideoDriver->createShader(
            ShaderClass_, SHADER_VERTEX, HLSL_VERTEX_4_0, ShaderBuffer, "VertexMain"
        );
        PixelShader_ = __spVideoDriver->createShader(
            ShaderClass_, SHADER_PIXEL, HLSL_PIXEL_4_0, ShaderBuffer, "PixelMain"
        );
    }
    else
    {
        io::Log::error("Could not create default shader because shader model is less than 4.0");
        return false;
    }
    
    Valid_ = ShaderClass_->link();
    
    return Valid_;
}

void D3D11DefaultShader::setupLight(
    u32 LightID, const scene::ELightModels LightType, bool isVolumetric,
    const dim::vector3df &Direction, f32 SpotInnerConeAngle, f32 SpotOuterConeAngle,
    f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic)
{
    if (LightID >= MAX_COUNT_OF_LIGHTS)
        return;
    
    SConstantBufferLights::SLight* Light = &(ConstBufferLights_.Lights[LightID]);
    
    switch (LightType)
    {
        case scene::LIGHT_DIRECTIONAL:
            Light->Model    = 0;
            Light->Position = ((__spVideoDriver->getViewMatrix() * __spVideoDriver->getWorldMatrix()).getRotationMatrix() * (-Direction)).normalize();
            break;
        case scene::LIGHT_POINT:
            Light->Model    = 1;
            Light->Position = (__spVideoDriver->getViewMatrix() * __spVideoDriver->getWorldMatrix()).getPosition();
            break;
        case scene::LIGHT_SPOT:
            Light->Model = 2;
            break;
    }
    
    /* Spot light attributes */
    Light->Theta    = SpotInnerConeAngle * 2.0f * math::RAD;
    Light->Phi      = SpotOuterConeAngle * 2.0f * math::RAD;
    
    /* Volumetric light attenuations */
    if (isVolumetric)
    {
        Light->Attn0 = AttenuationConstant;
        Light->Attn1 = AttenuationLinear;
        Light->Attn2 = AttenuationQuadratic;
    }
    else
    {
        Light->Attn0 = 1.0f;
        Light->Attn1 = 0.0f;
        Light->Attn2 = 0.0f;
    }
}

void D3D11DefaultShader::setupLightStatus(u32 LightID, bool Enable)
{
    if (LightID < MAX_COUNT_OF_LIGHTS)
        ConstBufferLights_.Lights[LightID].Enabled = Enable;
}

void D3D11DefaultShader::setupLightColor(
    u32 LightID, const video::color &Diffuse, const video::color &Ambient, const video::color &Specular)
{
    if (LightID < MAX_COUNT_OF_LIGHTS)
    {
        SConstantBufferLights::SLight* Light = &(ConstBufferLights_.Lights[LightID]);
        
        Diffuse.getFloatArray(&Light->Diffuse.X);
        Ambient.getFloatArray(&Light->Ambient.X);
        Specular.getFloatArray(&Light->Specular.X);
    }
}

void D3D11DefaultShader::setupFog(
    const EFogTypes Type, const EFogModes Mode, f32 Range, f32 Near, f32 Far)
{
    SConstantBufferExtension::SFogStates* FogStates = &(ConstBufferExtension_.Fog);
    
    /* Setup fog type */
    switch (Type)
    {
        case FOG_STATIC:
        {
            /* Set fog type */
            switch (Mode)
            {
                case FOG_PALE:
                    FogStates->Mode = SConstantBufferExtension::SFogStates::FOGMODE_STATIC_PALE;
                case FOG_THICK:
                    FogStates->Mode = SConstantBufferExtension::SFogStates::FOGMODE_STATIC_THICK;
            }
            
            /* Range settings */
            FogStates->Density  = Range;
            FogStates->Near     = Near;
            FogStates->Far      = Far;
        }
        break;
        
        case FOG_VOLUMETRIC:
        {
            FogStates->Mode     = SConstantBufferExtension::SFogStates::FOGMODE_VOLUMETRIC;
            FogStates->Density  = Range;
            FogStates->Near     = 0.0f;
            FogStates->Far      = 1.0f;
        }
        break;
        
        default:
            break;
    }
    
    /* Setup fog mode */
    switch (Mode)
    {
        case FOG_PALE:
            FogStates->Mode = SConstantBufferExtension::SFogStates::FOGMODE_STATIC_PALE;
            break;
        case FOG_THICK:
            FogStates->Mode = SConstantBufferExtension::SFogStates::FOGMODE_STATIC_THICK;
            break;
    }
    
    updateExtensions();
}

void D3D11DefaultShader::setupFogColor(const video::color &Color)
{
    ConstBufferExtension_.Fog.Color = math::Convert(Color);
    updateExtensions();
}

void D3D11DefaultShader::setupClipPlane(u32 Index, const dim::plane3df &Plane, bool Enable)
{
    if (Index < 8)
    {
        SConstantBufferExtension::SClipPlane* ClipPlane = &(ConstBufferExtension_.Planes[Index]);
        
        ClipPlane->Enabled  = Enable;
        ClipPlane->Plane    = Plane;
        
        updateExtensions();
    }
}

void D3D11DefaultShader::updateObject(scene::Mesh* MeshObj)
{
    const MaterialStates* Material = MeshObj->getMaterial();
    
    /* Update object transformation */
    ConstBufferObject_.WorldMatrix      = __spVideoDriver->getWorldMatrix();
    ConstBufferObject_.ViewMatrix       = __spVideoDriver->getViewMatrix();
    ConstBufferObject_.ProjectionMatrix = __spVideoDriver->getProjectionMatrix();
    
    /* Update material colors */
    Material->getDiffuseColor().getFloatArray(&ConstBufferObject_.Material.Diffuse.X);
    Material->getAmbientColor().getFloatArray(&ConstBufferObject_.Material.Ambient.X);
    
    /* Update material attributes */
    ConstBufferObject_.Material.Shading         = Material->getShading();
    ConstBufferObject_.Material.LightingEnabled = Material->getLighting() && __isLighting;
    ConstBufferObject_.Material.FogEnabled      = Material->getFog() && __isFog;
    ConstBufferObject_.Material.Shininess       = Material->getShininess();
    ConstBufferObject_.Material.AlphaMethod     = Material->getAlphaMethod();
    ConstBufferObject_.Material.AlphaReference  = Material->getAlphaReference();
    
    if (Valid_)
    {
        VertexShader_->setConstantBuffer(0, &ConstBufferLights_);
        VertexShader_->setConstantBuffer(1, &ConstBufferObject_);
        
        PixelShader_->setConstantBuffer(0, &ConstBufferLights_);
        PixelShader_->setConstantBuffer(1, &ConstBufferObject_);
    }
}

void D3D11DefaultShader::updateTextureLayers(const TextureLayerListType &TextureLayers)
{
    /* Update texture layers */
    ConstBufferSurface_.NumTextureLayers = TextureLayers.size();
    
    u32 i = 0;
    foreach (const TextureLayer* TexLayer, TextureLayers)
    {
        if (TexLayer->getType() == TEXLAYER_STANDARD)
        {
            const TextureLayerStandard* TexLayerDef = static_cast<const TextureLayerStandard*>(TexLayer);
            
            ConstBufferSurface_.TextureLayers[i].MapGenType = TexLayerDef->getMappingGen();
            ConstBufferSurface_.TextureLayers[i].TexEnvType = TexLayerDef->getTextureEnv();
            ConstBufferSurface_.TextureLayers[i].Matrix = TexLayerDef->getMatrix();
        }
        else
        {
            ConstBufferSurface_.TextureLayers[i].MapGenType = video::MAPGEN_DISABLE;
            ConstBufferSurface_.TextureLayers[i].TexEnvType = video::TEXENV_MODULATE;
            ConstBufferSurface_.TextureLayers[i].Matrix.reset();
        }
        
        ++i;
    }
    
    if (Valid_)
    {
        VertexShader_->setConstantBuffer(2, &ConstBufferSurface_);
        PixelShader_->setConstantBuffer(2, &ConstBufferSurface_);
    }
}

void D3D11DefaultShader::updateExtensions()
{
    if (Valid_)
    {
        VertexShader_->setConstantBuffer(3, &ConstBufferExtension_);
        PixelShader_->setConstantBuffer(3, &ConstBufferExtension_);
    }
}


/*
 * SConstantBufferLights::SLight structure
 */

D3D11DefaultShader::SConstantBufferLights::SLight::SLight() :
    Model   (0      ),
    Enabled (0      ),
    Attn0   (1.0f   ),
    Attn1   (0.0f   ),
    Attn2   (0.0f   ),
    Theta   (0.0f   ),
    Phi     (0.0f   ),
    Falloff (1.0f   ),
    Range   (1.0f   )
{
}
D3D11DefaultShader::SConstantBufferLights::SLight::~SLight()
{
}


/*
 * SConstantBufferObject::SMaterial structure
 */

D3D11DefaultShader::SConstantBufferObject::SMaterial::SMaterial() :
    Shading         (0      ),
    LightingEnabled (0      ),
    FogEnabled      (0      ),
    Shininess       (0.0f   ),
    AlphaMethod     (0      ),
    AlphaReference  (0.0f   )
{
}
D3D11DefaultShader::SConstantBufferObject::SMaterial::~SMaterial()
{
}


/*
 * SConstantBufferSurface structure
 */

D3D11DefaultShader::SConstantBufferSurface::SConstantBufferSurface() :
    NumTextureLayers(0)
{
}
D3D11DefaultShader::SConstantBufferSurface::~SConstantBufferSurface()
{
}


/*
 * SConstantBufferSurface::STextureLayer structure
 */

D3D11DefaultShader::SConstantBufferSurface::STextureLayer::STextureLayer() :
    TexEnvType(0)
{
}
D3D11DefaultShader::SConstantBufferSurface::STextureLayer::~STextureLayer()
{
}


/*
 * SConstantBufferExtension::SClipPlane structure
 */

D3D11DefaultShader::SConstantBufferExtension::SClipPlane::SClipPlane() :
    Enabled(0)
{
}
D3D11DefaultShader::SConstantBufferExtension::SClipPlane::~SClipPlane()
{
}


/*
 * SConstantBufferExtension::SFogStates structure
 */

D3D11DefaultShader::SConstantBufferExtension::SFogStates::SFogStates() :
    Mode    (0      ),
    Density (0.0f   ),
    Near    (0.0f   ),
    Far     (0.0f   )
{
}
D3D11DefaultShader::SConstantBufferExtension::SFogStates::~SFogStates()
{
}

} // /namespace video

} // /namespace sp


#endif



// ================================================================================
