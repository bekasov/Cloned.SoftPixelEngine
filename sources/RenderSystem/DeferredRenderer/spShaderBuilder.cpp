/*
 * Shader builder file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spShaderBuilder.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderClass.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


namespace ShaderBuilder
{

/*
 * Internal members
 */

static ERenderSystems RenderSys = RENDERER_OPENGL;
static std::list<io::stringc>* ActiveShaderCode = 0;


/*
 * Internal enumerations
 */

enum EBuiltInDataTypes
{
    UNIFORM_BOOL,
    UNIFORM_INT,
    UNIFORM_UINT,
    UNIFORM_FLOAT,
    UNIFORM_VEC2,
    UNIFORM_VEC3,
    UNIFORM_VEC4,
    UNIFORM_MAT2,
    UNIFORM_MAT3,
    UNIFORM_MAT4,
};

static const io::stringc GLSLDataTypeNames[] =
{
    "bool", "int", "uint", "float", "vec2", "vec3", "vec4", "mat2", "mat3", "mat4"
};

static const io::stringc HLSLDataTypeNames[] =
{
    "bool", "int", "uint", "float", "float2", "float3", "float4", "float2x2", "float3x3", "float4x4"
};


/*
 * Internal functions
 */

static void addString(const io::stringc &Str)
{
    if (ActiveShaderCode->empty())
        ActiveShaderCode->push_back(Str);
    else
        ActiveShaderCode->back() += Str;
}

static void addStringN(const io::stringc &Str)
{
    ActiveShaderCode->push_back(Str);
}

static void addUniform(const EBuiltInDataTypes Uniform, const io::stringc &Name)
{
    switch (RenderSys)
    {
        case RENDERER_OPENGL:
            addStringN("uniform " + GLSLDataTypeNames[Uniform] + " " + Name + ";");
            break;
        case RENDERER_DIRECT3D9:
            addStringN("uniform " + HLSLDataTypeNames[Uniform] + " " + Name + ";");
            break;
        case RENDERER_DIRECT3D11:
            addStringN(HLSLDataTypeNames[Uniform] + " " + Name + ";");
            break;
    }
}

static void addSampler2D(const io::stringc &Name, u32 Index)
{
    switch (RenderSys)
    {
        case RENDERER_OPENGL:
        case RENDERER_DIRECT3D9:
            addStringN("uniform sampler2D " + Name + ";");
            break;
        case RENDERER_DIRECT3D11:
            addStringN("Texture2D " + Name + " : register(t" + io::stringc(Index) + ");");
            addStringN("SamplerState Sampler" + Name + " : register(s" + io::stringc(Index) + ");");
            break;
    }
}

static void addMacro(const io::stringc &Name, const io::stringc &Value)
{
    addStringN("#define " + Name + " " + Value);
}


/*
 * Global functions
 */

SP_EXPORT void buildGBufferShaderCode(s32 Flags, std::list<io::stringc> &VertexShaderCode, std::list<io::stringc> &PixelShaderCode)
{
    RenderSys = __spVideoDriver->getRendererType();
    
    //todo
    
}

SP_EXPORT void buildDeferredShaderCode(s32 Flags, std::list<io::stringc> &VertexShaderCode, std::list<io::stringc> &PixelShaderCode)
{
    RenderSys = __spVideoDriver->getRendererType();
    
    //todo
    
}

} // /namespace ShaderBuilder


} // /namespace video

} // /namespace sp



// ================================================================================
