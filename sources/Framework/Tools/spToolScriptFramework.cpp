/*
 * Script framework file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#if 0

#include "Framework/Tools/spToolScriptFramework.hpp"

#ifdef SP_COMPILE_WITH_SCRIPTLOADER


#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern audio::SoundDevice* __spSoundDevice;
extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace tool
{


#define __DEBUG_SCRIPT__


const c8* ScriptFramework::ERRORMSG_WRONGTOKEN = "Unexpected token";

ScriptFramework::ScriptFramework() : FileSys_(0), File_(0)
{
    FileSys_ = new io::FileSystem();
}
ScriptFramework::~ScriptFramework()
{
    MemoryManager::deleteMemory(FileSys_);
}

io::stringc ScriptFramework::getVersion() const
{
    return "ConstructiveScriptFramework - v.3.1 beta";
}

bool ScriptFramework::loadScriptFile(const io::stringc &Filename)
{
    WorkDir_ = Filename.getPathPart();
    
    CurLine_        = "";
    CurColumnNr_    = 0;
    CurLineNr_      = 0;
    LineList_.clear();
    
    io::Log::message("Load script file: \"" + Filename + "\"");
    io::Log::upperTab();
    
    if ( !( File_ = FileSys_->readResourceFile(Filename) ) )
    {
        io::Log::lowerTab();
        return false;
    }
    
    bool Result = readScript();
    
    if (!Result)
        io::Log::error("Loading script failed");
    
    FileSys_->closeFile(File_);
    io::Log::lowerTab();
    
    return Result;
}

void ScriptFramework::updateFramework()
{
    
}

void ScriptFramework::clearLists()
{
    /* Delete all textures */
    for (std::vector<video::Texture*>::iterator it = Textures_.List.begin(); it != Textures_.List.end(); ++it)
    {
        if (*it && __spVideoDriver->isTexture(*it))
            __spVideoDriver->deleteTexture(*it);
    }
    Textures_.List.clear();
    
    /* Delete all sounds */
    if (__spSoundDevice)
    {
        for (std::vector<audio::Sound*>::iterator it = Sounds_.List.begin(); it != Sounds_.List.end(); ++it)
            __spSoundDevice->deleteSound(*it);
    }
    Sounds_.List.clear();
    
    /* Delete all meshes */
    for (std::vector<scene::Mesh*>::iterator it = Meshes_.List.begin(); it != Meshes_.List.end(); ++it)
    {
        if (*it)
            __spSceneManager->deleteNode(*it);
    }
    Meshes_.List.clear();
    
    /* Delete all cameras */
    for (std::vector<scene::Camera*>::iterator it = Cameras_.List.begin(); it != Cameras_.List.end(); ++it)
    {
        if (*it)
            __spSceneManager->deleteNode(*it);
    }
    Cameras_.List.clear();
    
    /* Delete all lights */
    for (std::vector<scene::Light*>::iterator it = Lights_.List.begin(); it != Lights_.List.end(); ++it)
    {
        if (*it)
            __spSceneManager->deleteNode(*it);
    }
    Lights_.List.clear();
    
    /* Delete all billboards */
    for (std::vector<scene::Billboard*>::iterator it = Billboards_.List.begin(); it != Billboards_.List.end(); ++it)
    {
        if (*it)
            __spSceneManager->deleteNode(*it);
    }
    Billboards_.List.clear();
    
    /* Delete each surface */
    //for (std::list<video::MeshBuffer*>::iterator it = CurSurfaceList_.begin(); it != CurSurfaceList_.end(); ++it)
    //    MemoryManager::deleteMemory(*it);
}


/*
 * ======= Protected: =======
 */

c8 ScriptFramework::readNextChar()
{
    while (CurColumnNr_ >= CurLine_.size())
    {
        /* Store last line */
        if (CurLineNr_ > 0)
            LineList_.push_back(CurLine_);
        
        /* Read next line (first blank is needed to have a new-line character replacement) */
        CurColumnNr_    = 0;
        CurLine_        = "\n" + File_->readString();
        ++CurLineNr_;
        
        if (File_->isEOF())
            return 0;
    }
    
    return CurChar_ = CurLine_[CurColumnNr_++];
}

