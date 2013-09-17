/*
 * Shader program header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHADER_PROGRAM_H__
#define __SP_SHADER_PROGRAM_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutput.hpp"
#include "Base/spDimension.hpp"
#include "Base/spVertexFormat.hpp"
#include "Base/spMaterialColor.hpp"
#include "RenderSystem/spShaderClass.hpp"

#include <vector>


namespace sp
{
namespace video
{


class ConstantBuffer;

/**
Shader compilation flags.
\since Version 3.3
\todo Currently unused. Must be added for "loadShader" and "createShader" functions in RenderSystem!
*/
enum EShaderCompilationFlags
{
    COMPILE_SHADER_NO_OPTIMIZATION      = 0x0001, //!< Optimization is disabled. By default optimization level 3 is used. Only supported for HLSL.
    COMPILE_SHADER_NO_VALIDATION        = 0x0002, //!< Validation against known capabilities and constraints are disabled. Only supported for HLSL.
    COMPILE_SHADER_AVOID_FLOW_CONTROL   = 0x0004, //!< Avoid flow control, i.e. unroll as many loops as possible. Only supported for HLSL.
    COMPILE_SHADER_PREFER_FLOW_CONTROL  = 0x0008, //!< Prefer flow control, i.e. keep as many loops as possible. Only supported for HLSL.
};


/**
Shader constant (or rather uniform) structure.
\ingroup group_shader
*/
struct SShaderConstant
{
    SShaderConstant() :
        Type    (CONSTANT_UNKNOWN   ),
        Count   (1                  ),
        Location(-1                 )
    {
    }
    ~SShaderConstant()
    {
    }
    
    /* === Operators === */
    
    inline bool operator == (const SShaderConstant &Other) const
    {
        return
            Type        == Other.Type       &&
            AltName     == Other.AltName    &&
            Name        == Other.Name       &&
            Count       == Other.Count      &&
            Location    == Other.Location;
    }
    
    /* === Functions === */
    
    /**
    Returns true if this is a valid shader constant (the 'Type' member must not
    be CONSTANT_UNKNOWN and the 'Location' member must not be -1).
    */
    inline bool valid() const
    {
        return Type != CONSTANT_UNKNOWN && Location != -1;
    }
    
    /* === Members === */
    
    EConstantTypes Type;    //!< Data type.
    io::stringc Name;       //!< Name of the uniform.
    /**
    Alternative name. This is used for arrays (but not for arrays of structs) in GLSL
    (e.g. Name = "MyUniformArray[0]" and AltName = "MyUniformArray").
    In this case you don't need to write "[0]" for each uniform array.
    */
    io::stringc AltName;
    u32 Count;              //!< Count of elements. 1 if this is not an array.
    s32 Location;           //!< Uniform location (only used for OpenGL).
};


/**
Shader objects are used for high- or low level shader effects. Supported are GLSL (OpenGL Shading Language),
HLSL (DirectX High Level Shading Language), OpenGL ARB Vertex- and Fragement Programs and DirectX Vertex- and Pixel Programs.
\ingroup group_shader
*/
class SP_EXPORT Shader
{
    
    public:
        
        Shader(ShaderClass* ShdClass, const EShaderTypes Type, const EShaderVersions Version);
        virtual ~Shader();
        
        /* === Functions === */
        
        /**
        Compiles the given shader source code.
        \param[in] ShaderBuffer Specifies the shader source code in form of a string container.
        Each string line has to end with the end-of-line character ('\n')!
        \param[in] EntryPoint Specifies the shader entry point. This not required for GLSL shaders.
        But for HLSL and Cg. It's actually just the shader's main function name.
        \param[in] CompilerOptions Specifies the pointer to the comiler option strings.
        \param[in] Flags Additional shader compilation flags. This can be a
        combination of the 'EShaderLoadingFlags' enumeration values.
        \return True if the shader could be compiled successful.
        \see EShaderLoadingFlags
        */
        virtual bool compile(
            const std::list<io::stringc> &ShaderBuffer,
            const io::stringc &EntryPoint = "",
            const c8** CompilerOptions = 0,
            u32 Flags = 0
        );
        
