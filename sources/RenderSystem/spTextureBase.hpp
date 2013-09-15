/*
 * Textrue base header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERSYSTEM_TEXTURE_H__
#define __SP_RENDERSYSTEM_TEXTURE_H__


#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spImageManagement.hpp"
#include "Base/spImageBuffer.hpp"
#include "Base/spBaseObject.hpp"
#include "Base/spMath.hpp"
#include "Base/spMaterialColor.hpp"
#include "Base/spImageBufferUByte.hpp"
#include "Base/spImageBufferFloat.hpp"
#include "RenderSystem/spTextureFlags.hpp"


namespace sp
{
namespace video
{


/**
This is the Texture base class. You only need to use this class (or rather interface). The main content
of this class is an instance of the ImageBuffer class which holds the image data in the RAM which can then be
uploaded to the graphics hardware.
\ingroup group_texture
*/
class SP_EXPORT Texture : public BaseObject
{
    
    public:
        
        Texture();
        Texture(const STextureCreationFlags &CreationFlags);
        virtual ~Texture();
        
        /* === Functions === */
        
        //! Returns true if the texture is correctly created by the used renderer.
        virtual bool valid() const;
        
        //! Save backup from the current image buffer. This can be useful before modifying the image buffer.
        virtual void saveBackup();
        //! Load backup to the current image buffer.
        virtual void loadBackup();
        //! Clear the image buffer backup.
        virtual void clearBackup();
        
        /**
        Enables or disables render target mode for the texture. If enabled the texture's image buffer can not be changed
        by the CPU only by the GPU via rendering into the texture.
        */
        virtual void setRenderTarget(bool Enable);
        
        /**
        Sets the new texture type.
        \param[in] Type Specifies the new texture type.
        If the type is a CubeMap the texture has 6 faces (+X, -X, +Y, -Y, +Z, -Z).
        If the dimension is set to a 3D texture "setDepth" or "getDepth" functions can be used to configure texture's depth.
        \param[in] Depth Specifies the depth for a 3D texture type. By default 1.
        \return True if the type could be set. Otherwise the height could not be divided by the specified depth.
        \see ETextureTypes
        */
        virtual bool setType(const ETextureTypes Type, s32 Depth = 1);
        
        /**
        Sets the current CubeMap render-target face. Use this function to switch between
        the six faces (+X, -X, +Y, -Y, +Z, -Z) when rendering the CubeMap.
        */
        virtual void setCubeMapFace(const ECubeMapDirections Face);
        
        //! Sets the current array layer. This can be used for render targets when rendering into an individual layer (e.g. for texture arrays).
        virtual void setArrayLayer(u32 Layer);
        
        /**
        Adds a new multi-render-target texture. To use several render targets at once you have to add several
        textures to a render-target-texture. e.g. convert a texture to a render target and add several textures.
        */
        virtual void addMultiRenderTarget(Texture* Tex);
        
        //! Removes the specified multi-render-target.
        virtual void removeMultiRenderTarget(Texture* Tex);
        
        //! Clears all multi-render-targets.
        virtual void clearMultiRenderTarget();
        
        /**
        Sets the source texture of the depth buffer for render targets.
        \param DepthBufferSourceTexture: Specifies the texture from which the depth buffer is to be used.
        This needs to be a render target texture. By default 0.
        */
        virtual void setDepthBufferSource(Texture* DepthBufferSourceTexture);
        
        //! Sets the new hardware format type.
        virtual void setHardwareFormat(const EHWTextureFormats HardwareFormat);
        /**
        Enables or disables MIP-mapping. By default MIP-mapping is enabled and in 3D graphics normally
        it should be always enabled. If MIP-mapping is disabled the texture can look very ugly when the number
        of rendered pixels are considerably less then the number of texels stored in the texture.
        Moreover using MIP-mapping makes the render process faster! This is due to the fact that less memory
        must be transmited for rendering operations. Particularly conspicuous in software rendering.
        However when using render-targets which are updated non-stop a texture without MIP-maps makes sense
        to avoid automatic MIP-map generation for every frame. By the way, the name MIP comes from the latin
        "multum in parvo", meaning 'a multitude in a small space' ;-)
        */
        virtual void setMipMapping(bool MipMaps);
        
