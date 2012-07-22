/*
 * Mesh loader SPM header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHLOADER_SPM_H__
#define __SP_MESHLOADER_SPM_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_SPM


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


static const s32 SPM_MAGIC_NUMBER   = *((s32*)"SPMD");  // SoftPixelMoDel
static const u16 SPM_VERSION_MIN_NR = 0x2000;
static const u16 SPM_VERSION_NUMBER = 0x2100;           // v.2.1

enum EModelSPMChunkFlags
{
    MDLSPM_CHUNK_NONE               = 0x0000,
    
    // Mesh flags
    MDLSPM_CHUNK_GOURAUDSHADING     = 0x0008,
    MDLSPM_CHUNK_NODE_ANIM          = 0x0010,
    MDLSPM_CHUNK_MORPHTARGET_ANIM   = 0x0020,
    MDLSPM_CHUNK_SKELETAL_ANIM      = 0x0040,
    
    // Surface flags
    MDLSPM_CHUNK_INDEX32BIT         = 0x0001,
    MDLSPM_CHUNK_VERTEXCOLOR        = 0x0002,
    MDLSPM_CHUNK_VERTEXFOG          = 0x0004,
    MDLSPM_CHUNK_VERTEXNORMAL       = 0x0008,
    
    // Texture flags
    MDLSPM_CHUNK_TEXTUREINTERN      = 0x0010,
    MDLSPM_CHUNK_TEXTUREMATRIX      = 0x0020,
};


class SP_EXPORT MeshLoaderSPM : public MeshLoader
{
    
    public:
        
        MeshLoaderSPM();
        ~MeshLoaderSPM();
        
        Mesh* loadMesh(const io::stringc &Filename, const io::stringc &TexturePath);
        
    private:
        
        /* === Structures === */
        
        struct SVertexWeightSPM
        {
            SVertexWeightSPM()
                : Surface(0), Index(0), Weight(0.0f)
            {
            }
            ~SVertexWeightSPM()
            {
            }
            
            /* Members */
            u32 Surface;    //!< Surface index.
            u32 Index;      //!< Vertex index.
            f32 Weight;     //!< Weight factor.
        };
        
        struct SKeyframeSPM
        {
            SKeyframeSPM() : Frame(0)
            {
            }
            ~SKeyframeSPM()
            {
            }
            
            /* Members */
            u32 Frame;
            dim::vector3df Position;
            dim::quaternion Rotation;
            dim::vector3df Scale;
        };
        
        struct SJointSPM
        {
            SJointSPM() : Parent(-1), JointObject(0)
            {
            }
            ~SJointSPM()
            {
            }
            
            /* Members */
            io::stringc Name;
            s32 Parent;                 //!< Parent joint index. No parent if -1.
            
            dim::vector3df Position;
            dim::quaternion Rotation;
            dim::vector3df Scale;
            
            scene::AnimationJoint* JointObject;
            std::list<SVertexWeightSPM> VertexWeights;
            std::list<SKeyframeSPM> Keyframes;
        };
        
        /* === Functions === */
        
        bool readHeader();
        
        void readChunkObject();
        void readChunkSubMesh(Mesh* SubMesh);
        void readChunkSurface();
        void readChunkVertex(u32 Index);
        void readChunkTriangle(u32 Index);
        void readChunkTexture();
        
        void readChunkAnimationNode();
        void readChunkAnimationMorphTarget();
        void readChunkAnimationSkeletal();
        void readChunkAnimationJoint(SJointSPM &Joint);
        
        /* === Members === */
        
        Mesh* CurMesh_;
        
        bool has32BitIndices_;
        bool hasVertexColors_;
        bool hasVertexFogCoords_;
        bool hasVertexNormals_;
        
        u8 TexCoordsDimensions_[MAX_COUNT_OF_TEXTURES];
        video::color DefaultVertexColor_;
        f32 DefaultVertexFogCoord_;
        
        u8 TexLayerCount_;
        
        std::vector<SJointSPM> Joints_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
