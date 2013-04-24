/*
 * Textrue flags header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERSYSTEM_TEXTUREFLAGS_H__
#define __SP_RENDERSYSTEM_TEXTUREFLAGS_H__


#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"
#include "Base/spInputOutputString.hpp"
#include "FileFormats/Image/spImageFormatInterfaces.hpp"


namespace sp
{
namespace video
{


/*
 * Enumerations
 */

//! Texture minification- and magnification filters.
enum ETextureFilters
{
    FILTER_LINEAR,          //!< Linear texture sampling ('retro style').
    FILTER_SMOOTH,          //!< Smooth texture sampling.
};

//! Texture mip-map filters.
enum ETextureMipMapFilters
{
    FILTER_BILINEAR,        //!< Bilinear filtering.
    FILTER_TRILINEAR,       //!< Trilinear filtering (default).
    FILTER_ANISOTROPIC,     //!< Anisotropic filtering.
};

//! Texture coordinate wrap modes.
enum ETextureWrapModes
{
    TEXWRAP_REPEAT,         //!< Texture coordinates are not wraped (default).
    TEXWRAP_MIRROR,         //!< Texture coordinates are mirrored.
    TEXWRAP_CLAMP,          //!< Texture coordinates are clamped in the range [0.0 .. 1.0) (e.g. used for sky-box and cube-map textures).
};

//! Texture dimensions.
enum ETextureDimensions
{
    /* Basic textures */
    TEXTURE_1D = 1,         //!< 1 dimensional textures (e.g. for Cel-/ Toon Shading).
    TEXTURE_2D,             //!< 2 dimensional textures (default dimension).
    TEXTURE_3D,             //!< 3 dimensional textures (with depth information).
    TEXTURE_CUBEMAP,        //!< Cube map textures (6 sides: +X, -X, +Y, -Y, +Z, -Z).
    
    /* Array textures */
    TEXTURE_1D_ARRAY,       //!< 1 dimensional array textures. Can have several layers.
    TEXTURE_2D_ARRAY,       //!< 2 dimensional array textures. Can have several layers.
    TEXTURE_CUBEMAP_ARRAY,  //!< Cube map array textures. Can have several layers.

    /* Advanced shader textures */
    TEXTURE_RECTANGLE,      //!< 2 dimensional rectengular textures (texture coordinates will not be normalized in a shader). \since Version 3.3
    TEXTURE_BUFFER,         //!< 1 dimensional texture buffer (for large buffers in a shader as an alternative to constant buffers). \since Version 3.3
};

//! Internal renderer image buffer format for textures.
enum EHWTextureFormats
{
    HWTEXFORMAT_UBYTE8,     //!< 8-bit unsigned byte color components (for PIXELFORMAT_DEPTH 16 bit).
    HWTEXFORMAT_FLOAT16,    //!< 16-bit floating point color components (for PIXELFORMAT_DEPTH 24 bit).
    HWTEXFORMAT_FLOAT32,    //!< 32-bit floating point color components (for PIXELFORMAT_DEPTH 32 bit).
};

//! Cubemap directions
enum ECubeMapDirections
{
    CUBEMAP_POSITIVE_X = 0, //!< Position X direction (+X).
    CUBEMAP_NEGATIVE_X,     //!< Negative X direction (-X).
    CUBEMAP_POSITIVE_Y,     //!< Position Y direction (+Y).
    CUBEMAP_NEGATIVE_Y,     //!< Negative Y direction (-Y).
    CUBEMAP_POSITIVE_Z,     //!< Position Z direction (+Z).
    CUBEMAP_NEGATIVE_Z      //!< Negative Z direction (-Z).
};

//! Alpha blending types for per-pixel blending in image buffers.
enum EAlphaBlendingTypes
{
    BLENDING_BRIGHT,        //!< Bright colors are opaque and dark colors are transparent.
    BLENDING_DARK           //!< Dark colors are opaque and bright colors are transparent.
};

//! Image buffer types.
enum EImageBufferTypes
{
    IMAGEBUFFER_UBYTE,      //!< Unsigned byte (8 bit) for each color component.
    IMAGEBUFFER_FLOAT,      //!< Floating point (32 bit) for each color component.
};

//! Data types for the renderer (vertex- and index buffer).
enum ERendererDataTypes
{
    DATATYPE_FLOAT,             //!< 32bit floating-point.
    DATATYPE_DOUBLE,            //!< 64bit fliating-point. This data type is not supported for OpenGL|ES.
    DATATYPE_BYTE,              //!< 8bit integer.
    DATATYPE_SHORT,             //!< 16bit integer.
    DATATYPE_INT,               //!< 32bit integer.
    DATATYPE_UNSIGNED_BYTE,     //!< 8bit unsigned integer.
    DATATYPE_UNSIGNED_SHORT,    //!< 16bit unsigned integer.
    DATATYPE_UNSIGNED_INT,      //!< 32bit unsigned integer. This data type is not supported for OpenGL|ES.
};


/*
 * Structures
 */

//! Texture creation flag structure. This will be used to initialize a texture object.
struct STextureCreationFlags
{
    STextureCreationFlags() :
        Depth       (1                  ),
        ImageBuffer (0                  ),
        BufferType  (IMAGEBUFFER_UBYTE  ),
        Dimension   (TEXTURE_2D         ),
        Format      (PIXELFORMAT_RGB    ),
        HWFormat    (HWTEXFORMAT_UBYTE8 ),
        MagFilter   (FILTER_SMOOTH      ),
        MinFilter   (FILTER_SMOOTH      ),
        MipMapFilter(FILTER_TRILINEAR   ),
        MipMaps     (true               ),
        Anisotropy  (1                  ),
        WrapMode    (TEXWRAP_REPEAT     )
    {
    }
    STextureCreationFlags(const STextureCreationFlags &other) :
        Filename    (other.Filename     ),
        Size        (other.Size         ),
        Depth       (other.Depth        ),
        ImageBuffer (other.ImageBuffer  ),
        BufferType  (other.BufferType   ),
        Dimension   (other.Dimension    ),
        Format      (other.Format       ),
        HWFormat    (other.HWFormat     ),
        MagFilter   (other.MagFilter    ),
        MinFilter   (other.MinFilter    ),
        MipMapFilter(other.MipMapFilter ),
        MipMaps     (other.MipMaps      ),
        Anisotropy  (other.Anisotropy   ),
        WrapMode    (other.WrapMode     )
    {
    }
    ~STextureCreationFlags()
    {
    }
    
    /* Members */
    io::stringc Filename;
    dim::size2di Size;
    s32 Depth;
    
    const void* ImageBuffer;
    
    EImageBufferTypes BufferType;
    ETextureDimensions Dimension;
    EPixelFormats Format;
    EHWTextureFormats HWFormat;
    ETextureFilters MagFilter, MinFilter;
    ETextureMipMapFilters MipMapFilter;
    
    bool MipMaps;
    s32 Anisotropy;
    dim::vector3d<ETextureWrapModes> WrapMode;
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
