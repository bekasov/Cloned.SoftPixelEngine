/*
 * Mesh loader SPM file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Mesh/spMeshLoaderSPM.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_SPM


#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace scene
{


MeshLoaderSPM::MeshLoaderSPM() :
    MeshLoader              (       ),
    CurMesh_                (0      ),
    has32BitIndices_        (false  ),
    hasVertexColors_        (false  ),
    hasVertexFogCoords_     (false  ),
    hasVertexNormals_       (false  ),
    DefaultVertexFogCoord_  (0.0f   ),
    TexLayerCount_          (0      )
{
}
MeshLoaderSPM::~MeshLoaderSPM()
{
}

Mesh* MeshLoaderSPM::loadMesh(const io::stringc &Filename, const io::stringc &TexturePath)
{
    if (!openLoadFile(Filename, TexturePath))
        return Mesh_;
    
    if (!readHeader())
    {
        io::Log::error("Loading SPM mesh failed");
        return Mesh_;
    }
    
    readChunkObject();
    
    return Mesh_;
}


/*
 * ======= Private: =======
 */

bool MeshLoaderSPM::readHeader()
{
    // Read header information: magic number ("SPMD"), format version
    const s32 MagicNumber = File_->readValue<s32>();
    
    if (MagicNumber != SPM_MAGIC_NUMBER)
    {
        io::Log::error("SPM file has invalid magic number");
        return false;
    }
    
    const s16 Version = File_->readValue<u16>();
    
    if (Version < SPM_VERSION_MIN_NR)
    {
        io::Log::error("SPM file version is too low so it may load incorrect");
        return false;
    }
    else if (Version > SPM_VERSION_NUMBER)
    {
        io::Log::error("SPM file version is too high, unknown format");
        return false;
    }
    
    return true;
}

void MeshLoaderSPM::readChunkObject()
{
    // Read main mesh and each sub mesh
    u32 SubMeshCount = File_->readValue<u32>();
    
    readChunkSubMesh(Mesh_);
    
    for (u32 i = 1; i < SubMeshCount; ++i)
    {
        readChunkSubMesh(0);
        Mesh_->addLODSubMesh(CurMesh_);
    }
}

void MeshLoaderSPM::readChunkSubMesh(Mesh* SubMesh)
{
    if (SubMesh)
        CurMesh_ = SubMesh;
    else
        CurMesh_ = __spSceneManager->createMesh();
    
    // Read object information: name, flags
    CurMesh_->setName(File_->readStringData());
    
    // Get the flags
    const u16 MeshFlags = File_->readValue<u16>();
    
    if (MeshFlags & MDLSPM_CHUNK_GOURAUDSHADING)
        CurMesh_->setShading(video::SHADING_GOURAUD);
    else
        CurMesh_->setShading(video::SHADING_FLAT);
    
    // Reserved data (leap data, just for editoring)
    const u32 ReservedDataSize = File_->readValue<u32>();
    File_->setSeek(ReservedDataSize, io::FILEPOS_CURRENT);
    
    // Read each surface
    const u32 SurfaceCount = File_->readValue<u32>();
    
    for (u32 s = 0; s < SurfaceCount; ++s)
        readChunkSurface();
    
    // Read animaions
    if (MeshFlags & MDLSPM_CHUNK_NODE_ANIM)
        readChunkAnimationNode();
    if (MeshFlags & MDLSPM_CHUNK_MORPHTARGET_ANIM)
        readChunkAnimationMorphTarget();
    if (MeshFlags & MDLSPM_CHUNK_SKELETAL_ANIM)
        readChunkAnimationSkeletal();
    
    // Update mesh buffers
    CurMesh_->updateMeshBuffer();
    
    if (!hasVertexNormals_)
        CurMesh_->updateNormals();
}

