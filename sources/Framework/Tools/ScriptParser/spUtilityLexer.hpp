/*
 * Lexer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILITY_LEXER_H__
#define __SP_UTILITY_LEXER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_TOKENSCANNER


#include "Base/spInputOutputString.hpp"
#include "Framework/Tools/ScriptParser/spUtilityTokenIterator.hpp"


namespace sp
{
namespace tool
{


#if 0

//! Namespace for lexical analysis. Used for in-house scripting languages.
namespace Lexer
{

enum EExpressionTypes
{
    EXP_DEFAULT,        //!< Unspecified default expresion (e.g. (, ), {, } etc.).
    EXP_DIGIT,          //!< Digits are numbers (integer or floating point numbers).
    EXP_IDENTIFIER,     //!< Identifiers are names (e.g. variables).
    EXP_ADD_OP,         //!< Additive operators (+ and -).
    EXP_MUL_OP,         //!< Multiplicative operators (* and /).
    EXP_BRACKET_OPEN,   //!< Open bracket, e.g. (, [, {.
    EXP_BRACKET_CLOSE,  //!< Close bracket, e.g. ), ], }.
};


class SP_EXPORT Expression
{
    
    public:
        
        Expression(const SToken* Token, Expression* Parent = 0);
        virtual ~Expression();
        
        /* === Functions === */
        
        void setParent(Expression* Parent);
        
        void addChild(Expression* Child);
        void removeChild(Expression* Child);
        
        Expression* popChildFront();
        Expression* popChildBack();
        
        Expression* findParent(const EExpressionTypes ParentType) const;
        Expression* findLowPrioParent() const;
        
        bool up();
        
        //! Swaps the parent nodes.
        void swap(Expression* Exp);
        
        //! Integrates the expression and moves it up within the tree when it's required (depending on the priority).
        void integrate();
        
        /* === Static functions === */
        
        static EExpressionTypes getExpType(const ETokenTypes TokenType);
        static s32 getPriority(const EExpressionTypes ExpType);
        
        /* === Inline functions === */
        
        inline EExpressionTypes getType() const
        {
            return Type_;
        }
        
        inline Expression* getParent() const
        {
            return Parent_;
        }
        inline const std::vector<Expression*>& getChildren() const
        {
            return Children_;
        }
        
        inline const SToken* getToken() const
        {
            return Token_;
        }
        
        //! Returns the expression's priority. "MyExp->prio()" is a short cut for "Expression::getPriority(MyExp->getType())".
        inline s32 prio() const
        {
            return Expression::getPriority(Type_);
        }
        
    private:
        
        /* === Members === */
        
        EExpressionTypes Type_;
        
        Expression* Parent_;
        std::vector<Expression*> Children_;
        
        const SToken* Token_;
        
};


enum EStatementTypes
{
    STMNT_LISTING,      //!< Statement listing.
    STMNT_EXPRESSION,   //!< Expression followed by a semicolon.
    STMNT_CLASS,        //!< Class declaration.
    STMNT_FUNC_DECL,    //!< Function declaration.
    STMNT_FUNC_DEF,     //!< Function definition.
    STMNT_BLOCK,        //!< Block, e.g. "{ statement }".
};

struct SStatement
{
    SStatement(const EStatementTypes StMntType) :
        Type(StMntType)
    {
    }
    virtual ~SStatement()
    {
    }
    
    /* Members */
    EStatementTypes Type;
};

struct SStatementListing : public SStatement
{
    SStatementListing(SStatement* NextStMnt) :
        SStatement  (STMNT_LISTING  ),
        Next        (NextStMnt      )
    {
    }
    ~SStatementListing()
    {
    }
    
    /* Members */
    SStatement* Next;
};

struct SStatementExpression : public SStatement
{
    SStatementExpression(Expression* StMntExp) :
        SStatement  (STMNT_EXPRESSION   ),
        Exp         (StMntExp           )
    {
    }
    ~SStatementExpression()
    {
    }
    
    /* Members */
    Expression* Exp;
};

struct SStatementClass : public SStatement
{
    SStatementClass(const io::stringc &ClassName, SStatement* StMntBlock) :
        SStatement  (STMNT_CLASS),
        Name        (ClassName  ),
        Block       (StMntBlock )
    {
    }
    ~SStatementClass()
    {
    }
    
    /* Members */
    io::stringc Name;
    std::vector<io::stringc> ParentClassNames;
    SStatement* Block;
};


enum EScriptDataTypes
{
    SCRIPT_DATATYPE_BYTE,
    SCRIPT_DATATYPE_SHORT,
    SCRIPT_DATATYPE_INT,
    SCRIPT_DATATYPE_UBYTE,
    SCRIPT_DATATYPE_USHORT,
    SCRIPT_DATATYPE_UINT,
    SCRIPT_DATATYPE_FLOAT,
    SCRIPT_DATATYPE_DOUBLE,
    SCRIPT_DATATYPE_STRING,
};

struct SFunctionParam
{
    io::stringc Name;
    EScriptDataTypes DataType;
    io::stringc Value;
};

struct SStatementFunctionDecl : public SStatement
{
    SStatementFunctionDecl(const io::stringc &Name) :
        SStatement  (STMNT_FUNC_DECL),
        Name        (Name           )
    {
    }
    ~SStatementFunctionDecl()
    {
    }
    
    /* Members */
    io::stringc Name;
    std::vector<SFunctionParam> Parameters;
};

struct SStatementFunctionDef : public SStatement
{
    SStatementFunctionDef(SStatementFunctionDecl* StMntFuncDecl, SStatement* FuncBlock) :
        SStatement  (STMNT_FUNC_DEF ),
        Decl        (StMntFuncDecl  ),
        Block       (FuncBlock      )
    {
    }
    ~SStatementFunctionDef()
    {
    }
    
    /* Members */
    SStatementFunctionDecl* Decl;
    SStatement* Block;
};


/**
Lexical analyzer used for in-house scripting languages.
\since Version 3.3
*/
class SP_EXPORT LexicalAnalyzer
{
    
    public:
        
        LexicalAnalyzer();
        ~LexicalAnalyzer();
        
        /* === Functions === */
        
        SStatement* buildTree(TokenStreamPtr TokenIt);
        
    private:
        
        /* === Members === */
        
        TokenStreamPtr TokenStream_;
        
        SStatement* RootStMnt_;     //!< Root tree node statement.
        
};

} // /namespace Lexer

#else

/*class SP_EXPORT TokenParser
{

};

//! Abstract-Syntax-Tree (AST) Node class.
class SP_EXPORT ASTNode
{

};*/

#endif


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
