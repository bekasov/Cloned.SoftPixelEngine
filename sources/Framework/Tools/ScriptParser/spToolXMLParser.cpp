/*
 * XML parser file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/ScriptParser/spToolXMLParser.hpp"

#include <boost/foreach.hpp>

#ifdef SP_COMPILE_WITH_XMLPARSER


namespace sp
{
namespace tool
{


//#define __XML_OUTPUT_DEBUG__
#ifdef __XML_OUTPUT_DEBUG__

static void __debPrintXMLTag__(const SXMLTag &block)
{
    io::Log::message("Tag \"" + block.Name + "\"");
    io::Log::message("{");
    
    io::Log::upperTab();
    
    // Attributes
    foreach (SXMLAttribute attr, block.Attributes)
        io::Log::message("Attrib \"" + attr.Name + "\" = \"" + attr.Value + "\"");
    
    if (block.Text.size())
        io::Log::message("Text = \"" + block.Text + "\"");
    
    // Inner tags
    foreach (SXMLTag tag, block.Tags)
        __debPrintXMLTag__(tag);
    
    io::Log::lowerTab();
    io::Log::message("}");
}

#endif

XMLParser::XMLParser() :
    File_(0)
{
}
XMLParser::~XMLParser()
{
}

bool XMLParser::loadFile(const io::stringc &Filename)
{
    LineRow_ = 0;
    RootTag_ = SXMLTag("RootTag");
    
    CurTag_ = &RootTag_;
    
    if ( !( File_ = FileSys_.openFile(Filename, io::FILE_READ) ) )
        return false;
    
    io::Log::message("Load XML file: \"" + Filename + "\"");
    io::Log::upperTab();
    
    bool Result = readXML();
    
    if (!Result)
        io::Log::error("Loading XML failed");
    
    FileSys_.closeFile(File_);
    io::Log::lowerTab();
    
    #ifdef __XML_OUTPUT_DEBUG__
    __debPrintXMLTag__(RootTag_);
    #endif
    
    return Result;
}

bool XMLParser::saveFile(const io::stringc &Filename, const SXMLTag &RootTag)
{
    if ( !( File_ = FileSys_.openFile(Filename, io::FILE_WRITE) ) )
        return false;
    
    io::Log::message("Save XML file: \"" + Filename + "\"");
    io::Log::upperTab();
    
    io::stringc Tab;
    bool Result = writeTag(RootTag, Tab);
    
    FileSys_.closeFile(File_);
    io::Log::lowerTab();
    
    return Result;
}


/*
 * ========== Protected: ==========
 */

c8 XMLParser::readChar()
{
    while (LineColumn_ >= Line_.size())
    {
        LineColumn_ = 0;
        Line_ = File_->readString();
        
        ++LineRow_;
        
        if (File_->isEOF())
            return 0;
    }
    
    return Line_[LineColumn_++];
}

void XMLParser::storePrevChar()
{
    PrevState_.FilePos  = File_->getSeek();
    PrevState_.Line     = Line_;
    PrevState_.Row      = LineRow_;
    PrevState_.Column   = LineColumn_;
}

void XMLParser::jumpPrevChar()
{
    File_->setSeek(PrevState_.FilePos);
    
    Line_       = PrevState_.Line;
    LineRow_    = PrevState_.Row;
    LineColumn_ = PrevState_.Column;
}

XMLParser::SToken XMLParser::readToken()
{
    /* Temporary memories */
    SToken Token;
    c8 CurChar = 0;
    bool isCurCharName = false;
    
    ETokenState State = TOKENSTATE_NONE;
    
    while (!File_->isEOF())
    {
        /* Read next character */
        CurChar = readChar();
        
        if (!CurChar)
            return Token;
        
        /* Check if name has ended */
        if (State == TOKENSTATE_NONE)
        {
            isCurCharName = isCharName(CurChar);
            
            if (Token.Type == TOKEN_NAME && !isCurCharName)
            {
                jumpPrevChar();
                break;
            }
        }
        
        /* Construct token */
        if (CurChar != '\"')
            Token.Value += io::stringc(CurChar);
        
        /* Check for comments */
        switch (State)
        {
            case TOKENSTATE_CHECK_COMMENT:
            {
                if ( ( Token.Value.size() == 2 && Token.Value[1] == '!' ) ||
                     ( Token.Value.size() == 3 && Token.Value[2] == '-' ) )
                {
                    continue;
                }
                else if (Token.Value.size() == 4 && Token.Value[3] == '-')
                {
                    State = TOKENSTATE_SEARCH_COMMENT_END;
                    continue;
                }
                else
                {
                    jumpPrevChar();
                    
                    /* Return single token */
                    Token.Type  = TOKEN_TAG_START;
                    Token.Value = "<";
                    
                    return Token;
                }
            }
            break;
            
            case TOKENSTATE_SEARCH_COMMENT_END:
            {
                if (Token.Value.right(3) == "-->")
                {
                    Token.reset();
                    State = TOKENSTATE_NONE;
                }
                continue;
            }
            break;
            
            case TOKENSTATE_STRING:
            {
                if (CurChar == '\"')
                    return Token;
                continue;
            }
            break;
            
            default:
                break;
        }
        
        /* Search key characters */
        if (CurChar == '\"')
        {
            State = TOKENSTATE_STRING;
            Token.Type = TOKEN_STRING;
        }
        else if (CurChar == '<')
        {
            State = TOKENSTATE_CHECK_COMMENT;
            storePrevChar();
        }
        else if (isCurCharName)
        {
            Token.Type = TOKEN_NAME;
            storePrevChar();
        }
        else if (CurChar == '>')
        {
            Token.Type = TOKEN_TAG_END;
            break;
        }
        else if (CurChar == '/')
        {
            Token.Type = TOKEN_TAG_CLOSE;
            break;
        }
        else if (CurChar == ' ' || CurChar == '\t')
        {
            Token.Type = TOKEN_BLANK;
            break;
        }
        else
        {
            Token.Type = TOKEN_SINGLE;
            break;
        }
    }
    
    return Token;
}

