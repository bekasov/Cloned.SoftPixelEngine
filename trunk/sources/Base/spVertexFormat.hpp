/*
 * Vertex format header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_HWBUFFER_VERTEXFORMAT_H__
#define __SP_HWBUFFER_VERTEXFORMAT_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spMath.hpp"
#include "Base/spMaterialConfigTypes.hpp"

#include <vector>


namespace sp
{
namespace video
{


//! Vertex format flags.
enum EVertexFormatFlags
{
    VERTEXFORMAT_COORD      = 0x01, //!< Use vertex coordinate.
    VERTEXFORMAT_COLOR      = 0x02, //!< Use vertex color.
    VERTEXFORMAT_NORMAL     = 0x04, //!< Use vertex normal.
    VERTEXFORMAT_BINORMAL   = 0x08, //!< Use vertex binormal.
    VERTEXFORMAT_TANGENT    = 0x10, //!< Use vertex tangent.
    VERTEXFORMAT_FOGCOORD   = 0x20, //!< Use vertex fog coordinate.
    VERTEXFORMAT_TEXCOORDS  = 0x40, //!< Use vertex texture coordinates.
    VERTEXFORMAT_UNIVERSAL  = 0x80, //!< Use universal components.
};

//! Vertex attributes.
enum EVertexAttributes
{
    ATTRIBUTE_COORD,
    ATTRIBUTE_COLOR,
    ATTRIBUTE_NORMAL,
    ATTRIBUTE_BINORMAL,
    ATTRIBUTE_TANGENT,
    ATTRIBUTE_FOGCOORD,
    ATTRIBUTE_TEXCOORD0,
    ATTRIBUTE_TEXCOORD1,
    ATTRIBUTE_TEXCOORD2,
    ATTRIBUTE_TEXCOORD3,
    ATTRIBUTE_TEXCOORD4,
    ATTRIBUTE_TEXCOORD5,
    ATTRIBUTE_TEXCOORD6,
    ATTRIBUTE_TEXCOORD7,
    ATTRIBUTE_UNIVERSAL,
};

//! Vertex format attribute structure.
struct SVertexAttribute
{
    SVertexAttribute()
        : Size(0), Offset(0), Type(DATATYPE_FLOAT), hasDefaultSetting(false), isReference(false)
    {
    }
    SVertexAttribute(
        s32 InitSize, const io::stringc InitName = "", const ERendererDataTypes InitType = DATATYPE_FLOAT,
        bool DefaultSetting = true, bool InitNormalize = false)
        : Size(InitSize), Offset(0), Type(InitType), Name(InitName), Normalize(InitNormalize),
        hasDefaultSetting(DefaultSetting), isReference(false)
    {
    }
    ~SVertexAttribute()
    {
    }
    
    s32 Size;                   //!< Count of components (1, 2, 3 or 4).
    s32 Offset;                 //!< Offset for each vertex in bytes.
    ERendererDataTypes Type;    //!< Data type (float, integer etc.).
    io::stringc Name;           //!< Attribute name.
    bool Normalize;             //!< If true, fixed-point data will be normalized.
    bool hasDefaultSetting;     // !!! CHECK IF WORKS CORRECT !!!
    bool isReference;           //!< Normally false. Only true when a universal attribute links to this attribute.
};


//! This is the vertex format class which specifies how vertices are stored on the VRAM.
class SP_EXPORT VertexFormat
{
    
    public:
        
        virtual ~VertexFormat();
        
        /* Functions */
        
        //! Returns the identifier (e.g. "vertex format 'MyVertexFormat'" or "anonymous vertex format").
        io::stringc getIdentifier() const;
        
        //! Returns the size in bytes of this vertex format.
        virtual u32 getFormatSize() const;
        
        //! Returns the size in bytes of the specified data type.
        static s32 getDataTypeSize(const ERendererDataTypes Type);
        
        /* Inline functions */
        
        //! Returns the vertex format flags.
        inline s32 getFlags() const
        {
            return Flags_;
        }
        
        inline void setName(const io::stringc &Name)
        {
            Name_ = Name;
        }
        inline io::stringc getName() const
        {
            return Name_;
        }
        
        inline const SVertexAttribute& getCoord() const
        {
            return Coord_;
        }
        inline const SVertexAttribute& getColor() const
        {
            return Color_;
        }
        inline const SVertexAttribute& getNormal() const
        {
            return Normal_;
        }
        inline const SVertexAttribute& getBinormal() const
        {
            return Binormal_;
        }
        inline const SVertexAttribute& getTangent() const
        {
            return Tangent_;
        }
        inline const SVertexAttribute& getFogCoord() const
        {
            return FogCoord_;
        }
        
        inline const std::vector<SVertexAttribute>& getTexCoords() const
        {
            return TexCoords_;
        }
        inline const std::vector<SVertexAttribute>& getUniversals() const
        {
            return Universals_;
        }
        
    protected:
        
        friend class Direct3D11RenderSystem;
        friend class Direct3D11Shader;
        
        /* Functions */
        
        VertexFormat();
        
        virtual void getFormatSize(u32 &Size) const;
        void addComponentFormatSize(u32 &Size, const SVertexAttribute &Attrib) const;
        
        void constructComponent(
            const EVertexFormatFlags Type, SVertexAttribute &Attrib, s32 &Offset, s32 MinSize, s32 MaxSize
        );
        virtual void constructFormat();
        
        /* Inline functions */
        
        inline void addFlag(const EVertexFormatFlags Flag)
        {
            Flags_ |= Flag;
        }
        inline void removeFlag(const EVertexFormatFlags Flag)
        {
            if (Flags_ & Flag)
                Flags_ ^= Flag;
        }
        
        /* Members */
        
        s32 Flags_;
        io::stringc Name_;
        
        SVertexAttribute Coord_;
        SVertexAttribute Color_;
        SVertexAttribute Normal_;
        SVertexAttribute Binormal_;
        SVertexAttribute Tangent_;
        SVertexAttribute FogCoord_;
        
        std::vector<SVertexAttribute> TexCoords_;
        std::vector<SVertexAttribute> Universals_;
        
        void* InputLayout_; // Only for Direct3D11
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
