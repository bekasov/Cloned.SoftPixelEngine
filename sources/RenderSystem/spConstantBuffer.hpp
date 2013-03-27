/*
 * Shader constant buffer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHADER_CONSTANT_BUFFER_H__
#define __SP_SHADER_CONSTANT_BUFFER_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spMaterialConfigTypes.hpp"


namespace sp
{
namespace video
{


class ShaderClass;

/**
Shader constant buffer class used for OpenGL 3 and Direct3D 11. Direct3D 9 does not support constant buffers!
For this you still have to use the old uniforms or rather individual shader constants.
Constant buffers are particularly used to group large shader uniform arrays.
\see Shader
\see ShaderClass
\see MeshBuffer
\since Version 3.2
\ingroup group_shader
*/
class SP_EXPORT ConstantBuffer
{
    
    public:
        
        /**
        Constant buffer constructor.
        \param[in] Owner Pointer to the shader-class which owns this constant buffer. This must not be a null pointer!
        \param[in] Name Specifies the constant buffer name. This must not be empty!
        \param[in] Index Specifies the constant buffer index.
        \throw io::NullPointerException If 'Owner' is a null pointer.
        \throw io::DefaultException If 'Name' is empty.
        \see ShaderClass
        */
        ConstantBuffer(ShaderClass* Owner, const io::stringc &Name, u32 Index);
        virtual ~ConstantBuffer();
        
        /* === Functions === */
        
        /**
        Updates the constant buffer by uploading the data to the shader.
        \param[in] Buffer Constant pointer to the buffer data.
        \param[in] Size Specifies the buffer size (in bytes).
        If zero the engine determines the size automatically. By default zero.
        \return True if the buffer could be updated successful. Otherwise false is returned and
        an error or warning message will be printed.
        */
        virtual bool updateBuffer(const void* Buffer, u32 Size = 0);
        
        //! Returns true if this is a valid and successful created constant buffer.
        virtual bool valid() const;
        
        /**
        Sets the buffer usage type. Set this to dynamic usage if the buffer is modified often.
        Otherwise set it to static usage. By default HWBUFFER_DYNAMIC.
        \note The new usage type will only become active when the "updateBuffer" function is called the next time.
        \see EHWBufferUsage
        \see MeshBuffer::setVertexBufferUsage
        \see MeshBuffer::setIndexBufferUsage
        */
        void setBufferUsage(const EHWBufferUsage Usage);
        
        /* === Inline functions === */
        
        //! Returns the buffer name. This is the same name as used in the shader and can not be changed.
        inline const io::stringc& getName() const
        {
            return Name_;
        }
        
        /**
        Returns the buffer size in bytes. By default constant buffers are aligned in 4 component vectors (i.e. 4 floats).
        Thus the size is normally 16, 32, 48 etc.
        */
        inline u32 getSize() const
        {
            return Size_;
        }
        
        //! Returns the constant buffer index.
        inline u32 getIndex() const
        {
            return Index_;
        }
        
        //! Returns the buffer usage. By default HWBUFFER_STATIC.
        inline EHWBufferUsage getBufferUsage() const
        {
            return Usage_;
        }
        
        //! Returns a pointer to the shader-class reference to which this buffer belongs.
        inline ShaderClass* getShaderClass() const
        {
            return Shader_;
        }
        
    protected:
        
        /* === Members === */
        
        ShaderClass* Shader_;   //!< Shader-class reference.
        
        EHWBufferUsage Usage_;
        bool HasUsageChanged_;
        
        u32 Size_;
        
    private:
        
        /* === Members === */
        
        io::stringc Name_;
        u32 Index_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
