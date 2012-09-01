/*
 * Mesh loader MS3D file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Mesh/spMeshLoaderMS3D.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_MS3D


#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace scene
{


/*
 * Internal structures
 */

// Packed structures

#if defined(_MSC_VER)
#   pragma pack(push, packing)
#   pragma pack(1)
#   define SP_PACK_STRUCT
#elif defined(__GNUC__)
#   define SP_PACK_STRUCT __attribute__((packed))
#else
#   define SP_PACK_STRUCT
#endif

// File header
struct SHeaderDataMS3D
{
    c8 ID[10];
    s32 Version;
}
SP_PACK_STRUCT;

// Vertex information
struct SVertexDataMS3D
{
    u8 Flags;
    f32 Position[3];
    s8 BoneID;
    u8 RefCount;
}
SP_PACK_STRUCT;

// Triangle information
struct STriangleDataMS3D
{
    u16 Flags;
    u16 VertexIndices[3];
    f32 VertexNormals[3][3];
    f32 TexCoordS[3], TexCoordT[3];
    u8 SmoothingGroup;
    u8 GroupIndex;
}
SP_PACK_STRUCT;

// Material information
struct SMaterialDataMS3D
{
    c8 Name[32];
    f32 Ambient[4];
    f32 Diffuse[4];
    f32 Specular[4];
    f32 Emissive[4];
    f32 Shininess;      // 0.0f - 128.0f
    f32 Transparency;   // 0.0f - 1.0f
    u8 Mode;            // 0, 1, 2 is unused now
    c8 Texture[128];
    c8 Alphamap[128];
}
SP_PACK_STRUCT;

//    Joint information
struct SJointDataMS3D
{
    u8 Flags;
    c8 Name[32];
    c8 ParentName[32];
    f32 Rotation[3];
    f32 Translation[3];
    u16 CountOfRotations;
    u16 CountOfTranslations;
}
SP_PACK_STRUCT;

// Keyframe data
struct SKeyframeDataMS3D
{
    f32 Time;
    f32 Vector[3]; // Rotation/ Translation
}
SP_PACK_STRUCT;

struct SVertexWeightsDataMS3D
{
    s8 BoneID[3];
    u8 Weights[3];
}
SP_PACK_STRUCT;

// Default alignment
#ifdef _MSC_VER
#   pragma pack(pop, packing)
#endif

#undef SP_PACK_STRUCT


/*
 * MeshLoaderMS3D class
 */

MeshLoaderMS3D::MeshLoaderMS3D() :
    MeshLoader          (       ),
    CountOfMeshes_      (0      ),
    pMeshes_            (0      ),
    CountOfMaterials_   (0      ),
    pMaterials_         (0      ),
    CountOfTriangles_   (0      ),
    pTriangles_         (0      ),
    CountOfVertcies_    (0      ),
    pVertices_          (0      ),
    CountOfJoints_      (0      ),
    pJoints_            (0      ),
    pNameList_          (0      ),
    TotalTime_          (0.0    ),
    TotalFrames_        (0.0f   ),
    FPS_                (0.0f   ),
    FileSize_           (0      ),
    pBuffer_            (0      )
{
}
MeshLoaderMS3D::~MeshLoaderMS3D()
{
    /* Delete all members */
    for (s32 i = 0; i < CountOfMeshes_; ++i)
        MemoryManager::deleteBuffer(pMeshes_[i].pTriangleIndices);
    for (s32 i = 0; i < CountOfMaterials_; ++i)
        MemoryManager::deleteBuffer(pMaterials_[i].TextureFilename);
    
    MemoryManager::deleteBuffer(pMeshes_);
    MemoryManager::deleteBuffer(pMaterials_);
    MemoryManager::deleteBuffer(pTriangles_);
    MemoryManager::deleteBuffer(pVertices_);
    MemoryManager::deleteBuffer(pJoints_);
    MemoryManager::deleteBuffer(pNameList_);
    MemoryManager::deleteBuffer(pBuffer_);
}

Mesh* MeshLoaderMS3D::loadMesh(const io::stringc &Filename, const io::stringc &TexturePath)
{
    if (!openLoadFile(Filename, TexturePath))
        return Mesh_;
    
    if (!loadModelData(Filename))
    {
        io::Log::error("Loading MS3D mesh failed");
        return Mesh_;
    }
    
    return buildModel();
}


/*
 * ======= Private: =======
 */