void ScriptFramework::decrementFilePos()
{
    while (CurColumnNr_ == 0)
    {
        CurLine_ = LineList_.back();
        LineList_.pop_back();
        
        CurColumnNr_ = CurLine_.size();
        
        --CurLineNr_;
    }
    
    --CurColumnNr_;
}

ScriptFramework::ETokens ScriptFramework::readNextToken(bool OnlySingleChar)
{
    #define mcrSingleToken(t)               \
        {                                   \
            if (CurToken_ == TOKEN_UNKNOWN) \
                return CurToken_ = t;       \
            decrementFilePos();             \
            break;                          \
        }
    
    CurToken_ = TOKEN_UNKNOWN;
    
    io::stringc Str;
    c8 LastChar = 0;
    s32 CharCount = 0;
    
    while (readNextChar())
    {
        if (LastChar == '*')
        {
            if (CurChar_ == '/')
                return CurToken_ = TOKEN_COMMENT_ME;
            
            decrementFilePos();
            CurChar_ = LastChar;
            return CurToken_ = TOKEN_STAR;
        }
        else if (LastChar == '/')
        {
            if (CurChar_ == '*')
                return CurToken_ = TOKEN_COMMENT_MS;
            else if (CurChar_ == '/')
                return CurToken_ = TOKEN_COMMENT;
            
            decrementFilePos();
            CurChar_ = LastChar;
            return CurToken_ = TOKEN_SLASH;
        }
        else if ( ( CurChar_ >= 'A' && CurChar_ <= 'Z' ) || ( CurChar_ >= 'a' && CurChar_ <= 'z' ) || CurChar_ == '_' )
        {
            if (CurToken_ != TOKEN_UNKNOWN && CurToken_ != TOKEN_STRING)
            {
                decrementFilePos();
                break;
            }
            
            CurToken_ = TOKEN_STRING;
            Str += CurChar_;
        }
        else if (CurChar_ >= '0' && CurChar_ <= '9')
        {
            if (CurToken_ != TOKEN_UNKNOWN && CurToken_ != TOKEN_STRING && CurToken_ != TOKEN_INTEGER && CurToken_ != TOKEN_FLOAT)
            {
                decrementFilePos();
                break;
            }
            
            if (CurToken_ == TOKEN_UNKNOWN)
                CurToken_ = TOKEN_INTEGER;
            Str += CurChar_;
        }
        else if (CurChar_ == '.')
        {
            if (CurToken_ == TOKEN_UNKNOWN)
                return CurToken_ = TOKEN_POINT;
            
            if (CurToken_ != TOKEN_INTEGER)
            {
                decrementFilePos();
                break;
            }
            
            CurToken_ = TOKEN_FLOAT;
            Str += CurChar_;
        }
        else if (CurChar_ == ',')
            mcrSingleToken(TOKEN_COMMA)
        else if (CurChar_ == ';')
            mcrSingleToken(TOKEN_SEMICOLON)
        else if (CurChar_ == '\\')
            mcrSingleToken(TOKEN_BACKSLASH)
        else if (CurChar_ == '\"')
            mcrSingleToken(TOKEN_QUOTE)
        else if (CurChar_ == ':')
            mcrSingleToken(TOKEN_DOUBLEPOINT)
        else if (CurChar_ == '+')
            mcrSingleToken(TOKEN_PLUS)
        else if (CurChar_ == '-')
            mcrSingleToken(TOKEN_MINUS)
        else if (CurChar_ == '=')
            mcrSingleToken(TOKEN_EQUAL)
        else if (CurChar_ == ' ' || CurChar_ == '\t' || CurChar_ == '\n')
            mcrSingleToken(TOKEN_BLANK)
        else if (CurChar_ == '#')
            mcrSingleToken(TOKEN_HASHKEY)
        else if (CurChar_ == '?')
            mcrSingleToken(TOKEN_QUESTION)
        else if (CurChar_ == '[')
            mcrSingleToken(TOKEN_BRACKET_SS)
        else if (CurChar_ == ']')
            mcrSingleToken(TOKEN_BRACKET_SE)
        else if (CurChar_ == '{')
            mcrSingleToken(TOKEN_BRACKET_CS)
        else if (CurChar_ == '}')
            mcrSingleToken(TOKEN_BRACKET_CE)
        else if (CurChar_ == '(')
            mcrSingleToken(TOKEN_BRACKET_NS)
        else if (CurChar_ == ')')
            mcrSingleToken(TOKEN_BRACKET_NE)
        
        if (OnlySingleChar)
            break;
        
        LastChar = CurChar_;
        ++CharCount;
    }
    
    switch (CurToken_)
    {
        case TOKEN_STRING:
            StrToken_ = Str; break;
        case TOKEN_INTEGER:
            IntToken_ = Str.val<s32>(); break;
        case TOKEN_FLOAT:
            FltToken_ = Str.val<f32>(); break;
    }
    
    return CurToken_;
    
    #undef mcrSingleToken
}

