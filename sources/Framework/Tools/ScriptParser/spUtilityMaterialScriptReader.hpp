/*
 * Material script reader header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILITY_MATERIAL_SCRIPT_READER_H__
#define __SP_UTILITY_MATERIAL_SCRIPT_READER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MATERIAL_SCRIPT


#include "Framework/Tools/ScriptParser/spUtilityScriptReaderBase.hpp"
#include "Base/spMaterialStates.hpp"
#include "Base/spMaterialColor.hpp"
#include "Base/spMaterialConfigTypes.hpp"
#include "RenderSystem/spShaderConfigTypes.hpp"

#include <map>
#include <string>


namespace sp
{
namespace video
{
    class MaterialStates;
    class ShaderClass;
    class VertexFormat;
}
namespace tool
{


/**
Script material reader/writer class. This reader can parse script files with an engine specific syntax.
For more information take a look in the FAQ of the softpixel engine community:
http://softpixelengine.sourceforge.net/forum/
\ingroup group_script
\since Version 3.3
*/
class SP_EXPORT MaterialScriptReader : public ScriptReaderBase
{
    
    public:
        
        MaterialScriptReader();
        virtual ~MaterialScriptReader();
        
        /* === Functions === */
        
        /**
        Loads the specified material script. Those scripts may contain declarations for:
        materials, shaders, vertex formats, textures and texture layers.
        \param[in] Filename Specifies the script file which is to be loaded.
        \return True if the script has been loaded successful. Otherwise the script has semantic or syntax errors.
        In this case the parsing errors will be printed to the log output.
        \see io::Log
        \see findMaterial
        \see findShader
        */
        bool loadScript(const io::stringc &Filename);
        
        /**
        Saves the specified script. Those scripts may contain materials only.
        All material names contain index numbers only.
        \param[in] Filename Specifies the script file which is to be saves.
        \param[in] Materials Specifies the material states list.
        \return True if the script has been saved successful. Otherwise the file could not be created.
        \see video::MaterialStates
        */
        bool saveScript(
            const io::stringc &Filename, const std::vector<const video::MaterialStates*> &Materials
        );
        
        video::MaterialStatesPtr findMaterial(const io::stringc &Name);
        video::ShaderClass* findShader(const io::stringc &Name);
        
        bool defineString(const io::stringc &VariableName, const io::stringc &Str);
        bool defineNumber(const io::stringc &VariableName, f64 Number);
        
        /**
        Parses the format name to a pre-defined vertex format.
        You can overwrite this function to support additional vertex formats.
        \param[in] FormatName Specifies the format name. Pre-defined formats are
        "vertexFormatDefault", "vertexFormatReduced", "vertexFormatExtended" and "vertexFormatFull".
        \return Constant pointer to the parsed vertex format.
        */
        virtual const video::VertexFormat* parseVertexFormat(const io::stringc &FormatName) const;
        
        /* === Static functions === */
        
        static video::EShadingTypes         parseShading        (const io::stringc &Identifier);
        static video::ESizeComparisionTypes parseCompareType    (const io::stringc &Identifier);
        static video::EBlendingTypes        parseBlendType      (const io::stringc &Identifier);
        static video::EWireframeTypes       parseWireframe      (const io::stringc &Identifier);
        static video::EFaceTypes            parseFaceType       (const io::stringc &Identifier);
        static video::EShaderTypes          parseShaderType     (const io::stringc &Identifier);
        static video::EShaderVersions       parseShaderVersion  (const io::stringc &Identifier);
        
        /* === Inline functions === */
        
        inline const std::map<std::string, video::MaterialStatesPtr>& getMaterialList() const
        {
            return Materials_;
        }
        inline const std::map<std::string, video::ShaderClass*>& getShaderList() const
        {
            return Shaders_;
        }
        
    protected:
        
        /* === Functions === */
        
        void printUnknownVar(const io::stringc &VariableName) const;
        
        //! Returns true if variable 'VariableName' already exists.
        bool hasVariable(const io::stringc &VariableName) const;
        bool isVariableFree(const io::stringc &VariableName) const;
        
        void registerString(const io::stringc &VariableName, const io::stringc &Str);
        void registerNumber(const io::stringc &VariableName, f64 Number);
        
        bool getVarValue(const io::stringc &VariableName, io::stringc &StrVal, f64 &NumVal, bool &IsStr) const;
        io::stringc getString(const io::stringc &VariableName) const;
        f64 getNumber(const io::stringc &VariableName) const;
        
        void breakEOF();
        void breakUnexpectedToken();
        void breakUnexpectedIdentifier();
        void breakExpectedIdentifier();
        void breakExpectedAssignment();
        void breakExpectedString();
        void breakSingleNumberOnly();
        void breakStringCombination();
        
        void nextTokenNoEOF(bool IgnoreWhiteSpaces = true);
        void ignoreNextBlock();
        
        void addMaterial(const io::stringc &Name);
        void addShader(const io::stringc &Name, const video::VertexFormat* InputLayout);
        
        void readMaterial();
        void readMaterialState();
        
        void readShaderClass();
        void readShaderType();
        void readShader();
        void readAllShaderPrograms();
        void readShaderProgram(const video::EShaderTypes ShaderType);
        void readShaderProgramCode();
        
        void readVertexFormat();
        
        void readVarDefinition();
        
        void readAssignment();
        void readBlockBegin();
        io::stringc readVarName();
        
        f64             readDouble      (bool ReadAssignment = true);
        io::stringc     readString      (bool ReadAssignment = true);
        io::stringc     readIdentifier  (bool ReadAssignment = true);
        bool            readBool        (bool ReadAssignment = true);
        video::color    readColor       (bool ReadAssignment = true);
        
        void clearVariables();
        void checkShaderVersion();
        
        /**
        Returns true if the specified shader type is valid for the current render system.
        \param[in] Name Specifies the shader type.
        Can be "glsl" (for OpenGL), "glslEs" (for OpenGL|ES), "hlsl3" (for Direct3D 9) or "hlsl5" (for Direct3D 11).
        \return True if the shader is valid.
        */
        bool validShaderForRenderSys(const io::stringc &Name) const;
        
        /**
        Reads the next script block. Pre-defined blocks are "material" and "shader".
        You can overwrite this function to also read your own script blocks.
        \return True if a pre-defined block has been read. Otherwise false.
        */
        virtual bool readScriptBlock();
        
        virtual void defineDefaultVariables();
        
        /* === Templates === */
        
        template <typename T> T readNumber(bool ReadAssignment = true)
        {
            return static_cast<T>(readDouble(ReadAssignment));
        }
        
        /* === Members === */
        
        std::map<std::string, video::MaterialStatesPtr> Materials_;
        std::map<std::string, video::ShaderClass*> Shaders_;
        
        std::map<std::string, io::stringc> StringVariables_;
        std::map<std::string, f64> NumericVariables_;
        
        video::MaterialStatesPtr CurMaterial_;
        video::ShaderClass* CurShader_;
        
        video::EShaderVersions CurShaderVersion_;
        std::list<io::stringc> CurShaderBuffer_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
