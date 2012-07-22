/*
 * Mesh loader MS3D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHLOADER_MS3D_H__
#define __SP_MESHLOADER_MS3D_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_MS3D


#include "Base/spInputOutputLog.hpp"
#include "Base/spDimension.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "FileFormats/Mesh/spMeshLoader.hpp"

#include <ios>
#include <vector>
#include <string>
#include <fstream>


namespace sp
{
namespace scene
{


class SP_EXPORT MeshLoaderMS3D : public MeshLoader
{
    
    public:
        
        MeshLoaderMS3D();
        ~MeshLoaderMS3D();
        
        Mesh* loadMesh(const io::stringc &Filename, const io::stringc &TexturePath);
        
    private:
        
        /* === Structures === */
        
        struct SMeshMS3D // Mesh
        {
            s32 MaterialIndex;
            s32 CountOfTriangles;
            s32* pTriangleIndices;
        };
        
        struct SMaterialMS3D // Material properties
        {
            f32 Ambient[4], Diffuse[4], Specular[4], Emissive[4];
            f32 Shininess;
            f32 Transparency;
            video::Texture* hTexture;
            c8* TextureFilename;
        };
        
        struct STriangleMS3D // Triangle structure
        {
            f32 VertexNormals[3][3];
            f32 TexCoordU[3];
            f32 TexCoordV[3];
            s32 Vertices[3];
        };
        
        struct SVertexMS3D // Vertex structure
        {
            s8 BoneID;
            dim::vector3df Position;
        };
        
        // Bone/ Skeletal animation
        
        struct SKeyframeMS3D
        {
            s32 JointIndex;
            f32 Time;
            dim::vector3df Vector; // translation or rotation
        };
        
        struct SJointMS3D
        {
            io::stringc Name;
            s32 Index, Parent;
            
            dim::vector3df Rotation, Translation;
            
            std::vector<SKeyframeMS3D> RotationKeyframes;
            std::vector<SKeyframeMS3D> TranslationKeyframes;
        };
        
        struct SJointNameListRecMS3D
        {
            s32 JointIndex;
            const char* Name;
        };
        
        /* === Functions === */
        
        bool loadModelData(io::stringc Filename);
        void buildAnimation();
        Mesh* buildModel();
        
        void addJointKeyframe(
            const s32 JointIndex, SJointMS3D &Joint, void* KeyframeBuffer, bool isTypeRotation
        );
        void fillBoneWeights(
            const s32 JointIndex, std::vector<SVertexGroup> &VertexGroups
        );
        
        bool setNextBufferPos(const u8* &BufferPtr, s32 NextPos);
        
        dim::matrix4f getAnimRotation(dim::vector3df Rotation);
        
        /* Inline functions */
        
        inline video::color getArrayColor(f32 Clr[4])
        {
            video::color Result;
            Result.setFloatArray(Clr);
            return Result;
        }
        
        /* === Members === */
        
        // Meshes
        s32 CountOfMeshes_;
        SMeshMS3D* pMeshes_;
        
        // Materials
        s32 CountOfMaterials_;
        SMaterialMS3D* pMaterials_;
        
        // Triangles
        s32 CountOfTriangles_;
        STriangleMS3D* pTriangles_;
        
        // Vertices
        s32 CountOfVertcies_;
        SVertexMS3D* pVertices_;
        
        // Joints (for bone/ skeletal animation)
        s32 CountOfJoints_;
        SJointMS3D* pJoints_;
        SJointNameListRecMS3D* pNameList_;
        
        f64 TotalTime_;
        
        f32 TotalFrames_, FPS_;
        
        s32 FileSize_;
        u8* pBuffer_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
