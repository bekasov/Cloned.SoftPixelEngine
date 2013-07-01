//
// SoftPixel Engine - Script Tests
//

#include <SoftPixelEngine.hpp>
#include <boost/foreach.hpp>

using namespace sp;

#ifdef SP_COMPILE_WITH_TOKENPARSER

#include "../common.hpp"

int main()
{
    tool::TokenParser Parser;
    
    // Parse file
    tool::TokenIteratorPtr TokenIt = Parser.parseFile("TestScript.txt", tool::COMMENTSTYLE_ANSI_C);
    
    bool Quit = false;
    
    io::stringc VarName, VarValue;
    
    // Validate brackets
    const tool::SToken* InvalidToken = TokenIt->validateBrackets();
    if (InvalidToken)
        io::Log::error("Invalid token at " + InvalidToken->getRowColumnString());
    
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
    
    io::Log::pauseConsole();
    
    return 0;
}

#else

int main()
{
    io::Log::error("This engine was not compiled with token-parser utility");
    return 0;
}

#endif
