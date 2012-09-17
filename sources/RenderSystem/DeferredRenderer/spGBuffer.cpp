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
    UseMultiSampling_   (false),
    UseHDR_             (false),
    UseBloom_           (false)
{
    memset(RenderTargets_, 0, sizeof(Texture*) * RENDERTARGET_COUNT);
}
GBuffer::~GBuffer()
{
    deleteGBuffer();
}

bool GBuffer::createGBuffer(
    const dim::size2di &Resolution, bool UseMultiSampling, bool UseHDR, bool UseBloom)
{
    /* Delete old GBuffer textures */
    deleteGBuffer();
    
    /* Copy new settings */
    Resolution_         = Resolution;
    UseMultiSampling_   = UseMultiSampling;
    UseHDR_             = UseHDR;
    UseBloom_           = UseBloom;
    
    /* General texture flags */
    STextureCreationFlags CreationFlags;
    
    CreationFlags.Size      = Resolution_;
    CreationFlags.MagFilter = FILTER_LINEAR;
    CreationFlags.MinFilter = FILTER_LINEAR;
    CreationFlags.MipMaps   = false;
    CreationFlags.WrapMode  = TEXWRAP_CLAMP;
    
    /* Create texture for diffuse and specular */
    CreationFlags.Format    = PIXELFORMAT_RGBA;
    
    if (UseHDR_)
    {
        CreationFlags.HWFormat      = HWTEXFORMAT_FLOAT16;
        CreationFlags.BufferType    = IMAGEBUFFER_FLOAT;
    }
    else
    {
        CreationFlags.HWFormat      = HWTEXFORMAT_UBYTE8;
        CreationFlags.BufferType    = IMAGEBUFFER_UBYTE;
    }
    
    RenderTargets_[RENDERTARGET_DIFFUSE_AND_SPECULAR] = __spVideoDriver->createTexture(CreationFlags);
    
    /* Create texture for normal vectors */
    CreationFlags.Format        = PIXELFORMAT_RGBA;
    CreationFlags.HWFormat      = HWTEXFORMAT_FLOAT16;
    CreationFlags.BufferType    = IMAGEBUFFER_UBYTE;
    
    RenderTargets_[RENDERTARGET_NORMAL_AND_DEPTH] = __spVideoDriver->createTexture(CreationFlags);
    
    if (UseBloom_)
    {
        /* Create textures for bloom filter */
        CreationFlags.Format    = PIXELFORMAT_RGB;
        CreationFlags.HWFormat  = HWTEXFORMAT_UBYTE8;
        
        RenderTargets_[RENDERTARGET_DEFERRED_COLOR] = __spVideoDriver->createTexture(CreationFlags);
        
        /* Create base gloss map  */
        CreationFlags.MagFilter = FILTER_SMOOTH;
        CreationFlags.MinFilter = FILTER_SMOOTH;
        CreationFlags.MipMaps   = true;
        
        RenderTargets_[RENDERTARGET_DEFERRED_GLOSS] = __spVideoDriver->createTexture(CreationFlags);
        
        /* Create temporary gloss map */
        CreationFlags.Size      /= 4;
        CreationFlags.MipMaps   = false;
        
        RenderTargets_[RENDERTARGET_GLOSS_1ST_PASS] = __spVideoDriver->createTexture(CreationFlags);
        RenderTargets_[RENDERTARGET_GLOSS_2ND_PASS] = __spVideoDriver->createTexture(CreationFlags);
    }
    
    /* Make the texture to render targets */
    return setupMultiRenderTargets();
}

void GBuffer::deleteGBuffer()
{
    for (s32 i = 0; i < RENDERTARGET_COUNT; ++i)
        __spVideoDriver->deleteTexture(RenderTargets_[i]);
    Resolution_ = 0;
}

void GBuffer::bindRTDeferredShading()
{
    __spVideoDriver->setRenderTarget(RenderTargets_[RENDERTARGET_DIFFUSE_AND_SPECULAR]);
}
void GBuffer::drawDeferredShading()
{
    drawMRTImage(RENDERTARGET_DIFFUSE_AND_SPECULAR, RENDERTARGET_NORMAL_AND_DEPTH);
}

void GBuffer::bindRTBloomFilter()
{
    __spVideoDriver->setRenderTarget(RenderTargets_[RENDERTARGET_DEFERRED_COLOR]);
}


/*
 * ======= Private: =======
 */

bool GBuffer::setupMultiRenderTargets()
{
    /* Setup render targets */
    s32 MaxCount = RENDERTARGET_COUNT - 1;
    
    if (!UseBloom_)
        MaxCount = RENDERTARGET_NORMAL_AND_DEPTH;
    
    for (s32 i = 0; i <= MaxCount; ++i)
    {
        if (!RenderTargets_[i])
            return false;
        
        RenderTargets_[i]->setRenderTarget(true);
    }
    
    /* Setup multi render targets for deferred shading */
    RenderTargets_[RENDERTARGET_DIFFUSE_AND_SPECULAR]->addMultiRenderTarget(
        RenderTargets_[RENDERTARGET_NORMAL_AND_DEPTH]
    );
    
    if (UseBloom_)
    {
        /* Setup multi render targets for bloom filter */
        RenderTargets_[RENDERTARGET_DEFERRED_COLOR]->addMultiRenderTarget(
            RenderTargets_[RENDERTARGET_DEFERRED_GLOSS]
        );
    }
    
    return true;
}

void GBuffer::drawMRTImage(s32 FirstIndex, s32 LastIndex)
{
    __spVideoDriver->setRenderState(RENDER_BLEND, false);
    {
        for (s32 i = FirstIndex; i <= LastIndex; ++i)
            RenderTargets_[i]->bind(i);
        
        __spVideoDriver->draw2DImage(RenderTargets_[FirstIndex], dim::point2di(0));
        
        for (s32 i = FirstIndex; i <= LastIndex; ++i)
            RenderTargets_[i]->unbind(i);
    }
    __spVideoDriver->setRenderState(RENDER_BLEND, true);
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
