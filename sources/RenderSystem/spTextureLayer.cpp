/*
 * Texture layer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spTextureLayer.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "RenderSystem/spTextureLayerStandard.hpp"
#include "RenderSystem/spTextureLayerRelief.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


ETextureLayerTypes TextureLayer::DefaultLayerType_ = TEXLAYER_STANDARD;

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
    return Texture_ != 0 && Enabled_ && (VisibleMask_ & GlbRenderSys->getTexLayerVisibleMask()) != 0;
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
        GlbRenderSys->noticeTextureLayerChanged(this);
    }
}

void TextureLayer::setDefaultLayerType(const ETextureLayerTypes Type)
{
    if (Type >= TEXLAYER_BASE && Type <= TEXLAYER_RELIEF)
        DefaultLayerType_ = Type;
}
//! Returns the default texture layer type. The initial default type is TEXLAYER_STANDARD.
ETextureLayerTypes TextureLayer::getDefaultLayerType()
{
    return DefaultLayerType_;
}

void TextureLayer::convert(TextureLayer* DestTexLayer, const TextureLayer* SrcTexLayer)
{
    /* Validate parameters */
    if (!DestTexLayer || !SrcTexLayer)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("TextureLayer::convert");
        #endif
        return;
    }
    
    /* Copy base data */
    DestTexLayer->setTexture    (SrcTexLayer->getTexture    ());
    DestTexLayer->setIndex      (SrcTexLayer->getIndex      ());
    DestTexLayer->setEnable     (SrcTexLayer->getEnable     ());
    DestTexLayer->setVisibleMask(SrcTexLayer->getVisibleMask());
    
    /* Convert data */
    if (DestTexLayer->getType() == SrcTexLayer->getType())
    {
        switch (DestTexLayer->getType())
        {
            case TEXLAYER_STANDARD:
            {
                TextureLayerStandard* DestTexLayerStd = dynamic_cast<TextureLayerStandard*>(DestTexLayer);
                const TextureLayerStandard* SrcTexLayerStd = dynamic_cast<const TextureLayerStandard*>(SrcTexLayer);
                
                if (DestTexLayerStd && SrcTexLayerStd)
                {
                    DestTexLayerStd->setMatrix          (SrcTexLayerStd->getMatrix          ());
                    DestTexLayerStd->setTextureEnv      (SrcTexLayerStd->getTextureEnv      ());
                    DestTexLayerStd->setMappingGen      (SrcTexLayerStd->getMappingGen      ());
                    DestTexLayerStd->setMappingGenCoords(SrcTexLayerStd->getMappingGenCoords());
                }
            }
            break;
            
            case TEXLAYER_RELIEF:
            {
                TextureLayerRelief* DestTexLayerRlf = dynamic_cast<TextureLayerRelief*>(DestTexLayer);
                const TextureLayerRelief* SrcTexLayerRlf = dynamic_cast<const TextureLayerRelief*>(SrcTexLayer);
                
                if (DestTexLayerRlf && SrcTexLayerRlf)
                {
                    DestTexLayerRlf->setReliefEnable    (SrcTexLayerRlf->getReliefEnable    ());
                    DestTexLayerRlf->setMinSamples      (SrcTexLayerRlf->getMinSamples      ());
                    DestTexLayerRlf->setMaxSamples      (SrcTexLayerRlf->getMaxSamples      ());
                    DestTexLayerRlf->setHeightMapScale  (SrcTexLayerRlf->getHeightMapScale  ());
                    DestTexLayerRlf->setViewRange       (SrcTexLayerRlf->getViewRange       ());
                }
            }
            break;
            
            default:
            break;
        }
    }
}


} // /namespace video

} // /namespace sp



// ================================================================================