        /**
        Returns a pointer to the specified shader constant.
        \param[in] Name Specifies the shader constant name (or rather shader uniform).
        \return Constant pointer to the specified shader constant or null if the shader constant does not exist.
        */
        virtual const SShaderConstant* getConstantRef(const io::stringc &Name) const;
        /**
        Returns the specified shader constant.
        \param[in] Name Specifies the shader constant name (or rather shader uniform).
        \return Constant reference to the SShaderConstant object. If the shader constant
        does not exist, the default dempty shader constant will be returned.
        */
        virtual const SShaderConstant& getConstant(const io::stringc &Name) const;
        
        /**
        Returns the specified constant buffer (or rather uniform buffer block).
        \param[in] Name Specifies the buffer block name in the shader.
        \return Pointer to the ConstantBuffer object or null if the active render system
        does not support constant buffers.
        \note Only the OpenGL 4 and Direct3D 11 render systems support constant buffers.
        \see ConstantBuffer
        */
        virtual ConstantBuffer* getConstantBuffer(const io::stringc &Name) const;
        
        /* === Index-based constant functions === */
        
        virtual bool setConstant(s32 Number, const EConstantTypes Type, const f32 Value);
        virtual bool setConstant(s32 Number, const EConstantTypes Type, const f32* Buffer, s32 Count);
        
        //virtual bool setConstant(s32 Number, const EConstantTypes Type, const s32 Value);
        //virtual bool setConstant(s32 Number, const EConstantTypes Type, const s32* Buffer, s32 Count);
        
        virtual bool setConstant(s32 Number, const EConstantTypes Type, const dim::vector3df &Position);
        virtual bool setConstant(s32 Number, const EConstantTypes Type, const video::color &Color);
        virtual bool setConstant(s32 Number, const EConstantTypes Type, const dim::matrix4f &Matrix);
        
        /* === String-based constant functions === */
        
        virtual bool setConstant(const io::stringc &Name, const f32 Value);
        virtual bool setConstant(const io::stringc &Name, const f32* Buffer, s32 Count);
        
        virtual bool setConstant(const io::stringc &Name, const s32 Value);
        virtual bool setConstant(const io::stringc &Name, const s32* Buffer, s32 Count);
        
        virtual bool setConstant(const io::stringc &Name, const dim::vector3df &Position);
        virtual bool setConstant(const io::stringc &Name, const dim::vector4df &Position);
        virtual bool setConstant(const io::stringc &Name, const video::color &Color);
        virtual bool setConstant(const io::stringc &Name, const dim::matrix4f &Matrix);
        
        /* === Structure-based constnat functions === */
        
        virtual bool setConstant(const SShaderConstant &Constant, const f32 Value);
        virtual bool setConstant(const SShaderConstant &Constant, const f32* Buffer, s32 Count);
        
        virtual bool setConstant(const SShaderConstant &Constant, const s32 Value);
        virtual bool setConstant(const SShaderConstant &Constant, const s32* Buffer, s32 Count);
        
        //virtual bool setConstant(const SShaderConstant &Constant, const dim::point2df &Vector);
        virtual bool setConstant(const SShaderConstant &Constant, const dim::vector3df &Vector);
        virtual bool setConstant(const SShaderConstant &Constant, const dim::vector4df &Vector);
        virtual bool setConstant(const SShaderConstant &Constant, const video::color &Color);
        //virtual bool setConstant(const SShaderConstant &Constant, const dim::matrix2f &Matrix);
        //virtual bool setConstant(const SShaderConstant &Constant, const dim::matrix3f &Matrix);
        virtual bool setConstant(const SShaderConstant &Constant, const dim::matrix4f &Matrix);
        
        /* === Other constant functions === */
        
        /**
        Sets floating-point constant arrays for assembly shaders.
        \param[in] Buffer Pointer to a floating-point buffer.
        \param[in] StartRegister Specifies the register start offset (in 4 bytes).
        \param[in] ConstAmount Specifies the count of constants (or rather array size).
        */
        virtual bool setConstant(const f32* Buffer, s32 StartRegister, s32 ConstAmount);
        
