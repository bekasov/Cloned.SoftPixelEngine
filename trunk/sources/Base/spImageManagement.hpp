/*
 * Image buffer management header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_IMAGEMANAGEMENT_H__
#define __SP_IMAGEMANAGEMENT_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutput.hpp"
#include "Base/spMath.hpp"
#include "Base/spMaterialColor.hpp"
#include "FileFormats/Image/spImageFormatInterfaces.hpp"

#include <cstdio>


namespace sp
{
namespace video
{


/*
 * Macros
 */

#define __SP_CONVERT_1_TO_2(src, dest, i, j)            \
    dest[j+0] = src[i+0];                               \
    dest[j+1] = DefVal;
#define __SP_CONVERT_1_TO_3(src, dest, i, j)            \
    dest[j+0] = src[i+0];                               \
    dest[j+1] = src[i+0];                               \
    dest[j+2] = src[i+0];
#define __SP_CONVERT_1_TO_4(src, dest, i, j)            \
    dest[j+0] = src[i+0];                               \
    dest[j+1] = src[i+0];                               \
    dest[j+2] = src[i+0];                               \
    dest[j+3] = DefVal;

#define __SP_CONVERT_2_TO_1(src, dest, i, j)            \
    dest[j+0] = src[i+0];
#define __SP_CONVERT_2_TO_3(src, dest, i, j)            \
    dest[j+0] = src[i+0];                               \
    dest[j+1] = src[i+0];                               \
    dest[j+2] = src[i+0];
#define __SP_CONVERT_2_TO_4(src, dest, i, j)            \
    dest[j+0] = src[i+0];                               \
    dest[j+1] = src[i+0];                               \
    dest[j+2] = src[i+0];                               \
    dest[j+3] = src[i+1];

#define __SP_CONVERT_3_TO_1(src, dest, i, j)            \
    dest[j+0] = ( src[i+0] + src[i+1] + src[i+2] ) / 3;
#define __SP_CONVERT_3_TO_2(src, dest, i, j)            \
    dest[j+0] = ( src[i+0] + src[i+1] + src[i+2] ) / 3; \
    dest[j+1] = DefVal;
#define __SP_CONVERT_3_TO_4(src, dest, i, j)            \
    dest[j+0] = src[i+0];                               \
    dest[j+1] = src[i+1];                               \
    dest[j+2] = src[i+2];                               \
    dest[j+3] = DefVal;

#define __SP_CONVERT_4_TO_1(src, dest, i, j)            \
    dest[j+0] = ( src[i+0] + src[i+1] + src[i+2] ) / 3;
#define __SP_CONVERT_4_TO_2(src, dest, i, j)            \
    dest[j+0] = ( src[i+0] + src[i+1] + src[i+2] ) / 3; \
    dest[j+1] = src[i+3];
#define __SP_CONVERT_4_TO_3(src, dest, i, j)            \
    dest[j+0] = src[i+0];                               \
    dest[j+1] = src[i+1];                               \
    dest[j+2] = src[i+2];


/*
 * Enumerations
 */

//! Methods of image data rotation
enum EImageTurnDegrees
{
    TURNDEGREE_90,  //!< Turn 90 degrees.
    TURNDEGREE_180, //!< Turn 180 degrees.
    TURNDEGREE_270, //!< Turn 270 degrees.
};


/*
 * Structures
 */

class Texture;

//! Heightmap texture structure for more precision.
struct SP_EXPORT SHeightMapTexture
{
    SHeightMapTexture();
    SHeightMapTexture(const Texture* Tex);
    ~SHeightMapTexture();
    
    /* Functions */
    
    //! Creates the buffer out of the texture's image buffer.
    void createBuffer(const Texture* Tex);
    
    //! Creates the buffer by copying the input buffer. Don't forget to delete the input buffer!
    void createBuffer(const dim::size2di &NewSize, s32 NewFormat, const f32* NewImageBuffer);
    
    //! Clears (or rather deletes) the buffer.
    void clearBuffer();
    
    /**
    Determines the height value at a specified position. The height value will be interpolated to
    have a smooth result with small heightmaps, too.
    \param Pos: Position where the height value shall be sampled (in a range or [0.0, 1.0]).
    \return Height value which is to be determined.
    */
    f32 getHeightValue(const dim::point2df &Pos) const;
    
