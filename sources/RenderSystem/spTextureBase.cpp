/*
 * Texture base file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spTextureBase.hpp"
#include "Base/spImageManagement.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


Texture::Texture() :
    BaseObject          (                   ),
    OrigID_             (0                  ),
    ID_                 (0                  ),
    HWFormat_           (HWTEXFORMAT_UBYTE8 ),
    DimensionType_      (TEXTURE_2D         ),
    MagFilter_          (FILTER_SMOOTH      ),
    MinFilter_          (FILTER_SMOOTH      ),
    MipMapFilter_       (FILTER_TRILINEAR   ),
    WrapMode_           (TEXWRAP_REPEAT     ),
    MipMaps_            (true               ),
    AnisotropicSamples_ (1                  ),
    MultiSamples_       (0                  ),
    CubeMapFace_        (CUBEMAP_POSITIVE_X ),
    ArrayLayer_         (0                  ),
    isRenderTarget_     (false              ),
    DepthBufferSource_  (0                  ),
    isAnim_             (false              ),
    ImageBuffer_        (0                  ),
    ImageBufferBackup_  (0                  )
{
    createImageBuffer();
}
Texture::Texture(const STextureCreationFlags &CreationFlags) :
    BaseObject          (                           ),
    OrigID_             (0                          ),
    ID_                 (0                          ),
    Filename_           (CreationFlags.Filename     ),
    HWFormat_           (CreationFlags.HWFormat     ),
    DimensionType_      (CreationFlags.Dimension    ),
    MagFilter_          (CreationFlags.MagFilter    ),
    MinFilter_          (CreationFlags.MinFilter    ),
    MipMapFilter_       (CreationFlags.MipMapFilter ),
    WrapMode_           (CreationFlags.WrapMode     ),
    MipMaps_            (CreationFlags.MipMaps      ),
    AnisotropicSamples_ (CreationFlags.Anisotropy   ),
    MultiSamples_       (0                          ),
    CubeMapFace_        (CUBEMAP_POSITIVE_X         ),
    ArrayLayer_         (0                          ),
    isRenderTarget_     (false                      ),
    DepthBufferSource_  (0                          ),
    isAnim_             (false                      ),
    ImageBuffer_        (0                          ),
    ImageBufferBackup_  (0                          )
{
    createImageBuffer(CreationFlags);
}
Texture::~Texture()
{
    /* Delete the backup */
    clearBackup();
    
    /* Delete the used image data */
    MemoryManager::deleteMemory(ImageBuffer_);
}

bool Texture::valid() const
{
    return false;
}

void Texture::addMultiRenderTarget(Texture* Tex)
{
    if (Tex)
    {
        MultiRenderTargetList_.push_back(Tex);
        updateMultiRenderTargets();
    }
}
void Texture::removeMultiRenderTarget(Texture* Tex)
{
    if (Tex)
    {
        MemoryManager::removeElement(MultiRenderTargetList_, Tex);
        updateMultiRenderTargets();
    }
}
void Texture::clearMultiRenderTarget()
{
    if (MultiRenderTargetList_.empty())
    {
        MultiRenderTargetList_.clear();
        updateMultiRenderTargets();
    }
}

void Texture::setDepthBufferSource(Texture* DepthBufferSourceTexture)
{
    DepthBufferSource_ = DepthBufferSourceTexture;
    updateImageBuffer();
}

void Texture::setHardwareFormat(const EHWTextureFormats HardwareFormat)
{
    HWFormat_ = HardwareFormat;
    updateImageBuffer();
}

void Texture::setMipMapping(bool MipMaps)
{
    if (MipMaps_ != MipMaps)
    {
        MipMaps_ = MipMaps;
        updateImageBuffer();
    }
}

void Texture::saveBackup()
{
    clearBackup();
    ImageBufferBackup_ = ImageBuffer_->copy();
}

void Texture::loadBackup()
{
    if (ImageBufferBackup_)
    {
        MemoryManager::deleteMemory(ImageBuffer_);
        ImageBuffer_ = ImageBufferBackup_->copy();
        updateImageBuffer();
    }
}