        /**
        Fills a constant buffer with content (actual only for HLSL ShaderModel 4+).
        \param Name: Constant buffer's name which is to be used.
        \param Buffer: Pointer to the buffer where the content for the constant buffer is stored.
        This buffer needs to have the same size like the constant buffer in the shader!
        */
        virtual bool setConstantBuffer(const io::stringc &Name, const void* Buffer);
        
        //! \param[in] Number Specifies the index number of the constant buffer which is to be used.
        virtual bool setConstantBuffer(u32 Number, const void* Buffer);
        
        /**
        Returns the shader description name (e.g. "vertex", "pixel" etc.).
        \since Version 3.3
        */
        io::stringc getDescription() const;
        
        /* === Static functions === */
        
        //! Returns a shader compiler option. This is equivalent to: "#define" + Op + "\n".
        static io::stringc getOption(const io::stringc &Op);
        
        /**
        Adds a shader compiler option to the given list. The behaivour of this function is equivalent to the following code:
        \code
        ShaderCompilerOp.push_back("#define " + Op + "\n");
        \endcode
        \param[in,out] ShaderCompilerOp Specifies the list of strings which is to be extended.
        \param[in] Op Specifies the new compiler option.
        \param[in] UseGuard Specifies whether a pre-processor compiler directive is to be
        used or not, to check if the macro has already been defined. By default false
        */
        static void addOption(
            std::list<io::stringc> &ShaderCompilerOp, const io::stringc &Op, bool UseGuard = false
        );
        
        /**
        Adds the shader core to the given source code. This should be added to the front of the list.
        It contains a lot of macros so that shaders can be written mainly independently of
        the underlying render system, e.g. for GLSL you can also use float2, float4x4 etc.
        \param[out] ShaderCode Specifies the resulting shader source code string list.
        \param[in] UseCg Specifies whether Cg shaders are to be used or not. By default false.
        \note This does not work without an active graphics context.
        */
        static void addShaderCore(std::list<io::stringc> &ShaderCode, bool UseCg = false);
        
        /* === Inline functions === */
        
        //! Returns the shader type.
        inline EShaderTypes getType() const
        {
            return Type_;
        }
        //! Returns the shader model version.
        inline EShaderVersions getVersion() const
        {
            return Version_;
        }
        //! Returns a pointer to the shader class.
        inline ShaderClass* getShaderClass() const
        {
            return ShdClass_;
        }
        
        //! Returns the list of all shader constants.
        inline const std::vector<SShaderConstant>& getConstantList() const
        {
            return ConstantList_;
        }
        //! Returns the count of shader constants.
        inline u32 getConstantCount() const
        {
            return ConstantList_.size();
        }
        
        //! Returns the list of all shader constant buffers.
        inline const std::vector<ConstantBuffer*>& getConstantBufferList() const
        {
            return ConstantBufferList_;
        }
        //! Returns the count of shader constant buffers.
        inline u32 getConstantBufferCount() const
        {
            return ConstantBufferList_.size();
        }
        
        //! Returns true if the shader has been compiled successfully otherwise false.
        inline bool valid() const
        {
            return CompiledSuccessfully_;
        }
        
        //! Returns true if this is a high-level shader.
        inline bool isHighLevel() const
        {
            return getType() >= SHADER_VERTEX;
        }
        
    protected:
        
        /* === Functions === */
        
        void updateShaderClass();
        
        void printError(const io::stringc &Message);
        void printWarning(const io::stringc &Message);
        
        static void createProgramString(const std::list<io::stringc> &ShaderBuffer, c8* &ProgramBuffer);
        
        /* === Members === */
        
        EShaderTypes Type_;
        EShaderVersions Version_;
        ShaderClass* ShdClass_;
        
        std::vector<SShaderConstant> ConstantList_;
        std::vector<ConstantBuffer*> ConstantBufferList_;
        
        bool CompiledSuccessfully_;
        
        static SShaderConstant EmptyConstant_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
