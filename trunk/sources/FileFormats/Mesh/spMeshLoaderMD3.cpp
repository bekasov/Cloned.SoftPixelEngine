/*
 * Mesh loader MD3 file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Mesh/spMeshLoaderMD3.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_MD3


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

static const s32 MD3_IDENTITY   = (('3'<<24) + ('P'<<16) + ('D'<<8) + 'I'); // magic number must be "IDP2" or 844121161
static const s32 MD3_VERSION    = 15;


/*
 * MeshLoaderMD3 class
 */

MeshLoaderMD3::MeshLoaderMD3() : MeshLoader()
{
}
MeshLoaderMD3::~MeshLoaderMD3()
{
}

Mesh* MeshLoaderMD3::loadMesh(const io::stringc &Filename, const io::stringc &TexturePath)
{
    if (!openLoadFile(Filename, TexturePath))
        return Mesh_;
    
    if (!loadModelData())
    {
        io::Log::error("Loading MD3 mesh failed");
        return Mesh_;
    }
    
    return buildModel();
}


/*
 * ======= Private: =======
 */

bool MeshLoaderMD3::loadModelData()
{
    return loadHeader() && loadSurfaces() && loadKeyframes();
}

bool MeshLoaderMD3::loadHeader()
{
    /* Temporary variables */
    memset(Header_.PathName, 0, sizeof(Header_.PathName));
    
    File_->readBuffer(&Header_, sizeof(SHeaderMD3));
    
    if (Header_.ID != MD3_IDENTITY)
    {
        io::Log::error("MD3 model has wrong identity (must be \"IDP3\")");
        return false;
    }
    
    if (Header_.Version != MD3_VERSION)
    {
        io::Log::error("MD3 model has unsupported version (" + io::stringc(Header_.Version) + ")");
        return false;
    }
    
    return true;
}

bool MeshLoaderMD3::loadSurfaces()
{
    /* Temporary memory */
    SSurfaceMD3 Surface;
    SVertexMD3 Vertex;
    STexCoordMD3 TexCoord;
    STriangleMD3 Triangle;
    
    dim::vector3df Coord, Normal;
    
    KeyframeList_.resize(Header_.SurfaceOffset);
    
    /* Read surfaces */
    File_->setSeek(Header_.SurfaceOffset);
    
    for (s32 s = 0; s < Header_.CountOfSurfaces; ++s)
    {
        /* Create new mesh buffer */
        video::MeshBuffer* Surf = Mesh_->createMeshBuffer();
        
        /* Read surface */
        s32 BaseOffset = File_->getSeek();
        
        memset(Surface.Name, 0, sizeof(Surface.Name));
        
        File_->readBuffer(&Surface, sizeof(SSurfaceMD3));
        
        /* Check frame count */
        if (Surface.CountOfFrames != Header_.CountOfKeyframes)
        {
            io::Log::error("Animation keyframe count differs between surface and header description");
            return false;
        }
        
        /* Allocate container for keyframes */
        KeyframeList_[s].Surface = Surf;
        KeyframeList_[s].Keyframes.resize(Surface.CountOfVertices);
        
        for (s32 i = 0; i < Surface.CountOfVertices; ++i)
            KeyframeList_[s].Keyframes[i].resize(Header_.CountOfKeyframes);
        
        /* Read vertices */
        File_->setSeek(BaseOffset + Surface.VertexOffset);
        
        for (s32 f = 0; f < Header_.CountOfKeyframes; ++f)
        {
            for (s32 i = 0; i < Surface.CountOfVertices; ++i)
            {
                File_->readBuffer(&Vertex, sizeof(SVertexMD3));
                
                /* Get vertex coordinate */
                Coord = dim::vector3df(
                    static_cast<f32>(Vertex.Coord[1]),
                    static_cast<f32>(Vertex.Coord[2]),
                    static_cast<f32>(Vertex.Coord[0])
                ) * (1.0f / 64.0f);
                
                /* Compute vertex normal */
                const f32 Latitude  = static_cast<f32>(Vertex.Normal[0]) * 2.0f * math::PI / 255;
                const f32 Longitude = static_cast<f32>(Vertex.Normal[1]) * 2.0f * math::PI / 255;
                
                Normal = dim::vector3df(
                    cos(Latitude) * sin(Longitude),
                    sin(Latitude) * sin(Longitude),
                    cos(Longitude)
                );
                
                /* Store vertex data for keyframes */
                (KeyframeList_[s].Keyframes[i])[f] = SVertexKeyframe(Coord, Normal);
                
                /* Add new vertex */
                if (!f)
                    Surf->addVertex(Coord, Normal, 0.0f, video::color(255));
            }
        }
        
        /* Read texture coordinates */
        File_->setSeek(BaseOffset + Surface.TexCoordOffset);
        
        for (s32 i = 0; i < Surface.CountOfVertices; ++i)
        {
            File_->readBuffer(&TexCoord, sizeof(STexCoordMD3));
            
            Surf->setVertexTexCoord(i, *(dim::point2df*)TexCoord.TexCoord);
        }
        
        /* Read triangles */
        File_->setSeek(BaseOffset + Surface.TriangleOffset);
        
        for (s32 i = 0; i < Surface.CountOfTriangles; ++i)
        {
            File_->readBuffer(&Triangle, sizeof(STriangleMD3));
            
            Surf->addTriangle(
                Triangle.Indices[2], Triangle.Indices[1], Triangle.Indices[0]
            );
        }
        
        File_->setSeek(BaseOffset + Surface.EndOffset);
    }
    
    return true;
}

bool MeshLoaderMD3::loadKeyframes()
{
    #if 0
    
    File_->setSeek(Header_.KeyframeOffset);
    
    SKeyFrameMD3 Keyframe;
    
    #if 1
    io::Log::message("Keyframes: " + io::stringc(Header_.CountOfKeyframes));
    #endif
    
    for (s32 i = 0; i < Header_.CountOfKeyframes; ++i)
    {
        /* Read keyframe */
        memset(Keyframe.Name, 0, sizeof(Keyframe.Name));
        
        File_->readBuffer(&Keyframe, sizeof(SKeyFrameMD3));
        
        #if 1
        io::Log::message("\tKeyframe #" + io::stringc(i) + " \"" + io::stringc(Keyframe.Name) + "\"");
        #endif
        
        
        
        
    }
    
    #endif
    
    return true;
}

Mesh* MeshLoaderMD3::buildModel()
{
    Mesh_->updateMeshBuffer();
    Mesh_->updateNormals();
    
    buildAnimation();
    
    return Mesh_;
}

void MeshLoaderMD3::buildAnimation()
{
    MorphTargetAnimation* Anim = __spSceneManager->createAnimation<MorphTargetAnimation>("MD3 Animation");
    
    foreach (SSurfaceKeyframes &Surf, KeyframeList_)
    {
        for (u32 i = 0; i < Surf.Keyframes.size(); ++i)
            Anim->addKeyframeSequence(Surf.Surface, i, Surf.Keyframes[i]);
    }
    
    Mesh_->addAnimation(Anim);
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
