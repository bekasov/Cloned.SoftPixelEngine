/*
 * Mesh saver SPM file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Mesh/spMeshSaverSPM.hpp"

#ifdef SP_COMPILE_WITH_MESHSAVER_SPM


namespace sp
{
namespace scene
{


bool MeshSaverSPM::isTextureIntern_ = false;

MeshSaverSPM::MeshSaverSPM() : MeshSaver()
{
}
MeshSaverSPM::~MeshSaverSPM()
{
}

bool MeshSaverSPM::saveMesh(Mesh* Model, const io::stringc &Filename)
{
    if (!openSaveFile(Model, Filename))
        return false;
    
    writeHeader();
    writeChunkObject();
    
    return true;
}

void MeshSaverSPM::setTextureIntern(bool isWriteIntern)
{
    isTextureIntern_ = isWriteIntern;
}
bool MeshSaverSPM::getTextureIntern()
{
    return isTextureIntern_;
}


/*
 * ========== Private: ==========
 */

void MeshSaverSPM::writeHeader()
{
    // Write header information: magic number ("SPMD"), format version
    File_->writeValue<s32>(SPM_MAGIC_NUMBER);
    File_->writeValue<u16>(SPM_VERSION_NUMBER);
}

void MeshSaverSPM::writeChunkObject()
{
    // Write main mesh and each sub mesh
    std::vector<Mesh*> SubMeshList = Mesh_->getLODSubMeshList();
    
    File_->writeValue<u32>(SubMeshList.size() + 1);
    
    writeChunkSubMesh(Mesh_);
    
    for (std::vector<Mesh*>::iterator it = SubMeshList.begin(); it != SubMeshList.end(); ++it)
        writeChunkSubMesh(*it);
}

void MeshSaverSPM::writeChunkSubMesh(Mesh* SubMesh)
{
    CurMesh_ = SubMesh;
    
    // Write object information: name, flags
    File_->writeStringData(CurMesh_->getName());
    
    isGouraudShading_ = (CurMesh_->getMaterial()->getShading() == video::SHADING_GOURAUD);
    
    // Set the flags
    u16 MeshFlags = MDLSPM_CHUNK_NONE;
    
    if (isGouraudShading_)
        MeshFlags |= MDLSPM_CHUNK_GOURAUDSHADING;
    
    File_->writeValue<u16>(MeshFlags);
    
    // Reserved data for editoring usage, here not used
    // Following value is the count of bytes used for the user data, here 0 (null)
    // If the reserved data is used, the bytes need to follow
    File_->writeValue<u32>(0);
    
    // Write each surface
    File_->writeValue<u32>(CurMesh_->getMeshBufferCount());
    
    for (u32 s = 0; s < CurMesh_->getMeshBufferCount(); ++s)
        writeChunkSurface(s);
    
    // Write animaions
    writeChunkAnimationNode();
    writeChunkAnimationMorphTarget();
    writeChunkAnimationSkeletal();
}

void MeshSaverSPM::writeChunkSurface(const u32 Surface)
{
    Surface_ = Mesh_->getMeshBuffer(Surface);
    
    // Write surface information: name, flags
    File_->writeStringData(Surface_->getName());
    
    areIndices32Bit_            = areIndex32BitNeeded();
    areVertexColorsEqual_       = areVertexColorsEqual();
    areVertexFogCoordsEqual_    = areVertexFogCoordsEqual();
    
    // Set the flags
    u16 SurfaceFlags = MDLSPM_CHUNK_NONE;
    
    if (areIndices32Bit_)
        SurfaceFlags |= MDLSPM_CHUNK_INDEX32BIT;
    if (!areVertexColorsEqual_)
        SurfaceFlags |= MDLSPM_CHUNK_VERTEXCOLOR;
    if (!areVertexFogCoordsEqual_)
        SurfaceFlags |= MDLSPM_CHUNK_VERTEXFOG;
    
    File_->writeValue<u16>(SurfaceFlags);
    
    // Wirte texture coordinates dimensions
    checkTexCoordsDimensions();
    
    for (u8 i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)
        File_->writeValue<u8>(TexCoordsDimensions_[i]);
    
    // Write each texture
    TexLayerCount_ = (u8)Surface_->getTextureCount();
    
    File_->writeValue<u8>(TexLayerCount_);
    
    for (u8 i = 0; i < TexLayerCount_; ++i)
        writeChunkTexture(i);
    
    // Write each vertex
    const u32 VertexCount = Surface_->getVertexCount();
    
    File_->writeValue<u32>(VertexCount);
    
    if (VertexCount)
    {
        // Write vertex default values: color, fog coord
        if (areVertexColorsEqual_)
            File_->writeColor(Surface_->getVertexColor(0));
        if (areVertexFogCoordsEqual_)
            File_->writeValue<f32>(Surface_->getVertexFog(0));
        
        // Write the vertex data
        for (u32 i = 0; i < VertexCount; ++i)
            writeChunkVertex(i);
    }
    
    // Write each triangle
    const u32 TriangleCount = Surface_->getTriangleCount();
    
    File_->writeValue<u32>(TriangleCount);
    
    if (TriangleCount)
    {
        // Write the triangle data
        for (u32 i = 0; i < TriangleCount; ++i)
            writeChunkTriangle(i);
    }
}

