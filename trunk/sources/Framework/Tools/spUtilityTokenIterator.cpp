/*
 * Token iterator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spUtilityTokenIterator.hpp"

#ifdef SP_COMPILE_WITH_TOKENPARSER


namespace sp
{
namespace tool
{


SToken TokenIterator::InvalidToken_;

TokenIterator::TokenIterator(const std::list<SToken> &TokenList) :
    Tokens_ (TokenList.size()   ),
    Index_  (0                  )
{
    std::copy(TokenList.begin(), TokenList.end(), Tokens_.begin());
}
TokenIterator::~TokenIterator()
{
}

const SToken& TokenIterator::getNextToken(bool IgnoreWhiteSpaces)
{
    while (Index_ < Tokens_.size())
    {
        const SToken& Tkn = Tokens_[Index_];
        
        ++Index_;
        
        if (!IgnoreWhiteSpaces || !Tkn.isWhiteSpace())
            return Tkn;
    }
    return TokenIterator::InvalidToken_;
}

const SToken& TokenIterator::getPrevToken(bool IgnoreWhiteSpaces)
{
    while (Index_ > 0)
    {
        --Index_;
        
        const SToken& Tkn = Tokens_[Index_];
        
        if (!IgnoreWhiteSpaces || !Tkn.isWhiteSpace())
            return Tkn;
    }
    return TokenIterator::InvalidToken_;
}

const SToken& TokenIterator::getNextToken(const ETokenTypes NextTokenType, bool IgnoreWhiteSpaces)
{
    u32 Unused = 0;
    return getNextToken(NextTokenType, Unused, IgnoreWhiteSpaces);
}

const SToken& TokenIterator::getPrevToken(const ETokenTypes NextTokenType, bool IgnoreWhiteSpaces)
{
    u32 Unused = 0;
    return getPrevToken(NextTokenType, Unused, IgnoreWhiteSpaces);
}

const SToken& TokenIterator::getNextToken(const ETokenTypes NextTokenType, u32 &SkipedTokens, bool IgnoreWhiteSpaces)
{
    while (1)
    {
        const SToken& Tkn = getNextToken(IgnoreWhiteSpaces);
        
        if (Tkn.Type == NextTokenType || Tkn.Type == TOKEN_EOF || Tkn.Type == TOKEN_UNKNOWN || Index_ >= Tokens_.size())
            return Tkn;
        
        ++SkipedTokens;
    }
    return TokenIterator::InvalidToken_;
}

const SToken& TokenIterator::getPrevToken(const ETokenTypes NextTokenType, u32 &SkipedTokens, bool IgnoreWhiteSpaces)
{
    while (1)
    {
        const SToken& Tkn = getPrevToken(IgnoreWhiteSpaces);
        
        if (Tkn.Type == NextTokenType || Tkn.Type == TOKEN_EOF || Tkn.Type == TOKEN_UNKNOWN || Index_ == 0)
            return Tkn;
        
        ++SkipedTokens;
    }
    return TokenIterator::InvalidToken_;
}

bool TokenIterator::next()
{
    if (Index_ < Tokens_.size())
    {
        ++Index_;
        return true;
    }
    return false;
}

bool TokenIterator::prev()
{
    if (Index_ > 0)
    {
        --Index_;
        return true;
    }
    return false;
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