void Texture::clearBackup()
{
    MemoryManager::deleteMemory(ImageBufferBackup_);
}


/* === Dimension === */

void Texture::setDimension(const ETextureDimensions Type, s32 Depth)
{
    if ( DimensionType_ != Type || ( Type == TEXTURE_3D && Depth > 0 && Depth != ImageBuffer_->getDepth() ) )
    {
        DimensionType_ = Type;
        
        switch (DimensionType_)
        {
            case TEXTURE_2D_ARRAY:
            case TEXTURE_3D:
                ImageBuffer_->setDepth(Depth); break;
            case TEXTURE_CUBEMAP:
                ImageBuffer_->setDepth(6); break;
            case TEXTURE_CUBEMAP_ARRAY:
                ImageBuffer_->setDepth(Depth * 6); break;
            default:
                ImageBuffer_->setDepth(1); break;
        }
        
        updateImageBuffer();
    }
}

void Texture::setCubeMapFace(const ECubeMapDirections Face)
{
    CubeMapFace_ = Face;
}
void Texture::setArrayLayer(u32 Layer)
{
    ArrayLayer_ = Layer;
}

void Texture::setRenderTarget(bool Enable)
{
    if (isRenderTarget_ != Enable)
    {
        isRenderTarget_ = Enable;
        if (__spVideoDriver->RenderQuery_[RenderSystem::RENDERQUERY_RENDERTARGET])
            updateImageBuffer();
    }
}

void Texture::setMultiSamples(s32 Samples)
{
    if (Samples != MultiSamples_)
    {
        MultiSamples_ = math::Max(0, Samples);
        
        if (isRenderTarget_ && __spVideoDriver->RenderQuery_[RenderSystem::RENDERQUERY_MULTISAMPLE_RENDERTARGET])
        {
            if (MultiRenderTargetList_.empty())
                updateImageBuffer();
            else
                updateMultiRenderTargets();
        }
    }
}

void Texture::setFormat(const EPixelFormats Format)
{
    ImageBuffer_->setFormat(Format);
    updateImageBuffer();
}
void Texture::setSize(const dim::size2di &Size)
{
    ImageBuffer_->setSize(Size);
    updateImageBuffer();
}
void Texture::setColorKey(const color &Color, u8 Tolerance)
{
    ImageBuffer_->setColorKey(Color, Tolerance);
    updateImageBuffer();
}
void Texture::setColorKey(const dim::point2di &Pos, u8 Alpha, u8 Tolerance)
{
    ImageBuffer_->setColorKey(Pos, Alpha, Tolerance);
    updateImageBuffer();
}
void Texture::setColorKeyAlpha(const EAlphaBlendingTypes Mode)
{
    ImageBuffer_->setColorKeyAlpha(Mode);
    updateImageBuffer();
}
void Texture::setColorKeyMask(ImageBuffer* MaskImage, const EAlphaBlendingTypes Mode)
{
    ImageBuffer_->setColorKeyMask(MaskImage, Mode);
    updateImageBuffer();
}


/* === Animation === */

void Texture::addAnimFrame(Texture* AnimFrame)
{
    if (AnimFrame)
        AnimFrameList_.push_back(AnimFrame);
}
void Texture::removeAnimFrame(Texture* AnimFrame)
{
    for (std::vector<Texture*>::iterator it = AnimFrameList_.begin(); it != AnimFrameList_.end(); ++it)
    {
        if (*it == AnimFrame)
        {
            AnimFrameList_.erase(it);
            break;
        }
    }
}

void Texture::setAnimation(bool Enable)
{
    if (isAnim_ = Enable && !AnimFrameList_.empty())
        ID_ = AnimFrameList_[0]->OrigID_;
    else
        ID_ = OrigID_;
}

void Texture::setAnimFrame(u32 Frame)
{
    if (isAnim_ && Frame < AnimFrameList_.size())
        ID_ = AnimFrameList_[Frame]->OrigID_;
}


/* === Filter, MipMapping, Texture coordinate wraps === */