    /**
    Calculates the normal vector at a specified position and adjustment.
    \param Pos: Position where the normal vector shall be calculated.
    \param Adjustment: Adjustment (or rather offset) to get the area of the face which normal shall be calculated.
    \return Calculated normal vector.
    */
    dim::vector3df getNormal(const dim::point2df &Pos, const dim::point2df &Adjustment) const;
    
    /* Members */
    
    dim::size2di Size;  //!< Heightmap texture size.
    f32* ImageBuffer;   //!< Floating-point array for more precision in height-mapping (array size = Size.Width * Size.Height).
};


//! ImageConverter namespace used for image buffer manipulation. This is particular used in the Texture class (e.g. for "setFormat", "setSize" etc.).
namespace ImageConverter
{

/*
 * Template declarations
 */

//! Inverts the colors.
template <typename T, s32 DefVal> void invertImageColors(T* ImageBuffer, u32 ImageBufferSize);

//! Flips the color values (BGR -> RGB)
template <typename T> void flipImageColors(T* ImageBuffer, s32 Width, s32 Height, s32 FormatSize);

//! Flips the image data on the x-axis
template <typename T> void flipImageHorz(T* ImageBuffer, s32 Width, s32 Height, s32 FormatSize);

//! Flips the image data on the y-axis
template <typename T> void flipImageVert(T* ImageBuffer, s32 Width, s32 Height, s32 FormatSize);

//! Scales the image to a new size
template <typename T> void scaleImage(T* &ImageBuffer, s32 Width, s32 Height, s32 NewWidth, s32 NewHeight, s32 FormatSize);

//! Scales the image to a half size (smooth)
template <typename T> void halveImage(T* &ImageBuffer, s32 Width, s32 Height, s32 FormatSize);

//! Converts the image data formats (e.g. RGB -> RGBA)
template <typename T, s32 DefVal> void convertImageFormat(T* &ImageBuffer, s32 Width, s32 Height, s32 OldFormatSize, s32 NewFormatSize);

//! Blurs the image data (for mipmap levels)
template <typename T> void blurImage(T* &ImageBuffer, s32 Width, s32 Height, s32 FormatSize);

//! Turns the image data in 90°, 180° or 270°
template <typename T> void turnImage(T* ImageBuffer, s32 Width, s32 Height, s32 FormatSize, const EImageTurnDegrees Degree);

/**
Appends the given frame image buffer to the base image buffer. If the new frame's size does not fit
it will be scaled and the pixel format will be adjusted as well.
\param ImageBuffer: Specifies the base image buffer. This buffer will be re-allocated.
\param NewFrame: Specifies the frame image buffer which is to be appended.
\param Width: Width of the base image buffer.
\param Height: Height of the base image buffer.
\param FrameWidth: Width of the frame image buffer.
\param FrameHeight: Height of the frame image buffer.
\param FormatSize: Format size of the base image buffer (1, 2, 3 or 4).
\param FrameFormatSize: Format size of the frame image buffer (1, 2, 3 or 4).
\param isAppendBottom: Specifies whether the frame buffer is to be appended to the bottom or to the right.
By default bottom which means the new buffer will be appended directly after the base buffer.
Otherwise it will be intergrated (slower).
*/
template <typename T> void appendImageFrame(
    T* &ImageBuffer, const T* NewFrame, s32 Width, s32 Height,
    s32 FrameWidth, s32 FrameHeight, s32 FormatSize, s32 FrameFormatSize,
    bool isAppendBottom = true
);

/**
Sets the color key of the given image buffer. The format size must be 4.
\param ImageBuffer: Specifies the image buffer which is to be modified.
\param Width: Width of the image buffer.
\param Height: Height of the image buffer.
\param Color: Specifies the color key. Each pixel with the same RGB color components will get the new alpha channel (Color.Alpha).
\param Tolerance: Specifies the tolerance factor. Each pixel will get the new alpha channel when the RGB color components
are in the same range (if PixelColor = ColorKey +/- Tolerance). By default 0.
*/
SP_EXPORT void setImageColorKey(u8* ImageBuffer, s32 Width, s32 Height, const video::color &Color, s32 Tolerance = 0);

//! Get the correct image size (2^x)
SP_EXPORT bool checkImageSize(dim::size2di &InputSize);

//! Get the count of mipmap levels
SP_EXPORT s32 getMipmapLevelsCount(s32 Width, s32 Height);


/*
 * Template definitions
 */

template <typename T, s32 DefVal> void invertImageColors(T* ImageBuffer, u32 ImageBufferSize)
{
    if (ImageBuffer)
    {
        for (u32 i = 0; i < ImageBufferSize; ++i)
            ImageBuffer[i] = static_cast<T>(DefVal) - ImageBuffer[i];
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("ImageConverter::invertImageColors");
    #endif
}

template <typename T> void flipImageColors(T* ImageBuffer, s32 Width, s32 Height, s32 FormatSize)
{
    if ( ImageBuffer && Width > 0 && Height > 0 && ( FormatSize == 3 || FormatSize == 4 ) )
    {
        const u32 ImageBufferSize = Width * Height * FormatSize;
        
        for (u32 i = 0; i < ImageBufferSize; i += FormatSize)
            math::Swap<T>(ImageBuffer[i], ImageBuffer[i + 2]);
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("ImageConverter::flipImageColors");
    #endif
}

template <typename T> void flipImageHorz(T* ImageBuffer, s32 Width, s32 Height, s32 FormatSize)
{
    /* Check for valid parameter values */
    if (!ImageBuffer || Width <= 0 || Height <= 0 || FormatSize < 1 || FormatSize > 4)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ImageConverter::flipImageHorz");
        #endif
        return;
    }
    
    /* Temporary variables */
    const s32 Pitch     = Width * FormatSize;
    const s32 HalfPitch = Pitch/2;
    
    /* Loop the array */
    for (s32 y = 0, x; y < Height; ++y)
    {
        for (x = 0; x < HalfPitch; x += 3)
        {
            math::Swap<T>(ImageBuffer[ y*Pitch + ( Pitch - x - 1 ) - 0 ], ImageBuffer[ y*Pitch + x + 2 ]);
            math::Swap<T>(ImageBuffer[ y*Pitch + ( Pitch - x - 1 ) - 1 ], ImageBuffer[ y*Pitch + x + 1 ]);
            math::Swap<T>(ImageBuffer[ y*Pitch + ( Pitch - x - 1 ) - 2 ], ImageBuffer[ y*Pitch + x + 0 ]);
        }
    }
}

template <typename T> void flipImageVert(T* ImageBuffer, s32 Width, s32 Height, s32 FormatSize)
{
    /* Check for valid parameter values */
    if (!ImageBuffer || Width <= 0 || Height <= 0 || FormatSize < 1 || FormatSize > 4)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ImageConverter::flipImageVert");
        #endif
        return;
    }
    
    /* Temporary variables */
    const s32 Pitch         = Width*FormatSize;
    const s32 HalfHeight    = Height/2;
    
    /* Loop the array */
    for (s32 y = 0, x; y < HalfHeight; ++y)
    {
        for (x = 0; x < Pitch; ++x)
            math::Swap<T>(ImageBuffer[ (Height - y - 1)*Pitch + x ], ImageBuffer[ y*Pitch + x ]);
    }
}

template <typename T> void scaleImage(T* &ImageBuffer, s32 Width, s32 Height, s32 NewWidth, s32 NewHeight, s32 FormatSize)
{
    /* Check if the memory is not empty */
    if ( !ImageBuffer || Width <= 0 || Height <= 0 || NewWidth <= 0 || NewHeight <= 0 ||
         FormatSize < 1 || FormatSize > 4 || ( Width == NewWidth && Height == NewHeight ) )
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ImageConverter::scaleImage");
        #endif
        return;
    }
    