bool MeshLoaderMS3D::loadModelData(io::stringc Filename)
{
    /* Temporary variables */
    s32 i, j;
    
    // Open the file
    if (!openForReading())
        return false;
    
    /*
     * Loading buffer
     */
    
    FileSize_ = File_->getSize();
    
    pBuffer_ = new u8[FileSize_];
    File_->readBuffer(pBuffer_, 1, FileSize_);
    
    FileSys_.closeFile(File_);
    
    const u8* pPtr = pBuffer_;
    SHeaderDataMS3D* pHeader = (SHeaderDataMS3D*)pPtr;
    
    if (setNextBufferPos(pPtr, sizeof(SHeaderDataMS3D)))
        return false;
    
    /* Check the identification */
    
    if (strncmp(pHeader->ID, "MS3D000000", 10) != 0)
    {
        io::Log::error("MS3D mesh has wrong identity (must be 'MS3D000000')");
        return false;
    }
    
    if (pHeader->Version < 3 || pHeader->Version > 4)
    {
        io::Log::error(
            "MS3D mesh has unsupported version (" + io::stringc(pHeader->Version) +
            "), only Milkshape3D 1.3 and 1.4 are supported"
        );
        return false;
    }
    
    /*
     * Reading vertices
     */
    
    s32 nVertices       = *(u16*)pPtr;
    CountOfVertcies_    = nVertices;
    pVertices_          = new SVertexMS3D[nVertices];
    
    if (setNextBufferPos(pPtr, sizeof(u16)))
        return false;
    
    for (i = 0; i < nVertices; ++i)
    {
        SVertexDataMS3D* pVertex = (SVertexDataMS3D*)pPtr;
        pVertices_[i].BoneID    = pVertex->BoneID;
        
        pVertices_[i].Position = dim::vector3df(
            pVertex->Position[0], pVertex->Position[1], -pVertex->Position[2]
        );
        
        if (setNextBufferPos(pPtr, sizeof(SVertexDataMS3D)))
            return false;
    }
    
    /*
     * Reading triangle indices
     */
    
    s32 nTriangles      = *(u16*)pPtr;
    CountOfTriangles_   = nTriangles;
    pTriangles_         = new STriangleMS3D[nTriangles];
    
    if (setNextBufferPos(pPtr, sizeof(u16)))
        return false;
    
    for (i = 0; i < nTriangles; ++i)
    {
        STriangleDataMS3D *pTriangle = (STriangleDataMS3D*)pPtr;
        s32 Vertices[3] = { pTriangle->VertexIndices[0], pTriangle->VertexIndices[1], pTriangle->VertexIndices[2] };
        
        memcpy(pTriangles_[i].VertexNormals, pTriangle->VertexNormals, sizeof(f32)*3*3);
        memcpy(pTriangles_[i].TexCoordU, pTriangle->TexCoordS, sizeof(f32)*3);
        memcpy(pTriangles_[i].TexCoordV, pTriangle->TexCoordT, sizeof(f32)*3);
        memcpy(pTriangles_[i].Vertices, Vertices, sizeof(s32)*3);
        
        if (setNextBufferPos(pPtr, sizeof(STriangleDataMS3D)))
            return false;
    }
    
    /*
     * Reading groups
     */
    
    s32 nGroups     = *(u16*)pPtr;
    CountOfMeshes_  = nGroups;
    pMeshes_        = new SMeshMS3D[nGroups];
    
    if (setNextBufferPos(pPtr, sizeof(u16)))
        return false;
    
    for (i = 0; i < nGroups; ++i)
    {
        pPtr += sizeof(u8); // flags
        pPtr += 32;         // name
        
        u16 nTriangles = *(u16*)pPtr;
        pPtr += sizeof(u16);
        s32* pTriangleIndices = new s32[nTriangles];
        
        for (j = 0; j < nTriangles; ++j)
        {
            pTriangleIndices[j] = *(u16*)pPtr;
            pPtr += sizeof(u16);
        }
        
        s8 materialIndex = *(s8*)pPtr;
        pPtr += sizeof(s8);
        
        pMeshes_[i].MaterialIndex       = materialIndex;
        pMeshes_[i].CountOfTriangles    = nTriangles;
        pMeshes_[i].pTriangleIndices    = pTriangleIndices;
    }
    
    /*
     * Reading materials (textures etc.)
     */
    
    s32 nMaterials      = *(u16*)pPtr;
    CountOfMaterials_   = nMaterials;
    pMaterials_         = new SMaterialMS3D[nMaterials];
    
    if (setNextBufferPos(pPtr, sizeof(u16)))
        return false;
    
    for (i = 0; i < nMaterials; ++i)
    {
        SMaterialDataMS3D* pMaterial = (SMaterialDataMS3D*)pPtr;
        
        memcpy(pMaterials_[i].Ambient, pMaterial->Ambient, sizeof(f32)*4);
        memcpy(pMaterials_[i].Diffuse, pMaterial->Diffuse, sizeof(f32)*4);
        memcpy(pMaterials_[i].Specular, pMaterial->Specular, sizeof(f32)*4);
        memcpy(pMaterials_[i].Emissive, pMaterial->Emissive, sizeof(f32)*4);
        
        pMaterials_[i].Shininess        = pMaterial->Shininess;
        pMaterials_[i].Transparency     = pMaterial->Transparency;
        pMaterials_[i].TextureFilename  = new c8[strlen(pMaterial->Texture)+1];
        
        strcpy(pMaterials_[i].TextureFilename, pMaterial->Texture);
        
        if (setNextBufferPos(pPtr, sizeof(SMaterialDataMS3D)))
            return false;
    }
    
    /*
     * Reading joints (bone/ skeletal animation)
     */
    
    f32 animFPS = *(f32*)pPtr;
    pPtr += sizeof(f32)*2;
    
    s32 totalFrames = *(s32*)pPtr;
    pPtr += sizeof(s32);
    
    TotalFrames_    = static_cast<f32>(totalFrames);
    FPS_            = animFPS;
    TotalTime_      = totalFrames * 1000.0f / animFPS;
    
    if (FPS_ < 1.0f)
        FPS_ = 1.0f;
    
    CountOfJoints_  = *(s16*)pPtr;
    pJoints_        = new SJointMS3D[CountOfJoints_];
    
    if (setNextBufferPos(pPtr, sizeof(s16)))
        return false;
    
    SKeyframeDataMS3D* pKeyframe;
    
    const u8* pTempPtr = pPtr;
    
    pNameList_ = new SJointNameListRecMS3D[CountOfJoints_];
    
    for (i = 0; i < CountOfJoints_; ++i)
    {
        SJointDataMS3D* pJoint = (SJointDataMS3D*)pTempPtr;
        
        if (setNextBufferPos(pTempPtr, sizeof(SJointDataMS3D)))
            return false;
        if (setNextBufferPos(pTempPtr, sizeof(SKeyframeDataMS3D) * (pJoint->CountOfRotations + pJoint->CountOfTranslations)))
            return false;
        
        pNameList_[i].JointIndex = i;
        pNameList_[i].Name = pJoint->Name;
    }
    
    for (i = 0; i < CountOfJoints_; ++i)
    {
        SJointDataMS3D* pJoint = (SJointDataMS3D*)pPtr;
        
        if (setNextBufferPos(pPtr, sizeof(SJointDataMS3D)))
            return false;
        
        if (pJoint->CountOfRotations != pJoint->CountOfTranslations)
        {
            io::Log::error("Unequal count of rotation- and translation keyframes");
            return false;
        }
        
        // Search for a parent bone
        s32 ParentIndex = -1;
        if (strlen(pJoint->ParentName) > 0)
        {
            for (j = 0; j < CountOfJoints_; ++j)
            {
                #if defined(SP_PLATFORM_WINDOWS)
                if (_stricmp(pNameList_[j].Name, pJoint->ParentName) == 0)
                #elif defined(SP_PLATFORM_LINUX)
                if (strcasecmp(pNameList_[j].Name, pJoint->ParentName) == 0)
                #endif
                {
                    ParentIndex = pNameList_[j].JointIndex;
                    break;
                }
            }
            if (ParentIndex == -1)
            {
                io::Log::error("Could not find parent bone");
                return false;
            }
        }
        
        pJoints_[i].Rotation    = dim::vector3df(pJoint->Rotation[0], pJoint->Rotation[1], pJoint->Rotation[2]);
        pJoints_[i].Translation = dim::vector3df(pJoint->Translation[0], pJoint->Translation[1], -pJoint->Translation[2]);
        pJoints_[i].Parent      = ParentIndex;
        
        // Proces the joint keyframes (rotations)
        for (j = 0; j < pJoint->CountOfRotations; ++j)
        {
            pKeyframe = (SKeyframeDataMS3D*)pPtr;
            
            addJointKeyframe(j, pJoints_[i], pKeyframe, true);
            
            if (setNextBufferPos(pPtr, sizeof(SKeyframeDataMS3D)))
                return false;
        }
        
        // Proces the joint keyframes (translations)
        for (j = 0; j < pJoint->CountOfTranslations; ++j)
        {
            pKeyframe = (SKeyframeDataMS3D*)pPtr;
            
            addJointKeyframe(j, pJoints_[i], pKeyframe, false);
            
            if (setNextBufferPos(pPtr, sizeof(SKeyframeDataMS3D)))
                return false;
        }
        
    }
    
    return true;
}

