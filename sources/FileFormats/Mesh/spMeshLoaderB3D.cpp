/*
 * Mesh loader B3D file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Mesh/spMeshLoaderB3D.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_B3D


#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace scene
{


MeshLoaderB3D::MeshLoaderB3D() :
    MeshLoader          (       ),
    Stack_              (1      ),
    CurPos_             (0      ),
    CurBone_            (-1     ),
    CurBrushID_         (0      ),
    AnimKeyframeCount_  (0      ),
    AnimFPS_            (1000.0f)
{
    Stack_[0] = 0;
}
MeshLoaderB3D::~MeshLoaderB3D()
{
    MemoryManager::deleteList(AnimBoneList_);
}

Mesh* MeshLoaderB3D::loadMesh(const io::stringc &Filename, const io::stringc &TexturePath)
{
    if (!openLoadFile(Filename, TexturePath))
        return Mesh_;
    
    if (!loadModelData())
        io::Log::error("Loading B3D mesh failed");
    
    return Mesh_;
}


/*
 * ======= Private: =======
 */

io::stringc MeshLoaderB3D::readChunk()
{
    // Read chunk name
    c8 ChunkName[5] = { 0 };
    File_->readBuffer(ChunkName, 4);
    
    // Read chunk size
    s32 Size = File_->readValue<s32>();
    
    ++CurPos_;
    
    if (CurPos_ >= static_cast<s32>(Stack_.size()))
        Stack_.resize(CurPos_ + 1);
    
    Stack_[CurPos_] = File_->getSeek() + Size;
    
    // Returns chunk name
    return io::stringc(ChunkName).upper();
}

void MeshLoaderB3D::breakChunk()
{
    File_->setSeek(Stack_[CurPos_]);
    --CurPos_;
}

s32 MeshLoaderB3D::getChunkSize()
{
    return Stack_[CurPos_] - File_->getSeek();
}

void MeshLoaderB3D::readChunkBlock(io::stringc Tab)
{
    // Temporary variables
    io::stringc ChunkName;
    Tab += io::stringc(' ');
    
    // Loop while there are chunk data
    while (getChunkSize())
    {
        ChunkName = readChunk();
        
        if (ChunkName == "ANIM") // Animation (general information)
            readChunkANIM();
        else if (ChunkName == "KEYS") // Keyframes (for bone information)
            readChunkKEYS();
        else if (ChunkName == "TEXS") // Textures
            readChunkTEXS();
        else if (ChunkName == "BRUS") // Brushes
            readChunkBRUS();
        else if (ChunkName == "VRTS") // Vertices
            readChunkVRTS();
        else if (ChunkName == "TRIS") // Triangles
            readChunkTRIS();
        else if (ChunkName == "MESH") // Mesh (texture information)
            readChunkMESH();
        else if (ChunkName == "BONE") // Bone (for skeletal animation)
            readChunkBONE(Tab);
        else if (ChunkName == "NODE") // Node (new mesh or new bone)
            readChunkNODE();
        else
            break;
        
        readChunkBlock(Tab);
        breakChunk();
    }
}


/*
 * ======= Chunk functions =======
 */

bool MeshLoaderB3D::readChunkANIM()
{
    /*s32 Flags           = */File_->readValue<s32>();
    AnimKeyframeCount_  = File_->readValue<s32>();
    AnimFPS_            = File_->readValue<f32>();
    
    if (AnimFPS_ == 0.0f)
        AnimFPS_ = 1000.0f / 60.0f;
    
    return true;
}

