/*
 * Mesh loader 3DS file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Mesh/spMeshLoader3DS.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_3DS


#include "Base/spSharedObjects.hpp"
#include "SceneGraph/spSceneManager.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;
extern scene::SceneGraph* GlbSceneGraph;

namespace scene
{


MeshLoader3DS::MeshLoader3DS() :
    MeshLoader  (               ),
    CurChunk_   (new SChunk3DS()),
    RootMesh_   (0              ),
    CurObjGroup_(0              ),
    CurJoint_   (0              )
{
}
MeshLoader3DS::~MeshLoader3DS()
{
    MemoryManager::deleteMemory(CurChunk_);
}

Mesh* MeshLoader3DS::loadMesh(const io::stringc &Filename, const io::stringc &TexturePath)
{
    if (!openLoadFile(Filename, TexturePath))
        return Mesh_;
    
    RootMesh_ = Mesh_;
    
    if (!readHeader() || !readNextChunk(CurChunk_))
    {
        io::Log::error("Loading 3DS mesh failed");
        return Mesh_;
    }
    
    foreach (SObjectGroup3DS &Group, ObjectGroupList_)
        buildMesh(Group);
    
    return RootMesh_;
}


/*
 * ========== Private: ==========
 */

void MeshLoader3DS::readChunk(SChunk3DS* Chunk)
{
    Chunk->ID       = File_->readValue<u16>();
    Chunk->Length   = File_->readValue<u32>();
    Chunk->Readed   = 6;
}

void MeshLoader3DS::readChunk()
{
    readChunk(CurChunk_);
}

void MeshLoader3DS::ignore(u32 ByteCount)
{
    File_->setSeek(ByteCount, io::FILEPOS_CURRENT);
    CurChunk_->Readed += ByteCount;
}

io::stringc MeshLoader3DS::readString()
{
    const io::stringc Str = File_->readStringC();
    CurChunk_->Readed += Str.size() + 1;
    return Str;
}

video::color MeshLoader3DS::readColor()
{
    SChunk3DS TmpChunk;
    readChunk(&TmpChunk);
    
    CurChunk_->Readed += TmpChunk.Readed;
    const u32 BufferSize = TmpChunk.Length - TmpChunk.Readed;
    video::color Color;
    
    switch (TmpChunk.ID)
    {
        case CHUNK_COLOR_F32:
            Color.Red   = static_cast<u8>(readValue<f32>() * 255);
            Color.Green = static_cast<u8>(readValue<f32>() * 255);
            Color.Blue  = static_cast<u8>(readValue<f32>() * 255);
            break;
            
        case CHUNK_COLOR_U8:
            Color.Red   = readValue<u8>();
            Color.Green = readValue<u8>();
            Color.Blue  = readValue<u8>();
            break;
            
        default:
            File_->setSeek(BufferSize, io::FILEPOS_CURRENT);
            CurChunk_->Readed += BufferSize;
            break;
    }
    
    return Color;
}


bool MeshLoader3DS::readHeader()
{
    readChunk();
    
    if (CurChunk_->ID != CHUNK_MAGICNUMBER)
    {
        io::Log::error("Wrong magic number in 3DS file");
        return false;
    }
    
    return true;
}