    /* Temporary variables */
    s32 x, y, i, j, k;
    
    /* Allocate new memory */
    const u32 ImageBufferSize = NewWidth * NewHeight * FormatSize;
    
    T* NewImageBuffer = new T[ImageBufferSize];
    
    /* Loop for the new image size */
    for (y = 0; y < NewHeight; ++y)
    {
        for (x = 0; x < NewWidth; ++x)
        {
            for (i = 0; i < FormatSize; ++i)
            {
                /* Get the image data indices */
                j = y * NewWidth + x;
                k = ( y * Height / NewHeight ) * Width + ( x * Width / NewWidth );
                
                /* Fill the new image data */
                NewImageBuffer[ j * FormatSize + i ] = ImageBuffer[ k * FormatSize + i ];
            }
        }
    }
    
    /* Delete the old memory */
    delete [] ImageBuffer;
    
    /* Use the new memory */
    ImageBuffer = NewImageBuffer;
}

template <typename T> void halveImage(T* &ImageBuffer, s32 Width, s32 Height, s32 FormatSize)
{
    /* Check for valid parameter values */
    if (!ImageBuffer || Width <= 0 || Height <= 0 || FormatSize < 1 || FormatSize > 4)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ImageConverter::halveImage");
        #endif
        return;
    }
    
    /* Temporary variables */
    s32 x, y, i, x2, y2;
    
    /* New image data size */
    s32 NewWidth    = Width;
    s32 NewHeight   = Height;
    
    /* Change the new size */
    if (NewWidth > 1)
        NewWidth >>= 1;
    if (NewHeight > 1)
        NewHeight >>= 1;
    
    /* Save the old image data */
    T* OldImageBuffer = ImageBuffer;
    
    /* Allocate new image data */
    ImageBuffer = new T[ NewWidth * NewHeight * FormatSize ];
    
    if (Width > 1 && Height > 1)
    {
        
        /* Loop for the half image size */
        for (y = y2 = 0; y < NewHeight; ++y, y2 = y << 1)
        {
            for (x = x2 = 0; x < NewWidth; ++x, x2 = x << 1)
            {
                for (i = 0; i < FormatSize; ++i)
                {
                    /* Fill the current new data elements */
                    ImageBuffer[ ( y * NewWidth + x ) * FormatSize + i ] = (
                        OldImageBuffer[ ( (y2    ) * Width + (x2    ) ) * FormatSize + i ] +
                        OldImageBuffer[ ( (y2    ) * Width + (x2 + 1) ) * FormatSize + i ] +
                        OldImageBuffer[ ( (y2 + 1) * Width + (x2    ) ) * FormatSize + i ] +
                        OldImageBuffer[ ( (y2 + 1) * Width + (x2 + 1) ) * FormatSize + i ]
                    ) / 4;
                } // next color component
            } // next pixel
        } // next line
        
    }
    else if (Width == 1 || Height == 1)
    {
        
        s32 MaxSize = math::Max(NewWidth, NewHeight);
        
        /* Loop for the half image size */
        for (x = 0, x2 = 0; x < MaxSize; ++x, x2 = x << 1)
        {
            for (i = 0; i < FormatSize; ++i)
            {
                /* Fill the current new data elements */
                ImageBuffer[ x * FormatSize + i ] = (
                    OldImageBuffer[ ( x2     ) * FormatSize + i ] +
                    OldImageBuffer[ ( x2 + 1 ) * FormatSize + i ]
                ) / 2;
            } // next color component
        } // next pixel
        
    }
    
    /* Delete the old image data */
    delete [] OldImageBuffer;
}