bool MeshLoaderB3D::readChunkKEYS()
{
    // Get current skeletal bone
    if (CurBone_ < 0 || CurBone_ >= static_cast<s32>(AnimBoneList_.size()))
    {
        io::Log::error("Animation bone array index (" + io::stringc(CurBone_) + ") out of bounds");
        breakChunk();
        return false;
    }
    
    SSkeletonBoneB3D* AnimBone = AnimBoneList_[CurBone_];
    
    // Temporary variables
    s32 Size = 4;
    
    SSkeletonBoneB3D::SBoneKeyframeInfo* Keyframe;
    
    // Read flags and get chunk size
    const s32 Flags = File_->readValue<s32>();
    
    if (Flags & 1) Size += 12;
    if (Flags & 2) Size += 12;
    if (Flags & 4) Size += 16;
    
    const s32 KeysCount = getChunkSize() / Size;
    
    if (KeysCount*Size == getChunkSize())
    {
        while (getChunkSize())
        {
            s32 Frame = File_->readValue<s32>() - 1;
            
            if (Frame < 0 || Frame >= AnimKeyframeCount_)
            {
                io::Log::error("Keyframe index out of bounds");
                breakChunk();
                break;
            }
            
            Keyframe = &(AnimBone->KeyframeList[static_cast<u32>(Frame)]);
            
            Keyframe->Frame = static_cast<u32>(Frame);
            
            /* === Keyframe === */
            
            // Keyframe - position
            if (Flags & 1)
            {
                Keyframe->Position.X = File_->readValue<f32>();
                Keyframe->Position.Y = File_->readValue<f32>();
                Keyframe->Position.Z = File_->readValue<f32>();
            }
            
            // Keyframe - scale
            if (Flags & 2)
            {
                Keyframe->Scale.X = File_->readValue<f32>();
                Keyframe->Scale.Y = File_->readValue<f32>();
                Keyframe->Scale.Z = File_->readValue<f32>();
            }
            
            // Keyframe - rotation
            if (Flags & 4)
            {
                Keyframe->Rotation.W = File_->readValue<f32>();
                Keyframe->Rotation.X = File_->readValue<f32>();
                Keyframe->Rotation.Y = File_->readValue<f32>();
                Keyframe->Rotation.Z = File_->readValue<f32>();
            }
        } // /while
    }
    else
    {
        io::Log::warning("Illegal number of keyframes in B3D model");
        return false;
    }
    
    return true;
}

bool MeshLoaderB3D::readChunkTEXS()
{
    // Temporary variables
    io::stringc TexFilename;
    s32 Flags, Blend;
    f32 Rotation;
    dim::point2df Pos;
    dim::point2df Scale;
    
    while (getChunkSize())
    {
        // Reading
        TexFilename = File_->readStringC();
        Flags       = File_->readValue<s32>();
        Blend       = File_->readValue<s32>();
        Pos.X       = File_->readValue<f32>();
        Pos.Y       = File_->readValue<f32>();
        Scale.X     = File_->readValue<f32>();
        Scale.Y     = File_->readValue<f32>();
        Rotation    = File_->readValue<f32>();
        
        // Loading
        STextureSurfaceB3D TextureSurfaceData;
        {
            // General settings
            TextureSurfaceData.hTexture         = (SceneGraph::getTextureLoadingState() ? loadChunkTexture(TexFilename) : 0);
            TextureSurfaceData.Pos              = Pos;
            TextureSurfaceData.Scale            = Scale;
            TextureSurfaceData.isSphereMapping  = (Flags & 64) ? true : false;
            
            /*
            Texture flags:
            1: Color
            2: Alpha
            4: Masked
            8: Mipmapped
            16: Clamp U
            32: Clamp V
            64: Spherical reflection map
            */
            // Flags
            if (Flags & 2) // Alpha
                TextureSurfaceData.hTexture->setColorKeyAlpha();
            else if (Flags & 4) // Masked
                TextureSurfaceData.hTexture->setColorKey(video::emptycolor);
        }
        TextureList_.push_back(TextureSurfaceData);
    }
    
    return true;
}

bool MeshLoaderB3D::readChunkBRUS()
{
    // Temporary variables
    io::stringc Name;
    f32 Red, Green, Blue, Alpha, Shininess;
    s32 Blend, Effects, TexID;
    s32 TexCount = File_->readValue<s32>();
    
    while (getChunkSize())
    {
        Name        = File_->readStringC();
        Red         = File_->readValue<f32>();
        Green       = File_->readValue<f32>();
        Blue        = File_->readValue<f32>();
        Alpha       = File_->readValue<f32>();
        Shininess   = File_->readValue<f32>();
        Blend       = File_->readValue<s32>();
        Effects     = File_->readValue<s32>();
        
        for (s32 i = 0; i < TexCount; ++i)
        {
            TexID = File_->readValue<s32>();
            BrushTextureList_.push_back(TexID);
        }
    }
    
    return true;
}

