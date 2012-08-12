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

void GBuffer::createGBuffer(const dim::size2di &Resolution, bool isMultiSampling)
{
    deleteGBuffer();
    
    Resolution_ = Resolution;
    
    //todo
    
}

void GBuffer::deleteGBuffer()
{
    for (u32 i = RENDERTARGET_COLORMAP; i < RENDERTARGET_COUNT; ++i)
        __spVideoDriver->deleteTexture(RenderTargets_[i]);
    Resolution_ = 0;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