bool MeshLoader3DS::readNextChunk(SChunk3DS* PrevChunk)
{
    // Create a new chunk to avoid overloaded recursive function calls
    CurChunk_ = new SChunk3DS;
    
    // Read the file until the previous chunk has end
    while (PrevChunk->Readed < PrevChunk->Length)
    {
        readChunk();
        
        switch (CurChunk_->ID)
        {
            case CHUNK_VERSION:
            {
                const s16 Version = readValue<s32>();
                
                if (Version > 3)
                    io::Log::warning("Version number of 3DS mesh is over 3 so it may load incorrect");
            }
            break;
            
            case CHUNK_EDIT:
            {
                if (!readNextChunk(CurChunk_))
                    return false;
            }
            break;
            
            case CHUNK_EDIT_OBJECT:
            {
                // Add a new object to the object group list
                ObjectGroupList_.resize(ObjectGroupList_.size() + 1);
                CurObjGroup_ = &(ObjectGroupList_.back());
                
                // Create a new mesh object
                if (ObjectGroupList_.size() > 1)
                {
                    Mesh_ = gSharedObjects.SceneMngr->createMesh();
                    Mesh_->setParent(RootMesh_);
                    RootMesh_->addChild(Mesh_);
                }
                
                CurObjGroup_->Object = Mesh_;
                
                // Read the object's name
                Mesh_->setName(readString());
                
                if (!readNextChunk(CurChunk_))
                    return false;
            }
            break;
            
            case CHUNK_OBJECT_MESH:
            {
                if (!readNextChunk(CurChunk_))
                    return false;
            }
            break;
            
            case CHUNK_EDIT_MATERIAL:
            {
                MaterialList_.resize(MaterialList_.size() + 1);
                
                if (!readNextChunk(CurChunk_))
                    return false;
            }
            break;
            
            case CHUNK_MATERIAL_NAME:
            {
                if (MaterialList_.empty())
                    return false;
                
                MaterialList_.back().Name = readString();
            }
            break;
            
            case CHUNK_MATERIAL_DIFFUSE:
            {
                if (MaterialList_.empty())
                    return false;
                
                MaterialList_.back().Diffuse = readColor();
            }
            break;
            
            case CHUNK_MATERIAL_COLORMAP:
            {
                if (!readNextChunk(CurChunk_))
                    return false;
            }
            break;
            
            case CHUNK_TEXTURE_FILE:
            {
                if (MaterialList_.empty())
                    return false;
                
                MaterialList_.back().TextureFilename = readString();
            }
            break;
            
            case CHUNK_KEYFRAME:
            {
                if (!readNextChunk(CurChunk_))
                    return false;
            }
            break;
            
            case CHUNK_KEYFRAME_TRACK:
            {
                // Create a new bone
                JointList_.resize(JointList_.size() + 1);
                CurJoint_ = &(JointList_.back());
                
                // Default settings
                CurJoint_->Object           = 0;
                CurJoint_->ParentJointID    = -1;
                
                if (!readNextChunk(CurChunk_))
                    return false;
            }
            break;
            
            case CHUNK_TRACK_BONENAME:
            {
                if (!CurJoint_)
                    return false;
                
                CurJoint_->Name = readString();
                
                ignore(2*sizeof(s16));
                
                CurJoint_->ParentJointID = readValue<s16>();
            }
            break;
            
            case CHUNK_TRACK_PIVOTPOINT:
            {
                if (!CurJoint_)
                    return false;
                
                //CurJoint_->NodeID = readValue<s16>();
                dim::vector3df Point;
                
                Point.X = readValue<f32>();
                Point.Y = readValue<f32>();
                Point.Z = readValue<f32>();
            }
            break;
            
            case CHUNK_TRACK_NODE_ID:
            {
                if (!CurJoint_)
                    return false;
                
                CurJoint_->NodeID = readValue<s16>();
            }
            break;
            
            case CHUNK_TRACK_BONEPOS:
            {
                if (!readTrackPosition())
                    return false;
            }
            break;
            
            case CHUNK_TRACK_BONEROT:
            {
                if (!readTrackRotation())
                    return false;
            }
            break;
            
            case CHUNK_TRACK_BONESCL:
            {
                if (!readTrackScale())
                    return false;
            }
            break;
            
            case CHUNK_MESH_VERTICES:
            {
                if (!readMeshVertices())
                    return false;
            };
            break;
            
            case CHUNK_MESH_TRIANGLES:
            {
                if (!readMeshTriangles())
                    return false;
                if (!readNextChunk(CurChunk_))
                    return false;
            };
            break;
            
            case CHUNK_MESH_MATERIAL:
            {
                if (!readMeshMaterial())
                    return false;
            };
            break;
            
            case CHUNK_MESH_TEXCOORDS:
            {
                if (!readMeshTexCoords())
                    return false;
            };
            break;
            
            case CHUNK_MESH_MATRIX:
            {
                if (!readMeshMatrix())
                    return false;
            };
            break;
        }
        
        // Seek until the end of this chunk
        File_->setSeek(CurChunk_->Length - CurChunk_->Readed, io::FILEPOS_CURRENT);
        PrevChunk->Readed += CurChunk_->Length;
    }
    
    // Delete the current chunk and use the previous one
    delete CurChunk_;
    CurChunk_ = PrevChunk;
    
    return true;
}


