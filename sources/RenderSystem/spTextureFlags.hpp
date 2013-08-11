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

//! Texture class types.
enum ETextureTypes
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
    
    /* R/W textures */
    TEXTURE_1D_RW,          //!< 1 dimensional texture with read-/ write access in pixel and compute shaders. \since Version 3.3
    TEXTURE_2D_RW,          //!< 2 dimensional texture with read-/ write access in pixel and compute shaders. \since Version 3.3
    TEXTURE_3D_RW,          //!< 3 dimensional texture with read-/ write access in pixel and compute shaders. \since Version 3.3
    TEXTURE_1D_ARRAY_RW,    //!< 1 dimensional array texture with read-/ write access in pixel and compute shaders. Can have several layers. \since Version 3.3
    TEXTURE_2D_ARRAY_RW,    //!< 2 dimensional array texture with read-/ write access in pixel and compute shaders. Can have several layers. \since Version 3.3
};

//! Internal renderer image buffer format for textures.
enum EHWTextureFormats
{
    HWTEXFORMAT_UBYTE8,     //!< 8-bit unsigned byte color components (for PIXELFORMAT_DEPTH 16 bit). \see EPixelFormats
    HWTEXFORMAT_FLOAT16,    //!< 16-bit floating point color components (for PIXELFORMAT_DEPTH 24 bit). \see EPixelFormats
    HWTEXFORMAT_FLOAT32,    //!< 32-bit floating point color components (for PIXELFORMAT_DEPTH 32 bit). \see EPixelFormats
    HWTEXFORMAT_INT32,      //!< 32-bit integer components. This can only be used for texture buffers (TEXTURE_BUFFER)! \see ETextureTypes \since Version 3.3
    HWTEXFORMAT_UINT32,     //!< 32-bit unsigned interger components. \see EPixelFormats \since Version 3.3
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
    DATATYPE_UNSIGNED_BYTE,     //!< 8bit unsigned integer. \todo Rename to "DATATYPE_UBYTE".
    DATATYPE_UNSIGNED_SHORT,    //!< 16bit unsigned integer. \todo Rename to "DATATYPE_USHORT".
    DATATYPE_UNSIGNED_INT,      //!< 32bit unsigned integer. This data type is not supported for OpenGL|ES. \todo Rename to "DATATYPE_UINT".
};


/*
 * Structures
 */

/**
Texture filtering structure. Contains all members to configure texture filtering.
\since Version 3.3
*/
struct STextureFilter
{
    STextureFilter() :
        HasMIPMaps  (true               ),
        Min         (FILTER_SMOOTH      ),
        Mag         (FILTER_SMOOTH      ),
        MIPMap      (FILTER_TRILINEAR   ),
        WrapMode    (TEXWRAP_REPEAT     ),
        Anisotropy  (1                  )
    {
    }
    STextureFilter(const STextureFilter &Other) :
        HasMIPMaps  (Other.HasMIPMaps   ),
        Min         (Other.Min          ),
        Mag         (Other.Mag          ),
        MIPMap      (Other.MIPMap       ),
        WrapMode    (Other.WrapMode     ),
        Anisotropy  (Other.Anisotropy   )
    {
    }
    ~STextureFilter()
    {
    }
    
    /* Members */
    bool HasMIPMaps;                            //!< Specifies whether MIP-maps are used or not. By default true.
    ETextureFilters Min;                        //!< Minification filter. By default FILTER_SMOOTH. \see ETextureFilters
    ETextureFilters Mag;                        //!< Magnification filter. By default FILTER_SMOOTH. \see ETextureFilters
    ETextureMipMapFilters MIPMap;               //!< MIP-mapping filter. By default FILTER_TRILINEAR. \see ETextureMipMapFilters
    dim::vector3d<ETextureWrapModes> WrapMode;  //!< Wrap mode. \see ETextureWrapModes
    s32 Anisotropy;                             //!< Number of anisotropic samples.
};

//! Texture creation flag structure. This will be used to initialize a texture object.
struct STextureCreationFlags
{
    STextureCreationFlags() :
        Depth       (1                  ),
        ImageBuffer (0                  ),
        Type        (TEXTURE_2D         ),
        BufferType  (IMAGEBUFFER_UBYTE  ),
        Format      (PIXELFORMAT_RGB    ),
        HWFormat    (HWTEXFORMAT_UBYTE8 ),
        Filter      (                   )
    {
    }
    STextureCreationFlags(const STextureCreationFlags &Other) :
        Filename    (Other.Filename     ),
        Size        (Other.Size         ),
        Depth       (Other.Depth        ),
        ImageBuffer (Other.ImageBuffer  ),
        Type        (Other.Type         ),
        BufferType  (Other.BufferType   ),
        Format      (Other.Format       ),
        HWFormat    (Other.HWFormat     ),
        Filter      (Other.Filter       )
    {
    }
    ~STextureCreationFlags()
    {
    }
    
    /* === Inline functions === */
    //! Returns the texture size as 3D vector.
    inline dim::vector3di getSizeVec() const
    {
        return dim::vector3di(Size.Width, Size.Height, Depth);
    }
    
    /* Members */
    io::stringc         Filename;
    dim::size2di        Size;
    s32                 Depth;
    
    const void*         ImageBuffer;
    
    ETextureTypes       Type;
    EImageBufferTypes   BufferType;
    EPixelFormats       Format;
    EHWTextureFormats   HWFormat;
    
    STextureFilter      Filter;
    
    #if 0//!to be removed!
    ETextureFilters MagFilter, MinFilter;
    ETextureMipMapFilters MipMapFilter;
    bool MipMaps;
    s32 Anisotropy;
    dim::vector3d<ETextureWrapModes> WrapMode;
    #endif
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