bool XMLParser::isCharName(const c8 Character) const
{
    return (
        ( Character >= 'a' && Character <= 'z' ) ||
        ( Character >= 'A' && Character <= 'Z' ) ||
        ( Character >= '0' && Character <= '9' ) ||
        Character == '_'
    );
}

bool XMLParser::readXML()
{
    SToken Token;
    
    while (!File_->isEOF())
    {
        Token = readToken();
        
        /* Check for new tag start */
        if (Token.Type == TOKEN_TAG_START)
        {
            bool hasTagClosed = false;
            
            if (!readTag(hasTagClosed))
                return false;
            
            if (hasTagClosed)
                return true;
        }
        else
            CurTag_->Text += Token.Value;
    }
    
    return true;
}

bool XMLParser::readTagName(SToken &Token, io::stringc &Name)
{
    while (Token.Type != TOKEN_NAME)
    {
        Token = readToken();
        
        if (File_->isEOF())
            return printErrorLI("Unexpected end of file while reading tag name");
    }
    
    Name = Token.Value.lower();
    
    return true;
}

bool XMLParser::readTag(bool &hasTagClosed)
{
    SToken Token = readToken();
    
    /* Check for closing current tag */
    if (Token.Type == TOKEN_TAG_CLOSE)
    {
        popTagName();
        
        /* Check for correct tag name */
        SToken NameToken;
        io::stringc Name;
        
        if (!readTagName(NameToken, Name))
            return false;
        
        if (Name.lower() != ParentTagName_)
            return printErrorLI("Closing tag with wrong name (\"" + ParentTagName_ + "\" and \"" + Name.lower() + "\")");
        
        /* Close current tag */
        hasTagClosed = true;
        popTag();
        
        /* Search tag end token */
        Token.reset();
        
        while (Token.Type != TOKEN_TAG_END)
            Token = readToken();
        
        return true;
    }
    
    /* Add new tag and read name */
    pushTag();
    
    if (!readTagName(Token, CurTag_->Name))
        return false;
    
    pushTagName();
    
    /* Read tag attributes */
    Token.reset();
    
    bool isTagClosed = false;
    s32 AttribInitCounter = 0;
    
    while (Token.Type != TOKEN_TAG_END)
    {
        Token = readToken();
        
        if (File_->isEOF())
            return printErrorLI("");
        
        switch (Token.Type)
        {
            case TOKEN_TAG_CLOSE:
                isTagClosed = true;
                break;
                
            case TOKEN_NAME:
                /* Add new attribute */
                addAttribute(Token.Value);
                AttribInitCounter = 1;
                break;
                
            case TOKEN_SINGLE:
                if (Token.Value == "=")
                {
                    if (AttribInitCounter != 1)
                        return printErrorLI("Wrong syntax for attribute definition");
                    AttribInitCounter = 2;
                }
                break;
                
            case TOKEN_STRING:
                if (AttribInitCounter != 2 || CurTag_->Attributes.empty())
                    return printErrorLI("Missing attribute definition");
                
                /* Set attribute value */
                CurTag_->Attributes.back().Value = Token.Value;
                
                AttribInitCounter = 0;
                break;
                
            default:
                break;
        }
    }
    
    /* Check if tag was already closed */
    if (isTagClosed)
    {
        popTag();
        popTagName();
        return true;
    }
    
    /* Read tag content */
    return readXML();
}

