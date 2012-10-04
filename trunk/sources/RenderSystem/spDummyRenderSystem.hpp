/*
 * Dummy render system header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERSYSTEM_DUMMY_H__
#define __SP_RENDERSYSTEM_DUMMY_H__


#include "Base/spStandard.hpp"
#include "RenderSystem/spRenderSystem.hpp"

#ifdef SP_PLATFORM_WINDOWS
#   include <windows.h>
#endif
#include <vector>


namespace sp
{
namespace video
{


/**
Dummy renderer (or rather null device). This is a renderer which will output nothing graphical.
It's designed for debugging or other none-graphical purposes.
*/
class SP_EXPORT DummyRenderSystem : public RenderSystem
{
    
    public:
        
        DummyRenderSystem();
        ~DummyRenderSystem();
        
        /* === Initialization functions === */
        
        void setupConfiguration();
        
        /* === Renderer information === */
        
        io::stringc getRenderer()           const;
        io::stringc getVersion()            const;
        io::stringc getVendor()             const;
        io::stringc getShaderVersion()      const;
        
        bool queryVideoSupport(const EVideoFeatureQueries Query) const;
        
        s32 getMultitexCount()              const;
        s32 getMaxAnisotropicFilter()       const;
        s32 getMaxLightCount()              const;
        
        /* === Video buffer control functions === */
        
        void clearBuffers(const s32 ClearFlags = BUFFER_COLOR | BUFFER_DEPTH);
        void flipBuffers();
        
        /* === Rendering functions === */
        
        void setupMaterialStates(const MaterialStates* Material);
        
        void createVertexBuffer(void* &BufferID);
        void createIndexBuffer(void* &BufferID);
        
        void deleteVertexBuffer(void* &BufferID);
        void deleteIndexBuffer(void* &BufferID);
        
        void updateVertexBuffer(
            void* BufferID, const dim::UniversalBuffer &BufferData, const VertexFormat* Format, const EMeshBufferUsage Usage
        );
        void updateIndexBuffer(
            void* BufferID, const dim::UniversalBuffer &BufferData, const IndexFormat* Format, const EMeshBufferUsage Usage
        );
        
        void updateVertexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index);
        void updateIndexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index);
        
        void drawMeshBuffer(const MeshBuffer* MeshBuffer);
        
        void setRenderState(const video::ERenderStates Type, s32 State);
        s32 getRenderState(const video::ERenderStates Type) const;
        
        /* === Texture loading and creating === */
        
        Texture* createTexture(const STextureCreationFlags &CreationFlags);
        
        /* === Matrix controll === */
        
        void updateModelviewMatrix();
        
    private:
        
        /* Functions */
        
        void clear();
        
        /* Members */
        
        s32 RenderStates_[16];
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
