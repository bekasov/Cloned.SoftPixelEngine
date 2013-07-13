/*
 * Shader resource header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHADER_RESOURCE_H__
#define __SP_SHADER_RESOURCE_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionPoint2D.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "RenderSystem/spTextureFlags.hpp"


namespace sp
{
namespace video
{


//! Shader resource types. Currently only supported for the Direct3D 11 render system.
enum EShaderResourceTypes
{
    //! Default buffer. Can also be used with texture buffers. 'Buffer' in HLSL.
    SHADERRESOURCE_BUFFER,
    //! Structured buffer. 'Stride' must be used. 'StructuredBuffer' in HLSL.
    SHADERRESOURCE_STRUCT_BUFFER,
    //! Read/write buffer. Can only be used in a pixel- or compute shader. 'RWBuffer' in HLSL.
    SHADERRESOURCE_RW_BUFFER,
    //! Read/write structured buffer. Can only be used in a pixel- or compute shader. 'RWStructuredBuffer' in HLSL.
    SHADERRESOURCE_RW_STRUCT_BUFFER,
    /**
    Read/write structured buffer with hidden counter. Enables the buffer to increment and
    decrement the hidden counter in a shader. 'RWStructuredBuffer' in HLSL.
    */
    SHADERRESOURCE_COUNTER_RW_STRUCT_BUFFER,
    /**
    Read/write append-/ consumer structured buffer. Enables the buffer to append or remove
    elements from the end of the buffer. 'AppendStructuredBuffer' or 'ConsumeStructuredBuffer' in HLSL.
    */
    SHADERRESOURCE_APPEND_STRUCT_BUFFER,
    //! Byte address buffer. 'ByteAddressBuffer' in HLSL.
    SHADERRESOURCE_BYTE_BUFFER,
    //! Read/write byte address buffer. 'RWByteAddressBuffer' in HLSL.
    SHADERRESOURCE_RW_BYTE_BUFFER,
};

/**
Extended flags for setting up shader resource buffer.
This is only useful for the "ShaderResource::setupBufferRW" function.
\see ShaderResource::setupBufferRW
*/
enum EShaderResourceBufferFalgs
{
    SHADERBUFFERFLAG_COUNTER    = 0x01,
    SHADERBUFFERFLAG_APPEND     = 0x02,
};


/**
Shader resource interface class. Currently only used for Direct3D 11 render system.
For HLSL shader model 5 a shader resource object represents a Buffer, RWBuffer, StructuredBuffer, RWStructuredBuffer etc.
\todo Add support for GLSL equivalent buffer objects: the SSBO (Shader Storage Buffer Object).
\since Version 3.3
\ingroup group_shader
*/
class ShaderResource
{
    
    public:
        
        virtual ~ShaderResource()
        {
        }
        
        /* === Functions === */
        
        /**
        Creates a read-only buffer with the given settings. For a more detailed configurable version of this
        function use "setupBufferRaw".
        \tparam T Specifies the buffer element type. If you have a default buffer you can use the basic types
        like integer, float, point2d, vector3d and vector4d. In this case the resource type is SHADERRESOURCE_BUFFER.
        If you have a structured-buffer you can use your own structures. In this case the
        resource type is SHADERRESOURCE_STRUCT_BUFFER.
        \param[in] ElementCount Specifies the count of element for this buffer. Must be greater than zero.
        \param[in] Buffer Constant pointer to the initialization buffer data. This may also be null.
        \return True if the resource could be created successful.
        \note For read/write access use the "setupBufferRW" function.
        \see setupBufferRaw
        \see setupBufferRW
        */
        template <typename T> bool inline setupBuffer(u32 ElementCount, const void* Buffer = 0)
        {
            return setupBufferTyped<T>(ElementCount, false, Buffer, 0);
        }