bool XMLParser::writeTag(const SXMLTag &Tag, io::stringc &Tab)
{
    if (Tag.Name.size())
    {
        io::stringc Header("<" + Tag.Name);
        
        /* Get attributes */
        foreach (const SXMLAttribute &Attrib, Tag.Attributes)
            Header += " " + Attrib.Name + "=\"" + Attrib.Value + "\"";
        
        /* Write header */
        if (Tag.Tags.empty() && !Tag.Text.size())
        {
            File_->writeStringN(Tab + Header + "/>");
            return true;
        }
        
        File_->writeStringN(Tab + Header + ">");
        
        Tab += "\t";
    }
    
    /* Write child tags */
    foreach (const SXMLTag &Child, Tag.Tags)
        writeTag(Child, Tab);
    
    /* Write body text */
    if (Tag.Text.size())
        File_->writeStringN(Tab + Tag.Text.replace("\n", "\n" + Tab));
    
    if (Tag.Name.size())
    {
        Tab = Tab.left(Tab.size() - 1);
        File_->writeStringN(Tab + "</" + Tag.Name + ">");
    }
    
    return true;
}

void XMLParser::pushTag()
{
    /* Add new tag */
    CurTag_->Tags.resize(CurTag_->Tags.size() + 1);
    CurTag_ = &CurTag_->Tags.back();
    
    TagStack_.push_back(CurTag_);
}

void XMLParser::popTag()
{
    /* Remove last tag */
    if (!TagStack_.empty())
        TagStack_.pop_back();
    
    if (TagStack_.empty())
    {
        ParentTagName_ = "";
        
        /* Check for wrong poping */
        if (CurTag_ == &RootTag_)
        {
            printWarningLI("Poping tag from stack not possible");
            return;
        }
        
        CurTag_ = &RootTag_;
    }
    else
    {
        /* Get previous tag */
        CurTag_ = TagStack_.back();
    }
}

void XMLParser::pushTagName()
{
    /* Store parent tag name */
    ParentTagName_ = CurTag_->Name.lower();
    TagNameStack_.push_back(ParentTagName_);
}

void XMLParser::popTagName()
{
    /* Remove last tag name */
    if (!TagNameStack_.empty())
    {
        ParentTagName_ = TagNameStack_.back();
        TagNameStack_.pop_back();
    }
    
}

void XMLParser::addAttribute(const io::stringc &Name)
{
    CurTag_->Attributes.resize(CurTag_->Attributes.size() + 1);
    CurTag_->Attributes.back().Name = Name;
}

io::stringc XMLParser::getLinesIndicate() const
{
    return " (Line #" + io::stringc(LineRow_) + ")";
}

bool XMLParser::printErrorLI(const io::stringc &Message) const
{
    io::Log::error(Message + getLinesIndicate());
    return false;
}
void XMLParser::printWarningLI(const io::stringc &Message) const
{
    io::Log::warning(Message + getLinesIndicate());
}


/*
 * SXMLAttribute structure
 */

SXMLAttribute::SXMLAttribute()
{
}
SXMLAttribute::SXMLAttribute(const io::stringc &AttribName, const io::stringc &AttribValue)
    : Name(AttribName), Value(AttribValue)
{
}
SXMLAttribute::~SXMLAttribute()
{
}

void SXMLAttribute::setColor(const video::color &Color)
{
    std::vector<s32> Array(4);
    {
        Array[0] = Color.Red;
        Array[1] = Color.Green;
        Array[2] = Color.Blue;
        Array[3] = Color.Alpha;
    }
    setArray(Array);
}
video::color SXMLAttribute::getColor() const
{
    std::vector<s32> Array(getArray<s32>());
    
    if (Array.size() >= 4)
        return video::color(Array[0], Array[1], Array[2], Array[3]);
    else if (Array.size() >= 3)
        return video::color(Array[0], Array[1], Array[2]);
    else if (Array.size() >= 2)
        return video::color(Array[0], Array[1], 0);
    else if (Array.size() >= 1)
        return video::color(Array[0], 0, 0);
    
    return video::color(0);
}

void SXMLAttribute::setVector(const dim::vector3df &Vec)
{
    std::vector<f32> Array(3);
    {
        Array[0] = Vec.X;
        Array[1] = Vec.Y;
        Array[2] = Vec.Z;
    }
    setArray(Array);
}
dim::vector3df SXMLAttribute::getVector() const
{
    std::vector<f32> Array(getArray<f32>());
    
    if (Array.size() >= 3)
        return dim::vector3df(Array[0], Array[1], Array[2]);
    else if (Array.size() >= 2)
        return dim::vector3df(Array[0], Array[1], 0);
    else if (Array.size() >= 1)
        return dim::vector3df(Array[0], 0, 0);
    
    return dim::vector3df(0.0);
}


/*
 * SXMLTag structure
 */

SXMLTag::SXMLTag()
{
}
SXMLTag::SXMLTag(const io::stringc &TagName)
    : Name(TagName)
{
}
SXMLTag::~SXMLTag()
{
}

/* Functions */
void SXMLTag::clear()
{
    Tags.clear();
    Attributes.clear();
    Name = "";
    Text = "";
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
