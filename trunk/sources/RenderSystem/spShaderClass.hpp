/*
 * Shader class header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHADER_CLASS_H__
#define __SP_SHADER_CLASS_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutput.hpp"
#include "Base/spBaseObject.hpp"
#include "RenderSystem/spShaderConfigTypes.hpp"
#include "RenderSystem/spComputeShader.hpp"


namespace sp
{

namespace scene { class MaterialNode; }

namespace video
{


class ConstantBuffer;

/**
Build shader flags. This is used for the two static functions "ShaderClass::getShaderVersion" and "ShaderClass::build".
\since Version 3.2
*/
enum EBuildShaderFlags
{
    SHADERBUILD_CG          = 0x0002,
    SHADERBUILD_GLSL        = 0x0004,
    SHADERBUILD_HLSL3       = 0x0008,
    SHADERBUILD_HLSL5       = 0x0010,
    
    SHADERBUILD_VERTEX      = 0x0100,
    SHADERBUILD_PIXEL       = 0x0200,
    SHADERBUILD_GEOMETRY    = 0x0400,
    SHADERBUILD_HULL        = 0x0800,
    SHADERBUILD_DOMAIN      = 0x1000,
};

/**
C-pre-processor directives used for shading languages.
\todo Move this to a token parser/ lexer or something like that.
\since Version 3.3
*/
enum ECPPDirectives
{
    CPPDIRECTIVE_NONE,              //!< Invalid CPP directive.
    CPPDIRECTIVE_INCLUDE_STRING,    //!< #include "HeaderFile.h"
    CPPDIRECTIVE_INCLUDE_BRACE,     //!< #include <core>
};


class Shader;
class ShaderResource;
class VertexFormat;

/**
Shader classes are used to link several shaders (Vertex-, Pixel shaders etc.) to one shader program.
Modern graphics hardware has the following shader stages:
- Vertex Shader
- Hull Shader (In OpenGL "Tessellation Control" Shader)
- Domain Shader (In OpenGL "Tessellation Evaluation" Shader)
- Geometry Shader
- Pixel Shader (In OpenGL "Fragment" Shader)
- Compute Shader (This is seperated from the graphics pipeline)
\ingroup group_shader
*/
class SP_EXPORT ShaderClass : public BaseObject
{
    
    public:
        
        virtual ~ShaderClass();
        
        /* === Functions === */
        
        /**
        Binds the table with its shaders.
        \param[in] Object Pointer to a MaterialNode object which shall be used for the shader callback if set.
        */
        virtual void bind(const scene::MaterialNode* Object = 0) = 0;
        
        //! Unbinds the table with its shaders.
        virtual void unbind() = 0;
        
        /**
        Links the program with all its shaders.
        \todo Rename this to "compile"
        */
        virtual bool link() = 0;
        
        /**
        Adds the specified shader resource object.
        \param[in] Resource Pointer to the shader resource object which is to be added.
        \note At first all textures are bound to a shader and then all shader resources.
        This is order is important for the resource registers in the shader.
        \see ShaderResource
        \see addRWTexture
        \since Version 3.3
        */
        virtual void addShaderResource(ShaderResource* Resource);
        /**
        Removes the specified shader resource object.
        \see addShaderResource
        \since Version 3.3
        */
        virtual void removeShaderResource(ShaderResource* Resource);
        virtual void clearShaderResources();
        
        /**
        Adds the specified shader R/W texture object.
        \param[in] Tex Pointer to the R/W texture object which is to be added.
        In this case the texture must be an R/W texture, i.e. from the type TEXTURE_*_RW.
        \see addShaderResource
        \see Texture
        \see ETextureTypes
        \since Version 3.3
        */
        virtual void addRWTexture(Texture* Tex);
        /**
        Removes the specified shader R/W texture object.
        \see addRWTexture
        \since Version 3.3
        */
        virtual void removeRWTexture(Texture* Tex);
        virtual void clearRWTextures();
        
        /* === Static functions === */
        
        /**
        Returns the shader version used for the specified flags.
        \param[in] Flags Specifies the build flags. This can be a combination
        of the values in the "EBuildShaderFlags" enumeration.
        \return Shader version specified in the "EShaderVersions" enumeration.
        If no version could be found "DUMMYSHADER_VERSION" will be returned.
        \see EBuildShaderFlags
        \since Version 3.2
        */
        static EShaderVersions getShaderVersion(s32 Flags);
        
        /**
        Builds a complete shader class with the specified vertex-format,
        shader source code and build flags.
        This is particularly used internally for the deferred-renderer and post-processing effects.
        \param[in] Name Specifies the shader name and is used for possible error messages.
        \param[out] ShdClass Specifies the resulting shader class object.
        \param[in] VertFmt Pointer to the vertex format used for the shader class.
        \param[in] ShdBufferVertex Constant pointer to the vertex shader source code (std::list<io::stringc>).
        \param[in] ShdBufferPixel Constant pointer to the pixel shader source code (std::list<io::stringc>).
        \param[in] VertexMain Specifies the name of the vertex shader main function.
        \param[in] PixelMain Specifies the name of the pixel shader main function.
        \param[in] Flags Specifies the compilation flags. This can be one of the following values:
        SHADERBUILD_CG, SHADERBUILD_GLSL, SHADERBUILD_HLSL3 or SHADERBUILD_HLSL5.
        \return True if the shader class could be created successful.
        \note This function always failes if "ShdBufferVertex" is null pointers.
        \see VertexFormat
        \see EBuildShaderFlags
        \since Version 3.2
        */
        static bool build(
            const io::stringc &Name,
            
            ShaderClass* &ShdClass,
            VertexFormat* VertFmt,
            
            const std::list<io::stringc>* ShdBufferVertex,
            const std::list<io::stringc>* ShdBufferPixel,
            
            const io::stringc &VertexMain = "VertexMain",
            const io::stringc &PixelMain = "PixelMain",
            
            s32 Flags = SHADERBUILD_CG
        );
        
