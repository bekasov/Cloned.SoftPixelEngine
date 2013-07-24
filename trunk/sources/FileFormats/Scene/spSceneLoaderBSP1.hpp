/*
 * Quake 1 BSP loader header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENELOADER_BSP1_H__
#define __SP_SCENELOADER_BSP1_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_SCENELOADER_BSP1


#include "Base/spInputOutputLog.hpp"
#include "Base/spInputOutputFileSystem.hpp"
#include "Base/spDimension.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "FileFormats/Mesh/spMeshLoader.hpp"
#include "FileFormats/Image/spImageLoaderWAD.hpp"

#include <vector>


namespace sp
{
namespace scene
{


//! This is the "Quake 1" BSP scene loader.
class SP_EXPORT SceneLoaderBSP1 : public SceneLoader
{
    
    public:
        
        SceneLoaderBSP1();
        ~SceneLoaderBSP1();
        
        Mesh* loadScene(const io::stringc &Filename, const io::stringc &TexturePath, const s32 Flags);
        
    private:
        
        #ifdef SP_COMPILE_WITH_TEXLOADER_WAD
        friend class video::ImageLoaderWAD;
        #endif
        
        /* ===== Enumerations ===== */
        
        enum ELumpTypes
        {
            BSP_LUMP_ENTITIES = 0,
            BSP_LUMP_PLANES,
            BSP_LUMP_WALLTEXTURES,
            BSP_LUMP_VERTICES,
            BSP_LUMP_VISDATA,
            BSP_LUMP_NODES,
            BSP_LUMP_TEXINFO,
            BSP_LUMP_FACES,
            BSP_LUMP_LIGHTMAPS,
            BSP_LUMP_CLIPNODES,
            BSP_LUMP_LEAFS,
            BSP_LUMP_FACESLIST,
            BSP_LUMP_EDGES,
            BSP_LUMP_EDGESLIST,
            BSP_LUMP_MODELS,
        };
        
        enum EPLaneTypes
        {
            BSP_PLANE_AXIALX = 0,
            BSP_PLANE_AXIALY,
            BSP_PLANE_AXIALZ,
            BSP_PLANE_NONAXIALX,
            BSP_PLANE_NONAXIALY,
            BSP_PLANE_NONAXIALZ
        };
        
        /* ===== Structures ===== */
        
        struct SDirEntryBSP
        {
            s32 Offset;
            s32 Length;
        };
        
        struct SHeaderBSP
        {
            s32 Version;
            SDirEntryBSP DirEntries[15];
        };
        
        struct SPlaneBSP
        {
            dim::vector3df Normal;
            f32 Dist;
            s32 Type;
        };
        
        struct SWallTextureBSP
        {
            c8 Name[16];
            u32 Width, Height;
            u32 Offset1;
            u32 Offset2;
            u32 Offset4;
            u32 Offset8;
        };
        
        struct STextureInfoBSP
        {
            dim::vector3df VectorS;
            f32 DistS;
            
            dim::vector3df VectorT;
            f32 DistT;
            
            u32 TextureID;
            u32 Animated;
        };
        
        struct SEdgeBSP
        {
            inline void swap()
            {
                std::swap(VertexStart, VertexEnd);
            }
            
            u16 VertexStart;
            u16 VertexEnd;
        };
        
        struct SFaceBSP
        {
            u16 PlaneID;
            u16 Side; // 0 = front / 1 = back
            
            s32 FirstEdge;
            u16 CountEdges;
            
            u16 TexInfoID;
            
            u8 LightType;
            u8 LightBase;
            u8 Light[2];
            
            s32 LightMap;
        };
        
        struct SScriptBlockBSP
        {
            void clear()
            {
                ClassName = "";
            }
            
            io::stringc ClassName;
            dim::vector3df Origin;
            dim::vector3df Angles;
            video::color Color;
        };
        
        
        /* ===== Functions ===== */
        
        inline void swapVector(dim::vector3df &Vec)
        {
            dim::vector3df tmp(Vec);
            Vec.X =   tmp.Y;
            Vec.Y =   tmp.Z;
            Vec.Z = - tmp.X;
        }
        
        /* Reading lumps functions */
        bool readHeader();
        void readLumps();
        
        void readLumpEntities();
        void readLumpPlanes();
        void readLumpWallTextures();
        void readLumpVertices();
        void readLumpVisData();
        void readLumpNodes();
        void readLumpTexInfo();
        void readLumpFaces();
        void readLumpLightMaps();
        void readLumpClipNodes();
        void readLumpLeafs();
        void readLumpFacesList();
        void readLumpEdges();
        void readLumpEdgesList();
        void readLumpModels();
        
        /* Texture loading */
        void readTexturesRAW();
        
        void findNextTexturePath(io::stringc Filename, s32 &Pos);
        
        /* Script analysing */
        void examineScript(std::vector<io::stringc> &ScriptData);
        void executeScriptBlock(const SScriptBlockBSP &Block);
        dim::vector3df convertVector(io::stringc &Arg);
        video::color convertColor(io::stringc &Arg);
        
        std::vector<io::stringc> readScriptSpecWAD(io::stringc &Arg);
        
        /* Createing & building functions */
        std::vector<SEdgeBSP> pairEdges(std::vector<s16> &EdgesIndicesList);
        std::vector<s16> pairTriangles(std::vector<SEdgeBSP> &EdgesList);
        void createPolygon(std::vector<s16> &IndicesList);
        
        dim::point2df calcTexCoord(const dim::vector3df &Pos, const STextureInfoBSP &TexInfo);
        
        void buildModel();
        
        /* ===== Members ===== */
        
        SHeaderBSP Header_;
        SFaceBSP CurFace_;
        STextureInfoBSP CurTexInfo_;
        
        c8* EntDesc_;
        u8* LightMap_;
        
        /* All container lists */
        std::list<video::Texture*> TextureList_, TextureListRAW_;
        //std::list<video::Texture*> LightMapList_;
        
        std::vector<SWallTextureBSP> WallTextureList_;
        std::vector<STextureInfoBSP> TextureInfoList_;
        
        std::vector<SPlaneBSP> PlaneList_;
        std::vector<SFaceBSP> FacesList_;
        std::vector<SEdgeBSP> EdgesList_;
        std::vector<s16> EdgesIndicesList_;
        std::vector<dim::vector3df> VerticesList_;
        
        std::vector<io::stringc> ScriptWADList_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ======================================
