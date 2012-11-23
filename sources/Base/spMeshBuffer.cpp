/*
 * Mesh buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spMeshBuffer.hpp"
#include "SceneGraph/spMeshModifier.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


/*
 * Internal members
 */

const c8* DEBERR_LAYER_RANGE = "'Layer' index out of range";


/*
 * Internal structures
 */

struct SCmpNormalCoord
{
    u32 Index;
    dim::vector3df Normal;
    dim::vector3df Position;
};


/*
 * Internal functions
 */

bool cmpVertexCoords(SCmpNormalCoord &obj1, SCmpNormalCoord &obj2)
{
    if (!math::Equal(obj1.Position.X, obj2.Position.X))
        return obj1.Position.X < obj2.Position.X;
    if (!math::Equal(obj1.Position.Y, obj2.Position.Y))
        return obj1.Position.Y < obj2.Position.Y;
    return obj1.Position.Z < obj2.Position.Z;
}


/*
 * MeshBuffer class
 */

MeshBuffer::MeshBuffer(const video::VertexFormat* VertexFormat, ERendererDataTypes IndexFormat) :
    VertexFormat_   (VertexFormat       ),
    Reference_      (0                  ),
    TextureList_    (&OrigTextureList_  ),
    IndexOffset_    (0                  ),
    InstanceCount_  (1                  ),
    PrimitiveType_  (PRIMITIVE_TRIANGLES),
    useIndexBuffer_ (true               ),
    UpdateImmediate_(false              ),
    Backup_         (0                  )
{
    if (!VertexFormat_)
        VertexFormat_ = __spVideoDriver->getVertexFormatDefault();
    
    setupDefaultBuffers();
    
    checkIndexFormat(IndexFormat);
    
    IndexFormat_.setDataType(IndexFormat);
    IndexBuffer_.RawBuffer.setStride(VertexFormat::getDataTypeSize(IndexFormat));
}
MeshBuffer::MeshBuffer(const MeshBuffer &Other, bool isCreateMeshBuffer) :
    Name_           (Other.Name_            ),
    VertexBuffer_   (Other.VertexBuffer_    ),
    IndexBuffer_    (Other.IndexBuffer_     ),
    VertexFormat_   (Other.VertexFormat_    ),
    Reference_      (0                      ),
    TextureList_    (&OrigTextureList_      ),
    IndexOffset_    (0                      ),
    InstanceCount_  (Other.InstanceCount_   ),
    PrimitiveType_  (Other.PrimitiveType_   ),
    useIndexBuffer_ (Other.useIndexBuffer_  ),
    UpdateImmediate_(Other.UpdateImmediate_ ),
    Backup_         (0                      )
{
    setupDefaultBuffers();
    
    /* Copy mesh buffer data */
    if (Other.hasTexturesReference())
        TextureList_    = Other.TextureList_;
    else
        *TextureList_   = *Other.TextureList_;
    
    IndexFormat_.setDataType(Other.getIndexFormat()->getDataType());
    IndexBuffer_.RawBuffer.setStride(VertexFormat::getDataTypeSize(Other.getIndexFormat()->getDataType()));
    
    if (isCreateMeshBuffer)
    {
        createMeshBuffer();
        updateMeshBuffer();
    }
    
    if (Other.Reference_ != &Other)
        Reference_ = Other.Reference_;
}
MeshBuffer::~MeshBuffer()
{
    deleteMeshBuffer();
    clearBackup();
}

/* === Buffer functions === */

io::stringc MeshBuffer::getIdentifier() const
{
    if (Name_.size())
        return "mesh buffer \"" + Name_ + "\"";
    return "anonymous mesh buffer";
}

MeshBuffer* MeshBuffer::getReference()
{
    if (Reference_)
        return Reference_->getReference();
    return this;
}
const MeshBuffer* MeshBuffer::getReference() const
{
    if (Reference_)
        return Reference_->getReference();
    return this;
}

bool MeshBuffer::sortCompare(const MeshBuffer &Other) const
{
    /* Compare count of textures */
    if (getTextureCount() != Other.getTextureCount())
        return getTextureCount() < Other.getTextureCount();
    
    /* Compare vertex- and index format */
    if (VertexFormat_ != Other.VertexFormat_)
        return reinterpret_cast<long>(VertexFormat_) < reinterpret_cast<long>(Other.VertexFormat_);
    if (IndexFormat_.getDataType() != Other.IndexFormat_.getDataType())
        return IndexFormat_.getDataType() < Other.IndexFormat_.getDataType();
    if (useIndexBuffer_ != Other.useIndexBuffer_)
        return useIndexBuffer_;
    if (PrimitiveType_ != Other.PrimitiveType_)
        return PrimitiveType_ < Other.PrimitiveType_;
    
    /* Compare surface textures */
    std::vector<SMeshSurfaceTexture>::const_iterator itTexA, itTexB, itEndA, itEndB;
    
    itTexA = TextureList_->begin();
    itTexB = Other.TextureList_->begin();
    
    itEndA = TextureList_->end();
    itEndB = Other.TextureList_->end();
    
    for (; itTexA != itEndA && itTexB != itEndB; ++itTexA, ++itTexB)
    {
        if (*itTexA != *itTexB)
            return itTexA->compare(*itTexB);
    }
    
    return false;
}

bool MeshBuffer::compare(const MeshBuffer &Other) const
{
    /* Compare count of textures */
    if (getTextureCount() != Other.getTextureCount())
        return false;
    
    /* Compare vertex- and index format */
    if (VertexFormat_ != Other.VertexFormat_)
        return false;
    if (IndexFormat_.getDataType() != Other.IndexFormat_.getDataType())
        return false;
    if (useIndexBuffer_ != Other.useIndexBuffer_)
        return false;
    if (PrimitiveType_ != Other.PrimitiveType_)
        return false;
    
    /* Compare surface textures */
    std::vector<SMeshSurfaceTexture>::const_iterator itTexA, itTexB, itEndA, itEndB;
    
    itTexA = TextureList_->begin();
    itTexB = Other.TextureList_->begin();
    
    itEndA = TextureList_->end();
    itEndB = Other.TextureList_->end();
    
    for (; itTexA != itEndA && itTexB != itEndB; ++itTexA, ++itTexB)
    {
        if (*itTexA != *itTexB)
            return false;
    }
    
    return true;
}

void MeshBuffer::setVertexFormat(const VertexFormat* Format)
{
    /* Check if format is valid */
    if (!Format || !VertexFormat_ || Format == VertexFormat_)
    {
        #ifdef SP_DEBUGMODE
        if (!Format)
            io::Log::debug("MeshBuffer::setVertexFormat");
        #endif
        return;
    }
    
    /* Copy the old buffer for temporary conversion */
    dim::UniversalBuffer OldBuffer = VertexBuffer_.RawBuffer;
    
    VertexBuffer_.RawBuffer.setStride(Format->getFormatSize());
    VertexBuffer_.RawBuffer.setCount(OldBuffer.getCount());
    
    /* Convert the vertex buffer */
    for (u32 i = 0, c = OldBuffer.getCount(); i < c; ++i)
    {
        /* Convert vertex coordinate */
        if (Format->getFlags() & VERTEXFORMAT_COORD)
            convertVertexAttribute(OldBuffer, i, VertexFormat_->getCoord(), Format->getCoord());
        if (Format->getFlags() & VERTEXFORMAT_COLOR)
            convertVertexAttribute(OldBuffer, i, VertexFormat_->getColor(), Format->getColor());
        if (Format->getFlags() & VERTEXFORMAT_NORMAL)
            convertVertexAttribute(OldBuffer, i, VertexFormat_->getNormal(), Format->getNormal());
        if (Format->getFlags() & VERTEXFORMAT_BINORMAL)
            convertVertexAttribute(OldBuffer, i, VertexFormat_->getBinormal(), Format->getBinormal());
        if (Format->getFlags() & VERTEXFORMAT_TANGENT)
            convertVertexAttribute(OldBuffer, i, VertexFormat_->getTangent(), Format->getTangent());
        if (Format->getFlags() & VERTEXFORMAT_FOGCOORD)
            convertVertexAttribute(OldBuffer, i, VertexFormat_->getFogCoord(), Format->getFogCoord());
        
        if (Format->getFlags() & VERTEXFORMAT_TEXCOORDS)
        {
            for (u32 j = 0; j < Format->getTexCoords().size(); ++j)
            {
                if (j < VertexFormat_->getTexCoords().size())
                    convertVertexAttribute(OldBuffer, i, VertexFormat_->getTexCoords()[j], Format->getTexCoords()[j]);
                else
                    fillVertexAttribute(i, Format->getTexCoords()[j]);
            }
        }
        
        if (Format->getFlags() & VERTEXFORMAT_UNIVERSAL)
        {
            for (u32 j = 0; j < Format->getUniversals().size(); ++j)
            {
                if (j < VertexFormat_->getUniversals().size())
                    convertVertexAttribute(OldBuffer, i, VertexFormat_->getUniversals()[j], Format->getUniversals()[j]);
                else
                    fillVertexAttribute(i, Format->getUniversals()[j]);
            }
        }
    }
    
    VertexFormat_ = Format;
    
    /* Update the hardware vertex buffer */
    updateVertexBuffer();
}

