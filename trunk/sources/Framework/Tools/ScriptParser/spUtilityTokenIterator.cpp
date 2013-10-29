/*
 * Token stream file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/ScriptParser/spUtilityTokenIterator.hpp"

#ifdef SP_COMPILE_WITH_TOKENSCANNER


#include <stack>
#include <boost/foreach.hpp>


namespace sp
{
namespace tool
{



/*
 * SToken structure
 */

SToken::SToken() :
    Type    (TOKEN_UNKNOWN  ),
    Chr     (0              ),
    Row     (0              ),
    Column  (0              )
{
}
SToken::SToken(const SToken &Other) :
    Type    (Other.Type     ),
    Str     (Other.Str      ),
    Chr     (Other.Chr      ),
    Row     (Other.Row      ),
    Column  (Other.Column   )
{
}
SToken::SToken(const ETokenTypes TokenType, s32 TokenRow, s32 TokenColumn) :
    Type    (TokenType  ),
    Chr     (0          ),
    Row     (TokenRow   ),
    Column  (TokenColumn)
{
}
SToken::SToken(
    const ETokenTypes TokenType, const io::stringc &TokenStr, s32 TokenRow, s32 TokenColumn) :
    Type    (TokenType  ),
    Str     (TokenStr   ),
    Chr     (0          ),
    Row     (TokenRow   ),
    Column  (TokenColumn)
{
}
SToken::SToken(
    const ETokenTypes TokenType, c8 TokenChr, s32 TokenRow, s32 TokenColumn) :
    Type    (TokenType  ),
    Chr     (TokenChr   ),
    Row     (TokenRow   ),
    Column  (TokenColumn)
{
}
SToken::~SToken()
{
}

io::stringc SToken::getRowColumnString() const
{
    return "[" + io::stringc(Row) + ":" + io::stringc(Column) + "]";
}

bool SToken::isName(const io::stringc &Name) const
{
    return Type == TOKEN_NAME && Str == Name;
}

bool SToken::isWhiteSpace(bool DisableNewLineChars) const
{
    return Type == TOKEN_BLANK || Type == TOKEN_TAB || ( !DisableNewLineChars && Type == TOKEN_NEWLINE );
}

bool SToken::isOpenBracket() const
{
    return Type == TOKEN_BRACKET_LEFT || Type == TOKEN_SQUARED_BRACKET_LEFT || Type == TOKEN_BRACE_LEFT;
}
bool SToken::isCloseBracket() const
{
    return Type == TOKEN_BRACKET_RIGHT || Type == TOKEN_SQUARED_BRACKET_RIGHT || Type == TOKEN_BRACE_RIGHT;
}

void SToken::appendString(io::stringc &OutputString) const
{
    std::string& OutStr = OutputString.str();
    
    if (Type >= TOKEN_COMMA && Type <= TOKEN_NEWLINE)
    {
        static const c8* SpecialSignTokens = ",.:;!?#@$()[]{}><=+-*/%~&|^ \t\n";
        OutStr += SpecialSignTokens[(Type - TOKEN_COMMA)];
    }
    else
        OutStr += Str.str();
}

io::stringc SToken::str() const
{
    io::stringc Str;
    appendString(Str);
    return Str;
}


/*
 * TokenStream class
 */

SToken TokenStream::InvalidToken_;

TokenStream::TokenStream(const std::list<SToken> &TokenList) :
    Tokens_     (TokenList.begin(), TokenList.end() ),
    Index_      (0                                  ),
    ForceNLChar_(false                              )
{
}
TokenStream::~TokenStream()
{
}

SToken& TokenStream::getToken()
{
    return !Tokens_.empty() && Index_ < Tokens_.size() ? Tokens_[Index_] : TokenStream::InvalidToken_;
}

SToken& TokenStream::getNextToken(bool IgnoreWhiteSpaces, bool RestoreIterator)
{
    if (RestoreIterator)
        push();
    
    while (Index_ < Tokens_.size())
    {
        SToken& Tkn = Tokens_[Index_];
        
        ++Index_;
        
        if (!IgnoreWhiteSpaces || !Tkn.isWhiteSpace(ForceNLChar_))
        {
            if (RestoreIterator)
                pop();
            return Tkn;
        }
    }
    
    if (RestoreIterator)
        pop();
    
    return TokenStream::InvalidToken_;
}

SToken& TokenStream::getPrevToken(bool IgnoreWhiteSpaces, bool RestoreIterator)
{
    if (RestoreIterator)
        push();
    
    while (Index_ > 0)
    {
        --Index_;
        
        SToken& Tkn = Tokens_[Index_];
        
        if (!IgnoreWhiteSpaces || !Tkn.isWhiteSpace(ForceNLChar_))
        {
            if (RestoreIterator)
                pop();
            return Tkn;
        }
    }
    
    if (RestoreIterator)
        pop();
    
    return TokenStream::InvalidToken_;
}

SToken& TokenStream::getNextToken(const ETokenTypes NextTokenType, bool IgnoreWhiteSpaces, bool RestoreIterator)
{
    u32 Unused = 0;
    return getNextToken(NextTokenType, Unused, IgnoreWhiteSpaces, RestoreIterator);
}

