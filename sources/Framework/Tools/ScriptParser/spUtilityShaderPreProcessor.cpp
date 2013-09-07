/*
 * Shader pre-processor file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/ScriptParser/spUtilityShaderPreProcessor.hpp"

#ifdef SP_COMPILE_WITH_SHADER_PREPROCESSOR


#include "Base/spInputOutputFileSystem.hpp"


namespace sp
{
namespace tool
{


ShaderPreProcessor::ShaderPreProcessor() :
    Tkn_        (0),
    OutString_  (0)
{
}
ShaderPreProcessor::~ShaderPreProcessor()
{
}

bool ShaderPreProcessor::preProcessShader(
    const io::stringc &InputShaderCode, io::stringc &OutputShaderCode,
    const video::EShaderTypes ShaderType, const video::EShaderVersions ShaderVersion,
    const io::stringc &EntryPoint, u32 Options)
{
    /* Parse tokens from input shader code */
    TokenIt_ = Parser_.parseTokens(InputShaderCode.c_str(), COMMENTSTYLE_ANSI_C);
    
    if (!TokenIt_)
        return false;
    
    OutString_ = &OutputShaderCode;
    
    /* Get options for easy access */
    const bool IsGLSL = (ShaderVersion >= video::GLSL_VERSION_1_20 && ShaderVersion <= video::GLSL_VERSION_4_30);
    const bool IsHLSL = (ShaderVersion >= video::HLSL_VERTEX_1_0 && ShaderVersion <= video::HLSL_DOMAIN_5_0);
    
    bool PrevTokenNL = false;
    
    /* Reconstruct shader code for output */
    while (1)
    {
        /* Get next token */
        if (!nextToken(false))
            break;
        
        /* Apply processing options */
        if ((Options & SHADER_PREPROCESS_SKIP_BLANKS) != 0)
        {
            if (type() == TOKEN_NEWLINE)
            {
                if (PrevTokenNL)
                    continue;
                PrevTokenNL = true;
            }
            else if (PrevTokenNL && Tkn_->isWhiteSpace())
                continue;
            else
                PrevTokenNL = false;
        }
        
        if ((Options & SHADER_PREPROCESS_NO_TABS) != 0 && type() == TOKEN_TAB)
        {
            append("    ");
            continue;
        }
        
        if (IsGLSL)
        {
            /* Solve macros directly */
            if ((Options & SHADER_PREPROCESS_SOLVE_MACROS) != 0 && type() == TOKEN_NAME)
                solveMacrosGLSL();
            
            /* Solve HLSL attributes (e.g. [loop], [unroll] etc.) */
            if (type() == TOKEN_SQUARED_BRACKET_LEFT)
            {
                TokenIt_->prev();
                const bool IsPrevName = (TokenIt_->getPrevToken(true, true).Type == TOKEN_NAME);
                TokenIt_->next();
                
                if (!IsPrevName && solveAttributesGLSL())
                    continue;
            }
        }
        
        /* Append current token string */
        append();
    }
    
    return true;
}


/*
 * ======= Private: ========
 */

bool ShaderPreProcessor::exitWithError(const io::stringc &Message, bool AppendTokenPos)
{
    if (AppendTokenPos)
        io::Log::error(Message + " at " + Tkn_->getRowColumnString());
    else
        io::Log::error(Message);
    return false;
}

bool ShaderPreProcessor::nextToken(bool IgnoreWhiteSpaces)
{
    Tkn_ = &TokenIt_->getNextToken(IgnoreWhiteSpaces);
    return !Tkn_->eof() && Tkn_->valid();
}

void ShaderPreProcessor::append()
{
    Tkn_->appendString(*OutString_);
}

void ShaderPreProcessor::append(const io::stringc &Str)
{
    *OutString_ += Str;
}

