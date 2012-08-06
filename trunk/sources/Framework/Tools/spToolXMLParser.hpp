/*
 * XML parser header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TOOL_XMLPARSER_H__
#define __SP_TOOL_XMLPARSER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_XMLPARSER


#include "Base/spInputOutputString.hpp"
#include "Base/spInputOutputFileSystem.hpp"

#include <list>
#include <vector>


namespace sp
{
namespace tool
{


struct SP_EXPORT SXMLAttribute
{
    SXMLAttribute();
    SXMLAttribute(const io::stringc &AttribName, const io::stringc &AttribValue);
    ~SXMLAttribute();
    
    /* === Functions === */
    
    template <typename T> static void setStringArray(io::stringc &Str, const std::vector<T> &Array)
    {
        Str = "";
        for (typename std::vector< T, std::allocator<T> >::const_iterator it = Array.begin(); it != Array.end(); ++it)
            Str += io::stringc(*it) + ";";
    }
    
    template <typename T> static std::vector<T> getStringArray(const io::stringc &Str)
    {
        s32 Pos1, Pos2 = 0;
        std::vector<T> Array;
        
        while ( ( Pos1 = Str.find(";", Pos2) ) != -1 && static_cast<u32>(Pos2) < Str.size() )
        {
            Array.push_back(
                static_cast<T>(Str.section(Pos2, Pos1).val<T>())
            );
            Pos2 = Pos1 + 1;
        }
        
        return Array;
    }
    
    template <typename T> void setArray(const std::vector<T> &Array)
    {
        SXMLAttribute::setStringArray<T>(Value, Array);
    }
    template <typename T> std::vector<T> getArray() const
    {
        return SXMLAttribute::getStringArray<T>(Value);
    }
    
    void setColor(const video::color &Color);
    video::color getColor() const;
    
    void setVector(const dim::vector3df &Vec);
    dim::vector3df getVector() const;
    
    template <typename T> inline T getValue() const
    {
        return Value.val<T>();
    }
    
    inline bool getBool() const
    {
        return Value.lower() == "true";
    }
    
    /* === Members === */
    
    io::stringc Name, Value;
};

struct SP_EXPORT SXMLTag
{
    SXMLTag();
    SXMLTag(const io::stringc &TagName);
    ~SXMLTag();
    
    /* Functions */
    void clear();
    
    /* Members */
    io::stringc Name, Text;
    std::list<SXMLAttribute> Attributes;
    std::list<SXMLTag> Tags;
};


/**
This is the class for XML file parsing. It also has a small function to write XML files.
An XML file is always represented in one root XML tag.
*/
class SP_EXPORT XMLParser
{
    
    public:
        
        XMLParser();
        virtual ~XMLParser();
        
        /* Functions */
        
        /**
        Loads an XML file (such as HTML websites).
        \param Filename: XML filename which is to be loaded.
        \return SXMLTag structure which represents the main block wherein all the data is stored.
        */
        bool loadFile(const io::stringc &Filename);
        
        //! Saves an XML file with the given root tag.
        bool saveFile(const io::stringc &Filename, const SXMLTag &RootTag);
        
        //! Returns the main block which will be generated after reading an XML file.
        inline const SXMLTag& getRootTag() const
        {
            return RootTag_;
        }
        
    protected:
        
        /* === Enumerations === */
        
        enum ETokenState
        {
            TOKENSTATE_NONE,
            TOKENSTATE_CHECK_COMMENT,
            TOKENSTATE_SEARCH_COMMENT_END,
            TOKENSTATE_STRING,
        };
        
        enum ETokenTypes
        {
            TOKEN_UNKNOWN,
            TOKEN_SINGLE,       //!< Single character (e.g. 't')
            TOKEN_STRING,       //!< String (e.g. "\"test\"")
            TOKEN_NAME,         //!< Name (e.g. "test")
            TOKEN_TAG_START,    //!< Tag start character '<'
            TOKEN_TAG_END,      //!< Tag end character '>'
            TOKEN_TAG_CLOSE,    //!< Tag close character '/'
            TOKEN_BLANK,        //!< ' ' and '\t'
        };
        
        /* === Structures === */
        
        struct SFileState
        {
            u32 FilePos;
            io::stringc Line;
            u32 Row;
            u32 Column;
        };
        
        struct SToken
        {
            SToken() : Type(TOKEN_UNKNOWN)
            {
            }
            ~SToken()
            {
            }
            
            /* Functions */
            inline void reset()
            {
                Type = TOKEN_UNKNOWN;
                Value = "";
            }
            
            /* Members */
            ETokenTypes Type;
            io::stringc Value;
        };
        
        /* === Functions === */
        
        c8 readChar();
        void storePrevChar();
        void jumpPrevChar();
        
        SToken readToken();
        
        bool readXML();
        bool readTag(bool &hasTagClosed);
        bool readTagName(SToken &Token, io::stringc &Name);
        
        bool writeTag(const SXMLTag &Tag, io::stringc &Tab);
        
        bool isCharName(const c8 Character) const;
        
        void pushTagStack();
        void popTagStack();
        
        void addAttribute(const io::stringc &Name);
        
        io::stringc getLinesIndicate() const;
        bool printErrorLI(const io::stringc &Message) const;
        void printWarningLI(const io::stringc &Message) const;
        
        /* === Members === */
        
        io::FileSystem FileSys_;
        io::File* File_;
        
        io::stringc Line_;
        u32 LineColumn_;
        u32 LineRow_;
        
        SFileState PrevState_;
        io::stringc ParentTagName_;
        
        std::list<SXMLTag*> TagStack_;
        std::list<io::stringc> TagNameStack_;
        
        SXMLTag* CurTag_;
        SXMLTag RootTag_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