SToken& TokenStream::getPrevToken(const ETokenTypes NextTokenType, bool IgnoreWhiteSpaces, bool RestoreIterator)
{
    u32 Unused = 0;
    return getPrevToken(NextTokenType, Unused, IgnoreWhiteSpaces, RestoreIterator);
}

SToken& TokenStream::getNextToken(const ETokenTypes NextTokenType, u32 &SkipedTokens, bool IgnoreWhiteSpaces, bool RestoreIterator)
{
    while (1)
    {
        SToken& Tkn = getNextToken(IgnoreWhiteSpaces, RestoreIterator);
        
        if (Tkn.Type == NextTokenType || Tkn.Type == TOKEN_EOF || Tkn.Type == TOKEN_UNKNOWN || Index_ >= Tokens_.size())
            return Tkn;
        
        ++SkipedTokens;
    }
    return TokenStream::InvalidToken_;
}

SToken& TokenStream::getPrevToken(const ETokenTypes NextTokenType, u32 &SkipedTokens, bool IgnoreWhiteSpaces, bool RestoreIterator)
{
    while (1)
    {
        SToken& Tkn = getPrevToken(IgnoreWhiteSpaces, RestoreIterator);
        
        if (Tkn.Type == NextTokenType || Tkn.Type == TOKEN_EOF || Tkn.Type == TOKEN_UNKNOWN || Index_ == 0)
            return Tkn;
        
        ++SkipedTokens;
    }
    return TokenStream::InvalidToken_;
}

bool TokenStream::next()
{
    if (Index_ < Tokens_.size())
    {
        ++Index_;
        return true;
    }
    return false;
}

bool TokenStream::prev()
{
    if (Index_ > 0)
    {
        --Index_;
        return true;
    }
    return false;
}

void TokenStream::push(bool UsePrevIndex)
{
    if (UsePrevIndex)
    {
        if (Index_ > 0)
            Stack_.push(Index_ - 1);
    }
    else
        Stack_.push(Index_);
}

SToken& TokenStream::pop(bool UsePrevIndex)
{
    if (!Stack_.empty())
    {
        if (UsePrevIndex)
            Index_ = Stack_.top();
        Stack_.pop();
    }
    return getToken();
}

void TokenStream::ignoreBlock(bool SearchNextBlock)
{
    /* Check if current token is a starting bracket */
    SToken& Tkn = getToken();
    
    if (!Tkn.isOpenBracket())
    {
        if (SearchNextBlock)
        {
            /* Find starting block */
            do
            {
                Tkn = getNextToken();
                if (!Tkn.valid())
                    return;
            }
            while (!Tkn.isOpenBracket());
        }
        else
            return;
    }
    
    /* Get starting block bracket */
    c8 StartBracket = Tkn.Chr;
    
    std::stack<c8> BracketStack;
    BracketStack.push(Tkn.Chr);
    
    /* Find respective closing bracket */
    while (!BracketStack.empty())
    {
        Tkn = getNextToken();
        
        if (Tkn.isOpenBracket())
            BracketStack.push(Tkn.Chr);
        else if (Tkn.isCloseBracket())
            BracketStack.pop();
    }
}

ETokenValidationErrors TokenStream::validateBrackets(const SToken* &InvalidToken, s32 Flags) const
{
    std::stack<const SToken*> BracketStack;
    
    foreach (const SToken &Tkn, Tokens_)
    {
        if (Flags & VALIDATE_BRACKET)
        {
            if (Tkn.Type == TOKEN_BRACKET_LEFT)
                BracketStack.push(&Tkn);
            if (Tkn.Type == TOKEN_BRACKET_RIGHT)
            {
                if (BracketStack.empty() || BracketStack.top()->Chr != '(')
                {
                    InvalidToken = &Tkn;
                    return VALIDATION_ERROR_UNEXPECTED;
                }
                BracketStack.pop();
            }
        }
        
        if (Flags & VALIDATE_SQUARED_BRACKET)
        {
            if (Tkn.Type == TOKEN_SQUARED_BRACKET_LEFT)
                BracketStack.push(&Tkn);
            if (Tkn.Type == TOKEN_SQUARED_BRACKET_RIGHT)
            {
                if (BracketStack.empty() || BracketStack.top()->Chr != '[')
                {
                    InvalidToken = &Tkn;
                    return VALIDATION_ERROR_UNEXPECTED;
                }
                BracketStack.pop();
            }
        }
        
        if (Flags & VALIDATE_BRACE)
        {
            if (Tkn.Type == TOKEN_BRACE_LEFT)
                BracketStack.push(&Tkn);
            if (Tkn.Type == TOKEN_BRACE_RIGHT)
            {
                if (BracketStack.empty() || BracketStack.top()->Chr != '{')
                {
                    InvalidToken = &Tkn;
                    return VALIDATION_ERROR_UNEXPECTED;
                }
                BracketStack.pop();
            }
        }
    }
    
    if (!BracketStack.empty())
    {
        InvalidToken = BracketStack.top();
        return VALIDATION_ERROR_UNCLOSED;
    }
    
    return VALIDATION_ERROR_NONE;
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
