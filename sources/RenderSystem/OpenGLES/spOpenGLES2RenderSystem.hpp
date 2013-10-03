/*
 * OpenGL|ES 2 render system header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERER_OPENGLES2_H__
#define __SP_RENDERER_OPENGLES2_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGLES2)


#include "RenderSystem/OpenGL/spOpenGLPipelineProgrammable.hpp"
#include "RenderSystem/OpenGLES/spOpenGLES2Texture.hpp"


namespace sp
{
namespace video
{


//! OpenGL|ES 2 render system. This renderer supports OpenGL|ES 2.0.
class SP_EXPORT OpenGLES2RenderSystem : public GLProgrammableFunctionPipeline
{
    
    public:
        
        OpenGLES2RenderSystem();
        ~OpenGLES2RenderSystem();
        
        /* === Render system information === */
        
        io::stringc getVersion() const;
        
        s32 getMultitexCount() const;
        s32 getMaxLightCount() const;
        
        bool queryVideoSupport(const EVideoFeatureSupport Query) const;
        
        /* === Context functions === */
        
        void setupConfiguration();
        
        /* === Render states === */
        
        void setRenderState(const video::ERenderStates Type, s32 State);
        s32 getRenderState(const video::ERenderStates Type) const;
        
        void disableTriangleListStates();
        void disable3DRenderStates();
        
        /* === Rendering functions === */
        
        bool setupMaterialStates(const MaterialStates* Material, bool Forced = false);
        
        /* === Hardware mesh buffers === */
        
        //! \todo Not implemented yet
        bool bindMeshBuffer(const MeshBuffer* Buffer);
        //! \todo Not implemented yet
        void unbindMeshBuffer();
        //! \todo Not implemented yet
        void drawMeshBufferPart(const MeshBuffer* Buffer, u32 StartOffset, u32 NumVertices);
        void drawMeshBuffer(const MeshBuffer* MeshBuffer);
        
        /* === Image drawing === */
        
        // todo
        
        /* === Primitive drawing === */
        
        // todo
        
        /* === Extra drawing functions === */
        
        // todo
        
        /* === 3D drawing functions === */
        
        // todo
        
        /* === Matrix controll === */
        
        void updateModelviewMatrix();

};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