bool MeshLoaderB3D::readChunkVRTS()
{
    s32 Flags = File_->readValue<s32>();
    s32 tcSets = File_->readValue<s32>();
    s32 tcSize = File_->readValue<s32>();
    
    // Temporary variables
    s32 Size = 12 + tcSets*tcSize*4;
    f32 TexCoord[3] = { 0.0f, 0.0f, 0.0f };
    
    SVertexB3D Vertex;
    
    if (Flags & 1) Size += 12;
    if (Flags & 2) Size += 16;
    
    s32 VerticesCount = getChunkSize() / Size;
    
    if (VerticesCount*Size == getChunkSize())
    {
        
        while (getChunkSize())
        {
            // Standard values
            Vertex.Normal   = dim::vector3df(0, 0, 1);
            Vertex.Color    = video::color(255);
            
            // Reading
            Vertex.Coord.X  = File_->readValue<f32>();
            Vertex.Coord.Y  = File_->readValue<f32>();
            Vertex.Coord.Z  = File_->readValue<f32>();
            
            if (Flags & 1)
            {
                Vertex.Normal.X = File_->readValue<f32>();
                Vertex.Normal.Y = File_->readValue<f32>();
                Vertex.Normal.Z = File_->readValue<f32>();
            }
            if (Flags & 2)
            {
                Vertex.Color.Red    = static_cast<s32>(File_->readValue<f32>() * 255);
                Vertex.Color.Green  = static_cast<s32>(File_->readValue<f32>() * 255);
                Vertex.Color.Blue   = static_cast<s32>(File_->readValue<f32>() * 255);
                Vertex.Color.Alpha  = static_cast<s32>(File_->readValue<f32>() * 255);
            }
            for (s32 j = 1; j <= tcSets*tcSize; ++j)
                TexCoord[j-1] = File_->readValue<f32>();
            
            Vertex.TexCoord.X   = TexCoord[0];
            Vertex.TexCoord.Y   = TexCoord[1];
            
            Vertex.SurfVertexID = VerticesList_.size();
            
            // Add new vertex
            VerticesList_.push_back(Vertex);
        } // wend
        
    }
    else
    {
        io::Log::warning("Illegal number of vertices in B3D model");
        return false;
    }
    
    return true;
}

bool MeshLoaderB3D::readChunkTRIS()
{
    // Temporary vairables
    s32 BrushID = File_->readValue<s32>();
    s32 Size = 12;
    s32 TriangleCount = getChunkSize() / Size;
    s32 v0, v1, v2;
    
    std::list<s32> VertexIndices, VertexIndicesUnique;
    
    if (BrushID == -1)
        BrushID = CurBrushID_;
    
    if (BrushID >= 0 && BrushID < static_cast<s32>(BrushTextureList_.size()))
        BrushID = BrushTextureList_[BrushID];
    
    if (TriangleCount*Size == getChunkSize())
    {
        
        while (getChunkSize())
        {
            // Reading
            v0 = File_->readValue<s32>();
            v1 = File_->readValue<s32>();
            v2 = File_->readValue<s32>();
            
            // Save in list
            VertexIndices.push_back(v0);
            VertexIndices.push_back(v1);
            VertexIndices.push_back(v2);
        }
        
        // Copy the list
        VertexIndicesUnique = VertexIndices;
        
        // Remove multiple elements
        VertexIndicesUnique.sort();
        VertexIndicesUnique.unique();
        
        // Create a new surface
        Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
        Surface_->setName(CurName_);
        
        SBrushSurfaceB3D BrushSurfaceData;
        {
            BrushSurfaceData.BrushID    = BrushID;
            BrushSurfaceData.Surface    = Surface_;
        }
        BrushSurfaceList_.push_back(BrushSurfaceData);
        
        SVertexB3D Vertex;
        
        // Create all vertices
        for (std::list<s32>::iterator it = VertexIndicesUnique.begin(); it != VertexIndicesUnique.end(); ++it)
        {
            Vertex = VerticesList_[*it];
            
            VerticesList_[*it].SurfVertexID = Surface_->addVertex(
                CurTransformation_ * Vertex.Coord,
                CurRotation_ * Vertex.Normal,
                Vertex.TexCoord,
                Vertex.Color
            );
            
            VerticesList_[*it].Surface      = Surface_;
            VerticesList_[*it].SurfaceNr    = Mesh_->getMeshBufferCount() - 1;
        }
        
        // Create all triangles
        for (std::list<s32>::iterator it = VertexIndices.begin(); it != VertexIndices.end();)
        {
            // Get the indices
            v0 = VerticesList_[*it++].SurfVertexID;
            v1 = VerticesList_[*it++].SurfVertexID;
            v2 = VerticesList_[*it++].SurfVertexID;
            
            // Creating
            Surface_->addTriangle(v0, v1, v2);
        }
        
    }
    else
    {
        io::Log::warning("Illegal number of triangles in B3D model");
        return false;
    }
    
    return true;
}

