/*
 * Script base file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/ScriptParser/spUtilityScriptReaderBase.hpp"

#ifdef SP_COMPILE_WITH_TOKENPARSER


#include "Base/spInputOutputLog.hpp"


namespace sp
{
namespace tool
{


ScriptReaderBase::ScriptReaderBase() :
    Tkn_(0)
{
}
ScriptReaderBase::~ScriptReaderBase()
{
}


/*
 * ======= Protected: ========
 */

bool ScriptReaderBase::exitWithError(const io::stringc &Message, const SToken* InvalidToken)
{
    io::Log::error(Message + " at " + InvalidToken->getRowColumnString());
    return false;
}

bool ScriptReaderBase::exitWithError(const io::stringc &Message, bool AppendTokenPos)
{
    if (AppendTokenPos && Tkn_)
        return exitWithError(Message, Tkn_);
    io::Log::error(Message);
    return false;
}

bool ScriptReaderBase::validateBrackets()
{
    const SToken* InvalidToken = 0;
    
    switch (TokenIt_->validateBrackets(InvalidToken))
    {
        case VALIDATION_ERROR_UNEXPECTED:
            return exitWithError("Unexpected bracket token", InvalidToken);
        case VALIDATION_ERROR_UNCLOSED:
            return exitWithError("Unclosed brackets", InvalidToken);
        default:
            break;
    }
    
    return true;
}

bool ScriptReaderBase::nextToken(bool IgnoreWhiteSpaces)
{
    /* Get next token */
    Tkn_ = &TokenIt_->getNextToken(IgnoreWhiteSpaces);
    return !Tkn_->eof() && Tkn_->valid();
}

bool ScriptReaderBase::nextToken(const ETokenTypes NextTokenType)
{
    /* Find next token with specified type */
    Tkn_ = &TokenIt_->getNextToken(NextTokenType);
    return !Tkn_->eof() && Tkn_->valid();
}

bool ScriptReaderBase::nextTokenCheck(const ETokenTypes CheckTokenType)
{
    /* Get next token and check if the type matches the specified type */
    if (!nextToken())
        return false;
    return type() == CheckTokenType;
}

void ScriptReaderBase::push(bool UsePrevIndex)
{
    TokenIt_->push(UsePrevIndex);
}

void ScriptReaderBase::pop(bool UsePrevIndex)
{
    if (UsePrevIndex)
        Tkn_ = &TokenIt_->pop();
    else
        TokenIt_->pop(false);
}

void ScriptReaderBase::enableNL()
{
    if (TokenIt_)
        TokenIt_->setForceNLChar(true);
}
void ScriptReaderBase::disableNL()
{
    if (TokenIt_)
        TokenIt_->setForceNLChar(false);
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