        /**
        Creates a read/write buffer with the given settings. For a more detailed configurable version of this
        function use "setupBufferRaw".
        \tparam T Specifies the buffer element type. If you have a default buffer you can use the basic types
        like integer, float, point2d, vector3d and vector4d. In this case the resource type is SHADERRESOURCE_RW_BUFFER.
        If you have a structured-buffer you can use your own structures. In this case the
        resource type is SHADERRESOURCE_RW_STRUCT_BUFFER.
        \param[in] ElementCount Specifies the count of element for this buffer. Must be greater than zero.
        \param[in] Buffer Constant pointer to the initialization buffer data. This may also be null.
        \return True if the resource could be created successful.
        \note For read-only access use the "setupBuffer" function.
        \see setupBufferRaw
        \see setupBuffer
        */
        template <typename T> bool inline setupBufferRW(u32 ElementCount, const void* Buffer = 0, s32 Flags = 0)
        {
            return setupBufferTyped<T>(ElementCount, true, Buffer, Flags);
        }

        /**
        Creates the buffer with the given settings.
        \param[in] Type Specifies the new resource type.
        \param[in] ElementCount Specifies the count of elements.
        \param[in] Stride Specifies the size of each element (in bytes).
        \param[in] DataType Specifies the element data type.
        \param[in] DataSize Specifies the count of element components (X -> 1, XY -> 2, XYZ -> 3, XYZW -> 4).
        \param[in] Buffer Constant pointer to the source memory buffer for initialization.
        \return True if the buffer could be created successful. Otherwise false.
        \see EShaderResourceTypes
        \see EShaderResourceBufferFalgs
        \see setupBuffer
        \see setupBufferRW
        */
        virtual bool setupBufferRaw(
            const EShaderResourceTypes Type, u32 ElementCount, u32 Stride,
            const ERendererDataTypes DataType, u32 DataSize, const void* Buffer = 0
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
        
        /**
        Copies the data from the specified source hardware buffer to this hardware buffer.
        \param[in] SourceBuffer Constant pointer to the source hardware buffer from which the data is to be copied.
        \return True on success, otherwise false.
        */
        virtual bool copyBuffer(const ShaderResource* SourceBuffer) = 0;

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
        SHADERRESOURCE_STRUCT_BUFFER, SHADERRESOURCE_RW_STRUCT_BUFFER,
        SHADERRESOURCE_COUNTER_RW_STRUCT_BUFFER or SHADERRESOURCE_APPEND_STRUCT_BUFFER.
        \see EShaderResourceTypes
        */
        inline bool isStruct() const
        {
            return
                Type_ == SHADERRESOURCE_STRUCT_BUFFER               ||
                Type_ == SHADERRESOURCE_RW_STRUCT_BUFFER            ||
                Type_ == SHADERRESOURCE_COUNTER_RW_STRUCT_BUFFER    ||
                Type_ == SHADERRESOURCE_APPEND_STRUCT_BUFFER;
        }

        /**
        Returns true if this shader resource is byte-addressable, i.e. from the type
        SHADERRESOURCE_BYTE_BUFFER or SHADERRESOURCE_RW_BYTE_BUFFER.
        \see EShaderResourceTypes
        */
        inline bool isByteAddr() const
        {
            return Type_ == SHADERRESOURCE_BYTE_BUFFER || Type_ == SHADERRESOURCE_RW_BYTE_BUFFER;
        }

        /**
        Sets the hidden counter initialization value. This will only be used for
        R/W structured buffers with a hidden counter.
        \param[in] Init Specifies the new initialization value. By default 0.
        Every time a compute- or pixel shader will be called (which uses this shader resource)
        will reset the hidden counter to this value before the shader will be executed.
        Use -1 to disable resetting the hidden counter.
        \see SHADERRESOURCE_COUNTER_RW_STRUCT_BUFFER
        */
        inline void setCounterInit(u32 Init)
        {
            CounterInit_ = Init;
        }
        /**
        Returns the hidden counter initialization value. By default 0.
        \see setCounterInit
        */
        inline u32 getCounterInit() const
        {
            return CounterInit_;
        }
        
    protected:
        
        /* === Functions === */
        
        ShaderResource() :
            Type_       (SHADERRESOURCE_BUFFER  ),
            Stride_     (0                      ),
            CounterInit_(0                      )
        {
        }
        
        /* === Inline functions === */

        inline EShaderResourceTypes getBufRW(bool ReadWrite) const
        {
            return ReadWrite ? SHADERRESOURCE_RW_BUFFER : SHADERRESOURCE_BUFFER;
        }

        inline EShaderResourceTypes getStructRW(bool ReadWrite, s32 Flags) const
        {
            if (ReadWrite)
            {
                if (Flags & SHADERBUFFERFLAG_COUNTER)
                    return SHADERRESOURCE_COUNTER_RW_STRUCT_BUFFER;
                if (Flags & SHADERBUFFERFLAG_APPEND)
                    return SHADERRESOURCE_APPEND_STRUCT_BUFFER;
                return SHADERRESOURCE_RW_STRUCT_BUFFER;
            }
            return SHADERRESOURCE_STRUCT_BUFFER;
        }

        /* === Templates === */

        template <typename T> inline bool setupBufferTyped(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Flags)
        {
            return setupBufferRaw(getStructRW(ReadWrite, Flags), ElementCount, sizeof(T), DATATYPE_FLOAT, 1, Buffer);
        }

        template <> inline bool setupBufferTyped<s32>(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Unused)
        {
            return setupBufferRaw(getBufRW(ReadWrite), ElementCount, sizeof(s32), DATATYPE_INT, 1, Buffer);
        }
        template <> inline bool setupBufferTyped<dim::point2di>(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Unused)
        {
            return setupBufferRaw(getBufRW(ReadWrite), ElementCount, sizeof(dim::point2di), DATATYPE_INT, 2, Buffer);
        }
        template <> inline bool setupBufferTyped<dim::vector3di>(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Unused)
        {
            return setupBufferRaw(getBufRW(ReadWrite), ElementCount, sizeof(dim::vector3di), DATATYPE_INT, 3, Buffer);
        }
        template <> inline bool setupBufferTyped<dim::vector4di>(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Unused)
        {
            return setupBufferRaw(getBufRW(ReadWrite), ElementCount, sizeof(dim::vector4di), DATATYPE_INT, 4, Buffer);
        }

        template <> inline bool setupBufferTyped<u32>(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Unused)
        {
            return setupBufferRaw(getBufRW(ReadWrite), ElementCount, sizeof(u32), DATATYPE_UNSIGNED_INT, 1, Buffer);
        }
        template <> inline bool setupBufferTyped< dim::point2d<u32> >(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Unused)
        {
            return setupBufferRaw(getBufRW(ReadWrite), ElementCount, sizeof(dim::point2d<u32>), DATATYPE_UNSIGNED_INT, 2, Buffer);
        }
        template <> inline bool setupBufferTyped< dim::vector3d<u32> >(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Unused)
        {
            return setupBufferRaw(getBufRW(ReadWrite), ElementCount, sizeof(dim::vector3d<u32>), DATATYPE_UNSIGNED_INT, 3, Buffer);
        }
        template <> inline bool setupBufferTyped< dim::vector4d<u32> >(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Unused)
        {
            return setupBufferRaw(getBufRW(ReadWrite), ElementCount, sizeof(dim::vector4d<u32>), DATATYPE_UNSIGNED_INT, 4, Buffer);
        }

        template <> inline bool setupBufferTyped<f32>(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Unused)
        {
            return setupBufferRaw(getBufRW(ReadWrite), ElementCount, sizeof(f32), DATATYPE_FLOAT, 1, Buffer);
        }
        template <> inline bool setupBufferTyped<dim::point2df>(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Unused)
        {
            return setupBufferRaw(getBufRW(ReadWrite), ElementCount, sizeof(dim::point2df), DATATYPE_FLOAT, 2, Buffer);
        }
        template <> inline bool setupBufferTyped<dim::vector3df>(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Unused)
        {
            return setupBufferRaw(getBufRW(ReadWrite), ElementCount, sizeof(dim::vector3df), DATATYPE_FLOAT, 3, Buffer);
        }
        template <> inline bool setupBufferTyped<dim::vector4df>(u32 ElementCount, bool ReadWrite, const void* Buffer, s32 Unused)
        {
            return setupBufferRaw(getBufRW(ReadWrite), ElementCount, sizeof(dim::vector4df), DATATYPE_FLOAT, 4, Buffer);
        }

        /* === Members === */
        
        EShaderResourceTypes Type_;
        
        u32 Stride_;
        u32 CounterInit_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