        /**
        Loads a shader resource file and parses it for '#include' directives.
        \param[in] FileSys Specifies the file system you want to use.
        \param[in] Filename Specifies the filename of the shader resource which is to be loaded.
        \param[in,out] ShaderBuffer Specifies the shader source code container which is to be filled.
        \param[in] UseCg Specifies whether Cg shaders are to be used or not. By default false.
        \since Version 3.3
        */
        static bool loadShaderResourceFile(
            io::FileSystem &FileSys, const io::stringc &Filename, std::list<io::stringc> &ShaderBuffer, bool UseCg = false
        );
        
        /**
        Determines whether the given string has an '#include' directive.
        \param[in] Line Specifies the string (or rather source code line) which is to be parsed.
        \param[out] Filename Specifies the output filename which can be expressed between the
        quotation marks inside the '#include' directive (e.g. #include "HeaderFile.h" -> will
        result in the string "HeaderFile.h").
        \return CPPDIRECTIVE_INCLUDE_STRING if the given string has an '#include' directive with a string,
        CPPDIRECTIVE_INCLUDE_BRACE if the given string has an '#include' directive with brace or CPPDIRECTIVE_NONE otherwise.
        \note This is actually only used by the "loadShaderResourceFile" function.
        \see ECPPDirectives
        \see loadShaderResourceFile
        \since Version 3.3
        */
        static ECPPDirectives parseIncludeDirective(const io::stringc &Line, io::stringc &Filename);
        
        /* === Inline functions === */
        
        /**
        Sets the shader object callback function.
        \param CallbackProc: Callback function in the form of
        "void Callback(ShaderClass* Table, const scene::MaterialNode* Object);".
        This callback normally is used to update the world- view matrix. In GLSL these matrices
        are integrated but in HLSL you have to set these shader-constants manually.
        */
        inline void setObjectCallback(const ShaderObjectCallback &CallbackProc)
        {
            ObjectCallback_ = CallbackProc;
        }
        
        /**
        Sets the shader surface callback function.
        \param[in] CallbackProc Specifies the surface callback function.
        This callback normally is used to update texture settings for each surface.
        \see ShaderSurfaceCallback
        */
        inline void setSurfaceCallback(const ShaderSurfaceCallback &CallbackProc)
        {
            SurfaceCallback_ = CallbackProc;
        }
        
        inline Shader* getVertexShader() const
        {
            return VertexShader_;
        }
        inline Shader* getPixelShader() const
        {
            return PixelShader_;
        }
        inline Shader* getGeometryShader() const
        {
            return GeometryShader_;
        }
        inline Shader* getHullShader() const
        {
            return HullShader_;
        }
        inline Shader* getDomainShader() const
        {
            return DomainShader_;
        }
        inline Shader* getComputeShader() const
        {
            return ComputeShader_;
        }
        
        /**
        Returns the list of all shader constant buffers used in the shader-class.
        To get the list of all shader constant buffers used in a single shader object,
        use the equivalent function of the respective shader.
        \see Shader::getConstantBufferList
        */
        inline const std::vector<ConstantBuffer*>& getConstantBufferList() const
        {
            return ConstBufferList_;
        }
        //! Returns the count of shader constant buffers.
        inline size_t getConstantBufferCount() const
        {
            return ConstBufferList_.size();
        }

        //! Returns the list of all shader resources.
        inline const std::vector<ShaderResource*>& getShaderResourceList() const
        {
            return ShaderResources_;
        }
        //! Returns the count of shader resources.
        inline size_t getShaderResourceCount() const
        {
            return ShaderResources_.size();
        }
        
        //! Returns the list of all R/W textures.
        inline const std::vector<Texture*>& getRWTextureList() const
        {
            return RWTextures_;
        }
        //! Returns the count of R/W textures.
        inline size_t getRWTextureCount() const
        {
            return RWTextures_.size();
        }
        
        //! Returns true if the shader is a high level shader.
        inline bool isHighLevel() const
        {
            return HighLevel_;
        }
        //! Returns true if the shader class has been compiled successfully.
        inline bool valid() const
        {
            return CompiledSuccessfully_;
        }
        
    protected:
        
        friend class Shader;
        
        ShaderClass();
        
        /* === Functions === */
        
        void printError(const io::stringc &Message);
        void printWarning(const io::stringc &Message);
        
        /* === Members === */
        
        ShaderObjectCallback ObjectCallback_;
        ShaderSurfaceCallback SurfaceCallback_;
        
        Shader* VertexShader_;
        Shader* PixelShader_;
        Shader* GeometryShader_;
        Shader* HullShader_;
        Shader* DomainShader_;
        Shader* ComputeShader_;
        
        std::vector<ConstantBuffer*> ConstBufferList_;  //!< List of constant buffers of all shaders in the shader-class.
        std::vector<ShaderResource*> ShaderResources_;
        std::vector<Texture*> RWTextures_;

        bool HighLevel_;
        bool CompiledSuccessfully_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