void MeshBuffer::setIndexFormat(ERendererDataTypes Format)
{
    /* Check if format is valid */
    if (Format == IndexFormat_.getDataType())
        return;
    
    checkIndexFormat(Format);
    
    if (Format != DATATYPE_UNSIGNED_BYTE && Format != DATATYPE_UNSIGNED_SHORT && Format != DATATYPE_UNSIGNED_INT)
    {
        io::Log::error("Invalid format for index buffer; only UBYTE, USHORT and UINT are allowed");
        return;
    }
    
    /* Copy the old buffer for temporary conversion */
    dim::UniversalBuffer OldBuffer = IndexBuffer_.RawBuffer;
    
    IndexBuffer_.RawBuffer.setStride(VertexFormat::getDataTypeSize(Format));
    IndexBuffer_.RawBuffer.setCount(OldBuffer.getCount());
    
    /* Convert the index buffer */
    switch (Format)
    {
        case DATATYPE_UNSIGNED_BYTE:
            switch (IndexFormat_.getDataType())
            {
                case DATATYPE_UNSIGNED_SHORT:
                    for (u32 i = 0, c = OldBuffer.getCount(); i < c; ++i)
                        IndexBuffer_.RawBuffer.set<u8>(i, 0, static_cast<u8>(OldBuffer.get<u16>(i, 0)));
                    break;
                case DATATYPE_UNSIGNED_INT:
                    for (u32 i = 0, c = OldBuffer.getCount(); i < c; ++i)
                        IndexBuffer_.RawBuffer.set<u8>(i, 0, static_cast<u8>(OldBuffer.get<u32>(i, 0)));
                    break;
                default:
                    break;
            }
            break;
            
        case DATATYPE_UNSIGNED_SHORT:
            switch (IndexFormat_.getDataType())
            {
                case DATATYPE_UNSIGNED_BYTE:
                    for (u32 i = 0, c = OldBuffer.getCount(); i < c; ++i)
                        IndexBuffer_.RawBuffer.set<u16>(i, 0, static_cast<u16>(OldBuffer.get<u8>(i, 0)));
                    break;
                case DATATYPE_UNSIGNED_INT:
                    for (u32 i = 0, c = OldBuffer.getCount(); i < c; ++i)
                        IndexBuffer_.RawBuffer.set<u16>(i, 0, static_cast<u16>(OldBuffer.get<u32>(i, 0)));
                    break;
                default:
                    break;
            }
            break;
            
        case DATATYPE_UNSIGNED_INT:
            switch (IndexFormat_.getDataType())
            {
                case DATATYPE_UNSIGNED_BYTE:
                    for (u32 i = 0, c = OldBuffer.getCount(); i < c; ++i)
                        IndexBuffer_.RawBuffer.set<u32>(i, 0, static_cast<u32>(OldBuffer.get<u8>(i, 0)));
                    break;
                case DATATYPE_UNSIGNED_SHORT:
                    for (u32 i = 0, c = OldBuffer.getCount(); i < c; ++i)
                        IndexBuffer_.RawBuffer.set<u32>(i, 0, static_cast<u32>(OldBuffer.get<u16>(i, 0)));
                    break;
                default:
                    break;
            }
            break;
            
        default:
            break;
    }
    
    /* Check if every vertex index is valid */
    u32 VertexIndex = 0;
    const u32 VertexCount = VertexBuffer_.RawBuffer.getCount();
    
    for (u32 i = 0, c = IndexBuffer_.RawBuffer.getCount(); i < c; ++i)
    {
        switch (Format)
        {
            case DATATYPE_UNSIGNED_BYTE:
                VertexIndex = static_cast<u32>(IndexBuffer_.RawBuffer.get<u8>(i, 0)); break;
            case DATATYPE_UNSIGNED_SHORT:
                VertexIndex = static_cast<u32>(IndexBuffer_.RawBuffer.get<u16>(i, 0)); break;
            case DATATYPE_UNSIGNED_INT:
                VertexIndex = static_cast<u32>(IndexBuffer_.RawBuffer.get<u32>(i, 0)); break;
            default:
                break;
        }
        
        /* Check if the current vertex index is invalid */
        if (VertexIndex >= VertexCount)
        {
            /* An index is invalid thus cancel conversion process and print an error message */
            IndexBuffer_.RawBuffer = OldBuffer;
            io::Log::error("Vertex index out of bounds while converting the index format of " + getIdentifier());
            return;
        }
    }
    
    IndexFormat_.setDataType(Format);
    
    /* Update the hardware index buffer */
    updateIndexBuffer();
}

void MeshBuffer::saveBackup()
{
    if (!Backup_)
        Backup_ = MemoryManager::createMemory<SMeshBufferBackup>("MeshBuffer::saveBackup");
    
    /* Save mesh buffer */
    Backup_->BUVertexBuffer = VertexBuffer_.RawBuffer;
    Backup_->BUIndexBuffer  = IndexBuffer_.RawBuffer;
    Backup_->BUVertexFormat = VertexFormat_;
    Backup_->BUIndexFormat  = IndexFormat_;
}
void MeshBuffer::loadBackup()
{
    if (Backup_)
    {
        /* Load backup */
        VertexBuffer_.RawBuffer = Backup_->BUVertexBuffer;
        IndexBuffer_.RawBuffer  = Backup_->BUIndexBuffer;
        VertexFormat_           = Backup_->BUVertexFormat;
        IndexFormat_            = Backup_->BUIndexFormat;
        
        /* Update mesh buffer */
        updateMeshBuffer();
    }
}
void MeshBuffer::clearBackup()
{
    MemoryManager::deleteMemory(Backup_);
}

void MeshBuffer::createVertexBuffer()
{
    if (!VertexBuffer_.Reference)
        __spVideoDriver->createVertexBuffer(VertexBuffer_.Reference);
}
void MeshBuffer::createIndexBuffer()
{
    if (!IndexBuffer_.Reference)
        __spVideoDriver->createIndexBuffer(IndexBuffer_.Reference);
}
void MeshBuffer::createMeshBuffer()
{
    createVertexBuffer();
    createIndexBuffer();
}

void MeshBuffer::deleteVertexBuffer()
{
    if (VertexBuffer_.Reference)
    {
        __spVideoDriver->deleteVertexBuffer(VertexBuffer_.Reference);
        VertexBuffer_.Validated = false;
    }
}
void MeshBuffer::deleteIndexBuffer()
{
    if (IndexBuffer_.Reference)
    {
        __spVideoDriver->deleteIndexBuffer(IndexBuffer_.Reference);
        IndexBuffer_.Validated = false;
    }
}
void MeshBuffer::deleteMeshBuffer()
{
    deleteVertexBuffer();
    deleteIndexBuffer();
}

void MeshBuffer::updateVertexBuffer()
{
    __spVideoDriver->updateVertexBuffer(
        VertexBuffer_.Reference, VertexBuffer_.RawBuffer, VertexFormat_, VertexBuffer_.Usage
    );
    VertexBuffer_.Validated = true;
}
void MeshBuffer::updateIndexBuffer()
{
    __spVideoDriver->updateIndexBuffer(
        IndexBuffer_.Reference, IndexBuffer_.RawBuffer, &IndexFormat_, IndexBuffer_.Usage
    );
    IndexBuffer_.Validated = true;
}
void MeshBuffer::updateMeshBuffer()
{
    updateVertexBuffer();
    updateIndexBuffer();
}

void MeshBuffer::updateVertexBufferElement(u32 Index)
{
    __spVideoDriver->updateVertexBufferElement(VertexBuffer_.Reference, VertexBuffer_.RawBuffer, Index);
}
void MeshBuffer::updateIndexBufferElement(u32 Index)
{
    __spVideoDriver->updateIndexBufferElement(IndexBuffer_.Reference, IndexBuffer_.RawBuffer, Index);
}

