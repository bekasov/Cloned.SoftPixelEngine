/*
 * Image buffer container header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_VIDEO_IMAGEBUFFER_CONTAINER_H__
#define __SP_VIDEO_IMAGEBUFFER_CONTAINER_H__


#include "Base/spStandard.hpp"
#include "Base/spImageBuffer.hpp"


namespace sp
{
namespace video
{


template <typename T, s32 DefVal> class ImageBufferContainer : public ImageBuffer
{
    
    public:
        
        virtual ~ImageBufferContainer()
        {
            deleteBuffer();
        }
        
        /* === Functions === */
        
        bool setFormat(const EPixelFormats Format)
        {
            if (Format_ == Format)
                return false;
            
            const u32 NewFormatSize = ImageBuffer::getFormatSize(Format);
            
            if (NewFormatSize == 0)
                return false;
            
            Format_ = Format;
            
            /* Convert the image buffer */
            if (NewFormatSize != FormatSize_ && Buffer_)
            {
                ImageConverter::convertImageFormat<T, DefVal>(
                    Buffer_, getSize().Width, getSize().Height, getFormatSize(), NewFormatSize
                );
                FormatSize_ = NewFormatSize;
            }
            
            return true;
        }
        
        void setSize(const dim::size2di &Size)
        {
            if (Size.Width > 0 && Size.Height > 0 && Size_ != Size)
            {
                if (Buffer_)
                {
                    ImageConverter::scaleImage<T>(
                        Buffer_, getSize().Width, getSize().Height * getDepth(),
                        Size.Width, Size.Height * getDepth(), getFormatSize()
                    );
                }
                Size_ = Size;
            }
        }
        
        bool setDepth(u32 Depth)
        {
            if (Depth_ == Depth)
                return true;
            
            if (Depth >= 1 && getSize().Height % Depth == 0)
            {
                Size_.Height *= Depth_;
                Depth_ = Depth;
                Size_.Height /= Depth_;
                return true;
            }
            
            #ifdef SP_DEBUGMODE
            io::Log::debug("ImageBufferContainer::setDepth", "Setting image buffer depth failed");
            #endif
            
            return false;
        }
        
        void invertColors()
        {
            ImageConverter::invertImageColors<T, DefVal>(Buffer_, getPixelCount() * getFormatSize());
        }
        void flipColors()
        {
            ImageConverter::flipImageColors<T>(Buffer_, getSize().Width, getSize().Height, getFormatSize());
        }
        void flipImageHorizontal()
        {
            ImageConverter::flipImageHorz<T>(Buffer_, getSize().Width, getSize().Height, getFormatSize());
        }
        void flipImageVertical()
        {
            ImageConverter::flipImageVert<T>(Buffer_, getSize().Width, getSize().Height, getFormatSize());
        }
        void turnImage(const EImageTurnDegrees Degree)
        {
            ImageConverter::turnImage<T>(Buffer_, getSize().Width, getSize().Height, getFormatSize(), Degree);
        }
        
        void grayToAlpha()
        {
            setFormat(PIXELFORMAT_RGBA);
            ImageConverter::convertImageGrayToAlpha<T, DefVal>(Buffer_, getSize().Width, getSize().Height);
        }
        
        u32 getDataTypeSize() const
        {
            return sizeof(T);
        }
        
        void* getBuffer()
        {
            return Buffer_;
        }
        const void* getBuffer() const
        {
            return Buffer_;
        }
        
        bool appendImageBuffer(const ImageBuffer* AdditionalBuffer, bool isAppendBottom = true)
        {
            if (AdditionalBuffer && AdditionalBuffer->getType() == getType() && AdditionalBuffer->getFormat() == getFormat())
            {
                const s32 w = AdditionalBuffer->getSize().Width;
                const s32 h = AdditionalBuffer->getSize().Height;
                
                const T* AddRawBuffer = static_cast<const ImageBufferContainer<T, DefVal>*>(AdditionalBuffer)->Buffer_;
                
                if (isAppendBottom)
                {
                    if (ImageConverter::appendImageBufferBottom<T>(Buffer_, AddRawBuffer, getSize().Width, getSize().Height, w, h, getFormatSize()))
                        Size_.Height += h;
                }
                else
                {
                    if (ImageConverter::appendImageBufferRight<T>(Buffer_, AddRawBuffer, getSize().Width, getSize().Height, w, h, getFormatSize()))
                        Size_.Width += w;
                }
                
                return true;
            }
            #ifdef SP_DEBUGMODE
            else
            {
                io::Log::debug(
                    "ImageBufferContainer::appendImageBuffer",
                    "Appending image buffer with incompatible types and/or pixel formats"
                );
            }
            #endif
            
            return false;
        }
        
        virtual void createBuffer(const void* InitBuffer = 0)
        {
            /* Delete the old buffer, create a new one and initialize it */
            deleteBuffer();
            Buffer_ = MemoryManager::createBuffer<T>(getPixelCount() * getFormatSize(), "ImageBufferContainer::Buffer");
            
            /* Initialize the image buffer */
            if (InitBuffer)
                memcpy(Buffer_, InitBuffer, getBufferSize());
            else
                memset(Buffer_, 0, getBufferSize());
        }
        
        virtual void deleteBuffer()
        {
            MemoryManager::deleteBuffer(Buffer_);
        }
        
    protected:
        
        ImageBufferContainer(const EImageBufferTypes Type) :
            ImageBuffer (Type   ),
            Buffer_     (0      )
        {
        }
        ImageBufferContainer(
            const EImageBufferTypes Type, const EPixelFormats Format, const dim::size2di &Size, u32 Depth = 1, const void* InitBuffer = 0) :
            ImageBuffer (Type, Format, Size, Depth  ),
            Buffer_     (0                          )
        {
            createBuffer(InitBuffer);
        }
        
        /* === Members === */
        
        T* Buffer_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
