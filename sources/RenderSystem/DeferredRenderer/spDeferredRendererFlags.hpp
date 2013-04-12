/*
 * Deferred renderer flags header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DEFERRED_RENDERER_FLAGS_H__
#define __SP_DEFERRED_RENDERER_FLAGS_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "Base/spMaterialConfigTypes.hpp"


namespace sp
{
namespace video
{


/**
Deferred renderer flags. Use this for the "DeferredRenderer::generateResources" function.
\see DeferredRenderer
*/
enum EDeferredRenderFlags
{
    /**
    Makes use of the texture transformation matrix. This will be used for the
    diffuse-, specular-, normal- and height map.
    */
    DEFERREDFLAG_USE_TEXTURE_MATRIX         = 0x0001,
    /**
    Enables individual specular map usage. If this option is enabled every model
    must have an additional texture (layer 1) with specular information.
    */
    DEFERREDFLAG_HAS_SPECULAR_MAP           = 0x0002,
    /**
    Enables individual light map usage. If this option is enabled every model must have
    an addition texture (layer 1 if there is no specular map, otherwise 2) with illumination data.
    Those lightmaps can be generated with the "SoftPixel Sandbox" or rather the lightmap-generator from this engine.
    */
    DEFERREDFLAG_HAS_LIGHT_MAP              = 0x0004,
    //! Allows the lighting to over-blend. By default disabled.
    DEFERREDFLAG_ALLOW_OVERBLENDING         = 0x0008,
    
    /**
    Enables normal-mapping. If this option is enabled every model must have an additional
    texture (layer 1 if there is no specular map and no light map, otherwise 2 or 3) with normal vector information.
    */
    DEFERREDFLAG_NORMAL_MAPPING             = 0x0010,
    /**
    Enables parallax-occlusion mapping. If this option is enabled every model must have an additional
    texture (layer 2 if there is no specular map and no light map, otherwise 3 or 4) with height map information.
    This can be a gray-scaled texture. If the DEFERREDFLAG_NORMALMAP_XYZ_H option is enabled,
    no height map is needed. In that case the height map information is get from the normal map's alpha channel.
    This requires normal-mapping (DEFERREDFLAG_NORMAL_MAPPING).
    */
    DEFERREDFLAG_PARALLAX_MAPPING           = 0x0020,
    /**
    Enables the normal map to also contain the height map data in the alpha channel. This this option is enabled
    no height map texture is used. This requires parallax-mapping (DEFERREDFLAG_PARALLAX_MAPPING).
    */
    DEFERREDFLAG_NORMALMAP_XYZ_H            = 0x0040,
    /**
    Enables shadow mapping. For this technique "variance shadow mapping" (VSM)
    is used for performance reasons.
    */
    DEFERREDFLAG_SHADOW_MAPPING             = 0x0080,
    /**
    Enables global illumination. For this technique "reflective shadow maps" are used.
    This requires shadow-mapping (DEFERREDFLAG_SHADOW_MAPPING).
    \note This is a very time consuming process. It is recommendable to use only one
    light source with this technique.
    \todo This is still in progress!
    */
    DEFERREDFLAG_GLOBAL_ILLUMINATION        = 0x0100,
    //! Enables the bloom effect. All glossy surfaces glow intensely.
    DEFERREDFLAG_BLOOM                      = 0x0200,
    
    #if 0
    /**
    Enables height-field tessellation. This can not be used together
    with parallax-mapping (DEFERREDFLAG_PARALLAX_MAPPING).
    */
    DEFERREDFLAG_TESSELLATION               = 0x0400,
    #endif
    
    /**
    This option can be used for debugging purposes. It renders the final image as four viewports
    containing the color buffer output, normal buffer output, depth buffer output and the final image.
    */
    DEFERREDFLAG_DEBUG_GBUFFER              = 0x1000,
    /**
    This option can be used for debugging purposes. It renders the
    world-position for each pixel instead of its distance to the view camera.
    This requires gbuffer debugging (DEFERREDFLAG_DEBUG_GBUFFER).
    */
    DEFERREDFLAG_DEBUG_GBUFFER_WORLDPOS     = 0x2000,
    /**
    This option can be used for debugging purposes. It renders the
    texture coordinates as colors for each pixel instead of the diffuse colors.
    This requires gbuffer debugging (DEFERREDFLAG_DEBUG_GBUFFER).
    */
    DEFERREDFLAG_DEBUG_GBUFFER_TEXCOORDS    = 0x4000,
    /**
    This option can be used for debugging purposes. It renders all
    virtual points lights as small colored cubes. These virtual points lights
    are generated from the reflective shadow maps.
    This requires global illumination (DEFERREDFLAG_GLOBAL_ILLUMINATION).
    */
    DEFERREDFLAG_DEBUG_VIRTUALPOINTLIGHTS   = 0x8000,
};


/**
Texture layer model for deferred-renderer. Use this structure to determine how the texture layers are constructed.
If a texture layer has the value of TEXTURE_IGNORE this layer is not used. By default every layer has the value of TEXTURE_IGNORE.
*/
struct STextureLayerModel
{
    STextureLayerModel() :
        DiffuseMap  (TEXTURE_IGNORE),
        SpecularMap (TEXTURE_IGNORE),
        LightMap    (TEXTURE_IGNORE),
        NormalMap   (TEXTURE_IGNORE),
        HeightMap   (TEXTURE_IGNORE)
    {
    }
    ~STextureLayerModel()
    {
    }
    
    /* Functions */
    void clear()
    {
        DiffuseMap  = TEXTURE_IGNORE;
        SpecularMap = TEXTURE_IGNORE;
        LightMap    = TEXTURE_IGNORE;
        NormalMap   = TEXTURE_IGNORE;
        HeightMap   = TEXTURE_IGNORE;
    }
    
    /* Members */
    u8 DiffuseMap;  //!< Diffuse map layer. Commonly 0.
    u8 SpecularMap; //!< Diffuse map layer. Commonly 1 or TEXTURE_IGNORE.
    u8 LightMap;    //!< Diffuse map layer. Commonly 1, 2 or TEXTURE_IGNORE.
    u8 NormalMap;   //!< Diffuse map layer. Commonly 1, 2, 3 or TEXTURE_IGNORE.
    /**
    Diffuse map layer. Commonly 1, 2, 3, 4 or TEXTURE_IGNORE.
    If the height-map information is stored in the alpha-channel
    of the normal-maps, this layer has the same value as the "NormalMap" layer.
    */
    u8 HeightMap;
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