void MeshLoaderSPM::readChunkSurface()
{
    Surface_ = CurMesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
    
    // Read surface information: name, flags
    Surface_->setName(File_->readStringData());
    
    // Set the flags
    const u16 SurfaceFlags = File_->readValue<u16>();
    
    has32BitIndices_    = (SurfaceFlags & MDLSPM_CHUNK_INDEX32BIT   ) != 0;
    hasVertexColors_    = (SurfaceFlags & MDLSPM_CHUNK_VERTEXCOLOR  ) != 0;
    hasVertexFogCoords_ = (SurfaceFlags & MDLSPM_CHUNK_VERTEXFOG    ) != 0;
    hasVertexNormals_   = (SurfaceFlags & MDLSPM_CHUNK_VERTEXNORMAL ) != 0;
    
    // Read texture coordinates dimensions
    for (u8 i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)
        TexCoordsDimensions_[i] = File_->readValue<u8>();
    
    // Read each texture
    TexLayerCount_ = File_->readValue<u8>();
    
    for (u8 i = 0; i < TexLayerCount_; ++i)
        readChunkTexture();
    
    // Read each vertex
    const u32 VertexCount = File_->readValue<u32>();
    
    if (VertexCount)
    {
        // Read vertex default values: color, fog coord
        if (!hasVertexColors_)
            DefaultVertexColor_ = File_->readColor();
        if (!hasVertexFogCoords_)
            DefaultVertexFogCoord_ = File_->readValue<f32>();
        
        // Read the vertex data
        Surface_->addVertices(VertexCount);
        
        for (u32 i = 0; i < VertexCount; ++i)
            readChunkVertex(i);
    }
    
    // Read each triangle
    const u32 TriangleCount = File_->readValue<u32>();
    
    if (TriangleCount)
    {
        // Read the triangle data
        Surface_->addTriangles(TriangleCount);
        
        for (u32 i = 0; i < TriangleCount; ++i)
            readChunkTriangle(i);
    }
}

void MeshLoaderSPM::readChunkVertex(u32 Index)
{
    // Temporary variables;
    dim::vector3df Position, Normal;
    dim::vector3df TexCoords[MAX_COUNT_OF_TEXTURES];
    video::color Color(DefaultVertexColor_);
    f32 FogCoord(DefaultVertexFogCoord_);
    
    // Read position
    Position = File_->readVector<f32>();
    
    // Read texture coordinates
    dim::vector3df TexCoord;
    
    for (u8 i = 0, j; i < MAX_COUNT_OF_TEXTURES; ++i)
    {
        if (TexCoordsDimensions_[i] > 0)
        {
            for (j = 0; j < TexCoordsDimensions_[i]; ++j)
                (TexCoords[i])[j] = File_->readValue<f32>();
        }
    }
    
    // Read color
    if (hasVertexColors_)
        Color = File_->readColor();
    
    // Read fog coordinate
    if (hasVertexFogCoords_)
        FogCoord = File_->readValue<f32>();
    
    // Read normal
    if (hasVertexNormals_)
    {
        Normal = File_->readVector<f32>();
        Normal.normalize();
    }
    
    // Create the vertex
    Surface_->setVertexCoord(Index, Position);
    Surface_->setVertexNormal(Index, Normal);
    Surface_->setVertexTexCoord(Index, TexCoords[0]);
    Surface_->setVertexColor(Index, Color);
    Surface_->setVertexFog(Index, FogCoord);
    
    for (u8 i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)
    {
        if (TexCoordsDimensions_[i] > 0)
            Surface_->setVertexTexCoord(Index, TexCoords[i], i);
    }
}

void MeshLoaderSPM::readChunkTriangle(u32 Index)
{
    // Read the indices (32 bits or 16 bits)
    u32 v[3];
    
    if (has32BitIndices_)
    {
        v[0] = File_->readValue<u32>();
        v[1] = File_->readValue<u32>();
        v[2] = File_->readValue<u32>();
    }
    else
    {
        v[0] = static_cast<u32>(File_->readValue<u16>());
        v[1] = static_cast<u32>(File_->readValue<u16>());
        v[2] = static_cast<u32>(File_->readValue<u16>());
    }
    
    // Create the triangle
    Surface_->setTriangleIndices(Index, v);
}

void MeshLoaderSPM::readChunkTexture()
{
    // Read texture basic information
    bool isTexValid = static_cast<bool>(File_->readValue<s8>() != 0);
    
    if (isTexValid)
    {
        const u8 Layer = CurMesh_->getTextureCount();
        
        // Read the texture resource information
        const io::stringc Filename(File_->readStringData());
        
        if (SceneGraph::getTextureLoadingState())
        {
            video::Texture* Tex = 0;
            
            if (FileSys_.findFile(TexturePath_ + Filename))
                Tex = __spVideoDriver->loadTexture(TexturePath_ + Filename);
            else
                Tex = __spVideoDriver->loadTexture(Filename);
            
            Surface_->addTexture(Tex);
        }
        
        // Get texture flags: environment mapping, matrix etc.
        const u16 TextureFlags = File_->readValue<u16>();
        
        // Read the flags data
        if (TextureFlags & MDLSPM_CHUNK_TEXTUREMATRIX)
            Surface_->setTextureMatrix(Layer, File_->readMatrix<f32>());
        
        Surface_->setTextureEnv(Layer, File_->readValue<video::ETextureEnvTypes>());
        Surface_->setMappingGen(Layer, File_->readValue<video::EMappingGenTypes>());
        Surface_->setMappingGenCoords(Layer, File_->readValue<s32>());
    }
}

