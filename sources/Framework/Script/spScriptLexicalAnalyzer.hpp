/*
 * Lexical analyzer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCRIPT_LEXICAL_ANALYZER_H__
#define __SP_SCRIPT_LEXICAL_ANALYZER_H__


#include "Base/spStandard.hpp"

//#ifdef SP_COMPILE_WITH_SCRIPT


#include "Framework/Script/spScriptTokenReader.hpp"


namespace sp
{
namespace script
{


//! Script token tree node.
struct SP_EXPORT STokenNode
{
    STokenNode();
    STokenNode(STokenNode* TokenParent, const SToken* TokenRef);
    ~STokenNode();
    
    /* Functions */
    STokenNode* getRoot();
    const STokenNode* getRoot() const;
    STokenNode* addChild(const SToken* TokenRef);
    void deleteChildren();
    
    /* Members */
    STokenNode* Parent;                 //!< Parent token tree node.
    std::vector<STokenNode*> Children;  //!< Children token tree nodes.
    const SToken* Ref;                  //!< Reference to the token content.
};


//! \since Version 3.2
class SP_EXPORT LexicalAnalyzer
{
    
    public:
        
        LexicalAnalyzer();
        ~LexicalAnalyzer();
        
        /* === Functions === */
        
        bool parseTokens(const std::vector<SToken> &InputTokens, STokenNode &OutputTokenRoot);
        
    private:
        
        /* === Functions === */
        
        bool nextToken();
        bool nextNoneWhiteSpace();
        bool checkNextToken(const ETokenTypes Type);
        
        ETokenTypes getTokenType(s32 Offset) const;
        
        bool exitWithError(const io::stringc &Message, const SToken* Token = 0);
        
        bool checkForCorrectCompounding();
        
        void pushBracket(io::stringc &BracketStack, const ETokenTypes Bracket) const;
        bool popBracket(io::stringc &BracketStack, const ETokenTypes Bracket) const;
        
        bool parseClass         (STokenNode* Node = 0);
        bool parseBlockHeader   (STokenNode* Node = 0);
        bool parseClassMember   (STokenNode* Node = 0);
        bool parseEnumeration   (STokenNode* Node = 0);
        bool parseProcedure     (STokenNode* Node = 0);
        bool parseVariable      (STokenNode* Node = 0);
        bool parseAssignment    (STokenNode* Node = 0);
        
        /* === Members === */
        
        STokenNode* CurrNode_;
        
        const std::vector<SToken>* InputTokens_;
        
        const SToken* CurrToken_;
        const SToken* NextToken_;
        
        u32 TokenCounter_;
        
};


} // /namespace script

} // /namespace sp


//#endif

#endif



// ================================================================================
