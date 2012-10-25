/*
 * Mesh loader MD2 file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Mesh/spMeshLoaderMD2.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_MD2


#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern scene::SceneGraph* __spSceneManager;

namespace scene
{


/*
 * Macros
 */

static const s32 MD2_IDENTITY       = (('2'<<24) + ('P'<<16) + ('D'<<8) + 'I'); // magic number must be "IDP2" or 844121161
static const s32 MD2_VERSION        = 8;
static const s32 MD2_MAX_VERTICES   = 1000;

const f32 md2NormalLookupTable[][3] = {
#include "Plugins/anorms.h"
};


/*
 * MeshLoaderMD2 class
 */

MeshLoaderMD2::MeshLoaderMD2() :
    MeshLoader      (       ),
    KeyFramesCount_ (0      ),
    VerticesCount_  (0      ),
    CommandsCount_  (0      ),
    pVertices_      (0      ),
    pCommands_      (0      ),
    pLightNormals_  (0      ),
    Scale_          (1.0f   )
{
    memset(&Animation_, 0, sizeof(SAnimStateMD2));
}
MeshLoaderMD2::~MeshLoaderMD2()
{
    MemoryManager::deleteBuffer(pVertices_);
    MemoryManager::deleteBuffer(pCommands_);
    MemoryManager::deleteBuffer(pLightNormals_);
}

Mesh* MeshLoaderMD2::loadMesh(const io::stringc &Filename, const io::stringc &TexturePath)
{
    if (!openLoadFile(Filename, TexturePath))
        return Mesh_;
    
    if (!loadModelData(Filename))
    {
        io::Log::error("Loading MD2 mesh failed");
        return Mesh_;
    }
    
    return buildModel();
}

void MeshLoaderMD2::getAnimationSeq(s32 Type, u32 &Start, u32 &End)
{
    Start   = AnimList_[Type].FirstFrame;
    End     = AnimList_[Type].LastFrame;
}


/*
 * ======= Private: =======
 */

bool MeshLoaderMD2::loadModelData(io::stringc Filename)
{
    /* Temporary variables */
    SHeaderMD2 Header;
    SKeyFrameMD2* pFrame = 0;
    dim::vector3df* pVertices = 0;
    u8* pNormals = 0;
    s8* pBuffer = 0;
    
    File_->readBuffer(&Header, sizeof(SHeaderMD2));
    
    if (Header.ID != MD2_IDENTITY)
    {
        io::Log::error("MD2 model has wrong identity (must be \"IDP2\")");
        return false;
    }
    
    if (Header.Version != MD2_VERSION)
    {
        io::Log::error("MD2 model has unsupported version (" + io::stringc(Header.Version) + ")");
        return false;
    }
    
    /* Initialize member variables */
    KeyFramesCount_     = Header.CountOfKeyframes;
    VerticesCount_      = Header.CountOfVertices;
    CommandsCount_      = Header.CountOfCommands;
    
    /* Allocate memory */
    pVertices_          = MemoryManager::createBuffer<dim::vector3df>(VerticesCount_ * KeyFramesCount_);
    pCommands_          = MemoryManager::createBuffer<s32>(CommandsCount_);
    pLightNormals_      = MemoryManager::createBuffer<u8>(VerticesCount_ * KeyFramesCount_);
    pBuffer             = MemoryManager::createBuffer<s8>(KeyFramesCount_ * Header.FrameSize);
    
    /* Read frame data ... */
    File_->setSeek(Header.KeyframeOffset, io::FILEPOS_BEGIN);
    File_->readBuffer(pBuffer, Header.FrameSize, KeyFramesCount_);
    
    /* Read commands ... */
    File_->setSeek(Header.CommandOffset, io::FILEPOS_BEGIN);
    File_->readBuffer(pCommands_, sizeof(s32), CommandsCount_);
    
    /* Vertex array initialization */
    for (s32 i = 0; i < KeyFramesCount_; ++i)
    {
        /* Adjust pointers */
        pFrame      = (SKeyFrameMD2*)&pBuffer[ Header.FrameSize * i ];
        pVertices   = &pVertices_[ VerticesCount_ * i ];
        pNormals    = &pLightNormals_[ VerticesCount_ * i ];
        
        for (s32 j = 0; j < VerticesCount_; ++j)
        {
            pVertices[j].X = (pFrame->Scale.X * pFrame->Vertices[j].Vertex.X) + pFrame->Translate.X;
            pVertices[j].Y = (pFrame->Scale.Y * pFrame->Vertices[j].Vertex.Y) + pFrame->Translate.Y;
            pVertices[j].Z = (pFrame->Scale.Z * pFrame->Vertices[j].Vertex.Z) + pFrame->Translate.Z;
            
            pNormals[j] = pFrame->Vertices[j].LightNormalIndex;
        }
    }
    
    MemoryManager::deleteBuffer(pBuffer);
    
    return true;
}

