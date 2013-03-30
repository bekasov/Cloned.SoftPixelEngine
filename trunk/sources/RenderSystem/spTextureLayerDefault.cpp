/*
 * Default texture layer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spTextureLayerDefault.hpp"
#include "RenderSystem/spRenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


TextureLayerDefault::TextureLayerDefault() :
    TextureLayer    (TEXLAYER_DEFAULT   ),
    MappingCoords_  (MAPGEN_NONE        ),
    MappingGen_     (MAPGEN_DISABLE     ),
    EnvType_        (TEXENV_MODULATE    )
{
}
TextureLayerDefault::~TextureLayerDefault()
{
}

void TextureLayerDefault::bind() const
{
    if (active())
    {
        Texture_->bind(LayerIndex_);
        
        __spVideoDriver->setupTextureLayer(
            LayerIndex_, Matrix_, EnvType_, MappingGen_, MappingCoords_
        );
    }
}

void TextureLayerDefault::unbind() const
{
    if (active())
    {
        Texture_->unbind(LayerIndex_);
        setupDefault();
    }
}

void TextureLayerDefault::setupDefault() const
{
    __spVideoDriver->setupTextureLayer(
        LayerIndex_, dim::matrix4f::IDENTITY, TEXENV_MODULATE, MAPGEN_DISABLE, 0
    );
}

bool TextureLayerDefault::sortCompare(const TextureLayer* Other) const
{
    /* Compare base texture layer and type */
    if (!TextureLayer::compare(Other))
        return TextureLayer::sortCompare(Other);
    
    if (Other->getType() != TEXLAYER_DEFAULT)
        return TEXLAYER_DEFAULT < Other->getType();
    
    /* Compare default texture layer data */
    const TextureLayerDefault* OtherDefault = static_cast<const TextureLayerDefault*>(Other);
    
    if (getMappingGen() != OtherDefault->getMappingGen())
        return getMappingGen() < OtherDefault->getMappingGen();
    if (getTextureEnv() != OtherDefault->getTextureEnv())
        return getTextureEnv() < OtherDefault->getTextureEnv();
    if (getMappingGenCoords() != OtherDefault->getMappingGenCoords())
        return getMappingGenCoords() < OtherDefault->getMappingGenCoords();
    
    return false;
}

bool TextureLayerDefault::compare(const TextureLayer* Other) const
{
    /* Compare base texture layer and type */
    if (Other->getType() != TEXLAYER_DEFAULT || !TextureLayer::compare(Other))
        return false;
    
    /* Compare default texture layer data */
    const TextureLayerDefault* OtherDefault = static_cast<const TextureLayerDefault*>(Other);
    
    return
        getMappingGen() == OtherDefault->getMappingGen() &&
        getTextureEnv() == OtherDefault->getTextureEnv() &&
        getMappingGenCoords() == OtherDefault->getMappingGenCoords();
}

void TextureLayerDefault::setMappingGen(const EMappingGenTypes Type, bool SetCoordsFlags)
{
    MappingGen_ = Type;
    
    if (SetCoordsFlags)
    {
        /* Set mapping generation coordinates flags */
        switch (MappingGen_)
        {
            case video::MAPGEN_DISABLE:
                setMappingGenCoords(MAPGEN_NONE);
                break;
            case video::MAPGEN_OBJECT_LINEAR:
            case video::MAPGEN_EYE_LINEAR:
            case video::MAPGEN_SPHERE_MAP:
            case video::MAPGEN_NORMAL_MAP:
                setMappingGenCoords(MAPGEN_S | MAPGEN_T);
                break;
            case video::MAPGEN_REFLECTION_MAP:
                setMappingGenCoords(MAPGEN_S | MAPGEN_T | MAPGEN_R);
                break;
        }
    }
}


} // /namespace video

} // /namespace sp



// ================================================================================
