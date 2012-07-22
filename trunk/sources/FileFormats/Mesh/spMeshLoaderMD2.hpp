/*
 * Mesh loader MD2 header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHLOADER_MD2_H__
#define __SP_MESHLOADER_MD2_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_MD2


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


enum EMD2AnimTypes
{
    MD2ANIM_STAND = 0,
    MD2ANIM_RUN,
    MD2ANIM_ATTACK,
    MD2ANIM_PAIN_A,
    MD2ANIM_PAIN_B,
    MD2ANIM_PAIN_C,
    MD2ANIM_JUMP,
    MD2ANIM_FLIP,
    MD2ANIM_SALUTE,
    MD2ANIM_FALLBACK,
    MD2ANIM_WAVE,
    MD2ANIM_POINT,
    MD2ANIM_CROUCH_STAND,
    MD2ANIM_CROUCH_WALK,
    MD2ANIM_CROUCH_ATTACK,
    MD2ANIM_CROUCH_PAIN,
    MD2ANIM_CROUCH_DEATH,
    MD2ANIM_DEATH_FALLBACK,
    MD2ANIM_DEATH_FALLFORWARD,
    MD2ANIM_DEATH_FALLBACKSLOW,
    MD2ANIM_BOOM,
    
    MD2ANIM_MAX_ANIMATIONS
};


class SP_EXPORT MeshLoaderMD2 : public MeshLoader
{
    
    public:
        
        MeshLoaderMD2();
        ~MeshLoaderMD2();
        
        Mesh* loadMesh(const io::stringc &Filename, const io::stringc &TexturePath);
        
        static void getAnimationSeq(s32 Type, u32 &Start, u32 &End);
        
    private:
        
        /* === Macros == */
        
        static const s32 MD2_SHADEDOT_QUANT = 16;
        static const s32 MD2_VERTEX_NORMALS = 162;
        
        /* === Structures === */
        
        struct SHeaderMD2
        {
            s32 ID;                 // Magic number (must be "IDP2")
            s32 Version;            // MD2 version (must be 8)
            
            s32 SkinWidth;          // Width of the texture
            s32 SkinHeight;         // Height of the texture
            s32 FrameSize;          // Size of one frame in bytes
            
            s32 CountOfTextures;
            s32 CountOfVertices;
            s32 CountOfTexCoords;
            s32 CountOfTriangles;
            s32 CountOfCommands;
            s32 CountOfKeyframes;
            
            s32 TextureOffset;
            s32 TexCoordOffset;
            s32 TriangleOffset;
            s32 KeyframeOffset;
            s32 CommandOffset;
            s32 EndOfFileOffset;
        };
        
        struct SVertexMD2
        {
            dim::vector3d<u8> Vertex;
            u8 LightNormalIndex;
        };
        
        struct SKeyFrameMD2
        {
            dim::vector3df Scale;
            dim::vector3df Translate;
            s8 Name[16];
            SVertexMD2 Vertices[1];
        };
        
        struct STriangleMD2
        {
            s16 IndexVertex[3];
            s16 IndexTexCoord[3];
        };
        
        struct SAnimationMD2
        {
            s32 FirstFrame; // first frame of animation
            s32 LastFrame;  // number of frames
            s32 FPS;        // number of frames
        };
        
        struct SAnimStateMD2
        {
            s32 StartFrame;
            s32 EndFrame;
            s32 FPS;
            
            f32 CurTime;
            f32 OldTime;
            f32 Interpol;   // percent of interpolation
            
            s32 Type;       // animation type
            s32 CurFrame;
            s32 NextFrame;
        };
        
        /* === Private functions === */
        
        void init();
        void clear();
        
        bool loadModelData(io::stringc Filename);
        void buildAnimation();
        Mesh* buildModel();
        
        void interpolate(dim::vector3df* CoordList, dim::vector3df* NormalList);
        
        /* === Members === */
        
        s32 KeyFramesCount_;
        s32 VerticesCount_;
        s32 CommandsCount_;
        
        dim::vector3df* pVertices_;
        s32* pCommands_;
        u8* pLightNormals_;
        
        SAnimStateMD2 Animation_;
        f32 Scale_;
        
        //static dim::vector3df Anorms_[MD2_VERTEX_NORMALS];
        //static f32 AnormsDots_[MD2_SHADEDOT_QUANT][256];
        static SAnimationMD2 AnimList_[21];
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