ScriptFramework::EKeywords ScriptFramework::readNextKeyword()
{
    /* Temporary storage */
    ETokens LastToken = TOKEN_UNKNOWN;
    io::stringc LastStrToken;
    
    s32 CommentDepth = 0;
    
    s32 FullCommentLineNr = -1;
    bool isStringReading = false;
    
    CurKeyword_     = KEYWORD_UNKNOWN;
    StrKeyword_     = "";
    Str2Keyword_    = "";
    OperatorType_   = 0;
    
    IntToken_       = 0;
    FltToken_       = 0.0f;
    
    /* Read until end of file or end of keyword */
    while (!File_->isEOF())
    {
        /* Read next token */
        readNextToken(isStringReading);
        
        /* Check for comments */
        if (!isStringReading)
        {
            if (CurToken_ == TOKEN_COMMENT_MS)
                ++CommentDepth;
            else if (CommentDepth > 0 && CurToken_ == TOKEN_COMMENT_ME)
                --CommentDepth;
            else if (CurToken_ == TOKEN_COMMENT)
                FullCommentLineNr = CurLineNr_;
        }
        
        if (FullCommentLineNr != -1 && CurLineNr_ > FullCommentLineNr)
        {
            FullCommentLineNr = -1;
            StrToken_ = "";
        }
        
        /* Check if currently comments are used */
        if (CommentDepth < 0)
        {
            printError("Unexpected token for end of multi-line comment");
            return CurKeyword_ = KEYWORD_UNKNOWN;
        }
        else if (CommentDepth > 0 || CurLineNr_ == FullCommentLineNr)
            continue;
        
        /* Check for string keyword */
        if (CurToken_ == TOKEN_QUOTE)
        {
            if (isStringReading)
            {
                if (LastToken == TOKEN_BACKSLASH)
                    StrKeyword_ += '\"';
                else
                    return CurKeyword_ = KEYWORD_STRING;
            }
            else
                isStringReading = true;
        }
        else if (isStringReading)
        {
            if (LastToken == TOKEN_BACKSLASH)
            {
                if (CurChar_ == '\\')
                    StrKeyword_ += '\\';
                else if (CurChar_ == 't')
                    StrKeyword_ += '\t';
                else if (CurChar_ == 'n')
                    StrKeyword_ += '\n';
            }
            else if (CurToken_ != TOKEN_BACKSLASH)
                StrKeyword_ += CurChar_;
        }
        
        /* Check for commands */
        else if (CurToken_ == TOKEN_HASHKEY)
            CurKeyword_ = KEYWORD_COMMAND;
        
        /* Check for strings */
        else if (CurToken_ == TOKEN_STRING)
        {
            if (CurKeyword_ == KEYWORD_COMMAND)
            {
                StrKeyword_ = StrToken_;
                return CurKeyword_;
            }
        }
        
        /* Check for normal brackets '(' and ')' */
        else if (CurToken_ == TOKEN_BRACKET_NS)
        {
            if (LastToken == TOKEN_STRING && LastStrToken == "vec")
                return CurKeyword_ = KEYWORD_VECTOR_BEGIN;
        }
        else if (CurToken_ == TOKEN_BRACKET_NE)
        {
            if (LastToken == TOKEN_STRING)
            {
                decrementFilePos();
                StrKeyword_ = LastStrToken;
                return CurKeyword_ = KEYWORD_ATTRIBUTE;
            }
        }
        
        /* Check for double point when a new block begins */
        else if (CurToken_ == TOKEN_DOUBLEPOINT)
        {
            if (LastToken == TOKEN_STRING)
            {
                StrKeyword_ = LastStrToken;
                return CurKeyword_ = KEYWORD_BLOCK;
            }
            else if (LastToken == TOKEN_BRACKET_SE)
            {
                math::Swap(StrKeyword_, Str2Keyword_);
                Str2Keyword_ = LastStrToken;
                return CurKeyword_ = KEYWORD_BLOCK;
            }
            else
            {
                printError("Unexpected token for block begin");
                return CurKeyword_ = KEYWORD_UNKNOWN;
            }
        }
        
        /* Check for curly brackets '{' and '}' */
        else if (CurToken_ == TOKEN_BRACKET_CE)
            return CurKeyword_ = KEYWORD_CONSTRUCT_END;
        
        /* Check for square brackets '[' and ']' */
        else if (CurToken_ == TOKEN_BRACKET_SS)
        {
            if (LastToken == TOKEN_STRING)
                Str2Keyword_ = LastStrToken;
        }
        
        /* Check for equal '=' */
        else if (CurToken_ == TOKEN_EQUAL)
        {
            if (LastToken == TOKEN_STRING)
            {
                StrKeyword_ = LastStrToken;
                return CurKeyword_ = KEYWORD_MEMBER;
            }
            else if (LastToken == TOKEN_BRACKET_SE)
            {
                math::Swap(StrKeyword_, Str2Keyword_);
                Str2Keyword_ = LastStrToken;
                return CurKeyword_ = KEYWORD_MEMBER;
            }
        }
        
        /* Check for blanks ' ' and '\t' when most keywords ends */
        else if (CurToken_ == TOKEN_BLANK && LastToken == TOKEN_STRING)
        {
            if (LastStrToken == "class")
                return CurKeyword_ = KEYWORD_CLASS;
            else if (LastStrToken == "loop")
                return CurKeyword_ = KEYWORD_LOOP_BEGIN;
            else if (LastStrToken == "from")
                return CurKeyword_ = KEYWORD_LOOP_FROM;
            else if (LastStrToken == "to")
                return CurKeyword_ = KEYWORD_LOOP_TO;
            else if (LastStrToken == "step")
                return CurKeyword_ = KEYWORD_LOOP_STEP;
            else if (LastStrToken == "next")
                return CurKeyword_ = KEYWORD_LOOP_END;
            else if (LastStrToken == "var")
            {
                StrKeyword_ = "?";
                return CurKeyword_ = KEYWORD_VARIABLE;
            }
            else if (LastStrToken == "string")
            {
                StrKeyword_ = "s";
                return CurKeyword_ = KEYWORD_VARIABLE;
            }
            else if (LastStrToken == "int")
            {
                StrKeyword_ = "i";
                return CurKeyword_ = KEYWORD_VARIABLE;
            }
            else if (LastStrToken == "float")
            {
                StrKeyword_ = "f";
                return CurKeyword_ = KEYWORD_VARIABLE;
            }
            else if (LastStrToken == "vector")
            {
                StrKeyword_ = "v";
                return CurKeyword_ = KEYWORD_VARIABLE;
            }
        }
        
        /* Check for semicolon ';' */
        else if (CurToken_ == TOKEN_SEMICOLON)
        {
            if (LastToken == TOKEN_BRACKET_NE)
                return CurKeyword_ = KEYWORD_VECTOR_END;
            else if (LastToken == TOKEN_STRING)
            {
                StrKeyword_ = LastStrToken;
                return CurKeyword_ = KEYWORD_ATTRIBUTE;
            }
            else
                break;
        }
        
        /* Check for point '.' */
        else if (CurToken_ == TOKEN_POINT)
        {
            if (LastToken == TOKEN_STRING)
            {
                StrKeyword_ = LastStrToken;
                return CurKeyword_ = KEYWORD_ACCESS;
            }
            else
            {
                printError("Unexpected token for member access");
                return CurKeyword_ = KEYWORD_UNKNOWN;
            }
        }
        
        /* Check for comma ',' */
        else if (CurToken_ == TOKEN_COMMA)
        {
            if (LastToken == TOKEN_STRING)
            {
                StrKeyword_ = LastStrToken;
                return CurKeyword_ = KEYWORD_ATTRIBUTE;
            }
        }
        
        /* Check for operators '+', '-', '*' and '/' */
        else if (CurToken_ == TOKEN_PLUS || CurToken_ == TOKEN_MINUS || CurToken_ == TOKEN_STAR || CurToken_ == TOKEN_SLASH)
        {
            if (LastToken == TOKEN_STRING)
            {
                OperatorType_ = CurChar_;
                StrKeyword_ = LastStrToken;
                return CurKeyword_ = KEYWORD_ATTRIBUTE;
            }
            else if (LastToken == TOKEN_UNKNOWN)
            {
                OperatorType_ = CurChar_;
                return CurKeyword_ = KEYWORD_OPERATOR;
            }
        }
        
        /* Check for value (integer or float) */
        else if (CurToken_ == TOKEN_INTEGER)
        {
            if (LastToken == TOKEN_BRACKET_SS)
                StrToken_ = io::stringc(IntToken_);
            else
                return CurKeyword_ = KEYWORD_INTEGER;
        }
        else if (CurToken_ == TOKEN_FLOAT)
            return CurKeyword_ = KEYWORD_FLOAT;
        
        /* Store last token and its data */
        if (CurToken_ != TOKEN_BLANK)
        {
            LastToken       = CurToken_;
            LastStrToken    = StrToken_;
        }
    }
    
    return CurKeyword_;
}

