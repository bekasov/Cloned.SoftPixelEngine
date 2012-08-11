/*
 * Shader builder header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHADER_BUILDER_H__
#define __SP_SHADER_BUILDER_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"

#include <list>


namespace sp
{
namespace video
{


//! \todo This is unfinished
namespace ShaderBuilder
{

/*
 * Enumerations
 */

enum EGBufferFlags
{
    GBUFFERSHADER_FLAG_POM = 0x00000001, //!< Adds parallax occlusion mapping. Requires additionally a height-map.
};

enum EDeferredShaderFlags
{
    DEFERREDSHADER_FLAG_USE_SPECULAR                = 0x00000001, //!< Adds specular to the light-computations.
    DEFERREDSHADER_FLAG_USE_SPECULAR_MATERIAL       = 0x00000002, //!< Adds specular exponent uniform to the material settings.
    DEFERREDSHADER_FLAG_USE_POINT_SHADOW_MAPPING    = 0x00000004, //!< Adds texture array for point light shadow maps.
    DEFERREDSHADER_FLAG_USE_SPOT_SHADOW_MAPPING     = 0x00000008, //!< Adds texture array for spot light shadow maps.
};

/*
 * Global functions
 */

SP_EXPORT void buildGBufferShaderCode(s32 Flags, std::list<io::stringc> &VertexShaderCode, std::list<io::stringc> &PixelShaderCode);
SP_EXPORT void buildDeferredShaderCode(s32 Flags, std::list<io::stringc> &VertexShaderCode, std::list<io::stringc> &PixelShaderCode);

} // /namespace ShaderBuilder


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
