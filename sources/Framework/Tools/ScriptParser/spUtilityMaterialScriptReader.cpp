/*
 * Material script reader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/ScriptParser/spUtilityMaterialScriptReader.hpp"

#ifdef SP_COMPILE_WITH_MATERIAL_SCRIPT


#include "Base/spMaterialStates.hpp"
#include "Base/spBaseExceptions.hpp"
#include "Base/spVertexFormat.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "Platform/spSoftPixelDevice.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern SoftPixelDevice* GlbEngineDev;
extern video::RenderSystem* GlbRenderSys;

namespace tool
{


MaterialScriptReader::MaterialScriptReader() :
    ScriptReaderBase    (                           ),
    CurMaterial_        (0                          ),
    CurShader_          (0                          ),
    CurShaderVersion_   (video::DUMMYSHADER_VERSION )
{
}
MaterialScriptReader::~MaterialScriptReader()
{
    /* Delete material states and shader classes */
    for (std::map<std::string, video::MaterialStates*>::iterator it = Materials_.begin(); it != Materials_.end(); ++it)
        delete it->second;
    for (std::map<std::string, video::ShaderClass*>::iterator it = Shaders_.begin(); it != Shaders_.end(); ++it)
        GlbRenderSys->deleteShaderClass(it->second);
}

bool MaterialScriptReader::readScript(const io::stringc &Filename)
{
    io::Log::message("Read material scripts: \"" + Filename + "\"");
    io::Log::ScopedTab Unused;
    
    /* Read file into string */
    io::stringc InputScript;
    if (!io::FileSystem().readFileString(Filename, InputScript))
        return false;
    
    /* Parse tokens from input shader code */
    TokenIt_ = Parser_.parseTokens(InputScript.c_str(), COMMENTSTYLE_BASIC);
    
    if (!TokenIt_)
        return exitWithError("Invalid token iterator");
    
    TokenIt_->setForceNLChar(true);
    
    /* Validate brackets */
    if (!validateBrackets())
        return false;
    
    /* Define all default variables */
    defineDefaultVariables();
    
    /* Iterate over all tokens */
    bool Result = true;
    
    try
    {
        while (nextToken())
        {
            if (type() == TOKEN_NAME)
                readScriptBlock();
            else
                readVarDefinition();
        }
    }
    catch (const std::exception &Err)
    {
        Result = exitWithError(Err.what());
    }
    
    /* Reset internal states */
    CurMaterial_    = 0;
    CurShader_      = 0;
    
    clearVariables();
    
    return Result;
}

video::MaterialStates* MaterialScriptReader::findMaterial(const io::stringc &Name)
{
    std::map<std::string, video::MaterialStates*>::iterator it = Materials_.find(Name.str());
    
    if (it != Materials_.end())
        return it->second;
    
    return 0;
}

video::ShaderClass* MaterialScriptReader::findShader(const io::stringc &Name)
{
    std::map<std::string, video::ShaderClass*>::iterator it = Shaders_.find(Name.str());
    
    if (it != Shaders_.end())
        return it->second;
    
    return 0;
}

bool MaterialScriptReader::defineString(const io::stringc &VariableName, const io::stringc &Str)
{
    if (isVariableFree(VariableName))
    {
        registerString(VariableName, Str);
        return true;
    }
    return false;
}

bool MaterialScriptReader::defineNumber(const io::stringc &VariableName, f64 Number)
{
    if (isVariableFree(VariableName))
    {
        registerNumber(VariableName, Number);
        return true;
    }
    return false;
}

const video::VertexFormat* MaterialScriptReader::parseVertexFormat(const io::stringc &FormatName) const
{
         if (FormatName == "vertexFormatDefault"    ) return GlbRenderSys->getVertexFormatDefault   ();
    else if (FormatName == "vertexFormatReduced"    ) return GlbRenderSys->getVertexFormatReduced   ();
    else if (FormatName == "vertexFormatExtended"   ) return GlbRenderSys->getVertexFormatExtended  ();
    else if (FormatName == "vertexFormatFull"       ) return GlbRenderSys->getVertexFormatFull      ();
    return 0;
}