bool ScriptFramework::readScript()
{
    while (!File_->isEOF())
    {
        switch (readNextKeyword())
        {
            case KEYWORD_COMMAND:
                readCommand(); break;
            case KEYWORD_VECTOR_BEGIN:
                readVector(); break;
            case KEYWORD_CLASS:
                readClass(); break;
            case KEYWORD_VARIABLE:
                readVariable(); break;
            case KEYWORD_BLOCK:
                readBlock(); break;
            case KEYWORD_LOOP_BEGIN:
                readLoopBegin(); break;
            case KEYWORD_LOOP_END:
                readLoopEnd(); break;
            default:
                break;
        }
        
        #ifdef __DEBUG_SCRIPT__
        if (OperatorType_)
            io::Log::message("[ Operator " + io::stringc(OperatorType_) + " ]");
        #endif
    }
    
    return true;
}

bool ScriptFramework::readCommand()
{
    const io::stringc Name(StrKeyword_);
    
    if (Name == "break")
    {
        CurLine_ = "";
        File_->setSeek(0, io::FILEPOS_END);
        return true;
    }
    
    /* Read following string */
    readNextKeyword();
    
    if (CurKeyword_ == KEYWORD_STRING)
    {
        if (Name == "message")
            io::Log::message(StrKeyword_);
        else if (Name == "warning")
            io::Log::warning(StrKeyword_);
        else if (Name == "error")
            io::Log::error(StrKeyword_);
        else if (Name == "import")
        {
            /* Store current configurations */
            io::File* PrevFile                  = File_;
            io::stringc PrevCurLine             = CurLine_;
            u32 PrevCurColumnNr                 = CurColumnNr_;
            u32 PrevCurLineNr                   = CurLineNr_;
            std::list<io::stringc> PrevLineList = LineList_;
            io::stringc PrevWorkDir             = WorkDir_;
            
            /* Import other script file */
            loadScriptFile(WorkDir_ + StrKeyword_);
            
            /* ReStore previous configurations */
            File_           = PrevFile;
            CurLine_        = PrevCurLine;
            CurColumnNr_    = PrevCurColumnNr;
            CurLineNr_      = PrevCurLineNr;
            LineList_       = PrevLineList;
            WorkDir_        = PrevWorkDir;
        }
    }
    else
        return printError("Expected string after command keyword");
    
    return true;
}

