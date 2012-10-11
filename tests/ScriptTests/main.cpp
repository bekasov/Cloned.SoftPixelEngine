//
// SoftPixel Engine - Input Tests
//

#include <SoftPixelEngine.hpp>

#include <Framework/Script/spScriptTokenReader.hpp>
#include <Framework/Script/spScriptLexicalAnalyzer.hpp>
#include <Framework/Script/spScriptCompiler.hpp>
#include <Framework/Script/spScriptRuntimeEnvironment.hpp>

#include <boost/foreach.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

void PrintNode(script::STokenNode* Node)
{
    if (Node->Ref && Node->Ref->Str.size())
        io::Log::message(Node->Ref->Str);
    else if (Node->Ref && Node->Ref->Type == script::TOKEN_ASSIGN)
        io::Log::message("=");
    else
        io::Log::message("<empty>");
    
    io::Log::upperTab();
    
    foreach (script::STokenNode* Child, Node->Children)
        PrintNode(Child);
    
    io::Log::lowerTab();
}

int main()
{
    SP_TESTS_INIT("Script")
    
    // Read tokens
    script::TokenReader tokenReader;
    std::vector<script::SToken> tokens;
    
    tokenReader.readFile("SoftPixelScriptTest.spl", tokens);
    
    // Parse tokens
    script::LexicalAnalyzer lexicalAnalyzer;
    script::STokenNode tokenNode;
    
    lexicalAnalyzer.parseTokens(tokens, tokenNode);
    
    #if 0
    
    // Debug tokens
    foreach (const script::SToken &token, tokens)
    {
        switch (token.Type)
        {
            case script::TOKEN_STRING:
                io::Log::message("String: \"" + token.Str + "\"");
                break;
            case script::TOKEN_NAME:
                io::Log::message("\"" + token.Str + "\"");
                break;
            case script::TOKEN_NUMBER_INT:
                io::Log::message("Int: " + token.Str);
                break;
            case script::TOKEN_NUMBER_FLOAT:
                io::Log::message("Float: " + token.Str);
                break;
            case script::TOKEN_BRACE_LEFT:
                io::Log::message("{");
                io::Log::upperTab();
                break;
            case script::TOKEN_BRACE_RIGHT:
                io::Log::lowerTab();
                io::Log::message("}");
                break;
        }
    }
    
    #endif
    
    #if 1
    
    PrintNode(&tokenNode);
    
    #endif
    
    SP_TESTS_MAIN_BEGIN
    {
        
        
        
    }
    SP_TESTS_MAIN_END
}
