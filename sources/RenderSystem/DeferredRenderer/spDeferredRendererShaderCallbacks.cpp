/*
 * Deferred renderer shader classbacks file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spDeferredRendererShaderCallbacks.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/DeferredRenderer/spDeferredRendererFlags.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Base/spSharedObjects.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace video
{


s32 gDRFlags = 0;


/*
 * Constant buffer structures
 */

#if defined(_MSC_VER)
#   pragma pack(push, packing)
#   pragma pack(1)
#   define SP_PACK_STRUCT
#elif defined(__GNUC__)
#   define SP_PACK_STRUCT __attribute__((packed))
#else
#   define SP_PACK_STRUCT
#endif

struct SGBufferMainCB
{
    dim::matrix4f WVPMatrix;
    dim::matrix4f WorldMatrix;
    dim::vector4df ViewPosition;
}
SP_PACK_STRUCT;

struct SGBufferReliefCB
{
    f32 SpecularFactor;
    f32 HeightMapScale;
    f32 ParallaxViewRange;
    f32 Pad0;
    s32 EnablePOM;
    s32 MinSamplesPOM;
    s32 MaxSamplesPOM;
    s32 Pad1;
}
SP_PACK_STRUCT;

struct SDeferredMainCB
{
    dim::matrix4f ProjectionMatrix;
    dim::matrix4f InvViewProjection;
    dim::matrix4f WorldMatrix;
    dim::vector4df ViewPosition;
}
SP_PACK_STRUCT;

#ifdef _MSC_VER
#   pragma pack(pop, packing)
#endif

#undef SP_PACK_STRUCT


/*
 * Shader callbacks
 */

SHADER_OBJECT_CALLBACK(DfRnGBufferObjectShaderCallback)
{
    /* Get vertex- and pixel shaders */
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    /* Setup transformations */
    const dim::vector3df ViewPosition(__spSceneManager->getActiveCamera()->getPosition(true));
    
    dim::matrix4f WVPMatrix(__spVideoDriver->getProjectionMatrix());
    WVPMatrix *= __spVideoDriver->getViewMatrix();
    WVPMatrix *= __spVideoDriver->getWorldMatrix();
    
    /* Setup shader constants */
    VertShd->setConstant("WorldViewProjectionMatrix", WVPMatrix);
    VertShd->setConstant("WorldMatrix", __spVideoDriver->getWorldMatrix());
    VertShd->setConstant("ViewPosition", ViewPosition);
    
    FragShd->setConstant("ViewPosition", ViewPosition);
}

SHADER_OBJECT_CALLBACK(DfRnGBufferObjectShaderCallbackCB)
{
    /* Get vertex- and pixel shaders */
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    /* Setup transformation */
    SGBufferMainCB BufferMain;
    {
        BufferMain.WVPMatrix = __spVideoDriver->getProjectionMatrix();
        BufferMain.WVPMatrix *= __spVideoDriver->getViewMatrix();
        BufferMain.WVPMatrix *= __spVideoDriver->getWorldMatrix();
        
        BufferMain.WorldMatrix = __spVideoDriver->getWorldMatrix();
        
        BufferMain.ViewPosition = __spSceneManager->getActiveCamera()->getPosition(true);
    }
    VertShd->setConstantBuffer(0, &BufferMain);
    FragShd->setConstantBuffer(0, &BufferMain);
}

SHADER_SURFACE_CALLBACK(DfRnGBufferSurfaceShaderCallback)
{
    /* Get vertex- and pixel shaders */
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    /* Setup texture layers */
    u32 TexCount = TexLayers.size();
    
    if (gDRFlags & DEFERREDFLAG_USE_TEXTURE_MATRIX)
    {
        /*if (TexCount > 0)
            VertShd->setConstant("TextureMatrix", TexLayers.front().Matrix);
        else*/
            VertShd->setConstant("TextureMatrix", dim::matrix4f::IDENTITY);
    }
    
    if ((gDRFlags & DEFERREDFLAG_HAS_SPECULAR_MAP) == 0)
        ++TexCount;
    
    if (gDRFlags & DEFERREDFLAG_HAS_LIGHT_MAP)
        FragShd->setConstant("EnableLightMap", TexCount >= ((gDRFlags & DEFERREDFLAG_PARALLAX_MAPPING) != 0 ? 5u : 4u));
    
    if (gDRFlags & DEFERREDFLAG_PARALLAX_MAPPING)
    {
        FragShd->setConstant("EnablePOM", TexCount >= 4);//!!!
        FragShd->setConstant("MinSamplesPOM", 0);//!!!
        FragShd->setConstant("MaxSamplesPOM", 50);//!!!
        FragShd->setConstant("HeightMapScale", 0.015f);//!!!
        FragShd->setConstant("ParallaxViewRange", 2.0f);//!!!
    }
    
    FragShd->setConstant("SpecularFactor", 1.0f);//!!!
}