bool ScriptFramework::readVector()
{
    SValue CurValue;
    SVector Vec;
    
    while (readNextKeyword() != KEYWORD_VECTOR_END && !File_->isEOF())
    {
        if (!readValue(CurValue))
            return false;
        Vec.List.push_back(CurValue);
    }
    
    #ifdef __DEBUG_SCRIPT__
    io::stringc s = "| ";
    for (std::vector<SValue>::iterator it = Vec.List.begin(); it != Vec.List.end(); ++it)
        s += io::stringc(it->Str) + " | ";
    io::Log::message(s);
    #endif
    
    return true;
}

bool ScriptFramework::readClass()
{
    /* Read block as next keyword */
    if (readNextKeyword() != KEYWORD_BLOCK)
        return printError("Expected block after class definition");
    
    if (Str2Keyword_.size())
        printWarning("Classes must not have secondary name");
    
    #ifdef __DEBUG_SCRIPT__
    io::Log::message("Class \"" + StrKeyword_ + "\"");
    #endif
    
    while (readNextKeyword() != KEYWORD_CONSTRUCT_END && !File_->isEOF())
    {
        switch (CurKeyword_)
        {
            case KEYWORD_CLASS:
                readClass(); break;
        }
        
        //todo
        
    }
    
    return true;
}