bool MeshLoaderB3D::readChunkMESH()
{
    // Read the current brush
    CurBrushID_ = File_->readValue<s32>();
    
    // Clear the vertices list of the last mesh
    VerticesList_.clear();
    
    return true;
}

bool MeshLoaderB3D::readChunkBONE(io::stringc &Tab)
{
    s32 Size = 8;
    s32 WeightsCount = getChunkSize() / Size;
    s32 VertexID;
    f32 Weight;
    
    if (WeightsCount*Size == getChunkSize())
    {
        
        s32 i = 0;
        
        // Temporary bone memory
        SSkeletonBoneB3D* BoneData = new SSkeletonBoneB3D();
        {
            BoneData->TabSize       = Tab.size();
            BoneData->ParentBoneID  = 0;
            BoneData->Name          = CurName_;
            
            BoneData->Translation   = Position_;
            BoneData->Quaternion    = Quaternion_;
            BoneData->Scale         = Scale_;
            
            BoneData->WeightsCount  = WeightsCount;
            BoneData->VerticesList  = new SSkeletonBoneB3D::SBoneVertexInfo[ WeightsCount ];
        }
        
        // Read the vertex identities
        while (getChunkSize())
        {
            VertexID    = File_->readValue<s32>();
            Weight      = File_->readValue<f32>();
            
            if (VertexID < 0 || VertexID >= VerticesList_.size())
            {
                io::Log::error("Corrupted vertex ID occured while reading joint vertex weights");
                return false;
            }
            
            BoneData->VerticesList[i].Surface   = VerticesList_[VertexID].SurfaceNr;
            BoneData->VerticesList[i].Index     = VerticesList_[VertexID].SurfVertexID;
            BoneData->VerticesList[i].Weight    = Weight;
            
            ++i;
        }
        
        // Add a new bone
        AnimBoneList_.push_back(BoneData);
        
        CurBone_ = AnimBoneList_.size() - 1;
        
    }
    else
    {
        io::Log::warning("Illegal number of bone weights in B3D model");
        return false;
    }
    
    return true;
}

bool MeshLoaderB3D::readChunkNODE()
{
    // Read name
    CurName_        = File_->readStringC();
    
    // Read position
    Position_.X      = File_->readValue<f32>();
    Position_.Y      = File_->readValue<f32>();
    Position_.Z      = File_->readValue<f32>();
    
    // Read scaling
    Scale_.X         = File_->readValue<f32>();
    Scale_.Y         = File_->readValue<f32>();
    Scale_.Z         = File_->readValue<f32>();
    
    // Read rotation
    Quaternion_.W    = File_->readValue<f32>();
    Quaternion_.X    = File_->readValue<f32>();
    Quaternion_.Y    = File_->readValue<f32>();
    Quaternion_.Z    = File_->readValue<f32>();
    
    // Update current transformation
    CurRotation_ = Quaternion_.getMatrixTransposed();
    
    CurTransformation_.reset();
    CurTransformation_.translate(Position_);
    CurTransformation_ *= CurRotation_;
    CurTransformation_.scale(Scale_);
    
    return true;
}

video::Texture* MeshLoaderB3D::loadChunkTexture(io::stringc Filename)
{
    if (!io::FileSystem().findFile(TexturePath_ + Filename))
    {
        const io::stringc AlternativeFilename(
            Filename.right(Filename.size() - Filename.getPathPart().size())
        );
        
        if (AlternativeFilename != Filename)
        {
            io::Log::warning(
                io::stringc("Could not find texture file: \"") + TexturePath_ + Filename +
                io::stringc("\", trying alternative: \"") + TexturePath_ + AlternativeFilename + "\""
            );
            Filename = AlternativeFilename;
        }
    }
    
    return __spVideoDriver->loadTexture(TexturePath_ + Filename);
}


/*
 * ======= Building functions =======
 */