        //! Copies the hardware image buffer into the texture image buffer.
        virtual bool shareImageBuffer();
        
        //! Copies the texture image buffer into the hardware image buffer (recreating the renderer texture).
        virtual bool updateImageBuffer();
        
        //! Copies only a specified area into the hardware image buffer.
        virtual bool updateImageBuffer(const dim::point2di &Pos, const dim::size2di &Size);
        
        //! Copies the given image buffer into the texture buffer. This image buffer must have the same size (ImageBuffer::getBufferSize()).
        virtual bool setupImageBuffer(const void* NewImageBuffer);
        
        //! Replaces the old image buffer by copying the new one.
        virtual bool setupImageBuffer(const ImageBuffer* NewImageBuffer);
        
        //! Copies the specified area from the image buffer.
        virtual bool setupImageBuffer(const ImageBuffer* SubImageBuffer, const dim::point2di &Position, const dim::size2di &Size);
        
        //! Generates the mipmaps if enabled.
        virtual void generateMipMap();
        
        virtual void bind(s32 Level = 0) const;
        virtual void unbind(s32 Level = 0) const;
        
        /**
        \param[in] ReferenceTexture Pointer to the new texture reference.
        Set this to null to disable referencing.
        \since Version 3.3
        */
        virtual void setReference(Texture* ReferenceTexture);
        
        /**
        Sets the new texture filter configuration. You can also set each filter types individually.
        \see STextureFilter
        \since Version 3.3
        */
        virtual void setFilter(const STextureFilter &Filter);
        
        /**
        Sets the texture filter mode. By default FILTER_SMOOTH.
        The renderers normally calls this type "linear". But in this engine a "linear" texture
        has none-smoothed texels. This is the second value: FILTER_LINEAR.
        Mag (magnification) is the filter when the texels are bigger then one pixel on the screen.
        Min (minification) is the filter when the texels are smaller then one pixel on the screen (normally using MipMaps).
        */
        virtual void setMinMagFilter(const ETextureFilters Filter);
        //! \see setMinMagFilter(const ETextureFilters)
        virtual void setMinMagFilter(const ETextureFilters MagFilter, const ETextureFilters MinFilter);
        //! \see setMinMagFilter(const ETextureFilters)
        virtual void setMagFilter(const ETextureFilters Filter);
        //! \see setMinMagFilter(const ETextureFilters)
        virtual void setMinFilter(const ETextureFilters Filter);
        
        /**
        Texture's mipmap filter. By default FILTER_TRILINEAR. Three filter types are supported: FILTER_BILINEAR,
        FILTER_TRILINEAR and FILTER_ANISOTROPIC. If the filter is FILTER_ANISOTROPIC the anisotropy factor
        can be set by "setAnisotropicSamples". The texture quality grows with these parameters.
        i.e. FILTER_BILINEAR looks worst and FILTER_ANISOTROPIC looks best. This is the method on how
        MIP-mapping is computed. If the texture has no MIP-maps the filter is unnecessary.
        */
        virtual void setMipMapFilter(const ETextureMipMapFilters MipMapFilter);
        
        /**
        Texture coordinate wrap mode. This type says the renderer how to handle the texture coordinate greater then 1.0
        or smaller then 0.0. Three types are supported: TEXWRAP_REPEAT (default value), TEXWRAP_MIRROR, TEXWRAP_CLAMP
        (normall used for SkyBoxes).
        */
        virtual void setWrapMode(const ETextureWrapModes Wrap);
        virtual void setWrapMode(
            const ETextureWrapModes WrapU, const ETextureWrapModes WrapV, const ETextureWrapModes WrapW = TEXWRAP_REPEAT
        );
        
        //! Sets multi samples (for anti-aliased textures).
        virtual void setMultiSamples(s32 Samples);
        
        /**
        Sets the new pixel format of the image buffer. This is equivalent to:
        \code
        Tex->getImageBuffer()->setFormat(Format);
        Tex->updateImageBuffer();
        \endcode
        */
        void setFormat(const EPixelFormats Format);
        
        /**
        Sets the new size of the image buffer. This is equivalent to:
        \code
        Tex->getImageBuffer()->setSize(Size);
        Tex->updateImageBuffer();
        \endcode
        */
        void setSize(const dim::size2di &Size);
        
