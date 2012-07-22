/*
 * Quake 3 BSP loader header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENELOADER_BSP3_H__
#define __SP_SCENELOADER_BSP3_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_SCENELOADER_BSP3


#include "Base/spInputOutputLog.hpp"
#include "Base/spInputOutputFileSystem.hpp"
#include "Base/spDimension.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "FileFormats/Mesh/spMeshLoader.hpp"

#include <vector>


namespace sp
{
namespace scene
{


//! This is the "Quake III Arena" BSP scene loader.
class SP_EXPORT SceneLoaderBSP3 : public SceneLoader
{
    
    public:
        
        SceneLoaderBSP3();
        ~SceneLoaderBSP3();
        
        Mesh* loadScene(const io::stringc &Filename, const io::stringc &TexturePath, const s32 Flags);
        
        /**
        Sets the tessellation factor for bezier patchs.
        Since version 2.1 this factor needs to be set with the SceneLoaderBSP3 class because tessellation
        is only used in the BSP3 (Quake III Arean) file format.
        */
        static void setTessellation(s32 Tessellation);
        
    private:
        
        /* ===== Enumerations ===== */
        
        enum ELumpTypes
        {
            BSP_LUMP_ENTITIES = 0,
            BSP_LUMP_TEXTURES,
            BSP_LUMP_PLANES,
            BSP_LUMP_NODES,
            BSP_LUMP_LEAFS,
            BSP_LUMP_LEAFFACES,
            BSP_LUMP_LEAFBRUSHES,
            BSP_LUMP_MODELS,
            BSP_LUMP_BRUSHES,
            BSP_LUMP_BRUSHSIDES,
            BSP_LUMP_VERTEXES,
            BSP_LUMP_MESHVERTS,
            BSP_LUMP_EFFECTS,
            BSP_LUMP_FACES,
            BSP_LUMP_LIGHTMAPS,
            BSP_LUMP_LIGHTVOLS,
            BSP_LUMP_VISDATA,
        };
        
        enum EFaceTypes
        {
            BSP_FACE_POLYGON = 1,   // Triangle fan
            BSP_FACE_PATCH,         // Triangle strip
            BSP_FACE_MESH,          // Triangle list
            BSP_FACE_BILLBOARD
        };
        
        enum ELightVolsDirectionTypes
        {
            BSP_LMDIR_PHI = 0,
            BSP_LMDIR_THETA
        };
        
        /* ===== Structures ===== */
        
        struct SDirEntryBSP
        {
            s32 Offset;
            s32 Length;
        };
        
        struct SHeaderBSP
        {
            c8 Magic[4];
            s32 Version;
            SDirEntryBSP DirEntries[17];
        };
        
        struct SNodeBSP
        {
            s32 Plane;
            s32 Children[2];
            dim::line3di BoundingBox;
        };
        
        struct SLeafBSP
        {
            s32 Cluster;
            s32 Area;
            dim::line3di BoundingBox;
            s32 LeafFace;
            s32 CountLeafFaces;
            s32 LeafBrush;
            s32 CountLeafBrushes;
        };
        
        struct SModelBSP
        {
            dim::line3df BoundingBox;
            s32 Face;
            s32 CountFaces;
            s32 Brush;
            s32 CountBrushes;
        };
        
        struct SBrushBSP
        {
            s32 BrushSide;
            s32 CountBrushSides;
            s32 Texture;
        };
        
        struct SBrushSideBSP
        {
            s32 Plane;
            s32 Texture;
        };
        
        struct SVertexBSP
        {
            /* Members */
            
            dim::vector3df Position;
            dim::point2df TexCoord[2];
            dim::vector3df Normal;
            video::color Color;
            
            /* Operators */
            
            SVertexBSP operator + (const SVertexBSP &other) const
            {
                SVertexBSP Result;
                
                Result.Position     = Position      + other.Position;
                Result.TexCoord[0]  = TexCoord[0]   + other.TexCoord[0];
                Result.TexCoord[1]  = TexCoord[1]   + other.TexCoord[1];
                
                return Result;
            }
            
            SVertexBSP operator * (const f32 size) const
            {
                SVertexBSP Result;
                
                Result.Position     = Position      * size;
                Result.TexCoord[0]  = TexCoord[0]   * size;
                Result.TexCoord[1]  = TexCoord[1]   * size;
                
                return Result;
            }
        };
        
        struct SEffectBSP
        {
            c8 Name[64];
            s32 Brush;
            s32 Unknown;
        };
        
        struct SFaceBSP
        {
            s32 Texture;
            s32 Effect;
            s32 Type; // EFaceTypes
            s32 FirstVertex;
            s32 CountVertices;
            s32 FirstMeshVerts;
            s32 CountMeshVerts;
            s32 LightMap;
            dim::point2di LightMapStart;
            dim::size2di LightMapSize;
            dim::vector3df LightMapSpaceOrigin;
            dim::vector3df LightMapUnitVectors[2];
            dim::vector3df SurfaceNormal;
            dim::size2di PatchSize;
        };
        
        struct SLightMapBSP
        {
            u8 LightMap[128][128][3];
        };
        
        struct SLightVolsBSP
        {
            u8 Ambient[3];
            u8 Directional[3];
            u8 Direction[2]; // ELightVolsDirectionTypes
        };
        
        struct SVisDataBSP
        {
            s32 CountVectors;
            s32 VectorSize;
            u8* VisibilityData;
        };
        
        /* ===== Classes ===== */
        
        class BiQuadraticPatch
        {
            
            public:
                
                BiQuadraticPatch();
                ~BiQuadraticPatch();
                
                /* Functions */
                
                void tessellate(s32 NewLevel);
                
                /* Members */
                
                SVertexBSP ControlPoints[9];
                
                std::vector<SVertexBSP> VerticesList;
                std::vector< std::vector<u32> > IndicesList;
                
        };
        
        /* ===== Functions ===== */
        
        bool readHeader();
        void readLumps();
        
        void readLumpEntities();
        void readLumpTextures();
        void readLumpPlanes();
        void readLumpNodes();
        void readLumpLeafs();
        void readLumpLeafFaces();
        void readLumpLeafBrushes();
        void readLumpModels();
        void readLumpBrushes();
        void readLumpBrushSides();
        void readLumpVertexes();
        void readLumpMeshVerts();
        void readLumpEffecs();
        void readLumpFaces();
        void readLumpLightMaps();
        void readLumpLightVols();
        void readLumpVisData();
        
        void createNewVertex(SVertexBSP &Vertex, SFaceBSP &Face);
        
        void buildModel();
        
        void examineScript(std::vector<io::stringc> &ScriptData);
        
        io::stringc getScriptLineType(io::stringc &Line);
        io::stringc getScriptLineValue(io::stringc &Line);
        dim::vector3df getScriptLineVector(io::stringc &Line, io::stringc Value);
        
        /* ===== Members ===== */
        
        SHeaderBSP Header_;
        
        /* Lists for map creating */
        std::vector<video::Texture*> TextureList_;
        std::vector<video::Texture*> LightMapList_;
        std::vector<dim::plane3df> PlaneList_;
        std::vector<SVertexBSP> VerticesList_;
        std::vector<SFaceBSP> FacesList_;
        std::vector<s32> MeshVertOffsetList_;
        
        static s32 Tessellation_;
        
};


/*
 * BSPLoaderExtensions class
 */

class BSPLoaderExtensions
{
    
    public:
        
        BSPLoaderExtensions();
        ~BSPLoaderExtensions();
        
        static void createScript(std::vector<io::stringc> &ScriptData, c8* MeshDescription);
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ==========================================================================