SHADER_SURFACE_CALLBACK(DfRnGBufferSurfaceShaderCallbackCB)
{
    /* Get vertex- and pixel shaders */
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    /* Setup texture layers */
    u32 TexCount = TexLayers.size();
    
    if ((gDRFlags & DEFERREDFLAG_HAS_SPECULAR_MAP) == 0)
        ++TexCount;
    
    /* Setup relief-mapping constant buffer */
    SGBufferReliefCB BufferRelief;
    {
        BufferRelief.SpecularFactor = 1.0f;

        if (gDRFlags & DEFERREDFLAG_PARALLAX_MAPPING)
        {
            BufferRelief.HeightMapScale     = 0.015f;
            BufferRelief.ParallaxViewRange  = 2.0f;
            BufferRelief.EnablePOM          = (TexCount >= 4 ? 1 : 0);
            BufferRelief.MinSamplesPOM      = 0;
            BufferRelief.MaxSamplesPOM      = 50;
        }
    }
    FragShd->setConstantBuffer(1, &BufferRelief);
}

SHADER_OBJECT_CALLBACK(DfRnDeferredShaderCallback)
{
    /* Get vertex- and pixel shaders */
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    /* Setup projection and inverse-view-projection matrices */
    scene::Camera* Cam = __spSceneManager->getActiveCamera();
    
    dim::matrix4f ViewMatrix(Cam->getTransformMatrix(true));
    const dim::vector3df ViewPosition(ViewMatrix.getPosition());
    ViewMatrix.setPosition(0.0f);
    ViewMatrix.setInverse();
    
    dim::matrix4f InvViewProj(Cam->getProjection().getMatrixLH());
    InvViewProj *= ViewMatrix;
    InvViewProj.setInverse();
    
    VertShd->setConstant("ProjectionMatrix", __spVideoDriver->getProjectionMatrix());
    VertShd->setConstant("InvViewProjection", InvViewProj);
    
    FragShd->setConstant("ViewPosition", ViewPosition);
}

SHADER_OBJECT_CALLBACK(DfRnDeferredShaderCallbackCB)
{
    /* Get vertex- and pixel shaders */
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    SDeferredMainCB BufferMain;
    {
        /* Setup view- matrix and position */
        scene::Camera* Cam = __spSceneManager->getActiveCamera();
        
        dim::matrix4f ViewMatrix(Cam->getTransformMatrix(true));
        
        BufferMain.ViewPosition = ViewMatrix.getPosition();
        
        ViewMatrix.setPosition(0.0f);
        ViewMatrix.setInverse();
        
        /* Setup projection and inverse-view-projection matrices */
        BufferMain.ProjectionMatrix = __spVideoDriver->getProjectionMatrix();
        
        BufferMain.InvViewProjection = Cam->getProjection().getMatrixLH();
        BufferMain.InvViewProjection *= ViewMatrix;
        BufferMain.InvViewProjection.setInverse();
        
        BufferMain.WorldMatrix.reset();
        BufferMain.WorldMatrix[0] = static_cast<f32>(gSharedObjects.ScreenWidth);
        BufferMain.WorldMatrix[5] = static_cast<f32>(gSharedObjects.ScreenHeight);
    }
    VertShd->setConstantBuffer(0, &BufferMain);
    FragShd->setConstantBuffer(0, &BufferMain);
}

SHADER_OBJECT_CALLBACK(DfRnShadowShaderCallback)
{
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    const dim::vector3df ViewPosition(
        __spSceneManager->getActiveCamera()->getPosition(true)
    );
    
    VertShd->setConstant(
        "WorldViewProjectionMatrix",
        __spVideoDriver->getProjectionMatrix() * __spVideoDriver->getViewMatrix() * __spVideoDriver->getWorldMatrix()
    );
    VertShd->setConstant(
        "WorldMatrix",
        __spVideoDriver->getWorldMatrix()
    );
    
    FragShd->setConstant("ViewPosition", ViewPosition);
}

SHADER_OBJECT_CALLBACK(DfRnDebugVPLShaderCallback)
{
    dim::matrix4f WVPMatrix(__spVideoDriver->getProjectionMatrix());
    WVPMatrix *= __spVideoDriver->getViewMatrix();
    WVPMatrix *= __spVideoDriver->getWorldMatrix();
    
    ShdClass->getVertexShader()->setConstant("WorldViewProjectionMatrix", WVPMatrix);
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================