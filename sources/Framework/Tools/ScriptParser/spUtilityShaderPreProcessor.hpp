/*
 * Shader pre-processor header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILITY_SHADER_PREPROCESSOR_H__
#define __SP_UTILITY_SHADER_PREPROCESSOR_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_SHADER_PREPROCESSOR


#include "Framework/Tools/ScriptParser/spUtilityTokenParser.hpp"
#include "RenderSystem/spShaderConfigTypes.hpp"


namespace sp
{
namespace tool
{


enum EShaderPreProcessorOptions
{
    SHADER_PREPROCESS_SKIP_BLANKS       = 0x0001, //!< Removes all blank lines.
    SHADER_PREPROCESS_SOLVE_MACROS      = 0x0002, //!< Solves all standard macros, i.e. macros for GLSL like "float2" will be solved directly into "vec2".
    SHADER_PREPROCESS_SOLVE_INCLUDES    = 0x0004, //!< Solves all include directives.
    SHADER_PREPROCESS_NO_TABS           = 0x0008, //!< Converts all tabulators to spaces.
};


/**
Shader pre-processor is used to make the port between GLSL and HLSL shaders as easy as possible.
\since Version 3.3
*/
class SP_EXPORT ShaderPreProcessor
{
    
    public:
        
        ShaderPreProcessor();
        ~ShaderPreProcessor();
        
        /* === Functions === */
        
        /**
        Preprocess the specified input shader code and returns the resulting output shader code.
        \param[in] InputShaderCode Specifies the input shader code. This must be written in language,
        which represents a common denominator for GLSL and HLSL. This 'meta' shading language is commonly
        specified against HLSL.
        \param[out] OutputShaderCode Holds the resulting preprocessed shader code.
        This is GLSL or HLSL code depending on the 'ShaderVersion' parameter.
        \param[in] ShaderType Specifies the output shader type, e.g. if the vertex- or pixel shader is to be preprocessed.
        \param[in] ShaderVersion Specifies the output shader version.
        This also specifies if the output shader code will be GLSL or HLSL.
        \param[in] ProcessingOptions Specifies the preprocessing options (or rather flags).
        This can be a combination of the 'EShaderPreProcessorOptions' enumeration flags.
        \return True if the shader code could be preprocessed successful. Otherwise error messages will be printed.
        \see video::EShaderTypes
        \see video::EShaderVersions
        \see EShaderPreProcessorOptions
        \todo Document more information about this 'meta' shading language.
        */
        bool preProcessShader(
            const io::stringc &InputShaderCode, io::stringc &OutputShaderCode,
            const video::EShaderTypes ShaderType, const video::EShaderVersions ShaderVersion,
            const io::stringc &EntryPoint, u32 ProcessingOptions = 0
        );
        
    private:
        
        /* === Structures === */
        
        struct SDataTypeConversion
        {
            /* Members */
            const c8* HLSLDataType;
            const c8* GLSLDataType;
            io::stringc GLSLVecType;
            io::stringc GLSLMatType;
        };
        
        struct SP_EXPORT SInputArgument
        {
            SInputArgument();
            ~SInputArgument();
            
            /* Functions */
            inline io::stringc getHeader() const
            {
                return DataType + " " + Identifier;
            }
            inline io::stringc getDecl() const
            {
                return getHeader() + ";";
            }
            
            /* Members */
            io::stringc DataType, Identifier, Semantic;
        };
        
        struct SP_EXPORT SInternalState
        {
            SInternalState();
            ~SInternalState();
            
            /* Members */
            u32 MaxVertexCount;
        };
        
        /* === Functions === */
        
        bool exitWithError(const io::stringc &Message, const SToken* InvalidToken);
        bool exitWithError(const io::stringc &Message, bool AppendTokenPos = true);
        
        bool validateBrackets();
        
        bool nextToken(bool IgnoreWhiteSpaces = true);
        bool nextToken(const ETokenTypes NextTokenType);
        bool nextTokenCheck(const ETokenTypes CheckTokenType);
        
        void append();
        void append(const io::stringc &Str);
        
        void pushIndent();
        void popIndent();
        
        void solveMacrosGLSL();
        bool solveMacroVectorGLSL(io::stringc &Name, const SDataTypeConversion &Type);
        
        bool solveAttributesGLSL();
        bool solveAttributeNumThreadsGLSL();
        bool solveAttributeMaxVertexCountGLSL();
        
        bool processEntryPointGLSL();
        bool processInputArgGLSL(SInputArgument &Arg);
        
        /* === Inline functions === */
        
        inline ETokenTypes type() const
        {
            return Tkn_->Type;
        }
        
        /* === Members === */
        
        TokenParser Parser_;
        TokenIteratorPtr TokenIt_;
        SToken* Tkn_;
        
        u32 Options_;
        io::stringc* OutString_;
        
        SInternalState State_;
        
        io::stringc IndentMask_;
        io::stringc Indent_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