template <typename T, s32 DefVal> void convertImageFormat(T* &ImageBuffer, s32 Width, s32 Height, s32 OldFormatSize, s32 NewFormatSize)
{
    /* Check if the memory is not empty */
    if (!ImageBuffer || Width <= 0 || Height <= 0 || OldFormatSize < 1 || OldFormatSize > 4 ||
        NewFormatSize < 1 || NewFormatSize > 4 || OldFormatSize == NewFormatSize)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ImageConverter::convertImageFormat");
        #endif
        return;
    }
    
    /* Temporary variables */
    s32 x, y, i = 0, j = 0;
    
    /* Allocate new memory */
    T* NewImageBuffer = new T[ Width * Height * NewFormatSize ];
    
    /* Loop for the image size */
    for (y = 0; y < Height; ++y)
    {
        for (x = 0; x < Width; ++x, i += OldFormatSize, j += NewFormatSize)
        {
            
            /* Select the source's internal format */
            switch (OldFormatSize)
            {
                case 1:
                    switch (NewFormatSize)
                    {
                        case 2: __SP_CONVERT_1_TO_2(ImageBuffer, NewImageBuffer, i, j); break;
                        case 3: __SP_CONVERT_1_TO_3(ImageBuffer, NewImageBuffer, i, j); break;
                        case 4: __SP_CONVERT_1_TO_4(ImageBuffer, NewImageBuffer, i, j); break;
                    }
                    break;
                    
                case 2:
                    switch (NewFormatSize)
                    {
                        case 1: __SP_CONVERT_2_TO_1(ImageBuffer, NewImageBuffer, i, j); break;
                        case 3: __SP_CONVERT_2_TO_3(ImageBuffer, NewImageBuffer, i, j); break;
                        case 4: __SP_CONVERT_2_TO_4(ImageBuffer, NewImageBuffer, i, j); break;
                    }
                    break;
                    
                case 3:
                    switch (NewFormatSize)
                    {
                        case 1: __SP_CONVERT_3_TO_1(ImageBuffer, NewImageBuffer, i, j); break;
                        case 2: __SP_CONVERT_3_TO_2(ImageBuffer, NewImageBuffer, i, j); break;
                        case 4: __SP_CONVERT_3_TO_4(ImageBuffer, NewImageBuffer, i, j); break;
                    }
                    break;
                    
                case 4:
                    switch (NewFormatSize)
                    {
                        case 1: __SP_CONVERT_4_TO_1(ImageBuffer, NewImageBuffer, i, j); break;
                        case 2: __SP_CONVERT_4_TO_2(ImageBuffer, NewImageBuffer, i, j); break;
                        case 3: __SP_CONVERT_4_TO_3(ImageBuffer, NewImageBuffer, i, j); break;
                    }
                    break;
            } // /switch
            
        } // next pixel
    } // next line
    
    /* Delete the old memory */
    delete [] ImageBuffer;
    
    /* Use the new memory */
    ImageBuffer = NewImageBuffer;
}

