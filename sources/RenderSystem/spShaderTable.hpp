/*
 * Shader table header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHADER_TABLE_H__
#define __SP_SHADER_TABLE_H__


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


class Shader;


// !!!TODO!!! -> rename to "ShaderClass"
//! Shader tables are used to link several shaders (Vertex-, Pixel shaders etc.) to one shader program.
class SP_EXPORT ShaderTable : public BaseObject
{
    
    public:
        
        virtual ~ShaderTable();
        
        /**
        Binds the table with its shaders.
        \param Object: Pointer to a MaterialNode object which shall be used for the shader callback if set.
        */
        virtual void bind(const scene::MaterialNode* Object = 0) = 0;
        
        //! Unbinds the table with its shaders.
        virtual void unbind() = 0;
        
        //! Links the program with all its shaders.
        virtual bool link() = 0;
        
        /**
        Sets the shader object callback function.
        \param CallbackProc: Callback function in the form of
        "void Callback(ShaderTable* Table, const scene::MaterialNode* Object);".
        This callback normally is used to update the world- view matrix. In GLSL these matrices
        are integrated but in HLSL you have to set these shader-constants manually.
        */
        inline void setObjectCallback(const ShaderObjectCallback &CallbackProc)
        {
            ObjectCallback_ = CallbackProc;
        }
        
        /**
        Sets the shader surface callback function.
        \param CallbackProc: Callback function in the form of
        "void Callback(ShaderTable* Table, const std::vector<SMeshSurfaceTexture>* TextureList);".
        This callback normally is used to update texture settings for each surface.
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
        
        //! Returns true if the shader is a high level shader.
        inline bool isHighLevel() const
        {
            return HighLevel_;
        }
        //! Returns true if the shader table has been compiled successfully.
        inline bool valid() const
        {
            return CompiledSuccessfully_;
        }
        
    protected:
        
        friend class Shader;
        
        ShaderTable();
        
        /* Functions */
        
        void printError(const io::stringc &Message);
        void printWarning(const io::stringc &Message);
        
        /* Members */
        
        ShaderObjectCallback ObjectCallback_;
        ShaderSurfaceCallback SurfaceCallback_;
        
        Shader* VertexShader_;
        Shader* PixelShader_;
        Shader* GeometryShader_;
        Shader* HullShader_;
        Shader* DomainShader_;
        Shader* ComputeShader_;
        
        bool HighLevel_;
        bool CompiledSuccessfully_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