        /**
        Sets the color key with the given tolerance. This is equivalent to:
        \code
        Tex->getImageBuffer()->setColorKey(Color, Tolerance);
        Tex->updateImageBuffer();
        \endcode
        */
        void setColorKey(const color &Color, u8 Tolerance = 0);
        void setColorKey(const dim::point2di &Pos, u8 Alpha = 0, u8 Tolerance = 0);
        void setColorKeyAlpha(const EAlphaBlendingTypes Mode = BLENDING_BRIGHT);
        void setColorKeyMask(ImageBuffer* MaskImage, const EAlphaBlendingTypes Mode = BLENDING_BRIGHT);
        
        /**
        Ensures that the texture is a POT (power-of-two) texture. This is equivalent to the following code:
        \code
        if (!Tex->getImageBuffer()->isSizePOT())
            Tex->setSize(Tex->getImageBuffer()->getSizePOT());
        \endcode
        */
        void ensurePOT();
        
        /* === Inline functions === */
        
        /**
        Sets the texture's filename. Every loaded (and thus not procedurally generated) texture has it's full path filename by default.
        Since version 3.3 the filename is stored in the object's name member.
        \see BaseObject::setName
        \deprecated Use "BaseObject::setName" instead.
        */
        inline void setFilename(const io::stringc &Filename)
        {
            setName(Filename);
        }
        /**
        Returns the texture's filename. Every loaded (and thus not procedurally generated) texture has it's full path filename by default.
        Since version 3.3 the filename is stored in the object's name member.
        \see BaseObject::getName
        \deprecated Use "BaseObject::getName" instead.
        */
        inline const io::stringc& getFilename() const
        {
            return getName();
        }
        
        //! Returns the pixel format of the image buffer.
        inline EPixelFormats getFormat() const
        {
            return ImageBuffer_->getFormat();
        }
        //! Returns the size of the image buffer.
        inline dim::size2di getSize() const
        {
            return ImageBuffer_->getSize();
        }
        //! Returns the color key of the image buffer.
        inline color getColorKey() const
        {
            return ImageBuffer_->getColorKey();
        }
        
        //! Returns true if this texture is a render-target.
        inline bool getRenderTarget() const
        {
            return isRenderTarget_;
        }
        
        /**
        Returns the texture type (e.g. 1D, 2D, 3D, 2D Array etc.).
        \see ETextureTypes
        */
        inline ETextureTypes getType() const
        {
            return Type_;
        }
        
        //! Returns the current active cube map face. By default CUBEMAP_POSITIVE_X.
        inline ECubeMapDirections getCubeMapFace() const
        {
            return CubeMapFace_;
        }
        //! Returns the current array layer. By default 0.
        inline u32 getArrayLayer() const
        {
            return ArrayLayer_;
        }
        
        //! Returns the multi-render-targets list.
        inline const std::vector<Texture*>& getMultiRenderTargets() const
        {
            return MRTList_;
        }
        
        /**
        Returns pointer to the Texture object which holds the depth buffer. This can be used when several
        render targets shall share the same depth buffer. By default 0 which means the render target has its own depth buffer.
        */
        inline Texture* getDepthBufferSource() const
        {
            return DepthBufferSource_;
        }
        
        //! Returns the hardware texture format.
        inline EHWTextureFormats getHardwareFormat() const
        {
            return HWFormat_;
        }
        
        //! Returns true if MIP-mapping is enabled.
        virtual bool getMipMapping() const
        {
            return Filter_.HasMIPMaps;
        }
        
        //! Returns Texture's current ID. This is the current frame's texture number.
        inline void* getID() const
        {
            return ID_;
        }
        //! Returns Texture's original ID. Normally only used internally for the renderer.
        inline void* getOrigID() const
        {
            return OrigID_;
        }
        
        /**
        Gets the ImageBuffer object. Use this function to get access to the image data.
        e.g. when you want to turn the image write:
        \code
        // Turn the image buffer.
        Tex->getImageBuffer()->turnImage(TURNDEGREE_90);
        // Update the image buffer: this uploades the buffer to the graphics hardware.
        Tex->updateImageBuffer();
        \endcode
        \return Pointer to the ImageBuffer instance.
        */
        inline ImageBuffer* getImageBuffer()
        {
            return ImageBuffer_;
        }
        inline const ImageBuffer* getImageBuffer() const
        {
            return ImageBuffer_;
        }
        