void MeshLoaderSPM::readChunkAnimationNode()
{
    //todo
}

void MeshLoaderSPM::readChunkAnimationMorphTarget()
{
    //todo
}

void MeshLoaderSPM::readChunkAnimationSkeletal()
{
    /* Read skeletal animation basics */
    const io::stringc AnimName(File_->readStringData());
    
    /* Add new skeletal animation */
    SkeletalAnimation* Anim = __spSceneManager->createAnimation<SkeletalAnimation>("SPM Animation");
    AnimationSkeleton* Skeleton = Anim->createSkeleton();
    
    /* Read animation joints */
    const u32 JointCount = File_->readValue<u32>();
    
    Joints_.resize(JointCount);
    
    for (u32 i = 0; i < JointCount; ++i)
        readChunkAnimationJoint(Joints_[i]);
    
    /* Build joint construction */
    foreach (SJointSPM &Joint, Joints_)
    {
        /* Create joint object */
        Joint.JointObject = Skeleton->createJoint(
            Transformation(Joint.Position, Joint.Rotation, Joint.Scale), Joint.Name
        );
        
        /* Setup vertex weights */
        std::vector<SVertexGroup> VertexGroups;
        VertexGroups.resize(Joint.VertexWeights.size());
        
        u32 i = 0;
        foreach (const SVertexWeightSPM &Vertex, Joint.VertexWeights)
        {
            VertexGroups[i++] = SVertexGroup(
                CurMesh_->getMeshBuffer(Vertex.Surface), Vertex.Index, Vertex.Weight
            );
        }
        
        Joint.JointObject->setVertexGroups(VertexGroups);
        
        /* Setup keyframes */
        foreach (const SKeyframeSPM &Keyframe, Joint.Keyframes)
        {
            Anim->addKeyframe(
                Joint.JointObject,
                Transformation(Keyframe.Position, Keyframe.Rotation, Keyframe.Scale),
                Keyframe.Frame
            );
        }
    }
    
    /* Setup parent joints */
    foreach (SJointSPM &Joint, Joints_)
    {
        if (Joint.Parent >= 0 && Joint.Parent < static_cast<s32>(Joints_.size()))
            Skeleton->setJointParent(Joint.JointObject, Joints_[Joint.Parent].JointObject);
    }
    
    /* Complete animation */
    Skeleton->updateSkeleton();
    CurMesh_->addAnimation(Anim);
}

void MeshLoaderSPM::readChunkAnimationJoint(SJointSPM &Joint)
{
    /* Read joint basics */
    Joint.Name      = File_->readStringData();
    Joint.Parent    = File_->readValue<s32>();
    
    Joint.Position  = File_->readVector<f32>();
    Joint.Rotation  = File_->readQuaternion();
    Joint.Scale     = File_->readVector<f32>();
    
    /* Read vertex weights */
    const u32 VertexCount = File_->readValue<u32>();
    
    for (u32 i = 0; i < VertexCount; ++i)
    {
        SVertexWeightSPM Vertex;
        {
            Vertex.Surface  = File_->readValue<u32>();
            Vertex.Index    = File_->readValue<u32>();
            Vertex.Weight   = File_->readValue<f32>();
        }
        Joint.VertexWeights.push_back(Vertex);
    }
    
    /* Read keyframes */
    const u32 KeyframeCount = File_->readValue<u32>();
    
    for (u32 i = 0; i < KeyframeCount; ++i)
    {
        SKeyframeSPM Keyframe;
        {
            Keyframe.Frame      = File_->readValue<u32>();
            Keyframe.Position   = File_->readVector<f32>();
            Keyframe.Rotation   = File_->readQuaternion();
            Keyframe.Scale      = File_->readVector<f32>();
        }
        Joint.Keyframes.push_back(Keyframe);
    }
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