bool ScriptFramework::readBlock()
{
    #ifdef __DEBUG_SCRIPT__
    io::stringc s("Block \"" + StrKeyword_ + "\"");
    if (Str2Keyword_.size())
        s += " [ " + Str2Keyword_ + " ]";
    io::Log::message(s);
    #endif
    
    static const io::stringc Err = " can not be inside a block";
    
    while (readNextKeyword() != KEYWORD_CONSTRUCT_END && !File_->isEOF())
    {
        switch (CurKeyword_)
        {
            case KEYWORD_BLOCK:
                readBlock(); break;
            case KEYWORD_CLASS:
                return printError("Class definition" + Err);
            case KEYWORD_COMMAND:
                return printError("Command directives" + Err);
        }
        
        //todo
        
    }
    
    return true;
}

bool ScriptFramework::readLoopBegin()
{
    if (readNextKeyword() != KEYWORD_BLOCK)
        return printError("Expected loop iterator");
    
    /* Loop range start */
    if (readNextKeyword() != KEYWORD_LOOP_FROM)
        return printError("Expected \"from\" keyword");
    
    readNextKeyword();
    SValue From;
    if (!readValue(From))
        return printError("Expected loop range start");
    
    /* Loop range end */
    if (readNextKeyword() != KEYWORD_LOOP_TO)
        return printError("Expected \"to\" keyword");
    
    readNextKeyword();
    SValue To;
    if (!readValue(To))
        return printError("Expected loop range start");
    
    #ifdef __DEBUG_SCRIPT__
    io::Log::message("Loop " + StrKeyword_ + " [ " + io::stringc(From.Integer) + " .. " + io::stringc(To.Integer) + " ]");
    #endif
    
    
    
    //todo
    
    return true;
}

bool ScriptFramework::readLoopEnd()
{
    
    //todo
    
    return true;
}

