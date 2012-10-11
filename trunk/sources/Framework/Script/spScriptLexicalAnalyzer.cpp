/*
 * Lexical analyzer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Script/spScriptLexicalAnalyzer.hpp"

//#ifdef SP_COMPILE_WITH_SCRIPT


#include "Base/spInputOutputLog.hpp"
#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace script
{


LexicalAnalyzer::LexicalAnalyzer() :
    CurrNode_       (0),
    CurrToken_      (0),
    NextToken_      (0),
    TokenCounter_   (0)
{
}
LexicalAnalyzer::~LexicalAnalyzer()
{
}

bool LexicalAnalyzer::parseTokens(const std::vector<SToken> &InputTokens, STokenNode &OutputTokenRoot)
{
    if (InputTokens.empty())
        return false;
    
    /* Get first token and root tree node */
    CurrNode_       = &OutputTokenRoot;
    InputTokens_    = &InputTokens;
    
    TokenCounter_   = 0;
    
    /* Check for correct compounding */
    if (!checkForCorrectCompounding())
        return false;
    
    /* Parse each token */
    while (nextToken())
    {
        STokenNode* RootNode = CurrNode_;
        
        /* Check for classes */
        if (CurrToken_->Type == TOKEN_NAME)
        {
            if (CurrToken_->Str == "class")
            {
                /* Parse tokens for class */
                CurrNode_ = OutputTokenRoot.addChild(CurrToken_);
                if (!parseClass(CurrNode_))
                    return false;
            }
            else
                return exitWithError("Expected \"class\" keyword");
        }
        else if (CurrToken_->Type != TOKEN_WHITESPACE)
            return exitWithError("Invalid token at this point");
    }
    
    return true;
}


/*
 * ======= Private: =======
 */

bool LexicalAnalyzer::nextToken()
{
    if (TokenCounter_ >= InputTokens_->size())
        return false;
    
    /* Get current and next token */
    CurrToken_ = &(InputTokens_->at(TokenCounter_++));
    NextToken_ = (TokenCounter_ < InputTokens_->size() ? &(InputTokens_->at(TokenCounter_)) : 0);
    
    return true;
}

ETokenTypes LexicalAnalyzer::getTokenType(s32 Offset) const
{
    Offset += static_cast<s32>(TokenCounter_);
    
    if (Offset < 0 || Offset >= static_cast<s32>(InputTokens_->size()))
        return TOKEN_UNKNOWN;
    
    return InputTokens_->at(Offset).Type;
}

bool LexicalAnalyzer::checkNextToken(const ETokenTypes Type)
{
    return nextToken() && CurrToken_->Type == Type;
}

bool LexicalAnalyzer::exitWithError(const io::stringc &Message, const SToken* Token)
{
    if (!Token)
        Token = CurrToken_;
    
    if (Token)
        io::Log::message("Token parser error " + Token->getRowColumnString() + ": " + Message + "!", io::LOG_ERROR);
    else
        io::Log::message("Token parser error: " + Message + "!", io::LOG_ERROR);
    
    if (CurrNode_)
    {
        STokenNode* RootNode = CurrNode_->getRoot();
        if (RootNode)
            RootNode->deleteChildren();
    }
    
    return false;
}

bool LexicalAnalyzer::checkForCorrectCompounding()
{
    /* Temporary memory */
    io::stringc BracketStack;
    
    /* Iterate through all tokens */
    foreach (const SToken &Token, *InputTokens_)
    {
        switch (Token.Type)
        {
            case TOKEN_BRACKET_LEFT:
            case TOKEN_SQUARED_BRACKET_LEFT:
            case TOKEN_BRACE_LEFT:
                pushBracket(BracketStack, Token.Type);
                break;
                
            case TOKEN_BRACKET_RIGHT:
            case TOKEN_SQUARED_BRACKET_RIGHT:
            case TOKEN_BRACE_RIGHT:
                if (!popBracket(BracketStack, Token.Type))
                    return exitWithError("Invalid bracket token", &Token);
                break;
                
            default:
                break;
        }
    }
    
    return true;
}

void LexicalAnalyzer::pushBracket(io::stringc &BracketStack, const ETokenTypes Bracket) const
{
    switch (Bracket)
    {
        case TOKEN_BRACKET_LEFT:
            BracketStack += '(';
            break;
        case TOKEN_SQUARED_BRACKET_LEFT:
            BracketStack += '[';
            break;
        case TOKEN_BRACE_LEFT:
            BracketStack += '{';
            break;
        default:
            break;
    }
}

bool LexicalAnalyzer::popBracket(io::stringc &BracketStack, const ETokenTypes Bracket) const
{
    /* Pop previous character */
    if (!BracketStack.size())
        return false;
    
    const c8 Char = BracketStack[BracketStack.size() - 1];
    BracketStack.resize(BracketStack.size() - 1);
    
    switch (Bracket)
    {
        case TOKEN_BRACKET_RIGHT:
            return Char == '(';
        case TOKEN_SQUARED_BRACKET_RIGHT:
            return Char == '[';
        case TOKEN_BRACE_RIGHT:
            return Char == '{';
        default:
            break;
    }
    
    return true;
}