void ShaderPreProcessor::solveMacrosGLSL()
{
    io::stringc& Name = Tkn_->Str;
    
    /* Check for vector and matrix macros */
    static const SDataTypeConversion ConversionTypes[] =
    {
        { "float", 0, "vec", "mat" },
        { "double", 0, "dvec", "dmat" },
        { "half", "float", "vec", "mat" },
        { "int", 0, "ivec", "imat" },
        { "uint", 0, "uvec", "umat" },
        { "bool", 0, "bvec", "bmat" },
        { 0, 0, "", "" }
    };
    
    const SDataTypeConversion* Entry = ConversionTypes;
    
    while (Entry->HLSLDataType != 0)
    {
        if (Name.leftEqual(Entry->HLSLDataType) && solveMacroVectorGLSL(Name, *Entry))
            return;
        ++Entry;
    }
    
    /* Check for other macros */
    struct SDefMacro
    {
        const c8* From;
        const c8* To;
    };
    
    static const SDefMacro DefMacros[] =
    {
        { "groupshared", "shared" },
        { "GroupMemoryBarrier", "groupMemoryBarrier" },
        { "ddx", "dFdx" },
        { "ddy", "dFdy" },
        { "frac", "fract" },
        { "lerp", "mix" },
        { 0, 0 }
    };
    
    const SDefMacro* Macro = DefMacros;
    
    while (Macro->From != 0)
    {
        if (Name == Macro->From)
        {
            Name = Macro->To;
            return;
        }
        ++Macro;
    }
}

bool ShaderPreProcessor::solveMacroVectorGLSL(io::stringc &Name, const SDataTypeConversion &Type)
{
    const u32 TypeLen = strlen(Type.HLSLDataType);
    
    if (Name.size() <= TypeLen)
    {
        if (Type.GLSLDataType != 0)
            Name = Type.GLSLDataType;
        return true;
    }
    
    /* Setup vector type */
    const c8 Num1 = Name[TypeLen];
    
    if (Num1 < '1' || Num1 > '4')
        return false;
    
    if (Name.size() == TypeLen + 1)
    {
        if (Num1 == '1')
        {
            /* Check for special case "half" -> "float" */
            if (Type.GLSLDataType != 0)
                Name = Type.GLSLDataType;
        }
        else
        {
            /* Solve vector macros: "vec2/3/4" etc. */
            Name = Type.GLSLVecType + Num1;
        }
        return true;
    }
    
    /* Check if name is an (n)x(m)-matrix */
    if (Name.size() < TypeLen + 3)
        return false;
    
    /* Setup matrix type */
    const c8 Num2 = Name[TypeLen + 2];
    
    if (Num2 < '1' || Num2 > '4')
        return false;
    
    if (Num1 == '1' && Num2 == '1')
    {
        /* Check for special case "half1x1" -> "float" */
        if (Type.GLSLDataType != 0)
            Name = Type.GLSLDataType;
    }
    else if (Num1 == '1')
        Name = Type.GLSLVecType + Num2;
    else if (Num2 == '1')
        Name = Type.GLSLVecType + Num1;
    else if (Num1 == Num2)
        Name = Type.GLSLMatType + Num1;
    else
        Name = Type.GLSLMatType + Num1 + io::stringc("x") + Num2;
    
    return true;
}

bool ShaderPreProcessor::solveAttributesGLSL()
{
    /* Get attribute name */
    nextToken();
    
    if (type() != TOKEN_NAME)
    {
        io::Log::error("Unexpected token while processing HLSL attribute at " + Tkn_->getRowColumnString());
        return false;
    }
    
    /* Process attribute */
    const io::stringc& Name = Tkn_->Str;
    
    if (Name == "numthreads")
        return solveAttributeNumThreadsGLSL();
    
    //todo ...
    
    /* Exit with error */
    return exitWithError("Unknown HLSL attribute \"" + Name + "\"");
}

bool ShaderPreProcessor::solveAttributeNumThreadsGLSL()
{
    append("layout");
    
    c8 LocalSizeCoord = 'x';
    
    while (nextToken())
    {
        if (type() == TOKEN_NUMBER_INT)
        {
            if (LocalSizeCoord <= 'z')
            {
                io::stringc LocalSizeStr = "local_size_x = ";
                LocalSizeStr[11] = LocalSizeCoord++;
                append(LocalSizeStr);
            }
            else
                return exitWithError("Too many arguments for \"numthreads\" attribute");
        }
        else if (type() == TOKEN_SQUARED_BRACKET_RIGHT)
        {
            append(" in;");
            break;
        }
        
        append();
    }
    
    return true;
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
