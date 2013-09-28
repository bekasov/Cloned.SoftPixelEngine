/*
 * Texture layer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TEXTURE_LAYER_H__
#define __SP_TEXTURE_LAYER_H__


#include "Base/spStandard.hpp"

#include <boost/shared_ptr.hpp>


namespace sp
{
namespace video
{


class Texture;

//! Standard texture layer types.
enum ETextureLayerTypes
{
    TEXLAYER_BASE,                  //!< Base texture layer. This has only the very basics information for texture mapping. It refers to the "TextureLayer" base class.
    TEXLAYER_STANDARD,              //!< Standard texture layer. This is the default texture layer for mesh objects. It refers to the "TextureLayerStandard" class.
    TEXLAYER_RELIEF,                //!< Texture layer for relief- (or rather parllax-occlusion-) mapping. It refers to the "TextureLayerRelief" class.
    
    TEXLAYER_CUSTOM,                //!< Custom texture layer. If you write your own texture layer, use this type.
    
    TEXLAYER_DEFAULT = 0xFFFFFFFF,  //!< Default texture layer. \see TextureLayer::setDefaultLayerType.
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
        \param[in] Other Pointer to the other texture layer object.
        \see compare
        */
        virtual bool sortCompare(const TextureLayer* Other) const;
        
        /**
        Compares this texture layer with the other one. This should not be used for sorting texture layers.
        \param[in] Other Pointer to the other texture layer object.
        \return True if the other layer is logical equal to this layer.
        \see sortCompare
        */
        virtual bool compare(const TextureLayer* Other) const;
        
        //! Sets the texture object. This may also be null.
        void setTexture(Texture* Tex);
        
        /* === Static functions === */
        
        /**
        Sets the default layer type. The initial default type is TEXLAYER_STANDARD.
        \param[in] Type Specifies the layer type which is to be set as the default one.
        This may only be TEXLAYER_BASE, TEXLAYER_STANDARD or TEXLAYER_RELIEF.
        This value will be used by the MeshBuffer::addTexture and Mesh::addTexture function,
        when the default texture layer class is selected.
        \note If you don't need the extra options in the standard texture layer, use the base texture layer
        as the default one, to increase rendering performance.
        \see getDefaultLayerType
        \see ETextureLayerTypes
        \see MeshBuffer::addTexture
        */
        static void setDefaultLayerType(const ETextureLayerTypes Type);
        //! Returns the default texture layer type. The initial default type is TEXLAYER_STANDARD.
        static ETextureLayerTypes getDefaultLayerType();
        
        /**
        Converts the specified source texture layer to the destination texture layer type.
        \param[out] DestTexLayer Pointer to the converted texture layer. This must not be null!
        \param[in] SrcTexLayer Constant pointer to the source texture layer. This must not be null!
        \note If the texture layer types are not equal or the type is not a pre-defined type,
        just the base data will be copied (i.e. all the data provided by the "TextureLayer" base class).
        \since Version 3.3
        */
        static void convert(TextureLayer* DestTexLayer, const TextureLayer* SrcTexLayer);
        
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
        
        static ETextureLayerTypes DefaultLayerType_;
        
};


//! Texture layer smart pointer.
typedef boost::shared_ptr<TextureLayer> TextureLayerPtr;


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
