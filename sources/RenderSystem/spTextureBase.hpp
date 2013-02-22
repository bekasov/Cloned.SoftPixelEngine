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
        Sets the texture's dimension. The dimension can be: 1D, 2D, 3D or CubeMap. If the dimension is set to a CubeMap
        the texture has 6 faces (+X, -X, +Y, -Y, +Z, -Z). If the dimension is set to a 3D texture "setDepth" or "getDepth"
        functions can be used to configure texture's depth.
        \param Type: Dimension type. There are 1D, 2D, 3D or CubeMap.
        \param Depth: Depth for a 3D texture. By default 1.
        \return True if the dimension could be set. Otherwise the height could not be divided by the specified depth.
        */
        virtual bool setDimension(const ETextureDimensions Type, s32 Depth = 1);
        
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
        it shoudl be always enabled. If MIP-mapping is disabled the texture can look very ugly when the count
        of rendered pixels are considerably less then the count of texels stored in the texture.
        Moreover using MIP-mapping makes the render process faster! This is due to the fact that less memory
        must be transmited for rendering operations. Particularly conspicuous in software rendering.
        However when using render-targets which are updated non-stop a texture without MIP-maps makes sense
        because auto.-MIP-map generation is not very fast with non-stop changes.
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
        
        //! Adds the specified texture to the animation frame list.
        virtual void addAnimFrame(Texture* AnimFrame);
        virtual void removeAnimFrame(Texture* AnimFrame);
        
        //! Enables or disables texture frame animation.
        virtual void setAnimation(bool Enable);
        
        //! Sets the new animation frame.
        virtual void setAnimFrame(u32 Frame);
        
        /**
        Texture's filter mode. By default FILTER_SMOOTH.
        The renderers normally calls this type "linear". But in this engine a "linear" texture
        has none-smoothed texels. This is the second value: FILTER_LINEAR.
        Mag (magnification) is the filter when the texels are bigger then one pixel on the screen.
        Min (minification) is the filter when the texels are smaller then one pixel on the screen (normally using MipMaps).
        */
        virtual void setFilter(const ETextureFilters Filter);
        virtual void setFilter(const ETextureFilters MagFilter, const ETextureFilters MinFilter);
        virtual void setMagFilter(const ETextureFilters Filter);
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
        
        //! Sets a new filename.
        inline void setFilename(const io::stringc &Filename)
        {
            Filename_ = Filename;
        }
        //! Returns filename. This will be set after this texture has been loaded.
        inline io::stringc getFilename() const
        {
            return Filename_;
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
        
        //! Returns the texture dimension. Can be 1-, 2- or 3 dimensional or a cube-map.
        inline ETextureDimensions getDimension() const
        {
            return DimensionType_;
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
            return MultiRenderTargetList_;
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
            return MipMaps_;
        }
        
        //! Returns a reference to the animation frame list.
        inline std::vector<Texture*>& getAnimFrameList()
        {
            return AnimFrameList_;
        }
        
        //! Returns true if this texture has an animation.
        inline bool getAnimation() const
        {
            return isAnim_;
        }
        //! Returns the count of animation frames.
        inline s32 getAnimFrameCount() const
        {
            return AnimFrameList_.size();
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
        
        //! Returns only the magnification filter. By default FILTER_SMOOTH.
        inline ETextureFilters getMagFilter() const
        {
            return MagFilter_;
        }
        //! Returns only the minification filter. By default FILTER_SMOOTH.
        inline ETextureFilters getMinFilter() const
        {
            return MinFilter_;
        }
        //! Returns the MIP-map filter. By default FILTER_TRILINEAR.
        inline ETextureMipMapFilters getMipMapFilter() const
        {
            return MipMapFilter_;
        }
        
        //! Returns the wrap-mode vector. By default TEXWRAP_REPEAT.
        inline dim::vector3d<ETextureWrapModes> getWrapMode() const
        {
            return WrapMode_;
        }
        
        //! Sets the anisotropic filter samples. By default 0 (Modern graphics-cards are able to use 16 samples or more).
        inline void setAnisotropicSamples(s32 Samples)
        {
            AnisotropicSamples_ = Samples;
        }
        //! Returns the anisotropic filter samples.
        inline s32 getAnisotropicSamples() const
        {
            return AnisotropicSamples_;
        }
        
        //! Returns count of multi samples. By default 0.
        inline s32 getMultiSamples() const
        {
            return MultiSamples_;
        }
        
    protected:
        
        /* === Functions === */
        
        virtual void updateMultiRenderTargets();
        
        /* === Members === */
        
        /* Renderer objects */
        void* OrigID_;  // OpenGL (GLuint*), Direct3D9 (SD3D9HWTexture*), Direct3D11 (SD3D11HWTexture*)
        void* ID_;      // Current ID (OrigID_ or AnimTex->OrigID_)
        
        /* Creation flags */
        io::stringc                         Filename_;
        EHWTextureFormats                   HWFormat_;
        ETextureDimensions                  DimensionType_;
        ETextureFilters                     MagFilter_, MinFilter_;
        ETextureMipMapFilters               MipMapFilter_;
        dim::vector3d<ETextureWrapModes>    WrapMode_;
        bool                                MipMaps_;
        
        /* Options */
        s32 AnisotropicSamples_;
        s32 MultiSamples_;
        ECubeMapDirections CubeMapFace_;
        u32 ArrayLayer_;
        
        /* Render target */
        bool isRenderTarget_;
        std::vector<Texture*> MultiRenderTargetList_;
        Texture* DepthBufferSource_;
        
        /* Animation */
        bool isAnim_;
        std::vector<Texture*> AnimFrameList_;
        
        /**
        Used image buffer (pixel buffer)
        This image buffer represents the current texture pixel buffer
        (normal, resized or with manipulated surface)
        */
        ImageBuffer* ImageBuffer_;
        
        /**
        Backup of the current image buffer (by default 0)
        This image buffer can be saved and loaded when changing format, size etc.
        Its format is always RGBA.
        */
        ImageBuffer* ImageBufferBackup_;
        
    private:
        
        /* === Functions === */
        
        void createImageBuffer();
        void createImageBuffer(const STextureCreationFlags &CreationFlags);
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