Mesh* MeshLoaderMS3D::buildModel()
{
    s32 TriangleIndex, VertexIndex;
    
    /* Load each texture */
    
    for (s32 i = 0; i < CountOfMaterials_; ++i)
    {
        if (SceneGraph::getTextureLoadingState() && strlen(pMaterials_[i].TextureFilename ) > 0)
        {
            pMaterials_[i].hTexture = __spVideoDriver->loadTexture(
                TexturePath_ + io::stringc(pMaterials_[i].TextureFilename)
            );
        }
        else
            pMaterials_[i].hTexture = 0;
    }
    
    /* Create the model */
    
    for (s32 i = 0, j, k; i < CountOfMeshes_; ++i)
    {
        
        Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
        
        /* Loop for each triangle */
        for (j = 0; j < pMeshes_[i].CountOfTriangles; ++j)
        {
            
            TriangleIndex = pMeshes_[i].pTriangleIndices[j];
            
            const STriangleMS3D* pTri = &pTriangles_[TriangleIndex];
            
            for (k = 0; k < 3; ++k)
            {
                VertexIndex = pTri->Vertices[k];
                
                // Add new vertex
                Surface_->addVertex(
                    pVertices_[VertexIndex].Position,
                    dim::vector3df(
                        pTri->VertexNormals[k][0], pTri->VertexNormals[k][1], -pTri->VertexNormals[k][2]
                    ),
                    dim::point2df(
                        pTri->TexCoordU[k], pTri->TexCoordV[k]
                    )
                );
            } // /vertices (3)
            
            // Add new triangle
            Surface_->addTriangle(2, 1, 0);
            Surface_->addIndexOffset(3);
            
        } // /triangles
        
        /* Add the material */
        s32 MaterialIndex = pMeshes_[i].MaterialIndex;
        
        if (MaterialIndex >= 0)
        {
            SMaterialMS3D* Material = &pMaterials_[MaterialIndex];
            
            Mesh_->getMaterial()->setDiffuseColor   (getArrayColor(Material->Diffuse    ));
            Mesh_->getMaterial()->setAmbientColor   (getArrayColor(Material->Ambient    ));
            Mesh_->getMaterial()->setSpecularColor  (getArrayColor(Material->Specular   ));
            Mesh_->getMaterial()->setEmissionColor  (getArrayColor(Material->Emissive   ));
            
            video::color Clr(
                255, 255, 255, (u8)(Material->Transparency * 255)
            );
            
            if (Material->hTexture)
                Surface_->addTexture(Material->hTexture);
            
            Mesh_->getMaterial()->setShininessFactor(Material->Shininess);
            Surface_->paint(Clr);
        }
        
    } // Next surface
    
    Mesh_->updateMeshBuffer();
    
    buildAnimation();
    
    return Mesh_;
}

