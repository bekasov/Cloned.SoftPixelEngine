/*
 * Mesh loader B3D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHLOADER_B3D_H__
#define __SP_MESHLOADER_B3D_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_B3D


#include "Base/spInputOutputLog.hpp"
#include "Base/spInputOutputFileSystem.hpp"
#include "Base/spDimension.hpp"
#include "FileFormats/Mesh/spMeshLoader.hpp"

#include <vector>


namespace sp
{
namespace scene
{


class SP_EXPORT MeshLoaderB3D : public MeshLoader
{
    
    public:
        
        MeshLoaderB3D();
        ~MeshLoaderB3D();
        
        Mesh* loadMesh(const io::stringc &Filename, const io::stringc &TexturePath);
        
    private:
        
        /* === Structures === */
        
        struct SBrushSurfaceB3D
        {
            s32 BrushID;
            video::MeshBuffer* Surface;
        };
        
        struct STextureSurfaceB3D
        {
            video::Texture* hTexture;
            dim::point2df Pos;
            dim::point2df Scale;
            bool isSphereMapping;
        };
        
        struct SSkeletonBoneB3D
        {
            // Constructor & destructor
            SSkeletonBoneB3D()
                : VerticesList(0), WeightsCount(0)
            {
                TabSize         = 0;
                ParentBoneID    = 0;
                Scale           = 1.0f;
            }
            ~SSkeletonBoneB3D()
            {
                // Delete all memories
                MemoryManager::deleteBuffer(VerticesList);
            }
            
            // General data
            s32 TabSize; // (later important to get the correct parnet bone)
            u32 ParentBoneID;
            io::stringc Name;
            
            dim::vector3df Translation, Scale;
            dim::quaternion Quaternion;
            
            // Bone vertex data
            struct SBoneVertexInfo
            {
                u32 Surface;
                u32 Index;
                f32 Weight;
            };
            
            SBoneVertexInfo* VerticesList;
            s32 WeightsCount;
            
            // Bone keyframe data
            struct SBoneKeyframeInfo
            {
                SBoneKeyframeInfo() : Frame(0), Scale(1.0f)
                {
                }
                ~SBoneKeyframeInfo()
                {
                }
                
                u32 Frame;
                
                dim::vector3df Position;
                dim::vector3df Scale;
                dim::quaternion Rotation;
            };
            
            std::map<u32, SBoneKeyframeInfo> KeyframeList;
        };
        
        struct SVertexB3D
        {
            u32 SurfaceNr;
            video::MeshBuffer* Surface;
            u32 SurfVertexID;
            dim::vector3df Coord;
            dim::vector3df Normal;
            dim::point2df TexCoord;
            video::color Color;
        };
        
        struct SJointParent
        {
            AnimationJoint* Joint;
            u32 ParentID;
        };
        
        /* === Functions === */
        
        io::stringc readChunk();
        void breakChunk();
        s32 getChunkSize();
        void readChunkBlock(io::stringc Tab = "");
        
        bool readChunkANIM();
        bool readChunkKEYS();
        bool readChunkTEXS();
        bool readChunkBRUS();
        bool readChunkVRTS();
        bool readChunkTRIS();
        bool readChunkMESH();
        bool readChunkBONE(io::stringc &Tab);
        bool readChunkNODE();
        
        video::Texture* loadChunkTexture(io::stringc Filename);
        
        void updateTexturing();
        
        void buildAnimation();
        
        bool loadModelData();
        
        /* === Members === */
        
        std::vector<STextureSurfaceB3D> TextureList_;
        std::vector<SBrushSurfaceB3D> BrushSurfaceList_;
        
        std::vector<SSkeletonBoneB3D*> AnimBoneList_;
        
        io::stringc CurName_;
        
        dim::vector3df Position_, Scale_;
        dim::quaternion Quaternion_;
        
        std::vector<s32> Stack_;
        s32 CurPos_;
        
        // Building information
        
        std::vector<SVertexB3D> VerticesList_;
        std::vector<s32> BrushTextureList_;
        
        s32 CurBone_;
        s32 CurBrushID_;
        dim::matrix4f CurTransformation_, CurRotation_;
        
        s32 AnimKeyframeCount_;
        f32 AnimFPS_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