template <typename T> void blurImage(T* &ImageBuffer, s32 Width, s32 Height, s32 FormatSize)
{
    /* Check if the iamge data is not empty */
    if (!ImageBuffer || Width <= 0 || Height <= 0 || FormatSize < 1 || FormatSize > 4)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ImageConverter::blurImage");
        #endif
        return;
    }
    
    /* Temporary variables */
    s32 y, x, i;
    
    /* Allocate new memory */
    T* NewImageBuffer = new T[ Width * Height * FormatSize ];
    
    /* Loop the image data (main body) */
    for (y = 0; y < Height - 1; ++y)
    {
        for (x = 0; x < Width - 1; ++x)
        {
            for (i = 0; i < FormatSize; ++i)
            {
                /* Fill the new image data */
                NewImageBuffer[ ( y * Width + x ) * FormatSize + i ] = (
                    ImageBuffer[ ( y * Width + x       ) * FormatSize + i ] + 
                    ImageBuffer[ ( y * Width + x+1     ) * FormatSize + i ] + 
                    ImageBuffer[ ( (y+1) * Width + x+1 ) * FormatSize + i ] + 
                    ImageBuffer[ ( (y+1) * Width + x   ) * FormatSize + i ]
                ) / 4;
            }
        } // next pixel
    } // next line
    
    /* Loop the image data (edge - x axis) */
    for (x = 0; x < Width - 1; ++x)
    {
        for (i = 0; i < FormatSize; ++i)
        {
            /* Fill the new image data */
            NewImageBuffer[ ( (Height-1) * Width + x ) * FormatSize + i ] = (
                ImageBuffer[ ( (Height-1) * Width + x   ) * FormatSize + i ] + 
                ImageBuffer[ ( (Height-1) * Width + x+1 ) * FormatSize + i ] + 
                ImageBuffer[ ( x+1                      ) * FormatSize + i ] + 
                ImageBuffer[ ( x                        ) * FormatSize + i ]
            ) / 4;
        }
    } // next pixel
    
    /* Loop the image data (edge - y axis) */
    for (y = 0; y < Height - 1; ++y)
    {
        for (i = 0; i < FormatSize; ++i)
        {
            /* Fill the new image data */
            NewImageBuffer[ ( y * Width + (Width-1) ) * FormatSize + i ] = (
                ImageBuffer[ ( y * Width + (Width-1)     ) * FormatSize + i ] + 
                ImageBuffer[ ( y * Width                 ) * FormatSize + i ] + 
                ImageBuffer[ ( (y+1) * Width + (Width-1) ) * FormatSize + i ] + 
                ImageBuffer[ ( (y+1) * Width             ) * FormatSize + i ]
            ) / 4;
        }
    } // next pixel
    
    /* Loop the image data (edge right bottom) */
    for (i = 0; i < FormatSize; ++i)
    {
        /* Fill the new image data */
        NewImageBuffer[ ( (Height-1) * Width + (Width-1) ) * FormatSize + i ] = (
            ImageBuffer[ ( (Height-1) * Width + (Width-1) ) * FormatSize + i ] + 
            ImageBuffer[ ( (Height-1) * Width             ) * FormatSize + i ] + 
            ImageBuffer[                                                       i ] + 
            ImageBuffer[ ( Width-1                        ) * FormatSize + i ]
        ) / 4;
    }
    
    /* Delete the old memory */
    delete [] ImageBuffer;
    
    /* Use the new memory */
    ImageBuffer = NewImageBuffer;
}