void MeshBuffer::setPrimitiveType(const ERenderPrimitives Type)
{
    /* Check primitive type for renderer */
    if ( ( __spVideoDriver->getRendererType() == RENDERER_DIRECT3D9 || __spVideoDriver->getRendererType() == RENDERER_DIRECT3D11 ) &&
         ( Type == PRIMITIVE_LINE_LOOP || Type == PRIMITIVE_QUADS || Type == PRIMITIVE_QUAD_STRIP || Type == PRIMITIVE_POLYGON ) )
    {
        io::Log::error("Specified primitive type is not supported for Direct3D");
        return;
    }
    if ( __spVideoDriver->getRendererType() == RENDERER_OPENGLES1 &&
         ( Type == PRIMITIVE_QUADS || Type == PRIMITIVE_QUAD_STRIP || Type == PRIMITIVE_POLYGON ) )
    {
        io::Log::error("Specified primitive type is not supported for OpenGL|ES");
        return;
    }

    PrimitiveType_ = Type;
}

s32 MeshBuffer::getPrimitiveSize(const ERenderPrimitives Type)
{
    static const s32 SizeList[] = { 1, 2, 1, 1, 3, 1, 1, 4, 1, 1 };
    return SizeList[Type];
}

s32 MeshBuffer::getPrimitiveSize() const
{
    return getPrimitiveSize(PrimitiveType_);
}

bool MeshBuffer::renderable() const
{
    if (!VertexBuffer_.Validated)
        return false;
    
    if (getIndexBufferEnable())
        return IndexBuffer_.Validated && getIndexCount() > 0;
    
    return getVertexCount() > 0;
}


/* === Mesh buffer manipulation functions */

u32 MeshBuffer::addVertex()
{
    const u32 LastOffset = VertexBuffer_.RawBuffer.getSize();
    
    VertexBuffer_.RawBuffer.setSize(LastOffset + VertexBuffer_.RawBuffer.getStride());
    VertexBuffer_.RawBuffer.fill(LastOffset, VertexBuffer_.RawBuffer.getStride());
    
    return getVertexCount() - 1;
}

void MeshBuffer::addVertices(const u32 Count)
{
    if (Count > 0)
    {
        const u32 LastOffset = VertexBuffer_.RawBuffer.getSize();
        
        VertexBuffer_.RawBuffer.setSize(LastOffset + VertexBuffer_.RawBuffer.getStride() * Count);
        VertexBuffer_.RawBuffer.fill(LastOffset, VertexBuffer_.RawBuffer.getStride() * Count);
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::addVertices", "Adding zero vertices has no effect");
    #endif
}

u32 MeshBuffer::addVertex(
    const dim::vector3df &Position, const dim::vector3df &TexCoord, const video::color &Color)
{
    const u32 i = addVertex();
    
    const s32 Flags = VertexFormat_->getFlags();
    
    if (Flags & VERTEXFORMAT_COORD)
        setVertexCoord(i, Position);
    if (Flags & VERTEXFORMAT_TEXCOORDS)
        setVertexTexCoord(i, TexCoord);
    if (Flags & VERTEXFORMAT_COLOR)
        setVertexColor(i, Color);
    
    return i;
}

u32 MeshBuffer::addVertex(
    const dim::vector3df &Position, const dim::vector3df &Normal, const dim::vector3df &TexCoord,
    const video::color &Color, const f32 Fog)
{
    const u32 i = addVertex();
    
    const s32 Flags = VertexFormat_->getFlags();
    
    if (Flags & VERTEXFORMAT_COORD)
        setVertexCoord(i, Position);
    if (Flags & VERTEXFORMAT_NORMAL)
        setVertexNormal(i, Normal);
    if (Flags & VERTEXFORMAT_TEXCOORDS)
        setVertexTexCoord(i, TexCoord);
    if (Flags & VERTEXFORMAT_COLOR)
        setVertexColor(i, Color);
    if (Flags & VERTEXFORMAT_FOGCOORD)
        setVertexFog(i, Fog);
    
    return i;
}

u32 MeshBuffer::addVertex(
    const dim::vector3df &Position, const dim::vector3df &Normal, const std::vector<dim::vector3df> &TexCoordList,
    const video::color &Color, const f32 Fog)
{
    const u32 i = addVertex();
    
    const s32 Flags = VertexFormat_->getFlags();
    
    if (Flags & VERTEXFORMAT_COORD)
        setVertexCoord(i, Position);
    if (Flags & VERTEXFORMAT_NORMAL)
        setVertexNormal(i, Normal);
    if (Flags & VERTEXFORMAT_COLOR)
        setVertexColor(i, Color);
    if (Flags & VERTEXFORMAT_FOGCOORD)
        setVertexFog(i, Fog);
    
    if (Flags & VERTEXFORMAT_TEXCOORDS)
    {
        for (u8 l = 0; l < TexCoordList.size(); ++l)
            setVertexTexCoord(i, TexCoordList[l], l);
    }
    
    return i;
}

u32 MeshBuffer::addVertex(const scene::SMeshVertex3D &VertexData)
{
    const u32 i = addVertex();
    
    const s32 Flags = VertexFormat_->getFlags();
    
    if (Flags & VERTEXFORMAT_COORD)
        setVertexCoord(i, VertexData.getPosition());
    if (Flags & VERTEXFORMAT_NORMAL)
        setVertexNormal(i, VertexData.getNormal());
    if (Flags & VERTEXFORMAT_COLOR)
        setVertexColor(i, VertexData.getColor());
    if (Flags & VERTEXFORMAT_FOGCOORD)
        setVertexFog(i, VertexData.getFog());
    
    if (Flags & VERTEXFORMAT_TEXCOORDS)
    {
        for (u8 l = 0; l < MAX_COUNT_OF_TEXTURES; ++l)
            setVertexTexCoord(i, VertexData.getTexCoord(l), l);
    }
    
    return i;
}

bool MeshBuffer::removeVertex(const u32 Index)
{
    /* Check if the index is too high */
    if (Index >= VertexBuffer_.RawBuffer.getCount())
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("MeshBuffer::removeVertex", "'Index' out of range");
        #endif
        return false;
    }
    
    /* Remove specified vertex */
    VertexBuffer_.RawBuffer.removeBuffer(Index, 0, VertexBuffer_.RawBuffer.getStride());
    
    const u32 IndexCount = getIndexCount();
    
    /*
     * Update each index which needs to be decremented because
     * of the removed vertex if this is not the last vertex
     */
    if (Index < getVertexCount() - 1)
    {
        for (u32 i = 0, j; i < IndexCount; ++i)
        {
            j = getPrimitiveIndex(i);
            if (j > Index)
                setPrimitiveIndex(i, j - 1);
        }
    }
    
    /* Remove each index which refers to the removed vertex */
    const u32 PrimitiveSize = (u32)getPrimitiveSize();
    
    for (u32 i = 0, j; i < IndexCount; i += PrimitiveSize)
    {
        for (j = 0; j < PrimitiveSize; ++j)
        {
            if (getPrimitiveIndex(i + j) == Index)
            {
                removePrimitive(i);
                break;
            }
        }
    }
    
    return true;
}

