/*
 * Matrix texture file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spUtilityMatrixTexture.hpp"

#ifdef SP_COMPILE_WITH_MATRIXTEXTURE


#include "RenderSystem/spRenderSystem.hpp"
#include "Base/spBaseExceptions.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace tool
{


/*
 * GeneralPurposeTexture class
 */

GeneralPurposeTexture::GeneralPurposeTexture(
    const io::stringc &TypeName, const dim::size2di &TexSize, u32 Count) :
    Tex_    (0                      ),
    Count_  (math::Max(1u, Count)   )
{
    if (!GlbRenderSys)
        throw io::RenderSystemException(TypeName);
    
    generate(TexSize, Count);
}
GeneralPurposeTexture::~GeneralPurposeTexture()
{
    if (GlbRenderSys)
        GlbRenderSys->deleteTexture(Tex_);
}

dim::size2di GeneralPurposeTexture::getSizeByCount(u32 Count)
{
    if (Count > 0)
    {
        s32 Width = math::roundPow2(static_cast<s32>(sqrt(static_cast<f32>(Count))));
        s32 Height = Width;
        
        while (static_cast<u32>(Width*Height) < Count)
            ++Height;
        
        return dim::size2di(Width, Height);
    }
    return 0;
}

void GeneralPurposeTexture::update()
{
    Tex_->updateImageBuffer();
}


/*
 * ======= Private: =======
 */

void GeneralPurposeTexture::generate(const dim::size2di &TexSize, u32 Count)
{
    /* Delete previous texture */
    if (Tex_)
        GlbRenderSys->deleteTexture(Tex_);
    
    /* Create texture */
    video::STextureCreationFlags CreationFlags;
    {
        CreationFlags.Size          = TexSize;
        CreationFlags.BufferType    = video::IMAGEBUFFER_FLOAT;
        CreationFlags.Format        = video::PIXELFORMAT_RGBA;
        CreationFlags.HWFormat      = video::HWTEXFORMAT_FLOAT32;
        CreationFlags.MagFilter     = video::FILTER_LINEAR;
        CreationFlags.MinFilter     = video::FILTER_LINEAR;
        CreationFlags.MipMaps       = false;
        CreationFlags.WrapMode      = video::TEXWRAP_CLAMP;
    }
    Tex_ = GlbRenderSys->createTexture(CreationFlags);
}


/*
 * MatrixTexture class
 */

MatrixTexture::MatrixTexture(u32 Count) :
    GeneralPurposeTexture(
        "MatrixTexture", GeneralPurposeTexture::getSizeByCount(Count*4), Count
    )
{
}
MatrixTexture::~MatrixTexture()
{
}

void MatrixTexture::setMatrix(u32 Index, const dim::matrix4f &Matrix, bool ImmediateUpdate)
{
    video::ImageBuffer* ImgBuffer = Tex_->getImageBuffer();
    
    if (ImgBuffer)
    {
        dim::point2di tc(getTexCoord(Index));
        
        const f32* M = Matrix.getArray();
        
        ImgBuffer->setPixelVector(dim::point2di(tc.X    , tc.Y), dim::vector4df(M[ 0], M[ 1], M[ 2], M[ 3]));
        ImgBuffer->setPixelVector(dim::point2di(tc.X + 1, tc.Y), dim::vector4df(M[ 4], M[ 5], M[ 6], M[ 7]));
        ImgBuffer->setPixelVector(dim::point2di(tc.X + 2, tc.Y), dim::vector4df(M[ 8], M[ 9], M[10], M[11]));
        ImgBuffer->setPixelVector(dim::point2di(tc.X + 3, tc.Y), dim::vector4df(M[12], M[13], M[14], M[15]));
        
        if (ImmediateUpdate)
            update();
    }
}

dim::matrix4f MatrixTexture::getMatrix(u32 Index) const
{
    video::ImageBuffer* ImgBuffer = Tex_->getImageBuffer();
    
    if (ImgBuffer)
    {
        dim::point2di tc(getTexCoord(Index));
        
        return dim::matrix4f(
            ImgBuffer->getPixelVector(dim::point2di(tc.X    , tc.Y)),
            ImgBuffer->getPixelVector(dim::point2di(tc.X + 1, tc.Y)),
            ImgBuffer->getPixelVector(dim::point2di(tc.X + 2, tc.Y)),
            ImgBuffer->getPixelVector(dim::point2di(tc.X + 3, tc.Y))
        );
    }
    
    return dim::matrix4f();
}

dim::point2di MatrixTexture::getTexCoord(u32 Index) const
{
    const div_t Quot = div(Index*4, Tex_->getSize().Width);
    return dim::point2di(Quot.rem, Quot.quot);
}

void MatrixTexture::resize(u32 Count)
{
    generate(GeneralPurposeTexture::getSizeByCount(Count*4), Count);
}


/*
 * VectorTexture class
 */

VectorTexture::VectorTexture(u32 Count) :
    GeneralPurposeTexture(
        "VectorTexture", GeneralPurposeTexture::getSizeByCount(Count), Count
    )
{
}
VectorTexture::~VectorTexture()
{
}

void VectorTexture::setVector(u32 Index, const dim::vector4df &Vector, bool ImmediateUpdate)
{
    video::ImageBuffer* ImgBuffer = Tex_->getImageBuffer();
    
    if (ImgBuffer)
    {
        ImgBuffer->setPixelVector(getTexCoord(Index), Vector);
        
        if (ImmediateUpdate)
            update();
    }
}

dim::vector4df VectorTexture::getVector(u32 Index) const
{
    video::ImageBuffer* ImgBuffer = Tex_->getImageBuffer();
    
    if (ImgBuffer)
        return ImgBuffer->getPixelVector(getTexCoord(Index));
    
    return dim::vector4df();
}

dim::point2di VectorTexture::getTexCoord(u32 Index) const
{
    const div_t Quot = div(Index, Tex_->getSize().Width);
    return dim::point2di(Quot.rem, Quot.quot);
}

void VectorTexture::resize(u32 Count)
{
    generate(GeneralPurposeTexture::getSizeByCount(Count), Count);
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
