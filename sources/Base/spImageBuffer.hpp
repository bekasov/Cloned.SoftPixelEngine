/*
 * Image buffer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_VIDEO_IMAGEBUFFER_H__
#define __SP_VIDEO_IMAGEBUFFER_H__


#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"
#include "Base/spImageManagement.hpp"
#include "Base/spMath.hpp"
#include "Base/spMaterialColor.hpp"
#include "RenderSystem/spTextureFlags.hpp"
#include "FileFormats/Image/spImageFormatInterfaces.hpp"


namespace sp
{
namespace video
{


/**
This is the ImageBuffer class particular used for the Texture image buffer representation.
An ImageBuffer is actually always 2 dimensional. When it is used as a 1 dimensional buffer the vertical axis and the depth are always 0.
When it is used as a 3 dimensional buffer for some functions you have to add (Z * Buffer.Height) to the Y axis.
This is e.g. for the "setBuffer" function where you can set a sub-buffer (individual part area of the whole buffer).
This function only takes a 2D position and size. For e.g. "setPixelColor" you can pass a 1, 2 or 3 dimensional position.
\since Version 3.1
\ingroup group_texture
*/
class SP_EXPORT ImageBuffer
{
    
    public:
        
        virtual ~ImageBuffer();
        
        /* === Functions === */
        
        //! Converts the given float position into the a real coordinate. Will be clamped to the range [0.0 .. 1.0).
        dim::point2di getPixelCoord(const dim::point2df &Pos) const;
        
        //! Sets the pixel color at given position.
        void setPixelColor(const s32 Pos, const color &Color);
        void setPixelColor(const dim::point2di &Pos, const color &Color);
        void setPixelColor(const dim::vector3di &Pos, const color &Color);
        
        //! Sets the pixel color at given position as float vector.
        void setPixelVector(const s32 Pos, const dim::vector4df &Color);
        void setPixelVector(const dim::point2di &Pos, const dim::vector4df &Color);
        void setPixelVector(const dim::vector3di &Pos, const dim::vector4df &Color);
        
        //! Returns pixel color at given position.
        color getPixelColor(const s32 Pos) const;
        color getPixelColor(const dim::point2di &Pos) const;
        color getPixelColor(const dim::vector3di &Pos) const;
        
        //! Returns pixel color at giben position as float vector.
        dim::vector4df getPixelVector(const s32 Pos) const;
        dim::vector4df getPixelVector(const dim::point2di &Pos) const;
        dim::vector4df getPixelVector(const dim::vector3di &Pos) const;
        
        /**
        Returns an interpolated 2D pixel (or rather texel) color.
        This returns always a smoothed color whether buffer type of 'float' or not.
        */
        dim::vector4df getInterpolatedPixel(const dim::point2df &Pos) const;
        
        //! Sets the new image buffer. The giben array must have the size (in bytes) given by "getBufferSize()".
        void setBuffer(const void* ImageBuffer);
        
        //! Returns true if the texture width and height are "Power Of Two" values (2^n: 1, 2, 4, 8, 16 etc.).
        virtual bool isSizePOT() const;
        
        //! Returns Texture size for a "Power Of Two" dimension (e.g. 120 to 128, 260 to 256 etc.).
        virtual dim::size2di getSizePOT() const;
        
        //! Resizes the texture to a "Power Of Two" dimension if it is currently not.
        virtual void setSizePOT();
        
        //! Adjusts the pixel format to be used for Direct3D. This will change RGB to RGBA and BGR to BGRA.
        virtual void adjustFormatD3D();
        
        /* === Abstract functions === */
        
        /**
        Creates a copy of this image buffer and returns the resulting pointer.
        \note Don't forget to delete this object!
        */
        virtual ImageBuffer* copy() const = 0;
        //! Copies the image bufer data into this image buffer. This can also be used to convert from UByte to Float.
        virtual void copy(const ImageBuffer* Other) = 0;
        
        //! Converts the pixel format and returns true if something has changed.
        virtual bool setFormat(const EPixelFormats Format) = 0;
        //! Resizes the image buffer.
        virtual void setSize(const dim::size2di &Size) = 0;
        //! Resizes the image buffer for the depth and returns true on success.
        virtual bool setDepth(u32 Depth) = 0;
        
        //! Inverts all pixel colors.
        virtual void invertColors() = 0;
        //! Flips all pixel colors (from RGB to BGR).
        virtual void flipColors() = 0;
        //! Flips the image on X-axis
        virtual void flipImageHorizontal() = 0;
        //! Flips the image on Y-axis
        virtual void flipImageVertical() = 0;
        //! Turns the image in 90, 180 or 270 degrees
        virtual void turnImage(const EImageTurnDegrees Degree) = 0;
        
        //! Returns the data type size (in bytes). For unsigned bytes it's 1 and for floats it's 4.
        virtual u32 getDataTypeSize() const = 0;
        
        //! Returns pointer to the image buffer stored in the RAM. This can be a UByte or Float pointer.
        virtual void* getBuffer() = 0;
        virtual const void* getBuffer() const = 0;
        
        //! Sets a new sub-buffer.
        virtual void setBuffer(const void* ImageBuffer, const dim::point2di &Pos, const dim::size2di &Size) = 0;
        