Mesh* MeshLoaderMD2::buildModel()
{
    /* Temporary variables */
    /*
    todo (VS2012 Code-Analysis) -> C6262 Excessive stack usage Function uses '24024' bytes of stack:
    exceeds /analyze:stacksize '16384'. Consider moving some data to heap.
    */
    dim::vector3df CoordList[MD2_MAX_VERTICES], NormalList[MD2_MAX_VERTICES];
    s32* pTriangleCmds = pCommands_;
    bool isTriangleFan, isStrip = false;
    u32 cnt1 = 0;
    
    interpolate(CoordList, NormalList);
    
    Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
    
    while (s32 i = *(pTriangleCmds++))
    {
        isTriangleFan = (i < 0);
        
        if (i < 0)
            i = -i;
        
        Surface_->addIndexOffset(cnt1);
        
        cnt1 = 0;
        isStrip = true;
        
        for (; i > 0; --i, pTriangleCmds += 3)
        {
            /*
             * pTriangleCmds[0]: texture coordinate u
             * pTriangleCmds[1]: texture coordinate v
             * pTriangleCmds[2]: vertex index to render
             */
            
            /* Add new vertex */
            Surface_->addVertex(
                CoordList[ pTriangleCmds[2] ],
                dim::point2df( ((f32*)pTriangleCmds)[0], ((f32*)pTriangleCmds)[1] )
            );
            
            /* Add new triangle */
            if (++cnt1 >= 3)
            {
                if (!isTriangleFan)
                {
                    isStrip = !isStrip;
                    if (isStrip)
                        Surface_->addTriangle(cnt1 - 3, cnt1 - 2, cnt1 - 1);
                    else
                        Surface_->addTriangle(cnt1 - 1, cnt1 - 2, cnt1 - 3);
                }
                else
                    Surface_->addTriangle(0, cnt1 - 1, cnt1 - 2);
            }
        }
    }
    
    Mesh_->updateIndexBuffer();
    Mesh_->updateNormals();
    
    buildAnimation();
    
    return Mesh_;
}

void MeshLoaderMD2::interpolate(dim::vector3df* CoordList, dim::vector3df* NormalList)
{
    for (s32 i = 0; i < VerticesCount_; ++i) // z, x, y (fastest rotation)
    {
        u32 Index = i + (VerticesCount_*Animation_.CurFrame);
        
        CoordList[i].Z = pVertices_[Index].X * Scale_;
        CoordList[i].X = pVertices_[Index].Y * Scale_;
        CoordList[i].Y = pVertices_[Index].Z * Scale_;
        
        NormalList[i].Z = md2NormalLookupTable[pLightNormals_[Index]][0];
        NormalList[i].X = md2NormalLookupTable[pLightNormals_[Index]][1];
        NormalList[i].Y = md2NormalLookupTable[pLightNormals_[Index]][2];
    }
}

/*
todo ->
C6262 Excessive stack usage Function uses '24072' bytes of stack:
exceeds /analyze:stacksize '16384'. Consider moving some data to heap.
*/
void MeshLoaderMD2::buildAnimation()
{
    if (KeyFramesCount_ <= 1 || !Mesh_->getMeshBufferCount())
        return;
    
    /* Temporary memory */
    video::MeshBuffer* MainSurface = Mesh_->getMeshBuffer(0);
    
    dim::vector3df CoordList[MD2_MAX_VERTICES], NormalList[MD2_MAX_VERTICES];
    s32* pTriangleCmds;
    
    /* Create morph-target animation */
    MorphTargetAnimation* Anim = __spSceneManager->createAnimation<MorphTargetAnimation>("MD2 Animation");
    
    std::vector< std::vector<SVertexKeyframe> > VertexKeyframes;
    VertexKeyframes.resize(MainSurface->getVertexCount());
    
    for (u32 i = 0; i < VertexKeyframes.size(); ++i)
        VertexKeyframes[i].resize(KeyFramesCount_);
    
    /* Build keyframe sequences */
    for (s32 Frame = 0; Frame < KeyFramesCount_; ++Frame)
    {
        pTriangleCmds = pCommands_;
        Animation_.CurFrame = Frame;
        
        interpolate(CoordList, NormalList);
        
        u32 j = 0;
        
        while (s32 i = *(pTriangleCmds++))
        {
            if (i < 0)
                i = -i;
            
            for (; i > 0; --i, pTriangleCmds += 3, ++j)
            {
                (VertexKeyframes[j])[Frame] = SVertexKeyframe(
                    CoordList[pTriangleCmds[2]], NormalList[pTriangleCmds[2]]
                );
            }
        }
    }
    
    /* Add all created keyframes to the animation */
    u32 i = 0;
    foreach (const std::vector<SVertexKeyframe> &Keyframes, VertexKeyframes)
        Anim->addKeyframeSequence(MainSurface, i++, Keyframes);
    
    Mesh_->addAnimation(Anim);
    
    Animation_.CurFrame = 0;
}

MeshLoaderMD2::SAnimationMD2 MeshLoaderMD2::AnimList_[21] =
{
    // first, last, fps
    {   0,  39,  9 },   // STAND
    {  40,  45, 10 },   // RUN
    {  46,  53, 10 },   // ATTACK
    {  54,  57,  7 },   // PAIN_A
    {  58,  61,  7 },   // PAIN_B
    {  62,  65,  7 },   // PAIN_C
    {  66,  71,  7 },   // JUMP
    {  72,  83,  7 },   // FLIP
    {  84,  94,  7 },   // SALUTE
    {  95, 111, 10 },   // FALLBACK
    { 112, 122,  7 },   // WAVE
    { 123, 134,  6 },   // POINT
    { 135, 153, 10 },   // CROUCH_STAND
    { 154, 159,  7 },   // CROUCH_WALK
    { 160, 168, 10 },   // CROUCH_ATTACK
    { 196, 172,  7 },   // CROUCH_PAIN
    { 173, 177,  5 },   // CROUCH_DEATH
    { 178, 183,  7 },   // DEATH_FALLBACK
    { 184, 189,  7 },   // DEATH_FALLFORWARD
    { 190, 197,  7 },   // DEATH_FALLBACKSLOW
    { 198, 198,  5 },   // BOOM
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