template <typename T> void turnImage(T* ImageBuffer, s32 Width, s32 Height, s32 FormatSize, const EImageTurnDegrees Degree)
{
    if (!ImageBuffer || Width <= 0 || Height <= 0 || FormatSize < 1 || FormatSize > 4)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ImageConverter::turnImage");
        #endif
        return;
    }
    
    /* Temporary variables */
    s32 x, y, i, j = 0, k;
    
    /* Get image buffer size */
    const u32 ImageBufferSize = Width * Height * FormatSize;
    
    /* Allocate temporary memory */
    T* TmpImageBuffer = new T[ImageBufferSize];
    
    /* Copy the old image buffer temporary */
    memcpy(TmpImageBuffer, ImageBuffer, ImageBufferSize);
    
    /* Select the degree type */
    switch (Degree)
    {
        case TURNDEGREE_90:
        {
            for (y = 0; y < Height; ++y)
            {
                for (x = 0, i = Width*(Height - 1) + y; x < Width; ++x, i -= Width, ++j)
                {
                    for (k = 0; k < FormatSize; ++k)
                        ImageBuffer[i*FormatSize + k] = TmpImageBuffer[j*FormatSize + k];
                }
            }
        }
        break;
        
        case TURNDEGREE_180:
        {
            for (j = 0, i = Width*Height - 1; j < Width*Height; --i, ++j)
            {
                for (k = 0; k < FormatSize; ++k)
                    ImageBuffer[i*FormatSize + k] = TmpImageBuffer[j*FormatSize + k];
            }
        }
        break;
        
        case TURNDEGREE_270:
        {
            for (y = 0; y < Height; ++y)
            {
                for (x = 0, i = Width - 1 - y; x < Width; ++x, i += Width, ++j)
                {
                    for (k = 0; k < FormatSize; ++k)
                        ImageBuffer[i*FormatSize + k] = TmpImageBuffer[j*FormatSize + k];
                }
            }
        }
        break;
    }
    
    /* Delete the temporary memory */
    delete [] TmpImageBuffer;
}

