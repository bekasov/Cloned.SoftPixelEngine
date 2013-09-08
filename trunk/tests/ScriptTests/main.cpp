//
// SoftPixel Engine - Script Tests
//

#include <SoftPixelEngine.hpp>
#include <boost/foreach.hpp>

#include "Framework/Tools/ScriptParser/spUtilityShaderPreProcessor.hpp"

using namespace sp;

#ifdef SP_COMPILE_WITH_SHADER_PREPROCESSOR

#include "../common.hpp"

int main()
{
    tool::TokenParser Parser;
    
    #if 1
    
    // Read input shader code
    //const io::stringc Filename = "../../sources/Framework/Tools/LightmapGenerator/spLightmapGenerationShader.hlsl";
    const io::stringc Filename = "../../sources/RenderSystem/DeferredRenderer/spLightGridShader.hlsl";
    
    io::FileSystem FileSys;
    std::list<io::stringc> ShdBuffer;
    
    video::ShaderClass::loadShaderResourceFile(FileSys, Filename, ShdBuffer);
    
    // Preprocess shader code
    tool::ShaderPreProcessor ShaderPP;
    
    io::stringc InputCode, OutputCode;
    
    foreach (const io::stringc &Str, ShdBuffer)
        InputCode += Str;
    
    ShaderPP.preProcessShader(
        InputCode,
        OutputCode,
        video::SHADER_COMPUTE,
        video::GLSL_VERSION_4_30,
        "ComputeMain",//"ComputeDirectIllumination",
        ( tool::SHADER_PREPROCESS_NO_TABS |
          //tool::SHADER_PREPROCESS_SKIP_BLANKS |
          tool::SHADER_PREPROCESS_SOLVE_MACROS )
    );
    
    #else
    
    // Parse file
    const io::stringc Filename = "../../sources/Framework/Tools/LightmapGenerator/spLightmapGenerationShader.hlsl";
    //const io::stringc Filename = "TestScript.txt";
    
    tool::TokenIteratorPtr TokenIt = Parser.parseFile(Filename, tool::COMMENTSTYLE_ANSI_C);
    
    if (!TokenIt)
    {
        io::Log::pauseConsole();
        return 0;
    }
    
    bool Quit = false;
    
    io::stringc VarName, VarValue;
    
    // Validate brackets
    const tool::SToken* InvalidToken = 0;
    const tool::ETokenValidationErrors Err = TokenIt->validateBrackets(InvalidToken);
    
    if (InvalidToken)
    {
        switch (Err)
        {
            case tool::VALIDATION_ERROR_UNCLOSED:
                io::Log::error("Unclosed bracket token at " + InvalidToken->getRowColumnString());
                break;
            default:
                io::Log::error("Invalid bracket token at " + InvalidToken->getRowColumnString());
                break;
        }
    }
    
    // Print information about parsed file
    while (!Quit)
    {
        tool::SToken Tkn = TokenIt->getNextToken();
        
        if (Tkn.eof())
            Quit = true;
        
        switch (Tkn.Type)
        {
            case tool::TOKEN_NAME:
                io::Log::message("Name: " + io::stringc(Tkn.Str));
                break;
            case tool::TOKEN_STRING:
                io::Log::message("String: " + io::stringc(Tkn.Str));
                break;
            case tool::TOKEN_NUMBER_INT:
                io::Log::message("Int: " + io::stringc(Tkn.Str));
                break;
            case tool::TOKEN_NUMBER_FLOAT:
                io::Log::message("Float: " + io::stringc(Tkn.Str));
                break;
            default:
                io::Log::message(Tkn.Chr);
                break;
        }
    }
    
    #endif
    
    io::Log::pauseConsole();
    
    return 0;
}

#else

int main()
{
    io::Log::error("This engine was not compiled with 'shader pre-processor' utility");
    return 0;
}

#endif