        //! Gets a sub-buffer.
        virtual void getBuffer(void* ImageBuffer, const dim::point2di &Pos, const dim::size2di &Size) const = 0;
        
        /**
        Appends the given image buffer to this one. The additional image buffer will be resized to fit into this image buffer.
        \param[in] NewFrame Pointer to the additional image buffer object which is to be appended.
        \param[in] isAppendBottom Specifies whether the additional image buffer is to be appended at the bottom
        or not. If not it will be inserted into the image buffer so that it looks like the image will be
        appended at the right. This is slower than appending it at the end of the buffer! By default true.
        \return True if succeeded. Otherwise the image buffer types are incompatible, i.e. 'Float' and 'UByte'.
        */
        virtual bool appendImageBuffer(const ImageBuffer* AdditionalBuffer, bool isAppendBottom = true) = 0;
        
        /**
        Sets the color key with the given tolerance.
        \param Color: Color which is to be blended out. The alpha channel specifies the transparency of this color.
        e.g. when you have a mesh-wire-fence the color between the mesh-wire (normally black {0|0|0} for green {0|255|0})
        shall be blended out the color key has to be (0, 0, 0, 0) for black or (0, 255, 0, 0) or green.
        \note The color key functions only work for the UBYTE image buffers (ImageBufferUByte).
        */
        virtual void setColorKey(const color &Color, u8 Tolerance = 0);
        //! Sets the color key which is get from the pixel at given position.
        virtual void setColorKey(const dim::point2di &Pos, u8 Alpha = 0, u8 Tolerance = 0);
        virtual void setColorKeyAlpha(const EAlphaBlendingTypes Mode = BLENDING_BRIGHT);
        virtual void setColorKeyMask(ImageBuffer* MaskImage, const EAlphaBlendingTypes Mode = BLENDING_BRIGHT);
        
        //! Converts the gray values to alpha channel. Among others this is used for textured font.
        virtual void grayToAlpha() = 0;
        
        /**
        Creates the buffer and deletes the old one.
        \param[in] InitBuffer Pointer to the buffer which is to be compied. This must point to a memory buffer
        with the same size as the image buffer. Otherwise it must be a null pointer. By default null.
        \see getBufferSize
        */
        virtual void createBuffer(const void* InitBuffer = 0) = 0;
        //! Deletes the image buffer.
        virtual void deleteBuffer() = 0;
        
        /* === Static functions === */
        
        //! Returns the format size of the giben pixel format (1, 2, 3 or 4). The format size is equivalent to the count of color components.
        static u32 getFormatSize(const EPixelFormats Format);
        //! Returns true if the given format has an alpha channel.
        static bool hasAlphaChannel(const EPixelFormats Format);
        
        /* === Inline functions === */
        
        inline EImageBufferTypes getType() const
        {
            return Type_;
        }
        
        //! Returns the 2 dimensional size of the image buffer (Width and Height only).
        inline dim::size2di getSize() const
        {
            return Size_;
        }
        //! Returns the 3 dimensional size of the image buffer (Width, Height and Depth).
        inline dim::vector3di getSizeVector() const
        {
            return dim::vector3di(Size_.Width, Size_.Height, Depth_);
        }
        //! Returns the count of pixels (Width * Height * Depth).
        inline u32 getPixelCount() const
        {
            return getSize().getArea() * Depth_;
        }
        //! Returns the count of bytes for one pixel (FormatSize * DataTypeSize).
        inline u32 getPixelSize() const
        {
            return getFormatSize() * getDataTypeSize();
        }
        //! Returns the count of bytes for the whole image buffer (PixelCount * PixelSize).
        inline u32 getBufferSize() const
        {
            return getPixelCount() * getPixelSize();
        }
        /**
        Returns the image buffer depth. For 1D and 2D textures it's always 1. For 3D textures it can be individual
        and for CubeMaps it's always 6.
        */
        inline u32 getDepth() const
        {
            return Depth_;
        }
        
        //! Returns the pixel format.
        inline EPixelFormats getFormat() const
        {
            return Format_;
        }
        //! Returns the count of components for the internal pixel format.
        inline u32 getFormatSize() const
        {
            return FormatSize_;
        }
        //! Returns true if the image buffer's pixel format has an alpha channel.
        inline bool hasAlphaChannel() const
        {
            return ImageBuffer::hasAlphaChannel(Format_);
        }
        
        //! Returns the color key. This is the color which can be blended out by its alpha channel.
        inline color getColorKey() const
        {
            return ColorKey_;
        }
        
    protected:
        
        ImageBuffer(const EImageBufferTypes Type);
        ImageBuffer(const EImageBufferTypes Type, const EPixelFormats Format, const dim::size2di &Size, u32 Depth = 1);
        
        /* === Functions === */
        
        //! Copies base data and returns true if something has changed that the image buffer must be re-allocated.
        bool copyBase(const ImageBuffer &Other);
        
        /* === Members === */
        
        EImageBufferTypes Type_;
        
        EPixelFormats Format_;  //!< Pixel format (RGB, BGR, RGBA, Gray etc.)
        u32 FormatSize_;        //!< Format size (1, 2, 3 or 4).
        
        dim::size2di Size_;     //!< Image buffer size (Always 2 dimensional).
        u32 Depth_;             //!< Depth (1, 6 or individual).
        
        color ColorKey_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