bool LexicalAnalyzer::parseClass(STokenNode* Node)
{
    /* Parse block header */
    if (!parseBlockHeader(Node))
        return false;
    
    Node = CurrNode_;
    
    /* Parse block body */
    while (nextToken() && CurrToken_->Type != TOKEN_BRACE_RIGHT)
    {
        if (CurrToken_->Type == TOKEN_WHITESPACE)
            continue;
        
        if ( CurrToken_->Type == TOKEN_NAME && ( CurrToken_->Str == "private" || CurrToken_->Str == "public" ) )
        {
            CurrNode_ = Node->addChild(CurrToken_);
            if (!parseClassMember(CurrNode_))
                return false;
        }
        else
            return exitWithError("Expected private or public keyword");
    }
    
    return true;
}

bool LexicalAnalyzer::nextNoneWhiteSpace()
{
    if (!nextToken())
        return false;
    
    if (CurrToken_->Type == TOKEN_WHITESPACE)
        return nextToken();
    
    return true;
}

bool LexicalAnalyzer::parseBlockHeader(STokenNode* Node)
{
    /* Find block name */
    if (!checkNextToken(TOKEN_WHITESPACE) || !checkNextToken(TOKEN_NAME))
        return exitWithError("Expected block name");
    
    CurrNode_ = Node->addChild(CurrToken_);
    
    /* Find for block begin token '{' */
    if (!nextNoneWhiteSpace() || CurrToken_->Type != TOKEN_BRACE_LEFT)
        return exitWithError("Expected block begin token '{'");
    
    return true;
}

bool LexicalAnalyzer::parseClassMember(STokenNode* Node)
{
    /* Find block name */
    if (!checkNextToken(TOKEN_WHITESPACE) || !checkNextToken(TOKEN_NAME))
        return exitWithError("Expected class member type");
    
    CurrNode_ = CurrNode_->addChild(CurrToken_);
    
    /* Check for enumeration */
    if (CurrToken_->Str == "enum")
        return parseEnumeration(CurrNode_);
    
    /* Check for member variable or procedure */
    if ( getTokenType(3) == TOKEN_BRACKET_LEFT || ( getTokenType(3) == TOKEN_WHITESPACE && getTokenType(4) == TOKEN_BRACKET_LEFT ) )
        return parseProcedure(CurrNode_);
    
    return parseVariable(CurrNode_);
}

bool LexicalAnalyzer::parseEnumeration(STokenNode* Node)
{
    /* Parse block header */
    if (!parseBlockHeader(Node))
        return false;
    
    Node = CurrNode_;
    
    /* Parse block body */
    bool ExpectComma = false;
    
    while (nextToken() && CurrToken_->Type != TOKEN_BRACE_RIGHT)
    {
        if (CurrToken_->Type == TOKEN_WHITESPACE)
            continue;
        
        if (ExpectComma)
        {
            if (CurrToken_->Type == TOKEN_COMMA)
                ExpectComma = false;
            else
                return exitWithError("Expected separator token ','");
        }
        else
        {
            if (CurrToken_->Type == TOKEN_NAME)
            {
                Node->addChild(CurrToken_);
                ExpectComma = true;
            }
            else
                return exitWithError("Expected enumeration item");
        }
    }
    
    return true;
}

bool LexicalAnalyzer::parseProcedure(STokenNode* Node)
{
    
    //todo
    
    return true;
}

bool LexicalAnalyzer::parseVariable(STokenNode* Node)
{
    /* Find member variable identifier */
    if (!checkNextToken(TOKEN_WHITESPACE) || !checkNextToken(TOKEN_NAME))
        return exitWithError("Expected member variable identifier");
    
    Node->addChild(CurrToken_);
    
    /* Check if variable will be assigned or not */
    if (!nextNoneWhiteSpace())
        return false;
    
    if (CurrToken_->Type == TOKEN_SEMICOLON)
        return true;
    
    if (CurrToken_->Type != TOKEN_ASSIGN)
        return exitWithError("Expected end-of-line token ';' or assign token '='");
    
    /* Find assignment */
    CurrNode_ = Node->addChild(CurrToken_);
    
    return parseAssignment(CurrNode_);
}

bool LexicalAnalyzer::parseAssignment(STokenNode* Node)
{
    //todo
    
    while (!checkNextToken(TOKEN_SEMICOLON))
    {
        
        Node->addChild(CurrToken_);
        
    }
    
    return true;
}


/*
 * STokenNode structure
 */

STokenNode::STokenNode() :
    Parent  (0),
    Ref     (0)
{
}
STokenNode::STokenNode(STokenNode* TokenParent, const SToken* TokenRef) :
    Parent  (TokenParent),
    Ref     (TokenRef   )
{
}
STokenNode::~STokenNode()
{
    deleteChildren();
}

STokenNode* STokenNode::getRoot()
{
    if (Parent)
        return Parent->getRoot();
    return this;
}

const STokenNode* STokenNode::getRoot() const
{
    if (Parent)
        return Parent->getRoot();
    return this;
}

STokenNode* STokenNode::addChild(const SToken* TokenRef)
{
    STokenNode* Node = new STokenNode(this, TokenRef);
    Children.push_back(Node);
    return Node;
}

void STokenNode::deleteChildren()
{
    MemoryManager::deleteList(Children);
}


} // /namespace script

} // /namespace sp


//#endif



// ================================================================================