void MeshLoaderB3D::updateTexturing()
{
    for (std::vector<SBrushSurfaceB3D>::iterator it = BrushSurfaceList_.begin(); it != BrushSurfaceList_.end(); ++it)
    {
        if (it->Surface && it->BrushID < static_cast<s32>(TextureList_.size()) && it->BrushID >= 0)
        {
            if (TextureList_[it->BrushID].hTexture)
                it->Surface->addTexture(TextureList_[it->BrushID].hTexture);
            
            it->Surface->textureTransform(0, TextureList_[it->BrushID].Scale);
            it->Surface->textureTranslate(0, TextureList_[it->BrushID].Pos);
            
            if (TextureList_[it->BrushID].isSphereMapping)
                it->Surface->setMappingGen(0, video::MAPGEN_SPHERE_MAP);
        }
    }
}

void MeshLoaderB3D::buildAnimation()
{
    /* === Check if the object has a surface and a frame === */
    
    if (!Mesh_->getMeshBufferCount() || AnimBoneList_.empty())
        return;
    
    /* === Create a new animation === */
    
    scene::SkeletalAnimation* Anim = __spSceneManager->createAnimation<scene::SkeletalAnimation>("B3D Animation");
    scene::AnimationSkeleton* Skeleton = Anim->createSkeleton();
    
    /* === Temporary variables === */
    
    std::vector<SVertexGroup> VertexGroups;
    
    /* === Loop for the frame bone list (get the correct parent bones) === */
    
    for (s32 i = 1, j; i < static_cast<s32>(AnimBoneList_.size()); ++i)
    {
        for (j = i - 1; j >= 0; --j)
        {
            if (AnimBoneList_[i]->TabSize == AnimBoneList_[j]->TabSize + 1)
            {
                AnimBoneList_[i]->ParentBoneID = j + 1;
                break;
            }
        }
    }
    
    /* Create animation joints */
    std::vector<SJointParent> JointList;
    JointList.resize(AnimBoneList_.size());
    
    u32 i = 0;
    AnimationJoint* Joint = 0;
    
    foreach (SSkeletonBoneB3D* Bone, AnimBoneList_)
    {
        /* Create new joint */
        Joint = Skeleton->createJoint(
            Transformation(Bone->Translation, Bone->Quaternion, Bone->Scale), Bone->Name
        );
        
        JointList[i].Joint      = Joint;
        JointList[i].ParentID   = Bone->ParentBoneID;
        ++i;
        
        /* Setup vertex groups */
        VertexGroups.resize(Bone->WeightsCount);
        
        if (Bone->VerticesList)
        {
            for (s32 j = 0; j < Bone->WeightsCount; ++j)
            {
                VertexGroups[j] = SVertexGroup(
                    Mesh_->getMeshBuffer(Bone->VerticesList[j].Surface),
                    Bone->VerticesList[j].Index,
                    Bone->VerticesList[j].Weight
                );
            }
            
            Joint->setVertexGroups(VertexGroups);
        }
        else
            io::Log::error("Vertex list for animation joint " + io::stringc(i) + " is invalid");
        
        /* Create joint keyframes */
        for (std::map<u32, SSkeletonBoneB3D::SBoneKeyframeInfo>::const_iterator itFrame = Bone->KeyframeList.begin();
             itFrame != Bone->KeyframeList.end(); ++itFrame)
        {
            Anim->addKeyframe(
                Joint,
                Transformation(itFrame->second.Position, itFrame->second.Rotation, itFrame->second.Scale),
                itFrame->second.Frame
            );
        }
    }
    
    /* Setup parent joints */
    foreach (const SJointParent &JntParent, JointList)
    {
        if (JntParent.ParentID > 0)
            Skeleton->setJointParent(JntParent.Joint, JointList[JntParent.ParentID - 1].Joint);
    }
    
    /* Update the skeleton bones */
    Skeleton->updateSkeleton();
    
    Mesh_->addAnimation(Anim);
}

bool MeshLoaderB3D::loadModelData()
{
    // Identity
    if (readChunk() != "BB3D")
    {
        io::Log::error("B3D model has invalid identity");
        return false;
    }
    
    // Version
    if (File_->readValue<s32>() / 100 > 0)
    {
        io::Log::error("B3D model has invalid version");
        return false;
    }
    
    // Read all chunks
    readChunkBlock();
    
    // Update texturing
    updateTexturing();
    
    // Update modeling
    Mesh_->updateIndexBuffer();
    Mesh_->updateNormals();
    
    // Create animation
    buildAnimation();
    
    return true;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