u32 MeshBuffer::addTriangle()
{
    if (getVertexCount() > 0)
    {
        const u32 LastOffset = IndexBuffer_.RawBuffer.getSize();
        
        IndexBuffer_.RawBuffer.setSize(LastOffset + IndexBuffer_.RawBuffer.getStride() * 3);
        IndexBuffer_.RawBuffer.fill(LastOffset, IndexBuffer_.RawBuffer.getStride() * 3);
        
        return getTriangleCount() - 1;
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::addTriangle", "No vertices to add a triangle");
    #endif
    
    return 0;
}

void MeshBuffer::addTriangles(const u32 Count)
{
    if (Count > 0 && getVertexCount() > 0)
    {
        const u32 LastOffset = IndexBuffer_.RawBuffer.getSize();
        const u32 Size = IndexBuffer_.RawBuffer.getStride() * Count * 3;
        
        IndexBuffer_.RawBuffer.setSize(LastOffset + Size);
        IndexBuffer_.RawBuffer.fill(LastOffset, Size);
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::addTriangles", "No vertices to add triangles");
    #endif
}

u32 MeshBuffer::addTriangle(u32 VertexA, u32 VertexB, u32 VertexC)
{
    if (PrimitiveType_ != PRIMITIVE_TRIANGLES)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("MeshBuffer::addTriangle", "Wrong primitive type to add a triangle");
        #endif
        return 0;
    }
    
    /* Get the maximal index for the triangle indices */
    u32 MaxIndex = VertexBuffer_.RawBuffer.getCount();
    
    switch (IndexFormat_.getDataType())
    {
        case DATATYPE_UNSIGNED_BYTE:
            if (MaxIndex > UCHAR_MAX) MaxIndex = UCHAR_MAX; break;
        case DATATYPE_UNSIGNED_SHORT:
            if (MaxIndex > USHRT_MAX) MaxIndex = USHRT_MAX; break;
        default:
            break;
    }
    
    /* Apply the index offset */
    VertexA += IndexOffset_;
    VertexB += IndexOffset_;
    VertexC += IndexOffset_;
    
    if (VertexA < MaxIndex && VertexB < MaxIndex && VertexC < MaxIndex)
    {
        /* Add the indices to the index buffer */
        switch (IndexFormat_.getDataType())
        {
            case DATATYPE_UNSIGNED_BYTE:
                addTriangleIndices<u8>(VertexA, VertexB, VertexC); break;
            case DATATYPE_UNSIGNED_SHORT:
                addTriangleIndices<u16>(VertexA, VertexB, VertexC); break;
            case DATATYPE_UNSIGNED_INT:
                addTriangleIndices<u32>(VertexA, VertexB, VertexC); break;
            default:
                return 0;
        }
        return getTriangleCount() - 1;
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::addTriangle", "Vertex index out of range");
    #endif
    
    return 0;
}

u32 MeshBuffer::addTriangle(const u32 Indices[3])
{
    if (Indices)
        return addTriangle(Indices[0], Indices[1], Indices[2]);
    return 0;
}

u32 MeshBuffer::addQuadrangle(u32 VertexA, u32 VertexB, u32 VertexC, u32 VertexD)
{
    if (PrimitiveType_ != PRIMITIVE_QUADS)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("MeshBuffer::addQuadrangle", "Wrong primitive type to add a quadrangle");
        #endif
        return 0;
    }
    
    /* Get the maximal index for the triangle indices */
    u32 MaxIndex = VertexBuffer_.RawBuffer.getCount();
    
    switch (IndexFormat_.getDataType())
    {
        case DATATYPE_UNSIGNED_BYTE:
            if (MaxIndex > UCHAR_MAX) MaxIndex = UCHAR_MAX; break;
        case DATATYPE_UNSIGNED_SHORT:
            if (MaxIndex > USHRT_MAX) MaxIndex = USHRT_MAX; break;
        default:
            break;
    }
    
    /* Apply the index offset */
    VertexA += IndexOffset_;
    VertexB += IndexOffset_;
    VertexC += IndexOffset_;
    VertexD += IndexOffset_;
    
    if (VertexA < MaxIndex && VertexB < MaxIndex && VertexC < MaxIndex && VertexD < MaxIndex)
    {
        /* Add the indices to the index buffer */
        switch (IndexFormat_.getDataType())
        {
            case DATATYPE_UNSIGNED_BYTE:
                addQuadrangleIndices<u8>(VertexA, VertexB, VertexC, VertexD); break;
            case DATATYPE_UNSIGNED_SHORT:
                addQuadrangleIndices<u16>(VertexA, VertexB, VertexC, VertexD); break;
            case DATATYPE_UNSIGNED_INT:
                addQuadrangleIndices<u32>(VertexA, VertexB, VertexC, VertexD); break;
            default:
                return 0;
        }
        
        return getIndexCount() / 4 - 1;
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::addQuadrangle", "Vertex index out of range");
    #endif
    
    return 0;
}

u32 MeshBuffer::addQuadrangle(const u32 Indices[4])
{
    if (Indices)
        return addQuadrangle(Indices[0], Indices[1], Indices[2], Indices[3]);
    return 0;
}

u32 MeshBuffer::addPrimitiveIndex(u32 Index)
{
    /* Get the maximal index */
    u32 MaxIndex = VertexBuffer_.RawBuffer.getCount();
    
    switch (IndexFormat_.getDataType())
    {
        case DATATYPE_UNSIGNED_BYTE:
            if (MaxIndex > UCHAR_MAX) MaxIndex = UCHAR_MAX; break;
        case DATATYPE_UNSIGNED_SHORT:
            if (MaxIndex > USHRT_MAX) MaxIndex = USHRT_MAX; break;
        default:
            break;
    }
    
    /* Apply the index offset */
    Index += IndexOffset_;
    
    if (Index < MaxIndex)
    {
        /* Add the index to the index buffer */
        switch (IndexFormat_.getDataType())
        {
            case DATATYPE_UNSIGNED_BYTE:
                IndexBuffer_.RawBuffer.add<u8>(Index); break;
            case DATATYPE_UNSIGNED_SHORT:
                IndexBuffer_.RawBuffer.add<u16>(Index); break;
            case DATATYPE_UNSIGNED_INT:
                IndexBuffer_.RawBuffer.add<u32>(Index); break;
            default:
                return 0;
        }
        
        return getIndexCount() - 1;
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::addPrimitiveIndex", "Vertex index out of range");
    #endif
    
    return 0;
}

void MeshBuffer::addIndices(const u32 Count)
{
    if (Count > 0)
    {
        const u32 LastOffset = IndexBuffer_.RawBuffer.getSize();
        
        IndexBuffer_.RawBuffer.setSize(LastOffset + IndexBuffer_.RawBuffer.getStride() * Count);
        IndexBuffer_.RawBuffer.fill(LastOffset, IndexBuffer_.RawBuffer.getStride() * Count);
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::addIndices", "Adding zero indices has no effect");
    #endif
}

bool MeshBuffer::removePrimitive(const u32 Index)
{
    const s32 PrimitiveSize     = getPrimitiveSize();
    const u32 PrimitiveIndex    = Index * PrimitiveSize;
    
    if (PrimitiveIndex <= getIndexCount() - PrimitiveSize)
    {
        IndexBuffer_.RawBuffer.removeBuffer(
            PrimitiveIndex, 0, PrimitiveSize * VertexFormat::getDataTypeSize(IndexFormat_.getDataType())
        );
        return true;
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::removePrimitive", "'Index' out of range");
    #endif
    
    return false;
}

void MeshBuffer::clearVertices()
{
    if (!VertexBuffer_.RawBuffer.empty())
    {
        VertexBuffer_.RawBuffer.clear();
        updateVertexBuffer();
        clearIndices();
    }
}

void MeshBuffer::clearIndices()
{
    if (!IndexBuffer_.RawBuffer.empty())
    {
        IndexBuffer_.RawBuffer.clear();
        updateIndexBuffer();
        IndexOffset_ = 0;
    }
}

bool MeshBuffer::insertMeshBuffer(const MeshBuffer &Other)
{
    if (!compare(Other) || !Other.getVertexCount())
        return false;
    
    /* Add new vertices */
    const u32 PrevVertexCount = getVertexCount();
    VertexBuffer_.RawBuffer.add(Other.VertexBuffer_.RawBuffer);
    
    /* Add new indices */
    if (getIndexBufferEnable())
    {
        const u32 PrevIndexCount = getIndexCount();
        addIndices(Other.getIndexCount());
        
        for (u32 i = 0, c = Other.getIndexCount(); i < c; ++i)
            setPrimitiveIndex(PrevIndexCount + i, PrevVertexCount + Other.getPrimitiveIndex(i));
    }
    
    return true;
}

void MeshBuffer::setTriangleIndices(const u32 Index, const u32 (&Indices)[3])
{
    if (Indices)
    {
        const u32 TriangleIndex = Index * 3;
        setPrimitiveIndex(TriangleIndex + 0, Indices[0]);
        setPrimitiveIndex(TriangleIndex + 1, Indices[1]);
        setPrimitiveIndex(TriangleIndex + 2, Indices[2]);
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::setTriangleIndices");
    #endif
}

void MeshBuffer::getTriangleIndices(const u32 Index, u32 (&Indices)[3]) const
{
    if (Indices)
    {
        const u32 TriangleIndex = Index * 3;
        Indices[0] = getPrimitiveIndex(TriangleIndex + 0);
        Indices[1] = getPrimitiveIndex(TriangleIndex + 1);
        Indices[2] = getPrimitiveIndex(TriangleIndex + 2);
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::getTriangleIndices");
    #endif
}

void MeshBuffer::setPrimitiveIndex(const u32 Index, const u32 VertexIndex)
{
    if (Index < getIndexCount() && VertexIndex < getVertexCount())
    {
        switch (IndexFormat_.getDataType())
        {
            case DATATYPE_UNSIGNED_BYTE:
                IndexBuffer_.RawBuffer.set<u8>(Index, 0, (u8)VertexIndex); break;
            case DATATYPE_UNSIGNED_SHORT:
                IndexBuffer_.RawBuffer.set<u16>(Index, 0, (u16)VertexIndex); break;
            case DATATYPE_UNSIGNED_INT:
                IndexBuffer_.RawBuffer.set<u32>(Index, 0, (u32)VertexIndex); break;
            default:
                break;
        }
    }
    #ifdef SP_DEBUGMODE
    else if (Index < getIndexCount())
        io::Log::debug("MeshBuffer::setPrimitiveIndex", "'Index' out of range");
    else
        io::Log::debug("MeshBuffer::setPrimitiveIndex", "'VertexIndex' out of range");
    #endif
}

u32 MeshBuffer::getPrimitiveIndex(const u32 Index) const
{
    if (Index < getIndexCount())
    {
        switch (IndexFormat_.getDataType())
        {
            case DATATYPE_UNSIGNED_BYTE:
                return static_cast<u32>(IndexBuffer_.RawBuffer.get<u8>(Index, 0));
            case DATATYPE_UNSIGNED_SHORT:
                return static_cast<u32>(IndexBuffer_.RawBuffer.get<u16>(Index, 0));
            case DATATYPE_UNSIGNED_INT:
                return static_cast<u32>(IndexBuffer_.RawBuffer.get<u32>(Index, 0));
            default:
                break;
        }
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::getPrimitiveIndex", "'Index' out of range");
    #endif
    
    return 0;
}

scene::SMeshVertex3D MeshBuffer::getVertex(const u32 Index) const
{
    scene::SMeshVertex3D Vertex;
    
    if (VertexFormat_->getFlags() & VERTEXFORMAT_COORD)
        Vertex.setPosition(getVertexCoord(Index));
    if (VertexFormat_->getFlags() & VERTEXFORMAT_COLOR)
        Vertex.setColor(getVertexColor(Index));
    if (VertexFormat_->getFlags() & VERTEXFORMAT_NORMAL)
        Vertex.setNormal(getVertexNormal(Index));
    if (VertexFormat_->getFlags() & VERTEXFORMAT_BINORMAL)
        Vertex.setBinormal(getVertexBinormal(Index));
    if (VertexFormat_->getFlags() & VERTEXFORMAT_TANGENT)
        Vertex.setTangent(getVertexTangent(Index));
    if (VertexFormat_->getFlags() & VERTEXFORMAT_FOGCOORD)
        Vertex.setFog(getVertexFog(Index));
    
    for (u32 i = 0; i < VertexFormat_->getTexCoords().size(); ++i)
        Vertex.setTexCoord(i, getVertexTexCoord(Index, i));
    
    return Vertex;
}
scene::SMeshTriangle3D MeshBuffer::getTriangle(const u32 Index) const
{
    return scene::SMeshTriangle3D(
        getPrimitiveIndex(Index * 3 + 0),
        getPrimitiveIndex(Index * 3 + 1),
        getPrimitiveIndex(Index * 3 + 2)
    );
}

dim::triangle3df MeshBuffer::getTriangleCoords(const u32 Index) const
{
    u32 Indices[3];
    getTriangleIndices(Index, Indices);
    
    return dim::triangle3df(
        getVertexCoord(Indices[0]),
        getVertexCoord(Indices[1]),
        getVertexCoord(Indices[2])
    );
}
dim::ptriangle3df MeshBuffer::getTriangleReference(const u32 Index) const
{
    if (!(VertexFormat_->getFlags() && VERTEXFORMAT_COORD) || VertexFormat_->getCoord().Type != DATATYPE_FLOAT || VertexFormat_->getCoord().Size < 3)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("MeshBuffer::getTriangleReference", "Null pointer reference is returned");
        #endif
        return dim::ptriangle3df(0, 0, 0);
    }
    
    u32 Indices[3];
    getTriangleIndices(Index, Indices);
    const u32 Offset = VertexFormat_->getCoord().Offset;
    
    dim::vector3df* A = (dim::vector3df*)VertexBuffer_.RawBuffer.getArray(Indices[0], Offset);
    dim::vector3df* B = (dim::vector3df*)VertexBuffer_.RawBuffer.getArray(Indices[1], Offset);
    dim::vector3df* C = (dim::vector3df*)VertexBuffer_.RawBuffer.getArray(Indices[2], Offset);
    
    return dim::ptriangle3df(A, B, C);
}

void MeshBuffer::flipTriangles()
{
    if (PrimitiveType_ == PRIMITIVE_TRIANGLES)
    {
        for (u32 i = 0, c = getIndexCount(), v; i < c; i += 3)
        {
            v = getPrimitiveIndex(i);
            setPrimitiveIndex(i, getPrimitiveIndex(i + 2));
            setPrimitiveIndex(i + 2, v);
        }
        updateIndexBuffer();
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::flipTriangles", "Wrong primitive type to flip triangles");
    #endif
}

bool MeshBuffer::cutTriangle(const u32 Index, const dim::plane3df &ClipPlane)
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("MeshBuffer::cutTriangle", "Not implemented yet");
    #endif
    return false; // todo !!!
}


/* === Vertex manipulation functions === */

void MeshBuffer::setVertexAttribute(const u32 Index, const SVertexAttribute &Attrib, const void* AttribData, u32 Size)
{
    VertexBuffer_.RawBuffer.setBuffer(
        Index, Attrib.Offset, AttribData, math::Min(Attrib.Size * VertexFormat::getDataTypeSize(Attrib.Type), static_cast<s32>(Size))
    );
}
void MeshBuffer::getVertexAttribute(const u32 Index, const SVertexAttribute &Attrib, void* AttribData, u32 Size)
{
    VertexBuffer_.RawBuffer.getBuffer(
        Index, Attrib.Offset, AttribData, math::Min(Attrib.Size * VertexFormat::getDataTypeSize(Attrib.Type), static_cast<s32>(Size))
    );
}

void MeshBuffer::setVertexCoord(const u32 Index, const dim::vector3df &Coord)
{
    if (VertexFormat_->getFlags() & VERTEXFORMAT_COORD)
        setDefaultVertexAttribute<dim::vector3df, f32>(DATATYPE_FLOAT, 3, Index, VertexFormat_->getCoord(), Coord);
}
dim::vector3df MeshBuffer::getVertexCoord(const u32 Index) const
{
    if (VertexFormat_->getFlags() & VERTEXFORMAT_COORD)
        return getDefaultVertexAttribute<dim::vector3df, f32>(DATATYPE_FLOAT, 3, Index, VertexFormat_->getCoord());
    return 0;
}

void MeshBuffer::setVertexNormal(const u32 Index, const dim::vector3df &Normal)
{
    if (VertexFormat_->getFlags() & VERTEXFORMAT_NORMAL)
        setDefaultVertexAttribute<dim::vector3df, f32>(DATATYPE_FLOAT, 3, Index, VertexFormat_->getNormal(), Normal);
}
dim::vector3df MeshBuffer::getVertexNormal(const u32 Index) const
{
    if (VertexFormat_->getFlags() & VERTEXFORMAT_NORMAL)
        return getDefaultVertexAttribute<dim::vector3df, f32>(DATATYPE_FLOAT, 3, Index, VertexFormat_->getNormal());
    return 0;
}

void MeshBuffer::setVertexTangent(const u32 Index, const dim::vector3df &Tangent)
{
    if (VertexFormat_->getFlags() & VERTEXFORMAT_TANGENT)
        setDefaultVertexAttribute<dim::vector3df, f32>(DATATYPE_FLOAT, 3, Index, VertexFormat_->getTangent(), Tangent);
}
dim::vector3df MeshBuffer::getVertexTangent(const u32 Index) const
{
    if (VertexFormat_->getFlags() & VERTEXFORMAT_TANGENT)
        return getDefaultVertexAttribute<dim::vector3df, f32>(DATATYPE_FLOAT, 3, Index, VertexFormat_->getTangent());
    return 0;
}

void MeshBuffer::setVertexBinormal(const u32 Index, const dim::vector3df &Binormal)
{
    if (VertexFormat_->getFlags() & VERTEXFORMAT_BINORMAL)
        setDefaultVertexAttribute<dim::vector3df, f32>(DATATYPE_FLOAT, 3, Index, VertexFormat_->getBinormal(), Binormal);
}
dim::vector3df MeshBuffer::getVertexBinormal(const u32 Index) const
{
    if (VertexFormat_->getFlags() & VERTEXFORMAT_BINORMAL)
        return getDefaultVertexAttribute<dim::vector3df, f32>(DATATYPE_FLOAT, 3, Index, VertexFormat_->getBinormal());
    return 0;
}

void MeshBuffer::setVertexColor(const u32 Index, const color &Color)
{
    if (VertexFormat_->getFlags() & VERTEXFORMAT_COLOR)
    {
        if (__spVideoDriver->getRendererType() == RENDERER_DIRECT3D9)
            setDefaultVertexAttribute<u32, u8>(DATATYPE_UNSIGNED_BYTE, 4, Index, VertexFormat_->getColor(), Color.getSingle());
        else
            setDefaultVertexAttribute<color, u8>(DATATYPE_UNSIGNED_BYTE, 4, Index, VertexFormat_->getColor(), Color);
    }
}
color MeshBuffer::getVertexColor(const u32 Index) const
{
    if (VertexFormat_->getFlags() & VERTEXFORMAT_COLOR)
    {
        if (__spVideoDriver->getRendererType() == RENDERER_DIRECT3D9)
        {
            color OutputColor;
            OutputColor.setSingle(getDefaultVertexAttribute<u32, u8>(DATATYPE_UNSIGNED_BYTE, 4, Index, VertexFormat_->getColor()));
            return OutputColor;
        }
        return getDefaultVertexAttribute<color, u8>(DATATYPE_UNSIGNED_BYTE, 4, Index, VertexFormat_->getColor());
    }
    return 0;
}

void MeshBuffer::setVertexTexCoord(const u32 Index, const dim::vector3df &TexCoord, const u8 Layer)
{
    const u32 Count = VertexFormat_->getTexCoords().size();
    
    if (Layer == TEXTURE_IGNORE)
    {
        for (u32 i = 0; i < Count; ++i)
            setVertexTexCoord(Index, TexCoord, i);
    }
    else if (Layer < Count)
        setDefaultVertexAttribute<dim::vector3df, f32>(DATATYPE_FLOAT, 3, Index, VertexFormat_->getTexCoords()[Layer], TexCoord);
}
dim::vector3df MeshBuffer::getVertexTexCoord(const u32 Index, const u8 Layer) const
{
    if (Layer < VertexFormat_->getTexCoords().size())
        return getDefaultVertexAttribute<dim::vector3df, f32>(DATATYPE_FLOAT, 3, Index, VertexFormat_->getTexCoords()[Layer]);
    return 0;
}

void MeshBuffer::setVertexFog(const u32 Index, const f32 FogCoord)
{
    if (VertexFormat_->getFlags() & VERTEXFORMAT_FOGCOORD)
        setDefaultVertexAttribute<f32, f32>(DATATYPE_FLOAT, 1, Index, VertexFormat_->getFogCoord(), FogCoord);
}
f32 MeshBuffer::getVertexFog(const u32 Index) const
{
    if (VertexFormat_->getFlags() & VERTEXFORMAT_FOGCOORD)
        return getDefaultVertexAttribute<f32, f32>(DATATYPE_FLOAT, 1, Index, VertexFormat_->getFogCoord());
    return 0;
}


/* === Mesh manipulation functions === */

void MeshBuffer::updateNormals(const EShadingTypes Shading)
{
    if (PrimitiveType_ != PRIMITIVE_TRIANGLES || !getTriangleCount())
    {
        #ifdef SP_DEBUGMODE
        if (PrimitiveType_ != PRIMITIVE_TRIANGLES)
            io::Log::debug("MeshBuffer::updateNormals", "Wrong primitive type to update normals");
        else
            io::Log::debug("MeshBuffer::updateNormals", "No triangles to update normals");
        #endif
        return;
    }
    
    if (Shading == SHADING_FLAT)
        updateNormalsFlat();
    else
        updateNormalsGouraud();
    
    updateVertexBuffer();
}

void MeshBuffer::updateTangentSpace(const u8 TangentLayer, const u8 BinormalLayer, bool UpdateNormals)
{
    if (PrimitiveType_ != PRIMITIVE_TRIANGLES)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("MeshBuffer::updateTangentSpace", "Wrong primitive type to update tangent space");
        #endif
        return;
    }
    
    #ifdef SP_DEBUGMODE
    if (TangentLayer == TEXTURE_IGNORE && !(VertexFormat_->getFlags() & VERTEXFORMAT_TANGENT))
    {
        io::Log::debug("MeshBuffer::updateTangentSpace", "'Tangent' not supported in active vertex format");
        return;
    }
    if (BinormalLayer == TEXTURE_IGNORE && !(VertexFormat_->getFlags() & VERTEXFORMAT_BINORMAL))
    {
        io::Log::debug("MeshBuffer::updateTangentSpace", "'Binormal' not supported in active vertex format");
        return;
    }
    if (math::Max(TangentLayer, BinormalLayer) >= VertexFormat_->getTexCoords().size())
    {
        io::Log::debug("MeshBuffer::updateTangentSpace", "Not enough texture coordinates in active vertex format");
        return;
    }
    if (VertexFormat_->getTexCoords()[TangentLayer].Size < 3)
    {
        io::Log::debug("MeshBuffer::updateTangentSpace", "Tangent texture layer has not enough components");
        return;
    }
    if (VertexFormat_->getTexCoords()[BinormalLayer].Size < 3)
    {
        io::Log::debug("MeshBuffer::updateTangentSpace", "Binormal texture layer has not enough components");
        return;
    }
    #endif
    
    dim::vector3df Tangent, Binormal, Normal;
    u32 TriIndices[3], Indices[3];
    
    for (u32 i = 0, j; i < getTriangleCount(); ++i)
    {
        getTriangleIndices(i, TriIndices);
        
        for (j = 0; j < 3; ++j)
        {
            switch (j)
            {
                case 0:
                    Indices[0] = TriIndices[0];
                    Indices[1] = TriIndices[1];
                    Indices[2] = TriIndices[2];
                    break;
                case 1:
                    Indices[0] = TriIndices[1];
                    Indices[1] = TriIndices[2];
                    Indices[2] = TriIndices[0];
                    break;
                case 2:
                    Indices[0] = TriIndices[2];
                    Indices[1] = TriIndices[0];
                    Indices[2] = TriIndices[1];
                    break;
            }
            
            math::getTangentSpace(
                getVertexCoord(Indices[0]), getVertexCoord(Indices[1]), getVertexCoord(Indices[2]),
                getVertexTexCoord(Indices[0]), getVertexTexCoord(Indices[1]), getVertexTexCoord(Indices[2]),
                Tangent, Binormal, Normal
            );
            
            if (TangentLayer == TEXTURE_IGNORE)
                setVertexTangent(Indices[0], Tangent);
            else
                setVertexTexCoord(Indices[0], Tangent, TangentLayer);
            
            if (BinormalLayer == TEXTURE_IGNORE)
                setVertexBinormal(Indices[0], Binormal);
            else
                setVertexTexCoord(Indices[0], Binormal, BinormalLayer);
            
            if (UpdateNormals)
                setVertexNormal(Indices[0], Normal);
        }
    }
    
    updateVertexBuffer();
}

void MeshBuffer::meshTranslate(const dim::vector3df &Direction)
{
    scene::MeshModifier::meshTranslate(*this, Direction);
}
void MeshBuffer::meshTransform(const dim::vector3df &Size)
{
    scene::MeshModifier::meshTransform(*this, Size);
}
void MeshBuffer::meshTransform(const dim::matrix4f &Matrix)
{
    scene::MeshModifier::meshTransform(*this, Matrix);
}
void MeshBuffer::meshTurn(const dim::vector3df &Rotation)
{
    scene::MeshModifier::meshTransform(*this, Rotation);
}
void MeshBuffer::meshFlip()
{
    scene::MeshModifier::meshFlip(*this);
}
void MeshBuffer::meshFlip(bool isXAxis, bool isYAxis, bool isZAxis)
{
    scene::MeshModifier::meshFlip(*this, isXAxis, isYAxis, isZAxis);
}

void MeshBuffer::clipConcatenatedTriangles()
{
    if (PrimitiveType_ != PRIMITIVE_TRIANGLES)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("MeshBuffer::clipConcatenatedTriangles", "Wrong primitive type to clip concatenated triangles");
        #endif
        return;
    }
    
    /* Store old buffer and resize the new one */
    const u32 BufferStride  = VertexBuffer_.RawBuffer.getStride();
    const u32 TriangleCount = getTriangleCount();
    u32 Indices[3];
    
    dim::UniversalBuffer OldVertexBuffer = VertexBuffer_.RawBuffer;
    VertexBuffer_.RawBuffer.setSize(BufferStride * TriangleCount * 3);
    
    s8* DestBuffer      = VertexBuffer_.RawBuffer.getArray();
    const s8* SrcBuffer = OldVertexBuffer.getArray();
    
    for (u32 i = 0, j = 0; i < TriangleCount; ++i, j += 3)
    {
        /* Get old indices */
        Indices[0] = getPrimitiveIndex(j + 0);
        Indices[1] = getPrimitiveIndex(j + 1);
        Indices[2] = getPrimitiveIndex(j + 2);
        
        /* Copy vertex data */
        memcpy(DestBuffer + BufferStride * (j + 0), SrcBuffer + BufferStride * Indices[0], BufferStride);
        memcpy(DestBuffer + BufferStride * (j + 1), SrcBuffer + BufferStride * Indices[1], BufferStride);
        memcpy(DestBuffer + BufferStride * (j + 2), SrcBuffer + BufferStride * Indices[2], BufferStride);
        
        /* Set new indices */
        setPrimitiveIndex(j + 0, j + 0);
        setPrimitiveIndex(j + 1, j + 1);
        setPrimitiveIndex(j + 2, j + 2);
    }
    
    updateMeshBuffer();
}

void MeshBuffer::paint(const video::color &Color, bool CombineColors)
{
    if (!(VertexFormat_->getFlags() & VERTEXFORMAT_COLOR))
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("MeshBuffer::paint", "'Color' not supported in active vertex format");
        #endif
        return;
    }
    
    if (CombineColors)
    {
        video::color TmpColor;
        
        for (u32 i = 0; i < getVertexCount(); ++i)
        {
            TmpColor = getVertexColor(i);
            
            TmpColor.Red    = static_cast<u8>(static_cast<s32>(TmpColor.Red     ) * static_cast<s32>(Color.Red  ) / 255);
            TmpColor.Green  = static_cast<u8>(static_cast<s32>(TmpColor.Green   ) * static_cast<s32>(Color.Green) / 255);
            TmpColor.Blue   = static_cast<u8>(static_cast<s32>(TmpColor.Blue    ) * static_cast<s32>(Color.Blue ) / 255);
            TmpColor.Alpha  = static_cast<u8>(static_cast<s32>(TmpColor.Alpha   ) * static_cast<s32>(Color.Alpha) / 255);
            
            setVertexColor(i, TmpColor);
        }
    }
    else
    {
        for (u32 i = 0; i < getVertexCount(); ++i)
            setVertexColor(i, Color);
    }
    
    updateVertexBuffer();
}


/* === Texture functions === */

void MeshBuffer::addTexture(Texture* Tex, const u8 Layer)
{
    addTexture(SMeshSurfaceTexture(Tex), Layer);
}

void MeshBuffer::addTexture(const SMeshSurfaceTexture &Tex, const u8 Layer)
{
    /* Add or insert the new texture */
    if (Layer >= TextureList_->size())
        TextureList_->push_back(Tex);
    else
        TextureList_->insert(TextureList_->begin() + static_cast<s32>(Layer), Tex);
}

void MeshBuffer::removeTexture(const u8 Layer)
{
    if (!TextureList_->empty())
    {
        if (Layer >= TextureList_->size())
            TextureList_->pop_back();
        else
            TextureList_->erase(TextureList_->begin() + static_cast<s32>(Layer));
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::removeTexture", "No textures to remove");
    #endif
}

void MeshBuffer::removeTexture(Texture* Tex)
{
    if (Tex)
    {
        for (std::vector<SMeshSurfaceTexture>::iterator it = TextureList_->begin();
             it != TextureList_->end();)
        {
            if (it->TextureObject == Tex)
                it = TextureList_->erase(it);
            else
                ++it;
        }
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::removeTexture");
    #endif
}

void MeshBuffer::clearTextureList()
{
    TextureList_->clear();
}

void MeshBuffer::textureTranslate(const u8 Layer, const dim::vector3df &Direction)
{
    if (Layer < VertexFormat_->getTexCoords().size())
    {
        for (u32 i = 0, c = getVertexCount(); i < c; ++i)
            setVertexTexCoord(i, getVertexTexCoord(i, Layer) + Direction, Layer);
        updateVertexBuffer();
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::textureTranslate", DEBERR_LAYER_RANGE);
    #endif
}

void MeshBuffer::textureTransform(const u8 Layer, const dim::vector3df &Size)
{
    if (Layer < VertexFormat_->getTexCoords().size())
    {
        for (u32 i = 0, c = getVertexCount(); i < c; ++i)
            setVertexTexCoord(i, getVertexTexCoord(i, Layer) * Size, Layer);
        updateVertexBuffer();
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::textureTransform", DEBERR_LAYER_RANGE);
    #endif
}

void MeshBuffer::textureTurn(const u8 Layer, const f32 Rotation)
{
    if (Layer < VertexFormat_->getTexCoords().size())
    {
        dim::matrix4f Matrix;
        Matrix.rotateZ(Rotation);
        
        for (u32 i = 0, c = getVertexCount(); i < c; ++i)
            setVertexTexCoord(i, Matrix * getVertexTexCoord(i, Layer), Layer);
        
        updateVertexBuffer();
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::textureTurn", DEBERR_LAYER_RANGE);
    #endif
}

void MeshBuffer::setSurfaceTexture(const u8 Layer, const SMeshSurfaceTexture &SurfaceTex)
{
    if (Layer < TextureList_->size())
        (*TextureList_)[Layer] = SurfaceTex;
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::setSurfaceTexture", DEBERR_LAYER_RANGE);
    #endif
}
SMeshSurfaceTexture MeshBuffer::getSurfaceTexture(const u8 Layer) const
{
    if (Layer < TextureList_->size())
        return (*TextureList_)[Layer];
    #ifdef SP_DEBUGMODE
    io::Log::debug("MeshBuffer::getSurfaceTexture", DEBERR_LAYER_RANGE);
    #endif
    return SMeshSurfaceTexture();
}

void MeshBuffer::setTexture(const u8 Layer, Texture* Tex)
{
    if (Layer < TextureList_->size())
        (*TextureList_)[Layer].TextureObject = Tex;
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::setTexture", DEBERR_LAYER_RANGE);
    #endif
}
Texture* MeshBuffer::getTexture(const u8 Layer) const
{
    if (Layer < TextureList_->size())
        return (*TextureList_)[Layer].TextureObject;
    return 0;
}

void MeshBuffer::setTextureMatrix(const u8 Layer, const dim::matrix4f &Matrix)
{
    if (Layer < TextureList_->size())
        (*TextureList_)[Layer].Matrix = Matrix;
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::setTextureMatrix", DEBERR_LAYER_RANGE);
    #endif
}
dim::matrix4f MeshBuffer::getTextureMatrix(const u8 Layer) const
{
    if (Layer < TextureList_->size())
        return (*TextureList_)[Layer].Matrix;
    #ifdef SP_DEBUGMODE
    io::Log::debug("MeshBuffer::getTextureMatrix", DEBERR_LAYER_RANGE);
    #endif
    return dim::matrix4f();
}

void MeshBuffer::setTextureEnv(const u8 Layer, const ETextureEnvTypes Type)
{
    if (Layer < TextureList_->size())
        (*TextureList_)[Layer].TexEnvType = Type;
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::setTextureEnv", DEBERR_LAYER_RANGE);
    #endif
}
ETextureEnvTypes MeshBuffer::getTextureEnv(const u8 Layer) const
{
    if (Layer < TextureList_->size())
        return (*TextureList_)[Layer].TexEnvType;
    #ifdef SP_DEBUGMODE
    io::Log::debug("MeshBuffer::getTextureEnv", DEBERR_LAYER_RANGE);
    #endif
    return TEXENV_MODULATE;
}

void MeshBuffer::setMappingGen(const u8 Layer, const EMappingGenTypes Type)
{
    if (Layer < TextureList_->size())
    {
        s32 Coords = 0;
        
        /* Set mapping generation coordinates */
        switch (Type)
        {
            case video::MAPGEN_DISABLE:
                Coords = video::MAPGEN_NONE; break;
            case video::MAPGEN_OBJECT_LINEAR:
            case video::MAPGEN_EYE_LINEAR:
            case video::MAPGEN_SPHERE_MAP:
            case video::MAPGEN_NORMAL_MAP:
                Coords = video::MAPGEN_S | video::MAPGEN_T; break;
            case video::MAPGEN_REFLECTION_MAP:
                Coords = video::MAPGEN_S | video::MAPGEN_T | video::MAPGEN_R; break;
        }
        
        /* Set mapping generation type */
        (*TextureList_)[Layer].TexMappingGen       = Type;
        (*TextureList_)[Layer].TexMappingCoords    = Coords;
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::setMappingGen", DEBERR_LAYER_RANGE);
    #endif
}
EMappingGenTypes MeshBuffer::getMappingGen(const u8 Layer) const
{
    if (Layer < TextureList_->size())
        return (*TextureList_)[Layer].TexMappingGen;
    #ifdef SP_DEBUGMODE
    io::Log::debug("MeshBuffer::getMappingGen", DEBERR_LAYER_RANGE);
    #endif
    return MAPGEN_DISABLE;
}

void MeshBuffer::setMappingGenCoords(const u8 Layer, s32 Coords)
{
    if (Layer < TextureList_->size())
        (*TextureList_)[Layer].TexMappingCoords = Coords;
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("MeshBuffer::setMappingGenCoords", DEBERR_LAYER_RANGE);
    #endif
}
s32 MeshBuffer::getMappingGenCoords(const u8 Layer) const
{
    if (Layer < TextureList_->size())
        return (*TextureList_)[Layer].TexMappingCoords;
    #ifdef SP_DEBUGMODE
    io::Log::debug("MeshBuffer::getMappingGenCoords", DEBERR_LAYER_RANGE);
    #endif
    return 0;
}

std::list<Texture*> MeshBuffer::getTextureList() const
{
    std::list<Texture*> TexList;
    
    for (std::vector<SMeshSurfaceTexture>::const_iterator it = TextureList_->begin(); it != TextureList_->end(); ++it)
        TexList.push_back(it->TextureObject);
    
    return TexList;
}

void MeshBuffer::setTexturesReference(MeshBuffer* Reference)
{
    if (Reference)
        TextureList_ = Reference->TextureList_;
    else
        TextureList_ = &OrigTextureList_;
}
void MeshBuffer::setTexturesReference(std::vector<SMeshSurfaceTexture>* &Reference)
{
    if (Reference)
        TextureList_ = Reference;
    else
        TextureList_ = &OrigTextureList_;
}


/*
 * ======= Protected: =======
 */

void MeshBuffer::convertVertexAttribute(
    const dim::UniversalBuffer &OldBuffer, u32 Index, const SVertexAttribute &OldAttrib, const SVertexAttribute &NewAttrib, bool isClamp)
{
    #define mcrReadComponent(t)     Component = static_cast<f64>(OldBuffer.get<t>(Index, OldAttrib.Offset + OldCompSize * i))
    #define mcrWriteComponent(t)    VertexBuffer_.RawBuffer.set<t>(Index, NewAttrib.Offset + NewCompSize * i, static_cast<t>(Component));
    
    s32 OldCompSize = VertexFormat::getDataTypeSize(OldAttrib.Type);
    s32 NewCompSize = VertexFormat::getDataTypeSize(NewAttrib.Type);
    
    for (s32 i = 0; i < NewAttrib.Size; ++i)
    {
        f64 Component = 0;
        
        bool isFinalClamp = false;
        
        if ( isClamp && NewAttrib.Type < DATATYPE_UNSIGNED_BYTE)
            isFinalClamp = true;
        
        switch (OldAttrib.Type)
        {
            case DATATYPE_FLOAT:
                mcrReadComponent(f32); break;
            case DATATYPE_DOUBLE:
                mcrReadComponent(f64); break;
            case DATATYPE_BYTE:
                mcrReadComponent(s8); break;
            case DATATYPE_SHORT:
                mcrReadComponent(s16); break;
            case DATATYPE_INT:
                mcrReadComponent(s32); break;
            case DATATYPE_UNSIGNED_BYTE:
                mcrReadComponent(u8);
                if (isFinalClamp) Component /= UCHAR_MAX;
                break;
            case DATATYPE_UNSIGNED_SHORT:
                mcrReadComponent(u16);
                if (isFinalClamp) Component /= USHRT_MAX;
                break;
            case DATATYPE_UNSIGNED_INT:
                mcrReadComponent(u32);
                if (isFinalClamp) Component /= UINT_MAX;
                break;
        }
        
        switch (NewAttrib.Type)
        {
            case DATATYPE_FLOAT:
                mcrWriteComponent(f32); break;
            case DATATYPE_DOUBLE:
                mcrWriteComponent(f64); break;
            case DATATYPE_BYTE:
                mcrWriteComponent(s8); break;
            case DATATYPE_SHORT:
                mcrWriteComponent(s16); break;
            case DATATYPE_INT:
                mcrWriteComponent(s32); break;
            case DATATYPE_UNSIGNED_BYTE:
                if (isFinalClamp) Component *= UCHAR_MAX;
                mcrWriteComponent(u8);
                break;
            case DATATYPE_UNSIGNED_SHORT:
                if (isFinalClamp) Component *= USHRT_MAX;
                mcrWriteComponent(u16);
                break;
            case DATATYPE_UNSIGNED_INT:
                if (isFinalClamp) Component *= UINT_MAX;
                mcrWriteComponent(u32);
                break;
        }
    }
    
    #undef mcrReadComponent
    #undef mcrWriteComponent
}

void MeshBuffer::fillVertexAttribute(u32 Index, const SVertexAttribute &Attrib)
{
    const s32 Size = VertexFormat::getDataTypeSize(Attrib.Type) * Attrib.Size;
    
    s8* Buffer = new s8[Size];
    memset(Buffer, 0, Size);
    
    VertexBuffer_.RawBuffer.setBuffer(Index, Attrib.Offset, Buffer, Size);
    
    delete [] Buffer;
}

void MeshBuffer::updateNormalsFlat()
{
    dim::vector3df Normal;
    u32 Indices[3];
    
    for (u32 i = 0, c = getTriangleCount(); i < c; ++i)
    {
        /* Compute normal vector for the current triangle */
        getTriangleIndices(i, Indices);
        
        Normal = math::getNormalVector(
            getVertexCoord(Indices[0]),
            getVertexCoord(Indices[1]),
            getVertexCoord(Indices[2])
        );
        
        /* Set normal for the three associated vertices */
        setVertexNormal(Indices[0], Normal);
        setVertexNormal(Indices[1], Normal);
        setVertexNormal(Indices[2], Normal);
    }
}

void MeshBuffer::updateNormalsGouraud()
{
    /* Temporary variables */
    u32 Indices[3];
    dim::vector3df VertexCoord[3];
    
    std::list<SCmpNormalCoord> NormalsList;
    SCmpNormalCoord NormalPackage;
    
    /* Compute the normal for each triangle */
    for (u32 i = 0, c = getTriangleCount(); i < c; ++i)
    {
        /* Compute normal vector for the current triangle */
        getTriangleIndices(i, Indices);
        
        VertexCoord[0] = getVertexCoord(Indices[0]);
        VertexCoord[1] = getVertexCoord(Indices[1]);
        VertexCoord[2] = getVertexCoord(Indices[2]);
        
        NormalPackage.Normal = math::getNormalVector(
            VertexCoord[0], VertexCoord[1], VertexCoord[2]
        );
        
        /* Store the vertex data */
        NormalPackage.Index     = Indices[0];
        NormalPackage.Position  = VertexCoord[0];
        NormalsList.push_back(NormalPackage);
        
        NormalPackage.Index     = Indices[1];
        NormalPackage.Position  = VertexCoord[1];
        NormalsList.push_back(NormalPackage);
        
        NormalPackage.Index     = Indices[2];
        NormalPackage.Position  = VertexCoord[2];
        NormalsList.push_back(NormalPackage);
    }
    
    /* Sort the list by means of vertex coordinates */
    NormalsList.sort(cmpVertexCoords);
    
    /* Temporary variables */
    s32 CurNormalCount = 0;
    dim::vector3df CurCoord = NormalsList.begin()->Position;
    dim::vector3df CurNormal;
    
    std::list<SCmpNormalCoord>::iterator it = NormalsList.begin(), itStart = NormalsList.begin(), itNext;
    
    /* Loop for each vertex in the sorted list */
    do
    {
        /* Add current normal vector to the sum */
        CurNormal += it->Normal;
        ++CurNormalCount;
        
        /* Get next iteration preview */
        itNext = it;
        ++itNext;
        
        /* Check if the vertex group is complete. This happens when the coordinate is unequal or the normal angle is too large */
        if (itNext == NormalsList.end() || !CurCoord.equal(itNext->Position))
        {
            /* Compute arithmetic average */
            CurNormal /= static_cast<f32>(CurNormalCount);
            
            /* Set normal for the vertex group */
            for (itNext = itStart; ; ++itNext)
            {
                setVertexNormal(itNext->Index, CurNormal);
                if (itNext == it)
                    break;
            }
            
            /* Reset normal sum */
            ++it;
            
            if (it != NormalsList.end())
            {
                CurNormalCount  = 0;
                CurNormal       = 0;
                CurCoord        = it->Position;
                itStart         = it;
            }
        }
        else
            ++it;
    }
    while (it != NormalsList.end());
}

void MeshBuffer::checkIndexFormat(ERendererDataTypes &Format)
{
    if ( Format == DATATYPE_UNSIGNED_BYTE &&
         ( __spVideoDriver->getRendererType() == RENDERER_DIRECT3D9 || __spVideoDriver->getRendererType() == RENDERER_DIRECT3D11 ) )
    {
        io::Log::warning("Direct3D does not support 8 bit index buffers; using 16 bit");
        Format = DATATYPE_UNSIGNED_SHORT;
    }
}


/*
 * ======= Private: =======
 */

void MeshBuffer::setupDefaultBuffers()
{
    /* Create index format */
    IndexFormat_.setDataType(DATATYPE_UNSIGNED_INT);
    
    /* Configure vertex- and index buffer */
    VertexBuffer_.RawBuffer.setStride(VertexFormat_->getFormatSize());
    IndexBuffer_.RawBuffer.setStride(4);
}


} // /namespace video

} // /namespace sp



// ================================================================================