video::EShadingTypes MaterialScriptReader::parseShading(const io::stringc &Identifier)
{
         if (Identifier == "flat"       ) return video::SHADING_FLAT;
    else if (Identifier == "gouraud"    ) return video::SHADING_GOURAUD;
    else if (Identifier == "phong"      ) return video::SHADING_PHONG;
    else if (Identifier == "perPixel"   ) return video::SHADING_PERPIXEL;
    
    io::Log::warning("Unknown shading type \"" + Identifier + "\"");
    
    return video::SHADING_FLAT;
}

video::ESizeComparisionTypes MaterialScriptReader::parseCompareType(const io::stringc &Identifier)
{
         if (Identifier == "never"          ) return video::CMPSIZE_NEVER;
    else if (Identifier == "equal"          ) return video::CMPSIZE_EQUAL;
    else if (Identifier == "notEqual"       ) return video::CMPSIZE_NOTEQUAL;
    else if (Identifier == "less"           ) return video::CMPSIZE_LESS;
    else if (Identifier == "lessEqual"      ) return video::CMPSIZE_LESSEQUAL;
    else if (Identifier == "greater"        ) return video::CMPSIZE_GREATER;
    else if (Identifier == "greaterEqual"   ) return video::CMPSIZE_GREATEREQUAL;
    else if (Identifier == "always"         ) return video::CMPSIZE_ALWAYS;
    
    io::Log::warning("Unknown size compare type \"" + Identifier + "\"");
    
    return video::CMPSIZE_NEVER;
}

video::EBlendingTypes MaterialScriptReader::parseBlendType(const io::stringc &Identifier)
{
         if (Identifier == "zero"           ) return video::BLEND_ZERO;
    else if (Identifier == "one"            ) return video::BLEND_ONE;
    else if (Identifier == "srcColor"       ) return video::BLEND_SRCCOLOR;
    else if (Identifier == "invSrcColor"    ) return video::BLEND_INVSRCCOLOR;
    else if (Identifier == "srcAlpha"       ) return video::BLEND_SRCALPHA;
    else if (Identifier == "invSrcAlpha"    ) return video::BLEND_INVSRCALPHA;
    else if (Identifier == "destColor"      ) return video::BLEND_DESTCOLOR;
    else if (Identifier == "invDestColor"   ) return video::BLEND_INVDESTCOLOR;
    else if (Identifier == "destAlpha"      ) return video::BLEND_DESTALPHA;
    else if (Identifier == "invDestAlpha"   ) return video::BLEND_INVDESTALPHA;
    
    io::Log::warning("Unknown blend type \"" + Identifier + "\"");
    
    return video::BLEND_ZERO;
}

video::EWireframeTypes MaterialScriptReader::parseWireframe(const io::stringc &Identifier)
{
         if (Identifier == "points" ) return video::WIREFRAME_POINTS;
    else if (Identifier == "lines"  ) return video::WIREFRAME_LINES;
    else if (Identifier == "solid"  ) return video::WIREFRAME_SOLID;
    
    io::Log::warning("Unknown wireframe type \"" + Identifier + "\"");
    
    return video::WIREFRAME_POINTS;
}

video::EFaceTypes MaterialScriptReader::parseFaceType(const io::stringc &Identifier)
{
         if (Identifier == "front"  ) return video::FACE_FRONT;
    else if (Identifier == "back"   ) return video::FACE_BACK;
    else if (Identifier == "both"   ) return video::FACE_BOTH;
    
    io::Log::warning("Unknown face type \"" + Identifier + "\"");
    
    return video::FACE_FRONT;
}

