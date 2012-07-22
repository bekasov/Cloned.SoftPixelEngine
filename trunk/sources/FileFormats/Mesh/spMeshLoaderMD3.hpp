/*
 * Mesh loader MD3 header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHLOADER_MD3_H__
#define __SP_MESHLOADER_MD3_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_MD3


#include "Base/spInputOutputLog.hpp"
#include "Base/spDimension.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "FileFormats/Mesh/spMeshLoader.hpp"

#include <vector>
#include <string>


namespace sp
{
namespace scene
{


class SP_EXPORT MeshLoaderMD3 : public MeshLoader
{
    
    public:
        
        MeshLoaderMD3();
        ~MeshLoaderMD3();
        
        Mesh* loadMesh(const io::stringc &Filename, const io::stringc &TexturePath);
        
    private:
        
        /* === Structures === */
        
        struct SHeaderMD3
        {
            s32 ID;                 // Magic number (must be "IDP3")
            s32 Version;            // MD2 version (must be 15)
            
            c8 PathName[64];
            
            s32 Flags;
            
            s32 CountOfKeyframes;
            s32 CountOfTags;
            s32 CountOfSurfaces;
            s32 CountOfSkins;
            
            s32 KeyframeOffset;
            s32 TagsOffset;
            s32 SurfaceOffset;
            s32 EndOfFileOffset;
        };
        
        struct SVertexMD3
        {
            s16 Coord[3];
            u8 Normal[2];
        };
        
        struct STexCoordMD3
        {
            f32 TexCoord[2];
        };
        
        struct STriangleMD3
        {
            s32 Indices[3];
        };
        
        struct SShaderMD3
        {
            c8 PathName[64];
            s32 ShaderIndex;
        };
        
        struct SKeyFrameMD3
        {
            dim::vector3df MinBounds;
            dim::vector3df MaxBounds;
            dim::vector3df LocalOrigin;
            f32 Radius;
            c8 Name[16];
        };
        
        struct STagMD3
        {
            c8 Name[64];
            dim::vector3df Origin;
            dim::vector3df RotationAxles[3];
        };
        
        struct SSurfaceMD3
        {
            s32 ID;             // Magic number (must be "IDP3")
            
            c8 Name[64];
            
            s32 Flags;
            
            s32 CountOfFrames;
            s32 CountOfShaders;
            s32 CountOfVertices;
            s32 CountOfTriangles;
            
            s32 TriangleOffset;
            s32 ShaderOffset;
            s32 TexCoordOffset;
            s32 VertexOffset;
            s32 EndOffset;
        };
        
        struct SSurfaceKeyframes
        {
            video::MeshBuffer* Surface;
            std::vector< std::vector<SVertexKeyframe> > Keyframes;
        };
        
        /* === Functions === */
        
        bool loadModelData();
        bool loadHeader();
        bool loadSurfaces();
        bool loadKeyframes();
        
        void buildAnimation();
        Mesh* buildModel();
        
        void interpolate(dim::vector3df* VerticesList);
        
        /* === Members === */
        
        SHeaderMD3 Header_;
        
        std::vector<SSurfaceKeyframes> KeyframeList_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
