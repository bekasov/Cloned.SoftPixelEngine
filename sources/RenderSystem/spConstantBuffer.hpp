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


class Shader;

/**
Shader constant buffer class used for OpenGL 4 and Direct3D 11. Direct3D 9 does not support constant buffers!
For this you still have to use the old uniforms or rather individual shader constants.
Constant buffers are particularly used to group large shader uniform arrays.
\see Shader
\see ShaderClass
\see MeshBuffer
\since Version 3.2
*/
class SP_EXPORT ConstantBuffer
{
    
    public:
        
        /**
        Constant buffer constructor.
        \param[in] Owner Pointer to the shader which owns this constant buffer. This must not be a null pointer!
        \param[in] Name Specifies the constant buffer name. This must not be empty!
        \throw io::NullPointerException If 'Owner' is a null pointer.
        \throw io::DefaultException If 'Name' is empty.
        */
        ConstantBuffer(Shader* Owner, const io::stringc &Name);
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
        
        /* === Inline functions === */
        
        //! Returns the buffer name. This is the same name as used in the shader and can not be changed.
        inline const io::stringc& getName() const
        {
            return Name_;
        }
        
        /**
        Sets the buffer usage type. Set this to dynamic usage if the buffer is modified often.
        Otherwise set it to static usage. By default HWBUFFER_STATIC.
        \see EHWBufferUsage
        \see MeshBuffer::setVertexBufferUsage
        \see MeshBuffer::setIndexBufferUsage
        */
        inline void setBufferUsage(const EHWBufferUsage Usage)
        {
            Usage_ = Usage;
        }
        //! Returns the buffer usage. By default HWBUFFER_STATIC.
        inline EHWBufferUsage getBufferUsage() const
        {
            return Usage_;
        }
        
        //! Returns a pointer to the shader reference to which this buffer belongs.
        inline Shader* getShader() const
        {
            return Shader_;
        }
        
    protected:
        
        /* === Members === */
        
        Shader* Shader_;        //!< Shader reference.
        
        EHWBufferUsage Usage_;
        
    private:
        
        /* === Members === */
        
        io::stringc Name_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
