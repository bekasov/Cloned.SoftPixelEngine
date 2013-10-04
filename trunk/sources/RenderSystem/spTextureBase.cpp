/*
 * Texture base file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spTextureBase.hpp"
#include "Base/spImageManagement.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


Texture::Texture() :
    BaseObject          (                   ),
    OrigID_             (0                  ),
    ID_                 (0                  ),
    Type_               (TEXTURE_2D         ),
    HWFormat_           (HWTEXFORMAT_UBYTE8 ),
    MultiSamples_       (0                  ),
    CubeMapFace_        (CUBEMAP_POSITIVE_X ),
    ArrayLayer_         (0                  ),
    isRenderTarget_     (false              ),
    DepthBufferSource_  (0                  ),
    ImageBuffer_        (0                  ),
    ImageBufferBackup_  (0                  )
{
    createImageBuffer();
}
Texture::Texture(const STextureCreationFlags &CreationFlags) :
    BaseObject          (CreationFlags.Filename ),
    OrigID_             (0                      ),
    ID_                 (0                      ),
    Type_               (CreationFlags.Type     ),
    HWFormat_           (CreationFlags.HWFormat ),
    Filter_             (CreationFlags.Filter   ),
    MultiSamples_       (0                      ),
    CubeMapFace_        (CUBEMAP_POSITIVE_X     ),
    ArrayLayer_         (0                      ),
    isRenderTarget_     (false                  ),
    DepthBufferSource_  (0                      ),
    ImageBuffer_        (0                      ),
    ImageBufferBackup_  (0                      )
{
    createImageBuffer(CreationFlags);
}
Texture::~Texture()
{
    /* Delete the backup */
    clearBackup();
    
    /* Delete the used image data */
    MemoryManager::deleteMemory(ImageBuffer_);
    
    /* Clear MRT reference lists */
    clearMRTList();
}

bool Texture::valid() const
{
    return false;
}