        inline ImageBuffer* getImageBufferBackup()
        {
            return ImageBufferBackup_;
        }
        inline const ImageBuffer* getImageBufferBackup() const
        {
            return ImageBufferBackup_;
        }
        
        //! Returns the texture filter settings. \see STextureFilter
        inline const STextureFilter& getFilter() const
        {
            return Filter_;
        }
        
        //! Returns only the magnification filter. By default FILTER_SMOOTH. \see ETextureFilters
        inline ETextureFilters getMagFilter() const
        {
            return Filter_.Mag;
        }
        //! Returns only the minification filter. By default FILTER_SMOOTH. \see ETextureFilters
        inline ETextureFilters getMinFilter() const
        {
            return Filter_.Min;
        }
        //! Returns the MIP-map filter. By default FILTER_TRILINEAR. \see ETextureMipMapFilters
        inline ETextureMipMapFilters getMipMapFilter() const
        {
            return Filter_.MIPMap;
        }
        
        //! Returns the wrap-mode vector. By default TEXWRAP_REPEAT. \see ETextureWrapModes
        inline const dim::vector3d<ETextureWrapModes>& getWrapMode() const
        {
            return Filter_.WrapMode;
        }
        
        //! Sets the anisotropic filter samples. By default 0 (Modern graphics-cards are able to use 16 samples or more).
        inline void setAnisotropicSamples(s32 Samples)
        {
            Filter_.Anisotropy = Samples;
        }
        //! Returns the anisotropic filter samples.
        inline s32 getAnisotropicSamples() const
        {
            return Filter_.Anisotropy;
        }
        
        //! Returns count of multi samples. By default 0.
        inline s32 getMultiSamples() const
        {
            return MultiSamples_;
        }
        
        /**
        Returns true if this texture has RW access on a pixel- or compute shader,
        i.e. the type of this texture is TEXTURE_1D_RW, TEXTURE_2D_RW, etc.
        \see ETextureTypes
        \since Version 3.3
        */
        inline bool hasRWAccess() const
        {
            return Type_ >= TEXTURE_1D_RW && Type_ <= TEXTURE_2D_ARRAY_RW;
        }
        
    protected:
        
        /* === Functions === */
        
        virtual void updateMultiRenderTargets();
        
        /* === Members === */
        
        /* Renderer objects */
        void* OrigID_;                      //!< Original renderer texture ID (OpenGL (GLuint*), Direct3D9 (SD3D9HWTexture*), Direct3D11 (SD3D11HWTexture*)).
        void* ID_;                          //!< Active renderer texture ID ('OrigID_' or 'AnimTex->OrigID_').
        
        /* Creation flags */
        ETextureTypes Type_;                //!< Texture class type. \see ETextureTypes
        EHWTextureFormats HWFormat_;        //!< Hardware texture format. \see EHWTextureFormats
        STextureFilter Filter_;             //!< Texture filtering settings. \see STextureFilter
        
        /* Options */
        s32 MultiSamples_;                  //!< Number of multi-samples.
        ECubeMapDirections CubeMapFace_;    //!< Active cube-map face. \see ECubeMapDirections
        u32 ArrayLayer_;                    //!< Active array-texture layer.
        
        /* Render target */
        bool isRenderTarget_;               //!< Specifies whether this texture is a render-target or not.
        std::vector<Texture*> MRTList_;     //!< List of multi-render-target textures.
        std::vector<Texture*> MRTRefList_;  //!< Reference list from multi-render-target textures. It conatins all textures which are using this texture as MRT entry.
        Texture* DepthBufferSource_;        //!< Depth-buffer source texture. By default null.
        
        /**
        Image- (or rather texel-) buffer object.
        This image buffer contains only texels for the first MIP-map level.
        \see ImageBuffer
        */
        ImageBuffer* ImageBuffer_;
        
        /**
        Backup of the active image buffer. By default null.
        This image buffer can be saved and loaded when changing format, size etc.
        Its format is always RGBA.
        \see ImageBuffer
        */
        ImageBuffer* ImageBufferBackup_;
        
    private:
        
        /* === Functions === */
        
        void createImageBuffer();
        void createImageBuffer(const STextureCreationFlags &CreationFlags);
        
        void clearMRTList();
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