video::EShaderVersions MaterialScriptReader::parseShaderVersion(const io::stringc &Identifier)
{
    static const c8* VerListGLSL[] = { "std120", "std130", "std140", "std150", "std330", "std400", "std410", "std420", "std430", 0 };
    static const c8* VerListDXVS[] = { "vs_1_0", "vs_2_0", "vs_2_a", "vs_3_0", "vs_4_0", "vs_4_1", "vs_5_0", 0 };
    static const c8* VerListDXPS[] = { "ps_1_0", "ps_1_1", "ps_1_2", "ps_1_3", "ps_1_4", "ps_2_0", "ps_2_a", "ps_2_b", "ps_3_0", "ps_4_0", "ps_4_1", "ps_5_0", 0 };
    static const c8* VerListDXGS[] = { "gs_4_0", "gs_4_1", "gs_5_0", 0 };
    static const c8* VerListDXCS[] = { "cs_4_0", "cs_4_1", "cs_5_0", 0 };
    static const c8* VerListDXHS[] = { "hs_5_0", 0 };
    static const c8* VerListDXDS[] = { "ds_5_0", 0 };
    static const c8* VerListCg  [] = { "cg_2_0", 0 };
    
    if (Identifier.size() == 6)
    {
        const c8** Ver = 0;
        u32 i = 0;
        
        /* Get shader version type */
        const c8 Type = Identifier[0];
        
        switch (Type)
        {
            case 's':
                Ver = VerListGLSL;
                i = static_cast<u32>(video::GLSL_VERSION_1_20);
                break;
            case 'v':
                Ver = VerListDXVS;
                i = static_cast<u32>(video::HLSL_VERTEX_1_0);
                break;
            case 'p':
                Ver = VerListDXPS;
                i = static_cast<u32>(video::HLSL_PIXEL_1_0);
                break;
            case 'g':
                Ver = VerListDXGS;
                i = static_cast<u32>(video::HLSL_GEOMETRY_4_0);
                break;
            case 'c':
                if (Identifier[1] == 's')
                {
                    Ver = VerListDXCS;
                    i = static_cast<u32>(video::HLSL_COMPUTE_4_0);
                }
                else
                {
                    Ver = VerListCg;
                    i = static_cast<u32>(video::CG_VERSION_2_0);
                }
                break;
            case 'h':
                Ver = VerListDXHS;
                i = static_cast<u32>(video::HLSL_HULL_5_0);
                break;
            case 'd':
                Ver = VerListDXDS;
                i = static_cast<u32>(video::HLSL_DOMAIN_5_0);
                break;
        }
        
        /* Search for shader version */
        if (Ver)
        {
            while (*Ver)
            {
                if (Identifier == *Ver)
                    return static_cast<video::EShaderVersions>(i);
                ++Ver;
                ++i;
            }
        }
    }
    
    io::Log::warning("Unknown shader version \"" + Identifier + "\"");
    
    return video::DUMMYSHADER_VERSION;
}


/*
 * ======= Protected: ========
 */

void MaterialScriptReader::printUnknownVar(const io::stringc &VariableName) const
{
    io::Log::warning("Unknown variable named \"" + VariableName + "\"");
}

bool MaterialScriptReader::hasVariable(const io::stringc &VariableName) const
{
    /* Check if variable is already registered */
    std::map<std::string, io::stringc>::const_iterator itStr = StringVariables_.find(VariableName.str());
    if (itStr != StringVariables_.end())
        return true;
    
    std::map<std::string, f64>::const_iterator itNum = NumericVariables_.find(VariableName.str());
    if (itNum != NumericVariables_.end())
        return true;
    
    return false;
}

bool MaterialScriptReader::isVariableFree(const io::stringc &VariableName) const
{
    if (hasVariable(VariableName))
    {
        io::Log::error("Variable \"" + VariableName + "\" already used in material script");
        return false;
    }
    return true;
}

void MaterialScriptReader::registerString(const io::stringc &VariableName, const io::stringc &Str)
{
    StringVariables_[VariableName.str()] = Str;
}

void MaterialScriptReader::registerNumber(const io::stringc &VariableName, f64 Number)
{
    NumericVariables_[VariableName.str()] = Number;
}

bool MaterialScriptReader::getVarValue(const io::stringc &VariableName, io::stringc &StrVal, f64 &NumVal, bool &IsStr) const
{
    /* Search variable in string list */
    std::map<std::string, io::stringc>::const_iterator itStr = StringVariables_.find(VariableName.str());
    
    if (itStr != StringVariables_.end())
    {
        StrVal = itStr->second;
        IsStr = true;
        return true;
    }
    
    /* Search variable in number list */
    std::map<std::string, f64>::const_iterator itNum = NumericVariables_.find(VariableName.str());
    
    if (itNum != NumericVariables_.end())
    {
        NumVal = itNum->second;
        IsStr = false;
        return true;
    }
    
    /* Print warning */
    printUnknownVar(VariableName);
    
    return false;
}