void MeshLoaderMS3D::buildAnimation()
{
    if (!CountOfJoints_)
        return;
    
    /* === Temporary variables === */
    
    std::vector<SVertexGroup> VertexGroups;
    
    SJointMS3D* CurJoint = 0;
    
    dim::vector3df Pos;
    dim::quaternion Rot;
    f32 Time = 0.0f;
    
    dim::matrix4f LocalMatrix;
    
    // Create the animation
    SkeletalAnimation* Anim = __spSceneManager->createAnimation<SkeletalAnimation>("MS3D Animation");
    scene::AnimationSkeleton* Skeleton = Anim->createSkeleton();
    
    AnimationJoint* Joint = 0;
    std::vector<AnimationJoint*> JointList;
    JointList.resize(CountOfJoints_);
    
    /* === Loop for each joint === */
    
    for (s32 i = 0; i < CountOfJoints_; ++i)
    {
        CurJoint = &pJoints_[i];
        
        LocalMatrix = getAnimRotation(CurJoint->Rotation);
        
        /* Create new animation joint */
        JointList[i] = Joint = Skeleton->createJoint(
            Transformation(CurJoint->Translation, dim::quaternion(LocalMatrix), 1.0f),
            CurJoint->Name
        );
        
        LocalMatrix.setPosition(CurJoint->Translation);
        
        /* Setup vertex groups */
        fillBoneWeights(i, VertexGroups);
        
        Joint->setVertexGroups(VertexGroups);
        
        VertexGroups.clear();
        
        /* Create animation keyframes */
        for (u32 j = 0; j < CurJoint->RotationKeyframes.size(); ++j)
        {
            Pos = CurJoint->Translation + CurJoint->TranslationKeyframes[j].Vector;
            Rot = dim::quaternion( LocalMatrix * getAnimRotation(CurJoint->RotationKeyframes[j].Vector) );
            
            Time = CurJoint->RotationKeyframes[j].Time;
            u32 Frame = static_cast<u32>(Time * FPS_ - 1);
            
            Anim->addKeyframe(Joint, Transformation(Pos, Rot, 1.0f), Frame);
        }
    }
    
    /* Setup joint parents */
    for (s32 i = 0; i < CountOfJoints_; ++i)
    {
        CurJoint = &pJoints_[i];
        
        if (CurJoint->Parent >= 0)
            Skeleton->setJointParent(JointList[i], JointList[CurJoint->Parent]);
    }
    
    /* Update the skeleton bones */
    Skeleton->updateSkeleton();
    
    Mesh_->addAnimation(Anim);
}

