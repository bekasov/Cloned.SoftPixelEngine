/*
 * Mesh saver B3D file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Mesh/spMeshSaverB3D.hpp"

#ifdef SP_COMPILE_WITH_MESHSAVER_B3D


namespace sp
{
namespace scene
{


MeshSaverB3D::MeshSaverB3D() : MeshSaver()
{
    CurPos_ = 0;
    memset(Stack_, 0, sizeof(Stack_));
}
MeshSaverB3D::~MeshSaverB3D()
{
}

bool MeshSaverB3D::saveMesh(Mesh* Model, const io::stringc &Filename)
{
    if (!openSaveFile(Model, Filename))
        return false;
    
    saveModelData();
    
    return true;
}


/*
 * ========== Private: ==========
 */

void MeshSaverB3D::beginChunk(io::stringc Tag)
{
    ++CurPos_;
    File_->writeBuffer(Tag.c_str(), 4);
    File_->writeValue<s32>(0);
    Stack_[CurPos_] = File_->getSeek();
}
void MeshSaverB3D::endChunk()
{
    s32 Pos = File_->getSeek();
    File_->setSeek(Stack_[CurPos_] - 4);
    File_->writeValue<s32>(Pos - Stack_[CurPos_]);
    File_->setSeek(Pos);
    --CurPos_;
}

/*
 * Texture flags:
 * 1: Color
 * 2: Alpha
 * 4: Masked
 * 8: Mipmapped
 * 16: Clamp U
 * 32: Clamp V
 * 64: Spherical reflection map
 */

void MeshSaverB3D::saveModelData()
{
    
    video::Texture* Tex = 0;
    s32 TextureFlags = 0;
    s32 TextureCounter = 0;
    f32 MeshColor[4];
    
    Mesh_->getMaterial()->getDiffuseColor().getFloatArray(MeshColor);
    
    beginChunk("BB3D");
    {
        File_->writeValue<s32>(1); // Version
        
        beginChunk("TEXS");
        {
            for (u32 s = 0; s < Mesh_->getMeshBufferCount(); ++s)
            {
                Surface_ = Mesh_->getMeshBuffer(s);
                
                if (Tex = Surface_->getTexture())
                {
                    ++TextureCounter;
                    if (Tex->getColorKey().Alpha != 255)
                        TextureFlags = 4;
                    
                    // Writing
                    File_->writeStringC(Tex->getFilename());
                    File_->writeValue<s32>(TextureFlags);
                    File_->writeValue<s32>(0);
                    File_->writeValue<f32>(0.0f);
                    File_->writeValue<f32>(0.0f);
                    File_->writeValue<f32>(1.0f);
                    File_->writeValue<f32>(1.0f);
                    File_->writeValue<f32>(0.0f);
                }
            }
        }
        endChunk();
        
        beginChunk("BRUS"); // !!! INCOMPLETE !!!
        {
            File_->writeValue<s32>(TextureCounter);
            File_->writeStringC("Brush1");
            File_->writeValue<f32>(MeshColor[0]);
            File_->writeValue<f32>(MeshColor[1]);
            File_->writeValue<f32>(MeshColor[2]);
            File_->writeValue<f32>(MeshColor[3]);
            File_->writeValue<f32>(Mesh_->getMaterial()->getShininess());
            File_->writeValue<s32>(0);
            File_->writeValue<s32>(0);
            
            for (s32 i = 0; i < TextureCounter; ++i)
            {
                File_->writeValue<s32>(i);
            }
        }
        endChunk();
        
        beginChunk("NODE");
        {
            File_->writeStringC(Mesh_->getName());
            
            File_->writeValue<f32>(Mesh_->getPosition().X);
            File_->writeValue<f32>(Mesh_->getPosition().Y);
            File_->writeValue<f32>(Mesh_->getPosition().Z);
            
            File_->writeValue<f32>(Mesh_->getScale().X);
            File_->writeValue<f32>(Mesh_->getScale().Y);
            File_->writeValue<f32>(Mesh_->getScale().Z);
            
            File_->writeValue<f32>(0.0f);
            File_->writeValue<f32>(Mesh_->getRotation().X);
            File_->writeValue<f32>(Mesh_->getRotation().Y);
            File_->writeValue<f32>(Mesh_->getRotation().Z);
            
            writeMesh();
        }
        endChunk();
        
    }
    endChunk();
    
}

void MeshSaverB3D::writeMesh()
{
    
    u32 SurfCount = Mesh_->getMeshBufferCount();
    s32 TriangleCount = 0;
    u32 Indices[3];
    dim::vector3df Coord;
    dim::point2df TexCoord;
    video::color Color;
    
    beginChunk("MESH");
    {
        File_->writeValue<s32>(0); // Brush ID
        
        beginChunk("VRTS");
        {
            File_->writeValue<s32>(3); // Flags
            File_->writeValue<s32>(1); // 1 TexCoords sets
            File_->writeValue<s32>(2); // 2 Coords per set
            
            for (u32 i = 0, j; i < SurfCount; ++i)
            {
                Surface_ = Mesh_->getMeshBuffer(i);
                
                for (j = 0; j < Surface_->getVertexCount(); ++j)
                {
                    // Coord
                    Coord = Surface_->getVertexCoord(j);
                    File_->writeValue<f32>(Coord.X);
                    File_->writeValue<f32>(Coord.Y);
                    File_->writeValue<f32>(Coord.Z);
                    
                    // Normal
                    Coord = Surface_->getVertexNormal(j);
                    File_->writeValue<f32>(Coord.X);
                    File_->writeValue<f32>(Coord.Y);
                    File_->writeValue<f32>(Coord.Z);
                    
                    // Color
                    Color = Surface_->getVertexColor(j);
                    File_->writeValue<f32>(static_cast<f32>(Color.Red   ) / 255);
                    File_->writeValue<f32>(static_cast<f32>(Color.Green ) / 255);
                    File_->writeValue<f32>(static_cast<f32>(Color.Blue  ) / 255);
                    File_->writeValue<f32>(static_cast<f32>(Color.Alpha ) / 255);
                    
                    // TexCoord
                    TexCoord = Surface_->getVertexTexCoord(j);
                    File_->writeValue<f32>(TexCoord.X);
                    File_->writeValue<f32>(TexCoord.Y);
                }
            }
        }
        endChunk();
        
        beginChunk("TRIS");
        {
            File_->writeValue<s32>(0); // Bursh for these triangles
            
            for (u32 i = 0, j; i < SurfCount; ++i)
            {
                Surface_ = Mesh_->getMeshBuffer(i);
                
                for (j = 0; j < Surface_->getTriangleCount(); ++j)
                {
                    Surface_->getTriangleIndices(j, Indices);
                    File_->writeValue<s32>(TriangleCount + Indices[0]);
                    File_->writeValue<s32>(TriangleCount + Indices[1]);
                    File_->writeValue<s32>(TriangleCount + Indices[2]);
                }
                TriangleCount += Surface_->getTriangleCount();
            }
        }
        endChunk();
        
    }
    endChunk();
    
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
