/*
 * Lexer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/ScriptParser/spUtilityLexer.hpp"

#ifdef SP_COMPILE_WITH_TOKENPARSER


#include "Base/spInputOutputFileSystem.hpp"

#include <boost/make_shared.hpp>


namespace sp
{
namespace tool
{


namespace Lexer
{


/*
 * Expression class
 */

Expression::Expression(const SToken* Token, Expression* Parent) :
    Type_   (Expression::getExpType(Token->Type)),
    Parent_ (Parent                             ),
    Token_  (Token                              )
{
}
Expression::~Expression()
{
}

void Expression::setParent(Expression* Parent)
{
    if (Parent_ != Parent)
    {
        /* Remove this node from previous parent's children list */
        if (Parent_)
        {
            Expression* ThisExp = this;
            MemoryManager::removeElement(Parent_->Children_, ThisExp);
        }
        
        /* Set new parent node */
        Parent_ = Parent;
        
        /* Add this node to new parent's children list */
        if (Parent_)
            Parent_->Children_.push_back(this);
    }
}

void Expression::addChild(Expression* Child)
{
    if (Child)
        Child->setParent(this);
}

void Expression::removeChild(Expression* Child)
{
    if (Child)
        Child->setParent(0);
}

Expression* Expression::popChildFront()
{
    if (!Children_.empty())
    {
        Expression* Child = Children_.front();
        removeChild(Child);
        return Child;
    }
    return 0;
}

Expression* Expression::popChildBack()
{
    if (!Children_.empty())
    {
        Expression* Child = Children_.back();
        removeChild(Child);
        return Child;
    }
    return 0;
}

Expression* Expression::findParent(const EExpressionTypes ParentType) const
{
    if (Parent_)
    {
        if (Parent_->Type_ == ParentType)
            return Parent_;
        return Parent_->findParent(ParentType);
    }
    return 0;
}

Expression* Expression::findLowPrioParent() const
{
    if (Parent_)
    {
        /* Check if parent's priority is lower than the current's node priority */
        const s32 p = Parent_->prio();
        
        if (p >= 0 && p < prio())
        {
            Expression* NextParent = Parent_->findLowPrioParent();
            if (NextParent)
                return NextParent;
            else
                return Parent_;
        }
    }
    return 0;
}

bool Expression::up()
{
    if (Parent_)
    {
        Expression* UpperParent = Parent_->Parent_;
        Parent_->setParent(this);
        setParent(UpperParent);
        return true;
    }
    return false;
}

void Expression::swap(Expression* Exp)
{
    if (Exp && Exp != this)
    {
        if (Exp == Parent_)
            up();
        else if (Exp->Parent_ == this)
            Exp->up();
        else
        {   
            Expression* UpperParent = Exp->Parent_;
            Exp->setParent(Parent_);
            setParent(UpperParent);
        }
    }
}

void Expression::integrate()
{
    if (Parent_)
    {
        Expression* LowPrioParent = findLowPrioParent();
        if (LowPrioParent)
            swap(LowPrioParent);
    }
}

EExpressionTypes Expression::getExpType(const ETokenTypes TokenType)
{
    switch (TokenType)
    {
        case TOKEN_NUMBER_INT:
        case TOKEN_NUMBER_FLOAT:
            return EXP_DIGIT;
        case TOKEN_NAME:
            return EXP_IDENTIFIER;
        case TOKEN_ADD:
        case TOKEN_SUB:
            return EXP_ADD_OP;
        case TOKEN_MUL:
        case TOKEN_DIV:
            return EXP_MUL_OP;
        case TOKEN_BRACKET_LEFT:
        case TOKEN_SQUARED_BRACKET_LEFT:
        case TOKEN_BRACE_LEFT:
            return EXP_BRACKET_OPEN;
        case TOKEN_BRACKET_RIGHT:
        case TOKEN_SQUARED_BRACKET_RIGHT:
        case TOKEN_BRACE_RIGHT:
            return EXP_BRACKET_CLOSE;
        default:
            break;
    }
    return EXP_DEFAULT;
}

s32 Expression::getPriority(const EExpressionTypes ExpType)
{
    switch (ExpType)
    {
        case EXP_ADD_OP:
            return 2;
        case EXP_MUL_OP:
            return 1;
        case EXP_BRACKET_OPEN:
        case EXP_BRACKET_CLOSE:
            return -1;
        default:
            break;
    }
    return 0;
}


/*
 * LexicalAnalyzer class
 */

LexicalAnalyzer::LexicalAnalyzer() :
    RootStMnt_(0)
{
}
LexicalAnalyzer::~LexicalAnalyzer()
{
}

SStatement* LexicalAnalyzer::buildTree(TokenIteratorPtr TokenIt)
{
    
    //todo ...
    
    return 0;
}


} // /namespace Lexer


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
