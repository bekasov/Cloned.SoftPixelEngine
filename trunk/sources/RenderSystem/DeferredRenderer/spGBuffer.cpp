/*
 * GBuffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spGBuffer.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spRenderSystem.hpp"
#include "Base/spSharedObjects.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


GBuffer::GBuffer() :
    UseIllumination_(false)
{
    memset(RenderTargets_, 0, sizeof(RenderTargets_));
}
GBuffer::~GBuffer()
{
    deleteGBuffer();
}

bool GBuffer::createGBuffer(
    const dim::size2di &Resolution, s32 MultiSampling, bool UseIllumination)
{
    /* Delete old GBuffer textures */
    deleteGBuffer();
    
    /* Copy new settings */
    Resolution_         = Resolution;
    UseIllumination_    = UseIllumination;
    
    /* General texture flags */
    STextureCreationFlags CreationFlags;
    
    CreationFlags.Size          = Resolution_;
    CreationFlags.MagFilter     = FILTER_LINEAR;
    CreationFlags.MinFilter     = FILTER_LINEAR;
    CreationFlags.MipMaps       = false;
    CreationFlags.WrapMode      = TEXWRAP_CLAMP;
    
    /* Create texture for diffuse and specular */
    CreationFlags.Format        = PIXELFORMAT_RGBA;
    CreationFlags.HWFormat      = HWTEXFORMAT_UBYTE8;
    CreationFlags.BufferType    = IMAGEBUFFER_UBYTE;
    
    RenderTargets_[RENDERTARGET_DIFFUSE_AND_SPECULAR] = __spVideoDriver->createTexture(CreationFlags);
    
    /* Create texture for normal vectors */
    CreationFlags.Format        = PIXELFORMAT_RGBA;
    CreationFlags.HWFormat      = HWTEXFORMAT_FLOAT16;
    CreationFlags.BufferType    = IMAGEBUFFER_UBYTE;
    
    RenderTargets_[RENDERTARGET_NORMAL_AND_DEPTH] = __spVideoDriver->createTexture(CreationFlags);
    
    if (UseIllumination_)
    {
        CreationFlags.Format    = PIXELFORMAT_GRAY;
        CreationFlags.HWFormat  = HWTEXFORMAT_UBYTE8;
        
        RenderTargets_[RENDERTARGET_ILLUMINATION] = __spVideoDriver->createTexture(CreationFlags);
    }
    
    /* Make the texture to render targets */
    if (!setupMultiRenderTargets(MultiSampling))
    {
        io::Log::error("Setting up multi-render-target textures failed");
        return false;
    }
    
    return true;
}

void GBuffer::deleteGBuffer()
{
    /* Delete all render targets */
    for (s32 i = 0; i < RENDERTARGET_COUNT; ++i)
        __spVideoDriver->deleteTexture(RenderTargets_[i]);
    
    /* Reset configuration */
    Resolution_         = 0;
    UseIllumination_    = false;
}

void GBuffer::bindRenderTargets()
{
    __spVideoDriver->setRenderTarget(RenderTargets_[RENDERTARGET_DIFFUSE_AND_SPECULAR]);
}

void GBuffer::drawDeferredShading()
{
    /* Setup render-target indices */
    const s32 FirstIndex    = RENDERTARGET_DIFFUSE_AND_SPECULAR;
    const s32 LastIndex     = (UseIllumination_ ? RENDERTARGET_ILLUMINATION : RENDERTARGET_NORMAL_AND_DEPTH);
    
    /* Bind and draw deferred-shading images */
    __spVideoDriver->setRenderMode(RENDERMODE_DRAWING_2D);
    __spVideoDriver->setRenderState(RENDER_BLEND, false);
    {
        for (s32 i = FirstIndex; i <= LastIndex; ++i)
            RenderTargets_[i]->bind(i);
        
        __spVideoDriver->draw2DImage(RenderTargets_[RENDERTARGET_DIFFUSE_AND_SPECULAR], dim::point2di(0));
        
        for (s32 i = FirstIndex; i <= LastIndex; ++i)
            RenderTargets_[i]->unbind(i);
    }
    __spVideoDriver->setRenderState(RENDER_BLEND, true);
}


/*
 * ======= Private: =======
 */

bool GBuffer::setupMultiRenderTargets(s32 MultiSampling)
{
    /* Setup render targets */
    for (s32 i = 0; i < RENDERTARGET_COUNT; ++i)
    {
        Texture* Tex = RenderTargets_[i];
        
        if (Tex)
        {
            Tex->setRenderTarget(true);
            
            if (MultiSampling > 0)
                Tex->setMultiSamples(MultiSampling);
        }
        else if (i == RENDERTARGET_ILLUMINATION && UseIllumination_)
            return false;
    }
    
    /* Setup multi render targets for deferred shading */
    RenderTargets_[RENDERTARGET_DIFFUSE_AND_SPECULAR]->addMultiRenderTarget(
        RenderTargets_[RENDERTARGET_NORMAL_AND_DEPTH]
    );
    
    if (UseIllumination_)
    {
        RenderTargets_[RENDERTARGET_DIFFUSE_AND_SPECULAR]->addMultiRenderTarget(
            RenderTargets_[RENDERTARGET_ILLUMINATION]
        );
    }
    
    return true;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
