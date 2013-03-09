/*
 * Texture layer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TEXTURE_LAYER_H__
#define __SP_TEXTURE_LAYER_H__


#include "Base/spStandard.hpp"


namespace sp
{
namespace video
{


class Texture;

//! Standard texture layer types.
enum ETextureLayerTypes
{
    TEXLAYER_BASE,      //!< Base texture layer. This has only the very basics information for texture mapping. It refers to the "TextureLayer" base class.
    TEXLAYER_DEFAULT,   //!< Default texture layer. This is the default texture layer for mesh objects. It refers to the "TextureLayerDefault" class.
    TEXLAYER_RELIEF,    //!< Texture layer for relief- (or rather parllax-occlusion-) mapping. It refers to the "TextureLayerRelief" class.
    
    TEXLAYER_CUSTOM,    //!< Custom texture layer. If you write your own texture layer, use this type.
};

/**
This enumeration contains all default texture-layer visiblity masks. You can also use your
own bit masks, but it is recommended to use this default masks, so that your texture-layer
lists work quite well with default sub render systems (such as the deferred-renderer).
*/
enum ETexLayerVisibleFlags
{
    TEXLAYERFLAG_DIFFUSE    = 0x00000001, //!< The texture layer contains a diffuse map.
    TEXLAYERFLAG_NORMAL     = 0x00000002, //!< The texture layer contains a normal map.
    TEXLAYERFLAG_SPECULAR   = 0x00000004, //!< The texture layer contains a specular map.
    TEXLAYERFLAG_HEIGHT     = 0x00000008, //!< The texture layer contains a height map.
    
    TEXLAYERFLAG_DEFAULT    = 0xFFFFFFFF, //!< Default texture layer flag. This is 0xFFFFFFFF.
};


/**
Base texture layer class. Can also be used as stand alone.
\since Version 3.2
\ingroup group_texture
*/
class SP_EXPORT TextureLayer
{
    
    public:
        
        TextureLayer();
        virtual ~TextureLayer();
        
        /* === Functions === */
        
        virtual void bind() const;
        virtual void unbind() const;
        
        /**
        Sets up the default settings for this layer type. This is used to increase performance by
        only deactivating texture-layer settings when another layer is activated with a different type
        of the previous one.
        */
        virtual void setupDefault() const;
        
        /**
        Returns true if this texture layer is active. That depends on whether a valid texture
        object has been set, the layer is enabled and the rule mask matches the global render-system
        rule mask for texture mapping.
        \see setTexture
        \see setEnable
        \see setRuleMask
        */
        virtual bool active() const;
        
        /**
        Compares this texture layer with the other one. This is used for sorting texture layers.
        \return True if the other layer is virtually identical to this layer.
        */
        virtual bool compare(const TextureLayer* Other) const;
        
        //! Sets the texture object. This may also be null.
        void setTexture(Texture* Tex);
        
        /* === Inline functions === */
        
        inline ETextureLayerTypes getType() const
        {
            return Type_;
        }
        
        //! Returns the texture object. By default null.
        inline Texture* getTexture() const
        {
            return Texture_;
        }
        
        /**
        Sets the layer index.
        \note The layer index should be unique within a mesh buffer.
        Remember that whenever you change the layer index using this function.
        */
        inline void setIndex(u8 LayerIndex)
        {
            LayerIndex_ = LayerIndex;
        }
        //! Returns the layer index.
        inline u8 getIndex() const
        {
            return LayerIndex_;
        }
        
        /**
        Enables or disables the texture layer. This can also be done dynamically with the rule mask.
        \see setRuleMask
        */
        inline void setEnable(bool Enable)
        {
            Enabled_ = Enable;
        }
        //! Returns true if the texture layer is enabled. By default true.
        inline bool getEnable() const
        {
            return Enabled_;
        }
        
        /**
        Sets the visibility mask. This can be used to hide bunches of texture layers quickly,
        e.g. when shaders are disabled for debugging purposes. The default bit mask is 0xFFFFFFFF.
        \see ETexLayerVisibleFlags
        */
        inline void setVisibleMask(s32 Mask)
        {
            VisibleMask_ = Mask;
        }
        /**
        Returns the default rule mask. For more information about this take a
        look at the "setRuleMask" function. The default value is 0xFFFFFFFF.
        \see setRuleMask
        */
        inline s32 getVisibleMask() const
        {
            return VisibleMask_;
        }
        
    protected:
        
        TextureLayer(const ETextureLayerTypes Type);
        
        /* === Members === */
        
        ETextureLayerTypes Type_;
        
        Texture* Texture_;
        u8 LayerIndex_;
        
        bool Enabled_;
        s32 VisibleMask_;   //!< Visibility bit mask.
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
