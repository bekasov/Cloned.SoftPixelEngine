/*
 * Shader configuration types header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHADER_CONFIG_TYPES_H__
#define __SP_SHADER_CONFIG_TYPES_H__


#include "Base/spStandard.hpp"

#include <boost/function.hpp>
#include <vector>


namespace sp
{

namespace scene { class MaterialNode; };

namespace video
{


/*
 * Enumerations
 */

//! Shader types.
enum EShaderTypes
{
    /* Dummy enum constant */
    SHADER_DUMMY,           //!< Dummy shader when shaders are not supported.
    
    /* Legacy ASM shaders */
    SHADER_VERTEX_PROGRAM,  //!< Vertex program since Direct3D 8 and OpenGL 1.3.
    SHADER_PIXEL_PROGRAM,   //!< Pixel program since Direct3D 8 and OpenGL 1.3.
    
    /* Graphics shader pipeline */
    SHADER_VERTEX,          //!< Vertex shader since Direct3D 9 and OpenGL 2.
    SHADER_PIXEL,           //!< Pixel shader since Direct3D 9 and OpenGL 2.
    SHADER_GEOMETRY,        //!< Geometry shader since Direct3D 10 and OpenGL 3.2.
    SHADER_HULL,            //!< Hull shader (for tessellation) since Direct3D 11 and OpenGL 4.
    SHADER_DOMAIN,          //!< Domain shader (for tessellation) since Direct3D 11 and OpenGL 4.
    
    /* Compute shader pipeline */
    SHADER_COMPUTE,         //!< DirectCompute shader since Direct3D 11 and OpenGL 4.3.
};

//! Shader constant data types.
enum EConstantTypes
{
    /*
    !TODO!
    CONSTANT_FLOAT,
    CONSTANT_FLOAT2,
    CONSTANT_FLOAT3,
    CONSTANT_FLOAT4,
    CONSTANT_FLOAT2x2,
    CONSTANT_FLOAT3x3,
    CONSTANT_FLOAT4x4,
    CONSTANT_INT,
    CONSTANT_INT2,
    CONSTANT_INT3,
    CONSTANT_INT4,
    */
    CONSTANT_UNKNOWN,   //!< Unknown constant type.
    CONSTANT_BOOL,      //!< Single Boolean.
    CONSTANT_INT,       //!< Single Integer.
    CONSTANT_FLOAT,     //!< Single float.
    CONSTANT_VECTOR2,   //!< 2D float vector.
    CONSTANT_VECTOR3,   //!< 3D float vector.
    CONSTANT_VECTOR4,   //!< 4D float vector.
    CONSTANT_MATRIX2,   //!< 2x2 float matrix.
    CONSTANT_MATRIX3,   //!< 3x3 float matrix.
    CONSTANT_MATRIX4,   //!< 4x4 float matrix.
    CONSTANT_STRUCT,    //!< Custom structure.
};

//! Shader model versions.
enum EShaderVersions
{
    DUMMYSHADER_VERSION,
    
    GLSL_VERSION_1_20,      //!< GLSL Shader v.1.20.
    GLSL_VERSION_1_30,      //!< GLSL Shader v.1.30.
    GLSL_VERSION_1_40,      //!< GLSL Shader v.1.40.
    GLSL_VERSION_1_50,      //!< GLSL Shader v.1.50.
    GLSL_VERSION_3_30_6,    //!< GLSL Shader v.3.30.6.
    GLSL_VERSION_4_00_8,    //!< GLSL Shader v.4.00.8.
    
    HLSL_VERTEX_1_0,        //!< HLSL Vertex Shader v.1.0.
    HLSL_VERTEX_2_0,        //!< HLSL Vertex Shader v.2.0.
    HLSL_VERTEX_2_a,        //!< HLSL Vertex Shader v.2.a.
    HLSL_VERTEX_3_0,        //!< HLSL Vertex Shader v.3.0.
    HLSL_VERTEX_4_0,        //!< HLSL Vertex Shader v.4.0.
    HLSL_VERTEX_4_1,        //!< HLSL Vertex Shader v.4.1.
    HLSL_VERTEX_5_0,        //!< HLSL Vertex Shader v.5.0.
    
    HLSL_PIXEL_1_0,         //!< HLSL Pixel Shader v.1.0.
    HLSL_PIXEL_1_1,         //!< HLSL Pixel Shader v.1.1.
    HLSL_PIXEL_1_2,         //!< HLSL Pixel Shader v.1.2.
    HLSL_PIXEL_1_3,         //!< HLSL Pixel Shader v.1.3.
    HLSL_PIXEL_1_4,         //!< HLSL Pixel Shader v.1.4.
    HLSL_PIXEL_2_0,         //!< HLSL Pixel Shader v.2.0.
    HLSL_PIXEL_2_a,         //!< HLSL Pixel Shader v.2.a.
    HLSL_PIXEL_2_b,         //!< HLSL Pixel Shader v.2.b.
    HLSL_PIXEL_3_0,         //!< HLSL Pixel Shader v.3.0.
    HLSL_PIXEL_4_0,         //!< HLSL Pixel Shader v.4.0.
    HLSL_PIXEL_4_1,         //!< HLSL Pixel Shader v.4.1.
    HLSL_PIXEL_5_0,         //!< HLSL Pixel Shader v.5.0.
    
    HLSL_GEOMETRY_4_0,      //!< HLSL Geometry Shader v.4.0.
    HLSL_GEOMETRY_4_1,      //!< HLSL Geometry Shader v.4.1.
    HLSL_GEOMETRY_5_0,      //!< HLSL Geometry Shader v.5.0.
    
    HLSL_COMPUTE_4_0,       //!< HLSL DirectCompute Shader v.4.0.
    HLSL_COMPUTE_4_1,       //!< HLSL DirectCompute Shader v.4.1.
    HLSL_COMPUTE_5_0,       //!< HLSL DirectCompute Shader v.5.0.
    
    HLSL_HULL_5_0,          //!< HSLL Tessellation Hull Shader v.5.0.
    
    HLSL_DOMAIN_5_0,        //!< HSLL Tessellation Domain Shader v.5.0.
    
    CG_VERSION_2_0,         //!< Cg Shader v.2.0.
};


class Texture;
class TextureLayer;
class Shader;
class ShaderClass;

/**
Construction of the shader object callback function. A shader callback can be used to update the
shader constants (or rather variables) before the shader is set and the object rendered.
When a ShaderClass is bounded to a Mesh or a Billboard both parameters are always none zero
and you do not need to check if they are valid pointers.
\param[in] ShdClass Pointer to a ShaderClass object which is currently used.
\param[in] Object Pointer to a MaterialNode object which is currently used.
*/
typedef boost::function<void (ShaderClass* ShdClass, const scene::MaterialNode* Object)> ShaderObjectCallback;

/**
Construction of the shader surface callback. This is similar to "PFNSHADEROBJECTCALLBACKPROC" but in this
case the callback will be called for each surface. You can update your shader settings for the individual
textures.
\param[in] ShdClass Pointer to a ShaderClass object which is currently used.
\param[in] TexLayers Constant reference of the texture layer container which is currently used.
*/
typedef boost::function<void (ShaderClass* ShdClass, const std::vector<TextureLayer*> &TexLayers)> ShaderSurfaceCallback;


#define SHADER_OBJECT_CALLBACK(n) void n(video::ShaderClass* ShdClass, const scene::MaterialNode* Object)
#define SHADER_SURFACE_CALLBACK(n) void n(video::ShaderClass* ShdClass, const std::vector<TextureLayer*> &TexLayers)


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
