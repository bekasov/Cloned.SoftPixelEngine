/*
 * Texture layer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spTextureLayer.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spTextureBase.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


TextureLayer::TextureLayer() :
    Type_       (TEXLAYER_BASE          ),
    Texture_    (0                      ),
    LayerIndex_ (0                      ),
    Enabled_    (true                   ),
    VisibleMask_(TEXLAYERFLAG_DEFAULT   )
{
}
TextureLayer::TextureLayer(const ETextureLayerTypes Type) :
    Type_       (Type                   ),
    Texture_    (0                      ),
    LayerIndex_ (0                      ),
    Enabled_    (true                   ),
    VisibleMask_(TEXLAYERFLAG_DEFAULT   )
{
}
TextureLayer::~TextureLayer()
{
}

void TextureLayer::bind() const
{
    if (active())
        Texture_->bind(LayerIndex_);
}

void TextureLayer::unbind() const
{
    if (active())
        Texture_->unbind(LayerIndex_);
}

void TextureLayer::setupDefault() const
{
    // do nothing
}

bool TextureLayer::active() const
{
    return Texture_ != 0 && Enabled_ && (VisibleMask_ & __spVideoDriver->getTexLayerVisibleMask()) != 0;
}

bool TextureLayer::sortCompare(const TextureLayer* Other) const
{
    if (Texture_ != Other->getTexture())
        return reinterpret_cast<long>(Texture_) < reinterpret_cast<long>(Other->getTexture());
    return false;
}

bool TextureLayer::compare(const TextureLayer* Other) const
{
    return Texture_ == Other->getTexture();
}

void TextureLayer::setTexture(Texture* Tex)
{
    if (Texture_ != Tex)
    {
        Texture_ = Tex;
        __spVideoDriver->noticeTextureLayerChanged(this);
    }
}


} // /namespace video

} // /namespace sp



// ================================================================================
