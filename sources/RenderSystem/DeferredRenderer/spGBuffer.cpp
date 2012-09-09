/*
 * GBuffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spGBuffer.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spRenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


GBuffer::GBuffer()
{
    memset(RenderTargets_, 0, sizeof(video::Texture*) * RENDERTARGET_COUNT);
}
GBuffer::~GBuffer()
{
    deleteGBuffer();
}

bool GBuffer::createGBuffer(
    const dim::size2di &Resolution, bool UseMultiSampling, bool UseHDR)
{
    /* Delete old GBuffer textures */
    deleteGBuffer();
    
    /* Copy new settings */
    Resolution_         = Resolution;
    UseMultiSampling_   = UseMultiSampling;
    UseHDR_             = UseHDR;
    
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
    CreationFlags.Format        = PIXELFORMAT_RGB;
    CreationFlags.HWFormat      = HWTEXFORMAT_UBYTE8;
    CreationFlags.BufferType    = IMAGEBUFFER_UBYTE;
    
    RenderTargets_[RENDERTARGET_NORMAL] = __spVideoDriver->createTexture(CreationFlags);
    
    /* Create texture for depth map */
    CreationFlags.Format = PIXELFORMAT_DEPTH;
    
    RenderTargets_[RENDERTARGET_DEPTH] = __spVideoDriver->createTexture(CreationFlags);
    
    /* Make the texture to render targets */
    return setupMultiRenderTargets();
}

void GBuffer::deleteGBuffer()
{
    for (s32 i = 0; i < RENDERTARGET_COUNT; ++i)
        __spVideoDriver->deleteTexture(RenderTargets_[i]);
    Resolution_ = 0;
}

void GBuffer::bindRenderTarget()
{
    __spVideoDriver->setRenderTarget(RenderTargets_[0]);
}

void GBuffer::draw2DImage()
{
    __spVideoDriver->beginDrawing2D();
    {
        for (s32 i = 0; i < RENDERTARGET_COUNT; ++i)
            RenderTargets_[i]->bind(i);
        
        __spVideoDriver->draw2DImage(RenderTargets_[0], dim::point2di(0));
        
        for (s32 i = 0; i < RENDERTARGET_COUNT; ++i)
            RenderTargets_[i]->unbind(i);
    }
    __spVideoDriver->endDrawing2D();
}


/*
 * ======= Private: =======
 */

bool GBuffer::setupMultiRenderTargets()
{
    for (s32 i = 0; i < RENDERTARGET_COUNT; ++i)
    {
        if (!RenderTargets_[i])
            return false;
        
        RenderTargets_[i]->setRenderTarget(true);
        
        if (i)
            RenderTargets_[0]->addMultiRenderTarget(RenderTargets_[i]);
    }
    
    return true;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
