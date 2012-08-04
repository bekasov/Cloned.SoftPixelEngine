/*
 * Script loader header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TOOL_SCRIPTFRAMEWORK_H__
#define __SP_TOOL_SCRIPTFRAMEWORK_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_SCRIPTLOADER


#include "Base/spInputOutputString.hpp"
#include "Base/spInputOutputFileSystem.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "SoundSystem/spSoundDevice.hpp"
#include "SceneGraph/spSceneGraph.hpp"

#include <list>
#include <map>
#include <vector>


namespace sp
{
namespace tool
{


class SP_EXPORT ScriptFramework
{
    
    public:
        
        ScriptFramework(scene::SceneGraph* ActiveSceneGraph = 0, audio::SoundDevice* ActiveSoundDevice = 0);
        virtual ~ScriptFramework();
        
        /* Functions */
        
        //! Returns version of the script framework (e.g. "ConstructiveScriptFramework - v.3.0").
        virtual io::stringc getVersion() const;
        
        /**
        Loads a "SoftPixel Engine" script file.
        \param Filename: Script filename which is to be loaded.
        \return True if the script file could be loaded successful. Otherwise false.
        */
        bool loadScriptFile(const io::stringc &Filename);
        
        virtual void updateFramework();
        
        virtual void clearLists();
        
        /* Inline functions */
        
        inline std::vector<video::Texture*> getTextureList() const
        {
            return Textures_.List;
        }
        inline std::vector<audio::Sound*> getSoundList() const
        {
            return Sounds_.List;
        }
        inline std::vector<scene::SceneNode*> getNodeList() const
        {
            return Nodes_.List;
        }
        inline std::vector<scene::Mesh*> getMeshList() const
        {
            return Meshes_.List;
        }
        inline std::vector<scene::Camera*> getCameraList() const
        {
            return Cameras_.List;
        }
        inline std::vector<scene::Light*> getLightList() const
        {
            return Lights_.List;
        }
        inline std::vector<scene::Billboard*> getBillboardList() const
        {
            return Billboards_.List;
        }
        
    protected:
        
        /* === Macros === */
        
        static const c8* ERRORMSG_WRONGTOKEN;
        
        /* === Enumerations === */
        
        enum ETokens
        {
            TOKEN_UNKNOWN,
            TOKEN_STRING,       // a-z, A-Z, _, number  (must not be the first char)
            TOKEN_INTEGER,      // 0-9                  (20, -5)
            TOKEN_FLOAT,        // 0-9, .               (3.14, 20, .2)
            TOKEN_SEMICOLON,    // ;
            TOKEN_POINT,        // .
            TOKEN_COMMA,        // ,
            TOKEN_DOUBLEPOINT,  // :
            TOKEN_QUOTE,        // "
            TOKEN_COMMENT,      // //
            TOKEN_COMMENT_MS,   // /*
            TOKEN_COMMENT_ME,   // */
            TOKEN_SLASH,        // /
            TOKEN_BACKSLASH,    // inverse /
            TOKEN_STAR,         // *
            TOKEN_PLUS,         // +
            TOKEN_MINUS,        // -
            TOKEN_EQUAL,        // =
            TOKEN_HASHKEY,      // #
            TOKEN_QUESTION,     // ?
            TOKEN_BLANK,        // white spaces (blanks and tabulators)
            TOKEN_BRACKET_SS,   // [ (sqaure bracket start)
            TOKEN_BRACKET_SE,   // ] (sqaure bracket end)
            TOKEN_BRACKET_CS,   // { (curly bracket start)
            TOKEN_BRACKET_CE,   // } (curly bracket end)
            TOKEN_BRACKET_NS,   // ( (normal bracket start)
            TOKEN_BRACKET_NE,   // ) (normal bracket end)
        };
        
        enum EKeywords
        {
            KEYWORD_UNKNOWN,
            KEYWORD_COMMAND,            // #command
            KEYWORD_VECTOR_BEGIN,       // vec(
            KEYWORD_VECTOR_END,         // );
            KEYWORD_VARIABLE,           // var, vector, string, int, float, int[3], float[4]
            KEYWORD_CLASS,              // class
            KEYWORD_BLOCK,              // block: {, block [name]: {
            KEYWORD_MEMBER,             // member =
            KEYWORD_ACCESS,             // variable.
            KEYWORD_ATTRIBUTE,          // MyObj, variable, x
            KEYWORD_CONSTRUCT_END,      // }
            KEYWORD_STRING,             // "string"
            KEYWORD_INTEGER,            // 5
            KEYWORD_FLOAT,              // -17.2
            KEYWORD_BOOLEAN,            // true, false
            KEYWORD_LOOP_BEGIN,         // loop index [start .. stop]
            KEYWORD_LOOP_END,           // next index
            KEYWORD_LOOP_FROM,          // from 0
            KEYWORD_LOOP_TO,            // to 10
            KEYWORD_LOOP_STEP,          // step 3
            KEYWORD_OPERATOR,           // +, -, *, / after a number
        };
        
        /* === Structures === */
        
        struct SValue
        {
            SValue()
                : Integer(0), Float(0.0f)
            {
            }
            SValue(s32 Value)
                : Integer(Value), Float(static_cast<f32>(Value)), Str(Value)
            {
            }
            SValue(f32 Value)
                : Integer(static_cast<s32>(Value)), Float(Value), Str(Value)
            {
            }
            SValue(const io::stringc &String)
                : Integer(String.val<s32>()), Float(String.val<f32>()), Str(String)
            {
            }
            SValue(s32 ValueInt, f32 ValueFlt, const io::stringc &String)
                : Integer(ValueInt), Float(ValueFlt), Str(String)
            {
            }
            ~SValue()
            {
            }
            
            /* Operator */
            SValue& operator += (const SValue &other)
            {
                Integer += other.Integer;
                Float   += other.Float;
                Str     += other.Str;
                return *this;
            }
            SValue& operator -= (const SValue &other)
            {
                Integer -= other.Integer;
                Float   -= other.Float;
                return *this;
            }
            SValue& operator *= (const SValue &other)
            {
                Integer *= other.Integer;
                Float   *= other.Float;
                return *this;
            }
            SValue& operator /= (const SValue &other)
            {
                if (other.Integer)
                    Integer /= other.Integer;
                Float /= other.Float;
                return *this;
            }
            
            /* Functions */
            void Operator(const SValue &other, c8 Op)
            {
                switch (Op)
                {
                    case '+': *this += other; break;
                    case '-': *this -= other; break;
                    case '*': *this *= other; break;
                    case '/': *this /= other; break;
                }
            }
            
            /* Members */
            s32 Integer;
            f32 Float;
            io::stringc Str;
        };
        
        struct SVector
        {
            dim::point2df vec2() const;
            dim::vector3df vec3() const;
            dim::vector4df vec4() const;
            video::color clr() const;
            
            std::vector<SValue> List;
        };
        
        template <class T> struct SContainer
        {
            s32 EnumIndex;
            std::vector<T*> List;
            std::map<io::stringc, T*> Map;
        };
        
        /* === Functions === */
        
        c8 readNextChar();
        void decrementFilePos();
        
        ETokens readNextToken(bool OnlySingleChar = false);
        
        EKeywords readNextKeyword();
        
        bool readScript();
        bool readCommand();
        bool readVector();
        bool readClass();
        bool readBlock();
        bool readLoopBegin();
        bool readLoopEnd();
        bool readVariable();
        bool readValue(SValue &Value);
        
        bool printError(const io::stringc &Message);
        void printWarning(const io::stringc &Message);
        
        bool getAccessMember(const io::stringc &Name);
        
        /* === Members === */
        
        io::FileSystem FileSys_;
        io::File* File_;
        
        scene::SceneGraph* ActiveSceneGraph_;
        audio::SoundDevice* ActiveSoundDevice_;
        
        // Data container
        SContainer<video::Texture>      Textures_;
        SContainer<audio::Sound>        Sounds_;
        SContainer<scene::SceneNode>    Nodes_;
        SContainer<scene::Mesh>         Meshes_;
        SContainer<scene::Camera>       Cameras_;
        SContainer<scene::Light>        Lights_;
        SContainer<scene::Billboard>    Billboards_;
        
        // Current states
        io::stringc CurLine_;
        u32 CurColumnNr_;
        u32 CurLineNr_;
        std::list<io::stringc> LineList_;
        
        c8 CurChar_;
        
        ETokens CurToken_;
        io::stringc StrToken_;
        s32 IntToken_;
        f32 FltToken_;
        
        EKeywords CurKeyword_;
        io::stringc StrKeyword_, Str2Keyword_;
        c8 OperatorType_;
        
        // Global script members
        io::stringc WorkDir_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
