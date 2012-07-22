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
#include "RenderSystem/spShaderTable.hpp"

#include <vector>


namespace sp
{
namespace video
{


//! Shader constant (or rather uniform) structure.
struct SShaderConstant
{
    SShaderConstant()
        : Type(CONSTANT_UNKNOWN), Count(1)
    {
    }
    ~SShaderConstant()
    {
    }
    
    /* Operators */
    inline bool operator == (const SShaderConstant &other) const
    {
        return
            Type == other.Type &&
            Name == other.Name &&
            Count == other.Count;
    }
    
    /* Members */
    EConstantTypes Type;    //!< Data type.
    io::stringc Name;       //!< Name of the uniform.
    s32 Count;              //!< Count of elements. 1 if this is not an array otherwise.
};


/**
Shader objects are used for high- or low level shader effects. Supported are GLSL (OpenGL Shading Language),
HLSL (DirectX High Level Shading Language), OpenGL ARB Vertex- and Fragement Programs and DirectX Vertex- and Pixel Programs.
*/
class SP_EXPORT Shader
{
    
    public:
        
        Shader(ShaderTable* Table, const EShaderTypes Type, const EShaderVersions Version);
        virtual ~Shader();
        
        /* === Functions === */
        
        virtual bool compile(const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "");
        
        virtual bool setConstant(s32 Number, const EConstantTypes Type, const f32 Value);
        virtual bool setConstant(s32 Number, const EConstantTypes Type, const f32* Buffer, s32 Count);
        
        //virtual bool setConstant(s32 Number, const EConstantTypes Type, const s32 Value);
        //virtual bool setConstant(s32 Number, const EConstantTypes Type, const s32* Buffer, s32 Count);
        
        virtual bool setConstant(s32 Number, const EConstantTypes Type, const dim::vector3df &Position);
        virtual bool setConstant(s32 Number, const EConstantTypes Type, const video::color &Color);
        virtual bool setConstant(s32 Number, const EConstantTypes Type, const dim::matrix4f &Matrix);
        
        virtual bool setConstant(const io::stringc &Name, const f32 Value);
        virtual bool setConstant(const io::stringc &Name, const f32* Buffer, s32 Count);
        
        virtual bool setConstant(const io::stringc &Name, const s32 Value);
        virtual bool setConstant(const io::stringc &Name, const s32* Buffer, s32 Count);
        
        virtual bool setConstant(const io::stringc &Name, const dim::vector3df &Position);
        virtual bool setConstant(const io::stringc &Name, const dim::vector4df &Position);
        virtual bool setConstant(const io::stringc &Name, const video::color &Color);
        virtual bool setConstant(const io::stringc &Name, const dim::matrix4f &Matrix);
        
        /**
        Sets floating-point constant arrays for assembly shaders
        \param pFloats: Pointer to a floating-point buffer.
        \param StartRegister: Constant register start offset (in 4 bytes).
        \param ConstAmount: Count of constants (or rather array size).
        */
        virtual bool setConstant(const f32* Buffer, s32 StartRegister, s32 ConstAmount);
        
        /**
        Fills a constant buffer with content (actual only for HLSL ShaderModel 4+).
        \param Name: Constant buffer's name which is to be used.
        \param Buffer: Pointer to the buffer where the content for the constant buffer is stored.
        This buffer needs to have the same size like the constant buffer in the shader!
        */
        virtual bool setConstantBuffer(const io::stringc &Name, const void* Buffer);
        
        //! \param Number: Index number of the constant buffer which is to be used.
        virtual bool setConstantBuffer(u32 Number, const void* Buffer);
        
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
        inline ShaderTable* getShaderTable() const
        {
            return Table_;
        }
        
        //! Returns the list of all shader constants.
        inline const std::vector<SShaderConstant>& getConstantList() const
        {
            return ConstantList_;
        }
        //! Returns the number of shader constants.
        inline u32 getConstantCount() const
        {
            return ConstantList_.size();
        }
        
        //! Returns true if the shader has been compiled successfully otherwise false.
        inline bool valid() const
        {
            return CompiledSuccessfully_;
        }
        
        //! Returns true if this is a high-level shader.
        inline bool isHighLevel() const
        {
            return HighLevel_;
        }
        
    protected:
        
        /* === Functions === */
        
        void updateTable();
        
        void printError(const io::stringc &Message);
        void printWarning(const io::stringc &Message);
        
        /* === Members === */
        
        EShaderTypes Type_;
        EShaderVersions Version_;
        ShaderTable* Table_;
        
        std::vector<SShaderConstant> ConstantList_;
        
        bool HighLevel_;
        bool OwnShaderTable_;
        bool CompiledSuccessfully_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