bool MeshLoader3DS::readMeshVertices()
{
    if (!CurObjGroup_)
        return false;
    
    const u32 VertexCount = (u32)readValue<u16>();
    dim::vector3df Coord;
    CurObjGroup_->VertexList.resize(VertexCount);
    
    for (u32 i = 0; i < VertexCount; ++i)
    {
        // Read the vertex coordinate
        Coord.X = readValue<f32>();
        Coord.Z = readValue<f32>();
        Coord.Y = readValue<f32>();
        
        CurObjGroup_->VertexList[i] = Coord;
    }
    
    return true;
}

bool MeshLoader3DS::readMeshTriangles()
{
    if (!CurObjGroup_)
        return false;
    
    const u32 TriangleCount = (u32)readValue<u16>();
    SMeshTriangle3D Triangle;
    CurObjGroup_->TriangleList.resize(TriangleCount);
    
    for (u32 i = 0; i < TriangleCount; ++i)
    {
        // Read the indices
        Triangle.a = (u32)readValue<u16>();
        Triangle.b = (u32)readValue<u16>();
        Triangle.c = (u32)readValue<u16>();
        
        // Read but ignore the flags
        ignore(sizeof(s16));
        
        CurObjGroup_->TriangleList[i] = Triangle;
    }
    
    return true;
}

bool MeshLoader3DS::readMeshMaterial()
{
    if (!CurObjGroup_)
        return false;
    
    SMaterialGroup3DS MatGroup;
    
    MatGroup.Name = readString();
    
    const u16 TriangleCount = readValue<u16>();
    MatGroup.TriangleEnum.resize(TriangleCount);
    
    for (u16 i = 0; i < TriangleCount; ++i)
        MatGroup.TriangleEnum[i] = readValue<u16>();
    
    CurObjGroup_->MaterialGroupList.push_back(MatGroup);
    
    return true;
}

bool MeshLoader3DS::readMeshTexCoords()
{
    if (!CurObjGroup_)
        return false;
    
    const u32 VertexCount = (u32)readValue<u16>();
    dim::point2df TexCoord;
    CurObjGroup_->TexCoordList.resize(VertexCount);
    
    for (u32 i = 0; i < VertexCount; ++i)
    {
        // Read the vertex coordinate
        TexCoord.X = readValue<f32>();
        TexCoord.Y = -readValue<f32>();
        
        CurObjGroup_->TexCoordList[i] = TexCoord;
    }
    
    return true;
}

bool MeshLoader3DS::readMeshMatrix()
{
    if (!CurObjGroup_)
        return false;
    
    f32 Matrix[4][3];
    
    for (s32 i = 0, j; i < 4; ++i)
    {
        for (j = 0; j < 3; ++j)
            Matrix[i][j] = readValue<f32>();
    }
    
    CurObjGroup_->Transformation = dim::matrix4f(
        Matrix[0][0], Matrix[1][0], Matrix[2][0], Matrix[3][0],
        Matrix[0][1], Matrix[1][1], Matrix[2][1], Matrix[3][1],
        Matrix[0][2], Matrix[1][2], Matrix[2][2], Matrix[3][2],
        0, 0, 0, 1
    );
    
    return true;
}