bool ScriptFramework::readVariable()
{
    const EKeywords VarType = CurKeyword_;
    
    /* Read variable name as next keyword */
    readNextKeyword();
    
    if (CurKeyword_ != KEYWORD_ATTRIBUTE && CurKeyword_ != KEYWORD_MEMBER)
        return printError("Expected variable name");
    
    const io::stringc VarName(StrKeyword_);
    
    
    
    #ifdef __DEBUG_SCRIPT__
    io::Log::message("Variable " + VarName);
    #endif
    
    return true;
}

bool ScriptFramework::readValue(SValue &Value)
{
    bool isRepeat = false;
    
    c8 Op = '+';
    
    do
    {
        if (isRepeat)
        {
            readNextKeyword();
            isRepeat = false;
        }
        
        switch (CurKeyword_)
        {
            case KEYWORD_INTEGER:
                Value.Operator(IntToken_, Op);
                break;
            case KEYWORD_FLOAT:
                Value.Operator(FltToken_, Op);
                break;
            case KEYWORD_STRING:
                Value.Operator(StrKeyword_, Op);
                break;
            case KEYWORD_ATTRIBUTE:
                if (StrKeyword_ == "true")
                    Value.Operator(SValue(1, 1.0f, StrKeyword_), Op);
                else if (StrKeyword_ == "false")
                    Value.Operator(SValue(0, 0.0f, StrKeyword_), Op);
                else
                    ;
                break;
            case KEYWORD_ACCESS:
                if (!getAccessMember(StrKeyword_))
                    return false;
                isRepeat = true;
                break;
            case KEYWORD_OPERATOR:
                Op = OperatorType_;
                isRepeat = true;
                break;
            default:
                return false;
        }
    }
    while (isRepeat);
    
    return true;
}

bool ScriptFramework::printError(const io::stringc &Message)
{
    io::Log::error("[ " + io::stringc(CurLineNr_) + ":" + io::stringc(CurColumnNr_) + " ] " + Message);
    return false;
}

void ScriptFramework::printWarning(const io::stringc &Message)
{
    io::Log::warning("[ " + io::stringc(CurLineNr_) + ":" + io::stringc(CurColumnNr_) + " ] " + Message);
}

bool ScriptFramework::getAccessMember(const io::stringc &Name)
{
    
    
    return true;
}


/*
 * SVector structure
 */

dim::point2df ScriptFramework::SVector::vec2() const
{
    if (List.size() >= 2)
        return dim::point2df(List[0].Float, List[1].Float);
    if (List.size() >= 1)
        return dim::point2df(List[0].Float);
    return 0.0f;
}
dim::vector3df ScriptFramework::SVector::vec3() const
{
    if (List.size() >= 3)
        return dim::vector3df(List[0].Float, List[1].Float, List[2].Float);
    if (List.size() >= 1)
        return dim::vector3df(List[0].Float);
    return 0.0f;
}
dim::vector4df ScriptFramework::SVector::vec4() const
{
    if (List.size() >= 4)
        return dim::vector4df(List[0].Float, List[1].Float, List[2].Float, List[3].Float);
    if (List.size() >= 3)
        return dim::vector4df(List[0].Float, List[1].Float, List[2].Float);
    if (List.size() >= 1)
        return dim::vector4df(List[0].Float);
    return 0.0f;
}
video::color ScriptFramework::SVector::clr() const
{
    if (List.size() >= 4)
    {
        return video::color(
            static_cast<u8>(List[0].Float * 255),
            static_cast<u8>(List[1].Float * 255),
            static_cast<u8>(List[2].Float * 255),
            static_cast<u8>(List[3].Float * 255)
        );
    }
    if (List.size() >= 3)
    {
        return video::color(
            static_cast<u8>(List[0].Float * 255),
            static_cast<u8>(List[1].Float * 255),
            static_cast<u8>(List[2].Float * 255)
        );
    }
    if (List.size() >= 1)
        return video::color(static_cast<u8>(List[0].Float * 255));
    return 0;
}


} // /namespace tool

} // /namespace sp


#endif
#endif



// ================================================================================