io::stringc MaterialScriptReader::getString(const io::stringc &VariableName) const
{
    /* Find variable by name */
    std::map<std::string, io::stringc>::const_iterator it = StringVariables_.find(VariableName.str());
    if (it != StringVariables_.end())
        return it->second;
    
    /* Print warning and return default value */
    printUnknownVar(VariableName);
    return "";
}

f64 MaterialScriptReader::getNumber(const io::stringc &VariableName) const
{
    /* Find variable by name */
    std::map<std::string, f64>::const_iterator it = NumericVariables_.find(VariableName.str());
    if (it != NumericVariables_.end())
        return it->second;
    
    /* Print warning and return default value */
    printUnknownVar(VariableName);
    return 0.0;
}

void MaterialScriptReader::breakEOF()
{
    throw io::DefaultException("Unexpected end-of-file");
}

void MaterialScriptReader::breakUnexpectedToken()
{
    throw io::DefaultException("Unexpected token");
}

void MaterialScriptReader::breakExpectedIdentifier()
{
    throw io::DefaultException("Expected identifier");
}

void MaterialScriptReader::breakExpectedAssignment()
{
    throw io::DefaultException("Expected assignment character");
}

void MaterialScriptReader::breakExpectedString()
{
    throw io::DefaultException("Expected string");
}

void MaterialScriptReader::breakSingleNumberOnly()
{
    throw io::DefaultException("Only strings can be combined with '+' characters");
}

void MaterialScriptReader::breakStringCombination()
{
    throw io::DefaultException("Strings must be combined with a '+' character");
}

void MaterialScriptReader::nextTokenNoEOF(bool IgnoreWhiteSpaces)
{
    if (!nextToken(IgnoreWhiteSpaces))
        breakEOF();
}

void MaterialScriptReader::ignoreNextBlock()
{
    TokenIt_->ignoreBlock(true);
}

void MaterialScriptReader::addMaterial(const io::stringc &Name)
{
    CurMaterial_ = new video::MaterialStates();
    Materials_[Name.str()] = CurMaterial_;
}

void MaterialScriptReader::addShader(const io::stringc &Name, const video::VertexFormat* InputLayout)
{
    CurShader_ = GlbRenderSys->createShaderClass(InputLayout);
    Shaders_[Name.str()] = CurShader_;
}

void MaterialScriptReader::readMaterial()
{
    /* Read material name */
    nextTokenNoEOF();
    
    if (type() != TOKEN_STRING || Tkn_->Str.empty())
        breakExpectedIdentifier();
    
    io::stringc Name = Tkn_->Str;
    
    /* Check if material name already exists */
    if (findMaterial(Name) != 0)
        throw io::DefaultException("Multiple defintion of material named \"" + Name + "\"");
    
    /* Create new material */
    addMaterial(Name);
    
    /* Start reading material settings */
    nextTokenNoEOF();
    
    if (type() != TOKEN_BRACE_LEFT)
        breakUnexpectedToken();
    
    /* Read script block */
    do
    {
        nextTokenNoEOF();
        
        if (type() == TOKEN_NAME)
            readMaterialState();
        else
            readVarDefinition();
    }
    while (type() != TOKEN_BRACE_RIGHT);
}

