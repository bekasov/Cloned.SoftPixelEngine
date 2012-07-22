/*
 * Mesh saver SPM header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHSAVER_SPM_H__
#define __SP_MESHSAVER_SPM_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MESHSAVER_SPM


#include "FileFormats/Mesh/spMeshLoaderSPM.hpp"
#include "FileFormats/Mesh/spMeshSaver.hpp"


namespace sp
{
namespace scene
{


class SP_EXPORT MeshSaverSPM : public MeshSaver
{
    
    public:
        
        MeshSaverSPM();
        ~MeshSaverSPM();
        
        bool saveMesh(Mesh* Model, const io::stringc &Filename);
        
        /* Static functions */
        
        static void setTextureIntern(bool isWriteIntern);
        static bool getTextureIntern();
        
    private:
        
        /* Functions */
        
        void writeHeader();
        
        void writeChunkObject();
        void writeChunkSubMesh(Mesh* SubMesh);
        void writeChunkSurface(const u32 Surface);
        void writeChunkVertex(const u32 Vertex);
        void writeChunkTriangle(const u32 Triangle);
        void writeChunkTexture(const u8 Layer);
        
        void writeChunkAnimationNode();
        void writeChunkAnimationMorphTarget();
        void writeChunkAnimationSkeletal();
        void writeChunkAnimationJoint(const scene::AnimationJoint* Joint);
        
        bool areIndex32BitNeeded() const;
        bool areVertexColorsEqual() const;
        bool areVertexFogCoordsEqual() const;
        
        void checkTexCoordsDimensions();
        
        /* Members */
        
        Mesh* CurMesh_;
        
        bool areIndices32Bit_;
        bool areVertexColorsEqual_;
        bool areVertexFogCoordsEqual_;
        bool isGouraudShading_;
        
        u8 TexCoordsDimensions_[MAX_COUNT_OF_TEXTURES];
        
        u8 TexLayerCount_;
        
        static bool isTextureIntern_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