void Texture::setFilter(const ETextureFilters Filter)
{
    MagFilter_ = MinFilter_ = Filter;
}
void Texture::setFilter(const ETextureFilters MagFilter, const ETextureFilters MinFilter)
{
    MagFilter_ = MagFilter;
    MinFilter_ = MinFilter;
}
void Texture::setMagFilter(const ETextureFilters Filter)
{
    MagFilter_ = Filter;
}
void Texture::setMinFilter(const ETextureFilters Filter)
{
    MinFilter_ = Filter;
}

void Texture::setMipMapFilter(const ETextureMipMapFilters MipMapFilter)
{
    MipMapFilter_ = MipMapFilter;
}

void Texture::setWrapMode(const ETextureWrapModes Wrap)
{
    WrapMode_.X = WrapMode_.Y = WrapMode_.Z = Wrap;
}
void Texture::setWrapMode(
    const ETextureWrapModes WrapU, const ETextureWrapModes WrapV, const ETextureWrapModes WrapW)
{
    WrapMode_.X = WrapU;
    WrapMode_.Y = WrapV;
    WrapMode_.Z = WrapW;
}

bool Texture::shareImageBuffer()
{
    return false; // do nothing (for dummy texture)
}
bool Texture::updateImageBuffer()
{
    return false; // do nothing (for dummy texture)
}
bool Texture::updateImageBuffer(const dim::point2di &Pos, const dim::size2di &Size)
{
    return false; // do nothing (for dummy texture)
}

bool Texture::setupImageBuffer(const void* NewImageBuffer)
{
    if (NewImageBuffer)
    {
        ImageBuffer_->setBuffer(NewImageBuffer);
        return updateImageBuffer();
    }
    return false;
}

bool Texture::setupImageBuffer(const ImageBuffer* NewImageBuffer)
{
    /* Just dopt the new image buffer */
    if (NewImageBuffer)
    {
        MemoryManager::deleteMemory(ImageBuffer_);
        ImageBuffer_ = NewImageBuffer->copy();
        return updateImageBuffer();
    }
    return false;
}

bool Texture::setupImageBuffer(const ImageBuffer* SubImageBuffer, const dim::point2di &Position, const dim::size2di &Size)
{
    if (SubImageBuffer && SubImageBuffer->getType() == ImageBuffer_->getType())
    {
        switch (SubImageBuffer->getType())
        {
            case IMAGEBUFFER_UBYTE:
                ImageBuffer_->setBuffer(static_cast<const u8*>(SubImageBuffer->getBuffer()), Position, Size);
                break;
            case IMAGEBUFFER_FLOAT:
                ImageBuffer_->setBuffer(static_cast<const f32*>(SubImageBuffer->getBuffer()), Position, Size);
                break;
        }
        return updateImageBuffer();
    }
    return false;
}

void Texture::generateMipMap()
{
    // do nothing
}

void Texture::bind(s32 Level) const
{
    // do nothing
}
void Texture::unbind(s32 Level) const
{
    // do nothing
}


/*
 * ======= Protected: =======
 */

void Texture::updateMultiRenderTargets()
{
}


/*
 * ======= Private: =======
 */

void Texture::createImageBuffer()
{
    /* Delete old image buffer */
    MemoryManager::deleteMemory(ImageBuffer_);
    
    /* Create default image buffer */
    ImageBuffer_ = new ImageBufferUByte();
}

void Texture::createImageBuffer(const STextureCreationFlags &CreationFlags)
{
    /* Delete old image buffer */
    MemoryManager::deleteMemory(ImageBuffer_);
    
    /* Create individual image buffer and fill with initial data */
    switch (CreationFlags.BufferType)
    {
        case IMAGEBUFFER_UBYTE:
        {
            ImageBuffer_ = new ImageBufferUByte(
                CreationFlags.Format, CreationFlags.Size, CreationFlags.Depth, static_cast<const u8*>(CreationFlags.ImageBuffer)
            );
        }
        break;
        
        case IMAGEBUFFER_FLOAT:
        {
            ImageBuffer_ = new ImageBufferFloat(
                CreationFlags.Format, CreationFlags.Size, CreationFlags.Depth, static_cast<const f32*>(CreationFlags.ImageBuffer)
            );
        }
        break;
    }
}


} // /namespace video

} // /namespace sp



// ================================================================================