void MaterialScriptReader::readMaterialState()
{
    #define PARSE_ENUM(n) MaterialScriptReader::n(readIdentifier())
    
    const io::stringc& Name = Tkn_->Str;
    
         if (Name == "ambient"          ) CurMaterial_->setAmbientColor         (readColor()        );
    else if (Name == "diffuse"          ) CurMaterial_->setDiffuseColor         (readColor()        );
    else if (Name == "specular"         ) CurMaterial_->setSpecularColor        (readColor()        );
    else if (Name == "emission"         ) CurMaterial_->setEmissionColor        (readColor()        );
    
    else if (Name == "shininess"        ) CurMaterial_->setShininess            (readNumber<f32>()  );
    else if (Name == "offsetFactor"     ) CurMaterial_->setPolygonOffsetFactor  (readNumber<f32>()  );
    else if (Name == "offsetUnits"      ) CurMaterial_->setPolygonOffsetUnits   (readNumber<f32>()  );
    else if (Name == "alphaReference"   ) CurMaterial_->setAlphaReference       (readNumber<f32>()  );
    
    else if (Name == "colorMaterial"    ) CurMaterial_->setColorMaterial        (readBool()         );
    else if (Name == "lighting"         ) CurMaterial_->setLighting             (readBool()         );
    else if (Name == "blending"         ) CurMaterial_->setBlending             (readBool()         );
    else if (Name == "depthTest"        ) CurMaterial_->setDepthBuffer          (readBool()         );
    else if (Name == "fog"              ) CurMaterial_->setFog                  (readBool()         );
    else if (Name == "polygonOffset"    ) CurMaterial_->setPolygonOffset        (readBool()         );
    
    else if (Name == "shading"          ) CurMaterial_->setShading              (PARSE_ENUM(parseShading    ));
    else if (Name == "wireframe"        ) CurMaterial_->setWireframe            (PARSE_ENUM(parseWireframe  ));
    else if (Name == "depthMethod"      ) CurMaterial_->setDepthMethod          (PARSE_ENUM(parseCompareType));
    else if (Name == "alphaMethod"      ) CurMaterial_->setAlphaMethod          (PARSE_ENUM(parseCompareType));
    else if (Name == "blendSource"      ) CurMaterial_->setBlendSource          (PARSE_ENUM(parseBlendType  ));
    else if (Name == "blendTarget"      ) CurMaterial_->setBlendTarget          (PARSE_ENUM(parseBlendType  ));
    else if (Name == "renderFace"       ) CurMaterial_->setRenderFace           (PARSE_ENUM(parseFaceType   ));
    
    else if (Name == "wireframeFront"   ) CurMaterial_->setWireframe            (PARSE_ENUM(parseWireframe), CurMaterial_->getWireframeBack());
    else if (Name == "wireframeBack"    ) CurMaterial_->setWireframe            (CurMaterial_->getWireframeFront(), PARSE_ENUM(parseWireframe));
    
    #undef PARSE_ENUM
}

void MaterialScriptReader::readShaderClass()
{
    /* Read material name */
    nextTokenNoEOF();
    
    if (type() != TOKEN_STRING || Tkn_->Str.empty())
        breakExpectedIdentifier();
    
    io::stringc Name = Tkn_->Str;
    
    /* Check if shader name already exists */
    if (findShader(Name) != 0)
        throw io::DefaultException("Multiple defintion of shader named \"" + Name + "\"");
    
    /* Read vertex input layout */
    nextTokenNoEOF();
    
    const video::VertexFormat* InputLayer = 0;
    
    if (type() == TOKEN_NAME)
    {
        InputLayer = parseVertexFormat(Tkn_->Str);
        if (!InputLayer)
            io::Log::warning("Unknown vertex format named \"" + Tkn_->Str + "\"");
    }
    else if (type() != TOKEN_BRACE_LEFT)
        breakUnexpectedToken();
    
    /* Create new shader */
    addShader(Name, InputLayer);
    
    /* Read script block */
    do
    {
        nextTokenNoEOF();
        
        if (type() == TOKEN_NAME)
            readShader();
        else
            readVarDefinition();
    }
    while (type() != TOKEN_BRACE_RIGHT);
}

//!INCOMPLETE!
void MaterialScriptReader::readShader()
{
    const io::stringc &Name = Tkn_->Str;
    
    if (Name == "glsl")
        ignoreNextBlock();
    else if (Name == "hlsl3")
        ignoreNextBlock();
    else if (Name == "hlsl5")
        ignoreNextBlock();
    else if (Name == "source")
    {
        io::stringc Source = readString();
        
        #if 1//!!!
        io::Log::message("Source: \"" + Source + "\"");
        #endif
    }
    else if (Name == "version")
        CurShaderVersion_ = MaterialScriptReader::parseShaderVersion(readIdentifier());
    
    //todo ...
    
}

