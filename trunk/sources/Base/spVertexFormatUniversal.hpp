/*
 * Universal vertex format header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_HWBUFFER_VERTEXFORMATUNIVERSAL_H__
#define __SP_HWBUFFER_VERTEXFORMATUNIVERSAL_H__


#include "Base/spStandard.hpp"
#include "Base/spVertexFormat.hpp"


namespace sp
{
namespace video
{


/**
This is the universal vertex format. This can be used particular when using shader programs.
*/
class SP_EXPORT VertexFormatUniversal : public VertexFormat
{
    
    public:
        
        VertexFormatUniversal();
        ~VertexFormatUniversal();
        
        u32 getFormatSize() const;
        
        /**
        Adds the coordinate component to the vertex format with the specified attributes.
        \param Type: Specifies the data type (float, int etc.).
        \param Size: Specifies the data size. Valid numbers are: 1 -> X, 2 -> XY, 3 -> XYZ and 4 -> XYZW. By default 3.
        \return Byte offset of this component.
        */
        void addCoord(const ERendererDataTypes Type = DATATYPE_FLOAT, s32 Size = 3);
        
        /**
        Adds the color componetn to the vertex format. For Direct3D9 this is always 4 ubytes (in the order ARGB).
        \param Size: Valid numbers are: 3 -> RGB and 4 -> RGBA. By default 4.
        */
        void addColor(const ERendererDataTypes Type = DATATYPE_UNSIGNED_BYTE, s32 Size = 4);
        
        //! Adds the normal vector component to the vertex format. The size is always 3.
        void addNormal(const ERendererDataTypes Type = DATATYPE_FLOAT);
        
        /**
        Adds the binormal vector component to the vertex format. The size is always 3.
        This feature is only supported with Direct3D11. For other renderers add a universal component.
        */
        void addBinormal(const ERendererDataTypes Type = DATATYPE_FLOAT);
        
        /**
        Adds the tangent vector component to the vertex format. The size is always 3.
        This feature is only supported with Direct3D11. For other renderers add a universal component.
        */
        void addTangent(const ERendererDataTypes Type = DATATYPE_FLOAT);
        
        /**
        Adds a new texture coordiante to the vertex format. You can not use texcoord 0 and 2 or anything like that. Only 0, 1, 2 etc.
        \param Size: Valid numbers are: 1 -> S, 2 -> ST, 3 -> STR and 4 -> STRQ. By default 2.
        */
        void addTexCoord(const ERendererDataTypes Type = DATATYPE_FLOAT, s32 Size = 2);
        
        /**
        Adds the fog coordinate component to the vertex format. The size is always 1.
        This feature is only supported in OpenGL for fixed function pipeline.
        */
        void addFogCoord(const ERendererDataTypes Type = DATATYPE_FLOAT);
        
        /**
        Adds a universal vertex attribute to the vertex format. This can only be used in combination with a shader program.
        \param Size: Must be in range [0, 3].
        \param Name: Specifies attribute name. For OpenGL it can be any name used in the vertex shader.
        For Direct3D11 it needs to be one of the HLSL semantics (POSITION[n], COLOR[n] etc.):
        \param Normalize: Specifies whether the attribute shall be normalized or not. e.g. when you use the ubyte data type
        this can be useful to normlize the vector to the range [0.0, 1.0].
        \param Attribute: Specifies which kind of attribute this universal is. By default its an unknown attribute.
        If you specify it as a vertex coordinate e.g. you can set its data by the "setVertexCoord" function. In this case
        morph-target- or skeletal animation will further more work fine.
        */
        void addUniversal(
            const ERendererDataTypes Type, s32 Size, const io::stringc &Name,
            bool Normalize = false, const EVertexFormatFlags Attribute = VERTEXFORMAT_UNIVERSAL
        );
        
        /**
        Removes the specified vertex attribute.
        \param Type: Specifies the vertex attribute which is to be removed.
        If the type is a texture coordiante or universal attribute the last one will be removed.
        */
        void remove(const EVertexFormatFlags Type);
        
        //! Clears the whole vertex format. After calling this function the format size is 0.
        void clear();
        
    private:
        
        /* Functions */
        
        void updateConstruction();
        
        void addAttribute(
            const EVertexFormatFlags Flag, SVertexAttribute &Attrib, s32 Size,
            const ERendererDataTypes Type, const io::stringc &Name,
            bool hasDefaultSetting = false, bool Normalize = false
        );
        
        void addVirtualAttribute(const EVertexFormatFlags Attribute, SVertexAttribute &DestAttrib);
        
        /* Members */
        
        u32 FormatSize_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