/**
Copies a part area from the SubBuffer to the ImageBuffer.
\param DestBuffer: Destination image buffer.
\param SrcBuffer: Source image buffer.
\param DestSize: Destination image buffer size.
\param FormatSize: Format size of both destination- and source buffer. Must be 1, 2, 3 or 4.
\param Pos: Position where the source buffer is to be copied into the destination buffer.
\param Size: Size of the source image buffer.
*/
template <typename T> void copySubBufferToBuffer(
    T* DestBuffer, const T* SrcBuffer, const dim::size2di &DestSize, u32 FormatSize, const dim::point2di &Pos, const dim::size2di &Size)
{
    /* Check parameter validity */
    if ( !DestBuffer || !SrcBuffer || FormatSize < 1 || FormatSize > 4 ||
         DestSize.Width <= 0 || DestSize.Height <= 0 ||
         Pos.X < 0 || Pos.Y < 0 || Size.Width <= 0 || Size.Height <= 0 ||
         Pos.X + Size.Width > DestSize.Width || Pos.Y + Size.Height > DestSize.Height )
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ImageConverter::copySubBufferToBuffer");
        #endif
        return;
    }
    
    /* Temporary memory */
    const u32 PixelSize = sizeof(T) * FormatSize;
    
    DestBuffer += (Pos.Y * DestSize.Width + Pos.X) * FormatSize;
    
    if (Size == dim::size2di(1))
        memcpy(DestBuffer, SrcBuffer, PixelSize);
    else if (DestSize.Width == Size.Width)
        memcpy(DestBuffer, SrcBuffer, PixelSize * Size.getArea());
    else
    {
        const u32 SrcLineSize   = FormatSize * Size.Width;
        const u32 DestLineSize  = FormatSize * DestSize.Width;
        
        /* Copy each image line */
        for (s32 y = Pos.Y; y < Pos.Y + Size.Height; ++y)
        {
            memcpy(DestBuffer, SrcBuffer, SrcLineSize);
            
            DestBuffer  += DestLineSize;
            SrcBuffer   += SrcLineSize;
        }
    }
}

template <typename T> void copyBufferToSubBuffer(
    T* DestBuffer, const T* SrcBuffer, const dim::size2di &SrcSize, u32 FormatSize, const dim::point2di &Pos, const dim::size2di &Size)
{
    /* Check parameter validity */
    if ( !DestBuffer || !SrcBuffer || FormatSize < 1 || FormatSize > 4 ||
         SrcSize.Width <= 0 || SrcSize.Height <= 0 ||
         Pos.X < 0 || Pos.Y < 0 || Size.Width <= 0 || Size.Height <= 0 ||
         Pos.X + Size.Width > SrcSize.Width || Pos.Y + Size.Height > SrcSize.Height )
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ImageConverter::copyBufferToSubBuffer");
        #endif
        return;
    }
    
    /* Temporary memory */
    const u32 PixelSize = sizeof(T) * FormatSize;
    
    SrcBuffer += (Pos.Y * SrcSize.Width + Pos.X) * FormatSize;
    
    if (Size == dim::size2di(1))
        memcpy(DestBuffer, SrcBuffer, PixelSize);
    else if (SrcSize.Width == Size.Width)
        memcpy(DestBuffer, SrcBuffer, PixelSize * Size.getArea());
    else
    {
        const u32 SrcLineSize   = FormatSize * SrcSize.Width;
        const u32 DestLineSize  = FormatSize * Size.Width;
        
        for (s32 y = Pos.Y; y < Pos.Y + Size.Height; ++y)
        {
            memcpy(DestBuffer, SrcBuffer, SrcLineSize);
            
            DestBuffer  += DestLineSize;
            SrcBuffer   += SrcLineSize;
        }
    }
}

//! \todo
template <typename T> void appendImageFrame(
    T* &ImageBuffer, const T* NewFrame, s32 Width, s32 Height, s32 FrameWidth,
    s32 FrameHeight, s32 FormatSize, s32 FrameFormatSize, bool isAppendBottom)
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("ImageConverter::appendImageFrame", "Not implemented yet");
    #endif
}

} // /namespace ImageConverter


/*
 * Undefine macros
 */

#undef __SP_CONVERT_1_TO_2
#undef __SP_CONVERT_1_TO_3
#undef __SP_CONVERT_1_TO_4

#undef __SP_CONVERT_2_TO_1
#undef __SP_CONVERT_2_TO_3
#undef __SP_CONVERT_2_TO_4

#undef __SP_CONVERT_3_TO_1
#undef __SP_CONVERT_3_TO_2
#undef __SP_CONVERT_3_TO_4

#undef __SP_CONVERT_4_TO_1
#undef __SP_CONVERT_4_TO_2
#undef __SP_CONVERT_4_TO_3


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
