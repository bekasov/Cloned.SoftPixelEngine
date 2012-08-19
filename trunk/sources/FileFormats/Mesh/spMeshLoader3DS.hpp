/*
 * Mesh loader 3DS header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHLOADER_3DS_H__
#define __SP_MESHLOADER_3DS_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_3DS


#include "Base/spInputOutput.hpp"
#include "Base/spDimension.hpp"
#include "FileFormats/Mesh/spMeshLoader.hpp"

#include <vector>


namespace sp
{
namespace scene
{


class SP_EXPORT MeshLoader3DS : public MeshLoader
{
    
    public:
        
        MeshLoader3DS();
        ~MeshLoader3DS();
        
        Mesh* loadMesh(const io::stringc &Filename, const io::stringc &TexturePath);
        
    private:
        
        /* === Enumerations === */
        
        enum EMesh3DSChunks
        {
            CHUNK_MAGICNUMBER       = 0x4D4D,
                CHUNK_VERSION           = 0x0002,
                
                CHUNK_COLOR_F32         = 0x0010,
                CHUNK_COLOR_U8          = 0x0011,
                
                CHUNK_EDIT              = 0x3D3D,
                    CHUNK_EDIT_OBJECT       = 0x4000,
                        CHUNK_OBJECT_MESH       = 0x4100,
                            CHUNK_MESH_VERTICES     = 0x4110,
                            CHUNK_MESH_TRIANGLES    = 0x4120,
                                CHUNK_MESH_MATERIAL     = 0x4130,
                            CHUNK_MESH_TEXCOORDS    = 0x4140,
                            CHUNK_MESH_MATRIX       = 0x4160,
                    
                    CHUNK_EDIT_MATERIAL      = 0xAFFF,
                        CHUNK_MATERIAL_NAME         = 0xA000,
                        CHUNK_MATERIAL_DIFFUSE      = 0xA020,
                        CHUNK_MATERIAL_SHADING      = 0xA100,
                        CHUNK_MATERIAL_COLORMAP     = 0xA200,
                            CHUNK_TEXTURE_FILE          = 0xA300,
                
                CHUNK_KEYFRAME          = 0xB000,
                    CHUNK_KEYFRAME_CURTIME  = 0xB009,
                    CHUNK_KEYFRAME_TRACK    = 0xB002,
                        CHUNK_TRACK_BONENAME    = 0xB010,
                        CHUNK_TRACK_PIVOTPOINT  = 0xB013,
                        CHUNK_TRACK_BOUNDBOX    = 0xB014,
                        CHUNK_TRACK_BONEPOS     = 0xB020,
                        CHUNK_TRACK_BONEROT     = 0xB021,
                        CHUNK_TRACK_BONESCL     = 0xB022,
                        CHUNK_TRACK_NODE_ID     = 0xB030,
        };
        
        /* === Structures === */
        
        struct SChunk3DS
        {
            u16 ID;
            u32 Length, Readed;
        };
        
        struct SMaterial3DS
        {
            io::stringc Name;
            io::stringc TextureFilename;
            video::color Diffuse;
        };
        
        struct SMaterialGroup3DS
        {
            io::stringc Name;
            std::vector<u16> TriangleEnum;
        };
        
        struct SObjectGroup3DS
        {
            Mesh* Object;
            dim::matrix4f Transformation;
            
            std::vector<dim::vector3df> VertexList;
            std::vector<dim::point2df> TexCoordList;
            std::vector<SMeshTriangle3D> TriangleList;
            
            std::vector<SMaterialGroup3DS> MaterialGroupList;
        };
        
        struct SJoint3DS
        {
            s16 NodeID;
            io::stringc Name;
            s16 ParentJointID;
            Mesh* Object;
            std::vector<dim::vector3df> PositionList;
            std::vector<dim::quaternion> RotationList;
            std::vector<dim::vector3df> ScaleList;
        };
        
        /* === Templates === */
        
        template <typename T> T readValue()
        {
            CurChunk_->Readed += sizeof(T);
            return File_->readValue<T>();
        }
        template <typename T> void readStream(T* Buffer, u32 Count)
        {
            CurChunk_->Readed += sizeof(T) * Count;
            File_->readBuffer(Buffer, sizeof(T), Count);
        }
        
        /* === Functions === */
        
        void readChunk(SChunk3DS* Chunk);
        void readChunk();
        void ignore(u32 ByteCount);
        
        io::stringc readString();
        video::color readColor();
        
        bool readHeader();
        bool readNextChunk(SChunk3DS* PrevChunk);
        
        bool readMeshVertices();
        bool readMeshTriangles();
        bool readMeshMaterial();
        bool readMeshTexCoords();
        bool readMeshMatrix();
        
        bool readTrackPosition();
        bool readTrackRotation();
        bool readTrackScale();
        
        void addNewMesh();
        
        void buildMesh(const SObjectGroup3DS &ObjectGroup);
        
        /* === Members === */
        
        SChunk3DS* CurChunk_;
        
        Mesh* RootMesh_;
        
        std::vector<SObjectGroup3DS> ObjectGroupList_;
        SObjectGroup3DS* CurObjGroup_;
        
        std::vector<SJoint3DS> JointList_;
        SJoint3DS* CurJoint_;
        
        std::vector<SMaterial3DS> MaterialList_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