void MeshLoaderMS3D::addJointKeyframe(
    const s32 JointIndex, SJointMS3D &Joint, void* KeyframeBuffer, bool isTypeRotation)
{
    // Temporary variabels
    SKeyframeDataMS3D* Keyframe = (SKeyframeDataMS3D*)KeyframeBuffer;
    
    SKeyframeMS3D NewKeyframe;
    
    NewKeyframe.JointIndex  = JointIndex;
    NewKeyframe.Time        = Keyframe->Time;
    
    // Add the keyframe
    if (isTypeRotation)
    {
        NewKeyframe.Vector  = dim::vector3df(Keyframe->Vector[0], Keyframe->Vector[1], Keyframe->Vector[2]);
        Joint.RotationKeyframes.push_back(NewKeyframe);
    }
    else
    {
        NewKeyframe.Vector  = dim::vector3df(Keyframe->Vector[0], Keyframe->Vector[1], -Keyframe->Vector[2]);
        Joint.TranslationKeyframes.push_back(NewKeyframe);
    }
}

void MeshLoaderMS3D::fillBoneWeights(
    const s32 JointIndex, std::vector<SVertexGroup> &VertexGroups)
{
    // Temporary variables
    s32 i, j, k;
    s32 MdlVertex;
    s32 TriangleIndex, VertexIndex;
    
    // Loop for each surface
    for (i = 0; i < CountOfMeshes_; ++i)
    {
        MdlVertex = 0;
        
        // Loop for each triangle
        for (j = 0; j < pMeshes_[i].CountOfTriangles; ++j)
        {
            TriangleIndex = pMeshes_[i].pTriangleIndices[j];
            
            const STriangleMS3D* pTri = &pTriangles_[TriangleIndex];
            
            for (k = 0; k < 3; ++k, ++MdlVertex)
            {
                VertexIndex = pTri->Vertices[k];
                
                if (pVertices_[VertexIndex].BoneID == JointIndex)
                    VertexGroups.push_back(SVertexGroup(Mesh_->getMeshBuffer(i), MdlVertex));
            } // /vertices (3)
        } // /triangles
    } // /surfaces
}


bool MeshLoaderMS3D::setNextBufferPos(const u8* &BufferPtr, s32 NextPos)
{
    BufferPtr += NextPos;
    
    if (BufferPtr > pBuffer_ + FileSize_)
    {
        io::Log::error("Corrupted data found");
        return true;
    }
    
    return false;
}

dim::matrix4f MeshLoaderMS3D::getAnimRotation(dim::vector3df Rotation)
{
    dim::matrix4f Mat;
    Mat.setRotation(Rotation, false);
    
    /* Convert from right-handed to left-handed */
    Mat[2] = -Mat[2];
    Mat[6] = -Mat[6];
    Mat[8] = -Mat[8];
    Mat[9] = -Mat[9];
    
    return Mat;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
