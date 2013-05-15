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


#define TEXTARGET(n) RenderTargets_[RENDERTARGET_##n]

GBuffer::GBuffer() :
    UseIllumination_(false),
    UseLowResVPL_   (false)
{
    memset(RenderTargets_, 0, sizeof(RenderTargets_));
}
GBuffer::~GBuffer()
{
    deleteGBuffer();
}

bool GBuffer::createGBuffer(
    const dim::size2di &Resolution, s32 MultiSampling, bool UseIllumination, bool UseLowResVPL)
{
    /* Delete old GBuffer textures */
    deleteGBuffer();
    
    /* Copy new settings */
    Resolution_         = Resolution;
    UseIllumination_    = UseIllumination;
    UseLowResVPL_       = UseLowResVPL;
    
    /* General texture flags */
    STextureCreationFlags CreationFlags;
    
    CreationFlags.Size          = Resolution_;
    CreationFlags.MagFilter     = FILTER_LINEAR;
    CreationFlags.MinFilter     = FILTER_LINEAR;
    CreationFlags.MipMaps       = false;
    CreationFlags.WrapMode      = TEXWRAP_CLAMP;
    
    /* Create texture for diffuse and specular */
    CreationFlags.Filename      = "Diffuse And Specular";
    CreationFlags.Format        = PIXELFORMAT_RGBA;
    CreationFlags.HWFormat      = HWTEXFORMAT_UBYTE8;
    CreationFlags.BufferType    = IMAGEBUFFER_UBYTE;
    
    TEXTARGET(DIFFUSE_AND_SPECULAR) = __spVideoDriver->createTexture(CreationFlags);
    
    /* Create texture for normal vectors */
    CreationFlags.Filename      = "Normal And Depth";
    CreationFlags.Format        = PIXELFORMAT_RGBA;
    CreationFlags.HWFormat      = HWTEXFORMAT_FLOAT16;
    CreationFlags.BufferType    = IMAGEBUFFER_UBYTE;
    
    TEXTARGET(NORMAL_AND_DEPTH) = __spVideoDriver->createTexture(CreationFlags);
    
    /* Create static-illumination texture */
    if (UseIllumination_)
    {
        CreationFlags.Filename  = "Illumination";
        CreationFlags.Format    = PIXELFORMAT_GRAY;
        CreationFlags.HWFormat  = HWTEXFORMAT_UBYTE8;
        
        TEXTARGET(ILLUMINATION) = __spVideoDriver->createTexture(CreationFlags);
    }
    
    /* Create low-resolution VPL texture */
    if (UseLowResVPL_)
    {
        CreationFlags.Filename  = "Low-resolution VPL";
        CreationFlags.Size      /= 2;
        CreationFlags.MagFilter = FILTER_SMOOTH;
        CreationFlags.MinFilter = FILTER_SMOOTH;
        CreationFlags.Format    = PIXELFORMAT_RGB;
        CreationFlags.HWFormat  = HWTEXFORMAT_UBYTE8;

        TEXTARGET(LOWRES_VPL) = __spVideoDriver->createTexture(CreationFlags);
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
    __spVideoDriver->setRenderTarget(TEXTARGET(DIFFUSE_AND_SPECULAR));
}

void GBuffer::drawDeferredShading()
{
    /* Bind and draw deferred-shading image */
    __spVideoDriver->setRenderMode(RENDERMODE_DRAWING_2D);
    __spVideoDriver->setRenderState(RENDER_BLEND, false);
    {
        /* Bind texture layers */
        TEXTARGET(DIFFUSE_AND_SPECULAR)->bind(0);
        TEXTARGET(NORMAL_AND_DEPTH)->bind(1);
        
        s32 Layer = 2;
        
        if (UseIllumination_)
            TEXTARGET(ILLUMINATION)->bind(Layer++);
        if (UseLowResVPL_)
            TEXTARGET(LOWRES_VPL)->bind(Layer);
        
        /* Draw 2D quad */
        __spVideoDriver->draw2DImage(TEXTARGET(DIFFUSE_AND_SPECULAR), dim::point2di(0));
        
        /* Unbind texture layers */
        TEXTARGET(DIFFUSE_AND_SPECULAR)->unbind(0);
        TEXTARGET(NORMAL_AND_DEPTH)->unbind(1);
        
        Layer = 2;
        
        if (UseIllumination_)
            TEXTARGET(ILLUMINATION)->unbind(Layer++);
        if (UseLowResVPL_)
            TEXTARGET(LOWRES_VPL)->unbind(Layer);
    }
    __spVideoDriver->setRenderState(RENDER_BLEND, true);
}

void GBuffer::drawLowResVPLDeferredShading()
{
    /* Get low-resolution VPL texture size and use it to draw the deferred shading */
    const dim::size2di TexSize(TEXTARGET(LOWRES_VPL)->getSize());
    
    /* Bind and draw low-resolution VPL deferred-shading image */
    __spVideoDriver->setRenderMode(RENDERMODE_DRAWING_2D);
    __spVideoDriver->setRenderState(RENDER_BLEND, false);
    {
        __spVideoDriver->draw2DImage(
            TEXTARGET(NORMAL_AND_DEPTH),
            dim::rect2di(0, 0, TexSize.Width, TexSize.Height)
        );
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
        else if ( ( i == RENDERTARGET_ILLUMINATION && UseIllumination_ ) ||
                  ( i == RENDERTARGET_LOWRES_VPL && UseLowResVPL_ ) )
        {
            return false;
        }
    }
    
    /* Setup multi render targets for deferred shading */
    TEXTARGET(DIFFUSE_AND_SPECULAR)->addMultiRenderTarget(TEXTARGET(NORMAL_AND_DEPTH));
    
    if (UseIllumination_)
        TEXTARGET(DIFFUSE_AND_SPECULAR)->addMultiRenderTarget(TEXTARGET(ILLUMINATION));
    
    return true;
}

#undef TEXTARGET


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