bool MeshLoader3DS::readTrackPosition()
{
    if (!CurJoint_)
        return false;
    
    ignore(5*sizeof(s16));
    
    const s16 KeyCount = readValue<s16>();
    CurJoint_->PositionList.resize((u32)KeyCount);
    
    ignore(sizeof(s16));
    
    s16 FrameEnum;
    dim::vector3df Position;
    
    for (s16 i = 0; i < KeyCount; ++i)
    {
        FrameEnum = readValue<s16>();
        
        ignore(sizeof(long));
        
        Position.X = readValue<f32>();
        Position.Z = readValue<f32>();
        Position.Y = readValue<f32>();
        
        //CurJoint_->PositionList[i] = Position;
    }
    
    return true;
}

bool MeshLoader3DS::readTrackRotation()
{
    if (!CurJoint_)
        return false;
    
    ignore(5*sizeof(s16));
    
    const s16 KeyCount = readValue<s16>();
    CurJoint_->RotationList.resize((u32)KeyCount);
    
    ignore(sizeof(s16));
    
    s16 FrameEnum;
    f32 RotRad;
    dim::vector3df RotVec;
    dim::quaternion Rotation;
    dim::matrix4f Matrix;
    
    for (s16 i = 0; i < KeyCount; ++i)
    {
        FrameEnum = readValue<s16>();
        
        ignore(sizeof(long));
        
        RotRad      = readValue<f32>();
        RotVec.X    = readValue<f32>();
        RotVec.Y    = readValue<f32>();
        RotVec.Z    = readValue<f32>();
        
        Rotation.setAngleAxis(RotRad, RotVec);
        
        CurJoint_->RotationList[i] = Rotation;
    }
    
    return true;
}

bool MeshLoader3DS::readTrackScale()
{
    if (!CurJoint_)
        return false;
    
    ignore(5*sizeof(s16));
    
    const s16 KeyCount = readValue<s16>();
    CurJoint_->ScaleList.resize((u32)KeyCount);
    
    ignore(sizeof(s16));
    
    s16 FrameEnum;
    dim::vector3df Scale;
    
    for (s16 i = 0; i < KeyCount; ++i)
    {
        FrameEnum = readValue<s16>();
        
        ignore(sizeof(long));
        
        Scale.X = readValue<f32>();
        Scale.Y = readValue<f32>();
        Scale.Z = readValue<f32>();
        
        CurJoint_->ScaleList[i] = Scale;
    }
    
    return true;
}


