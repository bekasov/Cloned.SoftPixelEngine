/*
 * Radial blur file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/PostProcessing/spRadialBlur.hpp"

#if defined(SP_COMPILE_WITH_POSTPROCESSING)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "Base/spSharedObjects.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


RadialBlur::RadialBlur() :
    PostProcessingEffect(       ),
    NumSamples_         (8      ),
    Scaling_            (0.1f   )
{
}
RadialBlur::~RadialBlur()
{
    deleteResources();
}

bool RadialBlur::createResources()
{
    /* Delete old shader */
    deleteResources();
    
    /* Create new resources */
    if (!compileShaders())
    {
        io::Log::error("Compiling shaders for radial-blur failed");
        deleteResources();
        return false;
    }
    
    /* Validate effect */
    Valid_ = true;
    
    return true;
}

void RadialBlur::deleteResources()
{
    /* Delete shaders */
    GlbRenderSys->deleteShaderClass(ShdClass_, true);
    ShdClass_ = 0;
    
    Valid_ = false;
}

const c8* RadialBlur::getName() const
{
    return "Radial Blur";
}

void RadialBlur::drawEffect(Texture* InputTexture, Texture* OutputTexture)
{
    /* Check if effect has already been created */
    if (!valid())
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("RadialBlur::drawEffect", "Effect is used but has not been created", io::LOG_UNIQUE);
        #endif
        return;
    }

    /* Check if the effect is not required to be drawn */
    if (!InputTexture || !active())
        return;
    
    /* Draw the effect with a fullscreen quad */
    GlbRenderSys->setRenderTarget(OutputTexture);
    {
        InputTexture->bind();
        ShdClass_->bind();

        GlbRenderSys->drawFullscreenQuad();

        ShdClass_->unbind();
        InputTexture->unbind();
    }
    GlbRenderSys->setRenderTarget(0);
}


/*
 * ======= Private: =======
 */

bool RadialBlur::compileShaders()
{
    const bool IsGL = (GlbRenderSys->getRendererType() == RENDERER_OPENGL);
    
    std::list<io::stringc> RadialBlurShdBufVert, RadialBlurShdBufFrag;
    s32 Flags = 0;

    switch (GlbRenderSys->getRendererType())
    {
        case RENDERER_DIRECT3D11:
        {
            /*RadialBlurShdBufVert.push_back(
                #include "Resources/spRadialBlurShaderStr.hlsl"
            );*/
            #if 1//!!!
            io::FileSystem fsys;
            ShaderClass::loadShaderResourceFile(fsys, "../../../sources/RenderSystem/PostProcessing/spRadialBlurShader.hlsl", RadialBlurShdBufVert);
            #endif

            Flags = SHADERBUILD_HLSL5;
        }
        break;
        
        default:
            return errShaderNotSupported();
    }
    
    if (!ShaderClass::build(
            "radial-blur", ShdClass_, GlbRenderSys->getVertexFormatReduced(),
            &RadialBlurShdBufVert, IsGL ? &RadialBlurShdBufFrag : &RadialBlurShdBufVert,
            "VertexMain", "PixelMain", Flags))
    {
        return false;
    }

    setupConstBuffers();

    return true;
}

void RadialBlur::setupConstBuffers()
{
    if (!ShdClass_)
        return;

    struct SBufferPS
    {
        u32 NumSamples;
        f32 Scaling;
        f32 Pad0[2];
    }
    Buffer;
    {
        Buffer.NumSamples   = NumSamples_;
        Buffer.Scaling      = Scaling_;
    }
    ShdClass_->getPixelShader()->setConstantBuffer(0, &Buffer);
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