void Texture::addMultiRenderTarget(Texture* Tex)
{
    if (Tex)
    {
        MRTList_.push_back(Tex);
        updateMultiRenderTargets();
        Tex->MRTRefList_.push_back(this);
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("Texture::addMultiRenderTarget");
    #endif
}

void Texture::removeMultiRenderTarget(Texture* Tex)
{
    if (Tex)
    {
        /* Remove the specified texture from the MRT list */
        if (MemoryManager::removeElement(MRTList_, Tex))
        {
            /* Update render targets */
            updateMultiRenderTargets();
            
            /* Remove this texture from the MRT reference list */
            Texture* ThisTex = this;
            MemoryManager::removeElement(Tex->MRTRefList_, ThisTex);
        }
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("Texture::removeMultiRenderTarget");
    #endif
}

void Texture::clearMultiRenderTarget()
{
    if (MRTList_.empty())
    {
        clearMRTList();
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
    if (Filter_.HasMIPMaps != MipMaps && Type_ != TEXTURE_RECTANGLE && Type_ != TEXTURE_BUFFER)
    {
        Filter_.HasMIPMaps = MipMaps;
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

bool Texture::setType(const ETextureTypes Type, s32 Depth)
{
    #ifdef SP_DEBUGMODE
    if (Type == TEXTURE_3D && Depth <= 0)
        io::Log::debug("Texture::setType", "'Depth' parameter must be greater than 0 for 3D textures");
    #endif
    
    if ( Type_ == Type && ( Type != TEXTURE_3D || Depth == 0 || Depth == ImageBuffer_->getDepth() ) )
        return false;
    
    /* Setup new image buffer depth */
    bool Result = false;
    
    switch (Type)
    {
        case TEXTURE_2D_ARRAY:
        case TEXTURE_3D:
            Result = ImageBuffer_->setDepth(Depth); break;
        case TEXTURE_CUBEMAP:
            Result = ImageBuffer_->setDepth(6); break;
        case TEXTURE_CUBEMAP_ARRAY:
            Result = ImageBuffer_->setDepth(Depth * 6); break;
        default:
            Result = ImageBuffer_->setDepth(1); break;
    }
    
    if (!Result)
    {
        io::Log::error("Setting texture dimension failed");
        return false;
    }
    
    /* Setup additional flags for special dimension types */
    if (Type == TEXTURE_RECTANGLE)
    {
        setWrapMode(TEXWRAP_CLAMP);
        Filter_.HasMIPMaps = false;
    }
    if (Type == TEXTURE_BUFFER)
        Filter_.HasMIPMaps = false;
    
    /* Store new dimension type and update image buffer */
    Type_ = Type;
    
    updateImageBuffer();
    
    return true;
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
        if (GlbRenderSys->RenderQuery_[RenderSystem::RENDERQUERY_RENDERTARGET])
            updateImageBuffer();
    }
}

void Texture::setMultiSamples(s32 Samples)
{
    if (Samples != MultiSamples_)
    {
        MultiSamples_ = math::Max(0, Samples);
        
        if (isRenderTarget_ && GlbRenderSys->RenderQuery_[RenderSystem::RENDERQUERY_MULTISAMPLE_RENDERTARGET])
        {
            if (MRTList_.empty())
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

void Texture::ensurePOT()
{
    if (!ImageBuffer_->isSizePOT())
        ImageBuffer_->setSize(ImageBuffer_->getSizePOT());
}

void Texture::setReference(Texture* ReferenceTexture)
{
    if (ReferenceTexture)
        ID_ = ReferenceTexture->OrigID_;
    else
        ID_ = OrigID_;
}


/* === Filter, MipMapping, Texture coordinate wraps === */

void Texture::setFilter(const STextureFilter &Filter)
{
    Filter_ = Filter;
}

void Texture::setMinMagFilter(const ETextureFilters Filter)
{
    Filter_.Mag = Filter_.Min = Filter;
}
void Texture::setMinMagFilter(const ETextureFilters MagFilter, const ETextureFilters MinFilter)
{
    Filter_.Mag = MagFilter;
    Filter_.Min = MinFilter;
}
void Texture::setMagFilter(const ETextureFilters Filter)
{
    Filter_.Mag = Filter;
}
void Texture::setMinFilter(const ETextureFilters Filter)
{
    Filter_.Min = Filter;
}

void Texture::setMipMapFilter(const ETextureMipMapFilters MipMapFilter)
{
    Filter_.MIPMap = MipMapFilter;
}

void Texture::setWrapMode(const ETextureWrapModes Wrap)
{
    setWrapMode(Wrap, Wrap, Wrap);
}
void Texture::setWrapMode(
    const ETextureWrapModes WrapU, const ETextureWrapModes WrapV, const ETextureWrapModes WrapW)
{
    if (Type_ == TEXTURE_RECTANGLE)
    {
        #ifdef SP_DEBUGMODE
        if (WrapU != TEXWRAP_CLAMP || WrapV != TEXWRAP_CLAMP || WrapW != TEXWRAP_CLAMP)
            io::Log::debug("Texture::setWrapMode", "Rectangle textures can only have 'clamp' as wrap mode");
        #endif
        
        Filter_.WrapMode = TEXWRAP_CLAMP;
    }
    else
    {
        Filter_.WrapMode.X = WrapU;
        Filter_.WrapMode.Y = WrapV;
        Filter_.WrapMode.Z = WrapW;
    }
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
    #ifdef SP_DEBUGMODE
    else if (SubImageBuffer)
        io::Log::debug("Texture::setupImageBuffer");
    else
        io::Log::debug("Texture::setupImageBuffer", "Image buffers must have same type");
    #endif
    
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

void Texture::clearMRTList()
{
    if (MRTList_.empty())
    {
        /* Remove this texture from the MRT reference list for each MRT entry */
        Texture* ThisTex = this;
        foreach (Texture* Tex, MRTList_)
            MemoryManager::removeElement(Tex->MRTRefList_, ThisTex);
        
        /* Clear MRT list and update render targets */
        MRTList_.clear();
    }
}


} // /namespace video

} // /namespace sp



// ================================================================================