void MeshLoader3DS::buildMesh(const SObjectGroup3DS &ObjectGroup)
{
    u32 i, v;
    
    if (!ObjectGroup.MaterialGroupList.empty())
    {
        // Create each surface out of the materail group
        for (std::vector<SMaterialGroup3DS>::const_iterator itGroup = ObjectGroup.MaterialGroupList.begin();
             itGroup != ObjectGroup.MaterialGroupList.end(); ++itGroup)
        {
            // Create a new surface
            Surface_ = ObjectGroup.Object->createMeshBuffer(
                SceneManager::getDefaultVertexFormat(), SceneManager::getDefaultIndexFormat()
            );
            Surface_->setName(itGroup->Name);
            
            // Create each triangle with its three vertices
            for (std::vector<u16>::const_iterator it = itGroup->TriangleEnum.begin(); it != itGroup->TriangleEnum.end(); ++it)
            {
                // Add the three vertices
                for (i = 0; i < 3; ++i)
                {
                    if (*it < ObjectGroup.TriangleList.size())
                    {
                        v = (ObjectGroup.TriangleList[*it])[i];
                        
                        if (v < ObjectGroup.VertexList.size())
                        {
                            if (v < ObjectGroup.TexCoordList.size())
                                Surface_->addVertex(ObjectGroup.VertexList[v], ObjectGroup.TexCoordList[v]);
                            else
                                Surface_->addVertex(ObjectGroup.VertexList[v]);
                        }
                    }
                }
                
                // Add the triangle
                Surface_->addTriangle(2, 1, 0);
                Surface_->addIndexOffset(3);
            }
            
            // Load the material texture
            if (SceneManager::getTextureLoadingState())
            {
                for (std::vector<SMaterial3DS>::const_iterator it = MaterialList_.begin(); it != MaterialList_.end(); ++it)
                {
                    if (it->Name == itGroup->Name)
                    {
                        // Load the material texture
                        Surface_->addTexture(GlbRenderSys->loadTexture(TexturePath_ + it->TextureFilename));
                        
                        // Set the diffuse color
                        ObjectGroup.Object->getMaterial()->setDiffuseColor(it->Diffuse);
                        break;
                    } // fi
                } // next material
            } // fi
        } // next material group
    }
    else if (!ObjectGroup.TriangleList.empty())
    {
        // Create a new surface
        Surface_ = ObjectGroup.Object->createMeshBuffer(
            SceneManager::getDefaultVertexFormat(), SceneManager::getDefaultIndexFormat()
        );
        
        // Create each triangle with its three vertices
        for (std::vector<SMeshTriangle3D>::const_iterator it = ObjectGroup.TriangleList.begin();
             it != ObjectGroup.TriangleList.end(); ++it)
        {
            // Add the three vertices
            for (i = 0; i < 3; ++i)
            {
                v = (*it)[i];
                
                if (v < ObjectGroup.VertexList.size())
                {
                    if (v < ObjectGroup.TexCoordList.size())
                        Surface_->addVertex(ObjectGroup.VertexList[v], ObjectGroup.TexCoordList[v]);
                    else
                        Surface_->addVertex(ObjectGroup.VertexList[v]);
                }
            }
            
            // Add the triangle
            Surface_->addTriangle(2, 1, 0);
            Surface_->addIndexOffset(3);
        }
    }
    
    // Update normals and finish mesh buiding
    ObjectGroup.Object->updateMeshBuffer();
    ObjectGroup.Object->updateNormals();
    
    // Search a joint which belongs to the current object
    SJoint3DS* Joint = 0;
    
    for (std::vector<SJoint3DS>::iterator it = JointList_.begin(); it != JointList_.end(); ++it)
    {
        if (it->Name == ObjectGroup.Object->getName())
        {
            Joint = &(*it);
            break;
        }
    }
    
    if (Joint)
    {
        Joint->Object = ObjectGroup.Object;
        
        // Set the parent fot the object
        if (Joint->ParentJointID >= 0 && Joint->ParentJointID < static_cast<s16>(JointList_.size()))
        {
            SJoint3DS* ParentJoint = &(JointList_[Joint->ParentJointID]);
            
            if (ParentJoint && ParentJoint != Joint && ParentJoint->Object)
                Joint->Object->setParent(ParentJoint->Object);
        }
        
        // Build the animation for the current joint
        NodeAnimation* Anim = gSharedObjects.SceneMngr->createAnimation<NodeAnimation>("3DS Animation");
        
        const u32 MaxCount = math::Max(
            Joint->PositionList.size(), Joint->RotationList.size(), Joint->ScaleList.size()
        );
        
        dim::vector3df Position, Scale(1.0f);
        dim::quaternion Rotation;
        
        for (u32 i = 0; i < MaxCount; ++i)
        {
            if (i < Joint->PositionList.size())
                Position = Joint->PositionList[i];
            if (i < Joint->RotationList.size())
                Rotation = Joint->RotationList[i];
            if (i < Joint->ScaleList.size())
                Scale = Joint->ScaleList[i];
            
            Anim->addKeyframe(Transformation(Position, Rotation, Scale));
        }
        
        ObjectGroup.Object->addAnimation(Anim);
    }
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