void MaterialScriptReader::readVertexFormat()
{
    
    //todo ...
    
}

void MaterialScriptReader::readVarDefinition()
{
    /* Check if a variable is about to be defined */
    if (type() != TOKEN_AT)
        return;
    
    /* Read variable name */
    const io::stringc Name = readVarName();
    
    /* Check if variable is already registered */
    if (hasVariable(Name))
        io::Log::warning("Multiple definition of variable named \"" + Name + "\"");
    
    /* Check if the name is followed by an assignment character */
    nextTokenNoEOF();
    
    if (type() != TOKEN_EQUAL)
        breakExpectedAssignment();
    
    /* Read variable initialization */
    io::stringc StrVal;
    f64 NumVal = 0.0;
    
    bool HasAnyVal = false;
    bool IsVarStr = false;
    bool IsNumNegative = false;
    
    while (1)
    {
        /* Read next token */
        nextToken();
        
        if (type() == TOKEN_NEWLINE)
            break;
        
        /* Check if strings will be added */
        if (HasAnyVal)
        {
            /* Check if initialization has started as string */
            if (!IsVarStr)
                breakSingleNumberOnly();
            
            /* Check if the previous string is followed by a '+' character */
            if (type() != TOKEN_ADD)
                breakStringCombination();
            
            /* Read next token after '+' character */
            nextTokenNoEOF();
            if (type() == TOKEN_NEWLINE)
                throw io::DefaultException("No more expressions after '+' character");
        }
        /* Check if token is a negative number */
        else if (type() == TOKEN_SUB)
        {
            /* Setup variable as negative number  */
            IsNumNegative = true;
            
            /* Read next token after '-' character */
            nextTokenNoEOF();
            if (type() == TOKEN_NEWLINE)
                throw io::DefaultException("No more expressions after '-' character");
        }
        
        /* Check if token is a number */
        if (type() == TOKEN_NUMBER_INT || type() == TOKEN_NUMBER_FLOAT)
        {
            /* Setup variable as number */
            NumVal = Tkn_->Str.val<f64>();
            
            if (IsNumNegative)
                NumVal = -NumVal;
        }
        /* Check if token is a variable */
        else if (type() == TOKEN_AT)
        {
            /* Read variable name */
            const io::stringc SubVarName = readVarName();
            
            /* Get variable value */
            io::stringc SubStrVal;
            f64 SubNumVal = 0.0;
            bool IsSubVarStr = false;
            
            getVarValue(SubVarName, SubStrVal, SubNumVal, IsSubVarStr);
            
            /* Add variable value */
            if (IsSubVarStr)
            {
                if (IsNumNegative)
                    throw io::DefaultException("Strings can not be negative");
                
                StrVal += SubStrVal;
                IsVarStr = true;
            }
            else
            {
                NumVal = SubNumVal;
                
                if (IsNumNegative)
                    NumVal = -NumVal;
            }
        }
        else if (type() == TOKEN_STRING)
        {
            /* Add variable value */
            IsVarStr = true;
            StrVal += Tkn_->Str;
        }
        else
            breakUnexpectedToken();
        
        HasAnyVal = true;
    }
    
    /* Check if initialization is empty */
    if (!HasAnyVal)
        throw io::DefaultException("Variable definition without initialization");
    
    /* Register new variable */
    if (IsVarStr)
        registerString(Name, StrVal);
    else
        registerNumber(Name, NumVal);
    
    #if 1//!!!
    if (IsVarStr)
        io::Log::message("String [" + Name + "]: \"" + StrVal + "\"");
    else
        io::Log::message("Number [" + Name + "]: " + io::stringc(NumVal));
    #endif
}

void MaterialScriptReader::readAssignment()
{
    /* Read assignement character */
    nextTokenNoEOF();
    
    if (type() != TOKEN_EQUAL)
        breakUnexpectedToken();
    
    /* Read next token to continue parsing */
    nextTokenNoEOF();
}