void MeshSaverSPM::writeChunkVertex(const u32 Vertex)
{
    // Write position
    File_->writeVector(Surface_->getVertexCoord(Vertex));
    
    // Write texture coordinates
    dim::vector3df TexCoord;
    
    for (u8 i = 0, j; i < MAX_COUNT_OF_TEXTURES; ++i)
    {
        if (TexCoordsDimensions_[i] > 0)
        {
            TexCoord = Surface_->getVertexTexCoord(Vertex, i);
            
            for (j = 0; j < TexCoordsDimensions_[i]; ++j)
                File_->writeValue<f32>(TexCoord[j]);
        }
    }
    
    // Write color
    if (!areVertexColorsEqual_)
        File_->writeColor(Surface_->getVertexColor(Vertex));
    
    // Wirte fog coordinate
    if (!areVertexFogCoordsEqual_)
        File_->writeValue<f32>(Surface_->getVertexFog(Vertex));
}

void MeshSaverSPM::writeChunkTriangle(const u32 Triangle)
{
    // Get the indices
    u32 Indices[3];
    Surface_->getTriangleIndices(Triangle, Indices);
    
    // Write the indices (32 bits or 16 bits)
    if (areIndices32Bit_)
    {
        File_->writeValue<u32>(Indices[0]);
        File_->writeValue<u32>(Indices[1]);
        File_->writeValue<u32>(Indices[2]);
    }
    else
    {
        File_->writeValue<u16>((u16)Indices[0]);
        File_->writeValue<u16>((u16)Indices[1]);
        File_->writeValue<u16>((u16)Indices[2]);
    }
}

void MeshSaverSPM::writeChunkTexture(const u8 Layer)
{
    // Write texture basic information
    video::Texture* Tex = Surface_->getTexture(Layer);
    
    File_->writeValue<s8>(s8(Tex != 0));
    
    if (Tex)
    {
        // Write the texture resource information
        File_->writeStringData(Tex->getFilename());
        
        // Set texture flags: environment mapping, matrix etc.
        u16 TextureFlags = MDLSPM_CHUNK_NONE;
        
        if (isTextureIntern_)
            TextureFlags |= MDLSPM_CHUNK_TEXTUREINTERN;
        if (!Surface_->getTextureMatrix(Layer).isIdentity())
            TextureFlags |= MDLSPM_CHUNK_TEXTUREMATRIX;
        
        File_->writeValue<u16>(TextureFlags);
        
        // Write the flags data
        if (TextureFlags & MDLSPM_CHUNK_TEXTUREMATRIX)
            File_->writeMatrix(Surface_->getTextureMatrix(Layer));
        
        File_->writeValue<video::ETextureEnvTypes>(Surface_->getTextureEnv(Layer));
        File_->writeValue<video::EMappingGenTypes>(Surface_->getMappingGen(Layer));
        File_->writeValue<s32>(Surface_->getMappingGenCoords(Layer));
    }
}


void MeshSaverSPM::writeChunkAnimationNode()
{
    
}

void MeshSaverSPM::writeChunkAnimationMorphTarget()
{
    
}

void MeshSaverSPM::writeChunkAnimationSkeletal()
{
    
}

void MeshSaverSPM::writeChunkAnimationJoint(const scene::AnimationJoint* Joint)
{
    
}


bool MeshSaverSPM::areVertexColorsEqual() const
{
    if (!Surface_->getVertexCount())
        return true;
    
    const video::color DefaultColor(Surface_->getVertexColor(0));
    
    for (u32 i = 1; i < Surface_->getVertexCount(); ++i)
    {
        if (Surface_->getVertexColor(i) != DefaultColor)
            return false;
    }
    
    return true;
}

bool MeshSaverSPM::areVertexFogCoordsEqual() const
{
    if (!Surface_->getVertexCount())
        return true;
    
    const f32 DefaultFogCoord(Surface_->getVertexFog(0));
    
    for (u32 i = 1; i < Surface_->getVertexCount(); ++i)
    {
        if (Surface_->getVertexFog(i) != DefaultFogCoord)
            return false;
    }
    
    return true;
}

bool MeshSaverSPM::areIndex32BitNeeded() const
{
    return Surface_->getVertexCount() >= USHRT_MAX;
}

void MeshSaverSPM::checkTexCoordsDimensions()
{
    memset(TexCoordsDimensions_, 0, sizeof(u8)*MAX_COUNT_OF_TEXTURES);
    
    if (!Surface_->getVertexCount())
        return;
    
    dim::vector3df DefaultTexCoord[MAX_COUNT_OF_TEXTURES];
    dim::vector3df CurTexCoord, CurTexCoordBase;
    
    for (u8 j = 0; j < MAX_COUNT_OF_TEXTURES; ++j)
        DefaultTexCoord[j] = Surface_->getVertexTexCoord(0, j);
    
    for (u32 i = 1; i < Surface_->getVertexCount(); ++i)
    {
        for (u8 j = 0; j < MAX_COUNT_OF_TEXTURES; ++j)
        {
            if (TexCoordsDimensions_[j] < 3)
            {
                CurTexCoord = Surface_->getVertexTexCoord(i, j);
                
                if (j)
                    CurTexCoordBase = Surface_->getVertexTexCoord(i, 0);
                
                if ( !DefaultTexCoord[j].equal(CurTexCoord) && ( !j || !CurTexCoordBase.equal(CurTexCoord) ) )
                {
                    if (TexCoordsDimensions_[j] < 1 && !math::equal(DefaultTexCoord[j].X, CurTexCoord.X))
                        TexCoordsDimensions_[j] = 1;
                    if (TexCoordsDimensions_[j] < 2 && !math::equal(DefaultTexCoord[j].Y, CurTexCoord.Y))
                        TexCoordsDimensions_[j] = 2;
                    if (!math::equal(DefaultTexCoord[j].Z, CurTexCoord.Z))
                        TexCoordsDimensions_[j] = 3;
                } // fi
            } // fi
        } // /for layers
    } // /for vertices
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
