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

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace tool
{


MaterialScriptReader::MaterialScriptReader() :
    ScriptReaderBase(   ),
    CurMaterial_    (0  ),
    CurShader_      (0  )
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
    
    /* Iterate over all tokens */
    while (nextToken())
    {
        try
        {
            if (type() == TOKEN_NAME)
                readScriptBlock();
            else
                readVarDefinition();
        }
        catch (const std::exception &Err)
        {
            return exitWithError(Err.what());
        }
    }
    
    /* Reset internal states */
    CurMaterial_    = 0;
    CurShader_      = 0;
    
    return true;
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


/*
 * ======= Protected: ========
 */

void MaterialScriptReader::printUnknownVar(const io::stringc &VariableName) const
{
    io::Log::warning("Unknown variable named \"" + VariableName + "\"");
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

void MaterialScriptReader::nextTokenNoEOF()
{
    if (!nextToken())
        breakEOF();
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

void MaterialScriptReader::readShader()
{
    
    //todo ...
    
}

void MaterialScriptReader::readVarDefinition()
{
    /* Check if a variable is about to be defined */
    if (type() == TOKEN_AT)
    {
        //todo ...
    }
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
    return "";
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

bool MaterialScriptReader::readScriptBlock()
{
    if (Tkn_->Str == "material")
        readMaterial();
    else if (Tkn_->Str == "shader")
        readShaderClass();
    else
        return false;
    return true;
}

const video::VertexFormat* MaterialScriptReader::parseVertexFormat(const io::stringc &FormatName) const
{
         if (FormatName == "vertexFormatDefault"    ) return GlbRenderSys->getVertexFormatDefault   ();
    else if (FormatName == "vertexFormatReduced"    ) return GlbRenderSys->getVertexFormatReduced   ();
    else if (FormatName == "vertexFormatExtended"   ) return GlbRenderSys->getVertexFormatExtended  ();
    else if (FormatName == "vertexFormatFull"       ) return GlbRenderSys->getVertexFormatFull      ();
    return 0;
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