io::stringc MaterialScriptReader::readVarName()
{
    /* Read variable name */
    nextTokenNoEOF(false);
    
    if (type() != TOKEN_NAME)
        breakExpectedIdentifier();
    
    return Tkn_->Str;
}

f64 MaterialScriptReader::readDouble(bool ReadAssignment)
{
    /* Read assignment character */
    if (ReadAssignment)
        readAssignment();
    
    /* Check if the number is negative */
    f64 Factor = 1.0;
    
    if (type() == TOKEN_SUB)
    {
        Factor = -1.0;
        nextTokenNoEOF();
    }
    
    /* Read float number */
    switch (type())
    {
        case TOKEN_NUMBER_INT:
        case TOKEN_NUMBER_FLOAT:
            return Factor * Tkn_->Str.val<f64>();
            
        case TOKEN_AT:
            /* Read variable name */
            nextTokenNoEOF();
            
            if (type() != TOKEN_NAME)
                breakExpectedIdentifier();
            
            /* Return variable name */
            return Factor * getNumber(Tkn_->Str);
            
        default:
            breakUnexpectedToken();
    }
    
    return 0.0;
}

io::stringc MaterialScriptReader::readString(bool ReadAssignment)
{
    /* Read assignment character */
    if (ReadAssignment)
        readAssignment();
    
    if (type() == TOKEN_NEWLINE)
        breakExpectedString();
    
    io::stringc Str;
    
    while (1)
    {
        if (type() == TOKEN_STRING)
        {
            /* Add string value */
            Str += Tkn_->Str;
        }
        else if (type() == TOKEN_AT)
        {
            /* Add string variable value */
            if (type() != TOKEN_NAME)
                breakExpectedIdentifier();
            Str += getString(Tkn_->Str);
        }
        else
            throw io::DefaultException("Excepted string or string-variable");
        
        /* Read next token (new-line or '+' character) */
        nextToken();
        
        if (type() == TOKEN_NEWLINE)
            break;
        else if (type() != TOKEN_ADD)
            breakStringCombination();
        
        /* Read next token (must be a string or a variable) */
        nextTokenNoEOF();
    }
    
    return Str;
}

io::stringc MaterialScriptReader::readIdentifier(bool ReadAssignment)
{
    /* Read assignment character */
    if (ReadAssignment)
        readAssignment();
    
    /* Read identifier name */
    if (type() != TOKEN_NAME)
        breakUnexpectedToken();
    
    return Tkn_->Str;
}

bool MaterialScriptReader::readBool(bool ReadAssignment)
{
    /* Read boolean keyword */
    const io::stringc Keyword = readIdentifier(ReadAssignment);
    
    if (Keyword == "true")
        return true;
    else if (Keyword == "false")
        return false;
    
    /* Unknwon keyword -> throw exception */
    throw io::DefaultException("Unknown keyword \"" + Keyword + "\"");
    
    return false;
}

video::color MaterialScriptReader::readColor(bool ReadAssignment)
{
    /* Read assignment character */
    if (ReadAssignment)
        readAssignment();
    
    /* Read color components */
    video::color Color;
    u32 Comp = 0;
    
    while (type() != TOKEN_NEWLINE)
    {
        /* Read color component */
        if (Comp < 4)
            Color[Comp++] = readNumber<u8>(false);
        
        /* Read comma or new-line character */
        nextTokenNoEOF();
        
        if (type() == TOKEN_NEWLINE)
            break;
        else if (type() != TOKEN_COMMA)
            breakUnexpectedToken();
        
        nextTokenNoEOF();
    }
    
    if (Comp == 1)
        Color = video::color(Color[0]);
    
    return Color;
}

void MaterialScriptReader::clearVariables()
{
    StringVariables_.clear();
    NumericVariables_.clear();
}

bool MaterialScriptReader::readScriptBlock()
{
    if (Tkn_->Str == "material")
        readMaterial();
    else if (Tkn_->Str == "shader")
        readShaderClass();
    else if (Tkn_->Str == "vertexFormat")
        readVertexFormat();
    else
        return false;
    return true;
}

void MaterialScriptReader::defineDefaultVariables()
{
    registerString("workingDir", GlbEngineDev->getWorkingDir());
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
