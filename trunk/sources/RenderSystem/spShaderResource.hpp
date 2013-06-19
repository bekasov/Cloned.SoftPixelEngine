/*
 * Shader resource header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHADER_RESOURCE_H__
#define __SP_SHADER_RESOURCE_H__


#include "Base/spStandard.hpp"


namespace sp
{
namespace video
{


//! Shader resource types. Currently only supported for the Direct3D 11 render system.
enum EShaderResourceTypes
{
    SHADERRESOURCE_BUFFER,              //!< Default buffer. Can also be used with texture buffers.
    SHADERRESOURCE_STRUCT_BUFFER,       //!< Structured buffer. 'Stride' must be used.
    SHADERRESOURCE_RW_BUFFER,           //!< Read/write buffer. Can only be used in a pixel- or compute shader.
    SHADERRESOURCE_RW_STRUCT_BUFFER,    //!< Read/write structured buffer. Can only be used in a pixel- or compute shader.
};


/**
Shader resource interface class. Currently only used for Direct3D 11 render system.
A shader resource object represents a Buffer, RWBuffer, StructuredBuffer or RWStructuredBuffer in
HLSL Shader Model 5.
\todo Add support for GLSL equivalent buffer objects (Shader Storage Buffer Object).
\since Version 3.3
*/
class ShaderResource
{
    
    public:
        
        virtual ~ShaderResource()
        {
        }
        
        /* === Functions === */
        
        /**
        Creates the buffer with the given settings.
        \param[in] Type Specifies the new resource type.
        \param[in] Size Specifies the buffer size (in bytes).
        \param[in] Stride Specifies the stride (in bytes) for structured buffers.
        \param[in] Buffer Constant pointer to the source memory buffer for initialization.
        \return True if the buffer could be created successful. Otherwise false.
        \see EShaderResourceTypes
        */
        virtual bool setupBuffer(
            const EShaderResourceTypes Type, u32 Size, u32 Stride = 0, const void* Buffer = 0
        ) = 0;
        
        /**
        Copies the data from the specified buffer to the hardware buffer.
        \param[in] Buffer Constant pointer to the source buffer.
        \param[in] Size Specifies the buffer size (in bytes). By default 0 which
        specifies the whole buffer size given at create time when "setupBuffer" was called.
        \return True of success.
        \see setupBuffer
        \see readBuffer
        */
        virtual bool writeBuffer(const void* Buffer, u32 Size = 0) = 0;
        /**
        Copies the data from the hardware buffer to the specified buffer.
        \param[out] Buffer Pointer to the destination buffer.
        \param[in] Size Specifies the buffer size (in bytes). By default 0 which
        specifies the whole buffer size given at create time when "setupBuffer" was called.
        \return True of success.
        \see setupBuffer
        \see writeBuffer
        */
        virtual bool readBuffer(void* Buffer, u32 Size = 0) = 0;
        
        //! Returns the size (in bytes) of the shader resource.
        virtual u32 getSize() const = 0;

        /* === Inline functions === */
        
        /**
        Returns the shader resource type. By default SHADERRESOURCE_BUFFER.
        \see EShaderResourceTypes
        */
        inline EShaderResourceTypes getType() const
        {
            return Type_;
        }
        
        //! Returns the stride (in bytes) of the shader resource. By default 0.
        inline u32 getStride() const
        {
            return Stride_;
        }
        
        /**
        Returns the count of elements if 'stride' is greater than zero
        (count = size / stride) or just the size otherwise.
        */
        inline u32 getCount() const
        {
            return Stride_ > 0 ? getSize() / Stride_ : getSize();
        }

        //! Returns true if this shader resource is read only (on the GPU).
        inline bool readOnly() const
        {
            return Type_ <= SHADERRESOURCE_STRUCT_BUFFER;
        }

        /**
        Returns true if this shader resource is structured, i.e. from the type
        SHADERRESOURCE_STRUCT_BUFFER or SHADERRESOURCE_RW_STRUCT_BUFFER.
        \see EShaderResourceTypes
        */
        inline bool isStruct() const
        {
            return Type_ == SHADERRESOURCE_STRUCT_BUFFER || Type_ == SHADERRESOURCE_RW_STRUCT_BUFFER;
        }
        
    protected:
        
        /* === Functions === */
        
        ShaderResource(const EShaderResourceTypes Type) :
            Type_   (SHADERRESOURCE_BUFFER  ),
            Stride_ (0                      )
        {
        }
        
        /* === Members === */
        
        EShaderResourceTypes Type_;
        
        u32 Stride_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
