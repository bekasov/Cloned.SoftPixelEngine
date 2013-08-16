/*
 * Light grid header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_LIGHT_GRID_H__
#define __SP_LIGHT_GRID_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spTextureBase.hpp"


namespace sp
{

namespace scene
{
    class Light;
    class Camera;
    class SceneGraph;
}

namespace video
{


class ShaderResource;
class ShaderClass;

/**
The light grid is used by the deferred renderer for tiled shading.
\since Version 3.3
*/
class SP_EXPORT LightGrid
{
    
    public:
        
        LightGrid();
        ~LightGrid();
        
        /* === Functions === */
        
        /**
        Creates the light grid.
        \param[in] Resolution Specifies the resolution. This should be the same as specified for the engine's graphics device.
        \param[in] TileCount Specifies the count of tiles on the X and Y axes.
        \param[in] MaxNumLights Specifies the maximal number of lights.
        \return True if the grid could be created successful.
        */
        bool createGrid(const dim::size2di &Resolution, const dim::size2di &TileCount, u32 MaxNumLights);
        void deleteGrid();
        
        /**
        Updates the data for the light raw models (position, radius etc. but no color).
        \param[in] PointLights Specifies the list of all point lights. Each element is a 4D vector
        containing the position (XYZ) and radius (W).
        \param[in] LightCount Specifies how many lights from the list are to be used.
        */
        void updateLights(const std::vector<dim::vector4df> &PointLights, u32 LightCount);

        /**
        Builds the light grid. For Direct3D 11 this function uses a compute shader.
        Otherwise the grid will be computed on the CPU.
        */
        void build(scene::SceneGraph* Graph, scene::Camera* ActiveCamera);

        //! Binds the TLI texture.
        s32 bind(s32 TexLayerBase);
        //! Unbinds the TLI texture.
        s32 unbind(s32 TexLayerBase);

        /* === Inline functions === */
        
        //! Returns the TLI (Tile Light Index List) texture object. This is an signed integer texture buffer.
        inline Texture* getTLITexture() const
        {
            return TLITexture_;
        }

        /**
        Returns the LG (Light Grid) shader resource object. This is an 'uint shader buffer'.
        \code
        // HLSL Example:
        Buffer<uint> MyLGBuffer : register(t0);

        // GLSL Example:
        layout(std430, binding = 0) buffer MyLGBuffer
        {
            uint Offset;
        }
        \endcode
        */
        inline ShaderResource* getLGShaderResource() const
        {
            return LGShaderResourceOut_;
        }
        /**
        Returns the TLI (Tile Light Index List) shader resource object. This is a 'structured shader buffer'.
        \code
        // HLSL Example:
        struct SLightNode
        {
            uint LightID;
            uint Next;
        };
        StructuredBuffer<SLightNode> MyTLIBuffer : register(t2);

        // GLSL Example:
        layout(std430, binding = 2) buffer MyTLIBuffer
        {
            uint LightID;
            uint Next;
        }
        \endcode
        */
        inline ShaderResource* getTLIShaderResource() const
        {
            return TLIShaderResourceOut_;
        }
        
        /**
        Returns the count of tiles on X and Y axes. This can only be set on light-grid creation time.
        \see createGrid
        */
        inline const dim::size2di& getTileCount() const
        {
            return TileCount_;
        }
        /**
        Returns the grid size. This will be computed on light-grid creation time.
        It depends on the count of tiles on X and Y axes and the specified resolution.
        \see createGrid
        */
        inline const dim::size2di& getGridSize() const
        {
            return GridSize_;
        }

    private:
        
        /* === Functions === */
        
        bool createTLITexture();

        bool createShaderResources(u32 MaxNumLights);
        bool createComputeShaders();

        void buildOnGPU(scene::SceneGraph* Graph, scene::Camera* Cam);
        void buildOnCPU(scene::SceneGraph* Graph, scene::Camera* Cam);

        dim::size2di getGridSize(const dim::size2di &Resolution, const dim::size2di &TileCount) const;

        /* === Members === */
        
        //! This is a texture buffer storing the light indicies. Currently used for OpenGL.
        Texture* TLITexture_;
        
        ShaderResource* LGShaderResourceOut_;
        ShaderResource* LGShaderResourceIn_;

        //! This is a shader resource storing the light indicies. Currently used for Direct3D 11. OpenGL will follow.
        ShaderResource* TLIShaderResourceOut_;
        //! This is the shader resource filled by the compute shader. This is private only.
        ShaderResource* TLIShaderResourceIn_;
        
        //! Shader class for building the tile-light-index list buffer.
        ShaderClass* ShdClass_;
        //! Shader class for initializing the light-grid buffer.
        ShaderClass* ShdClassInit_;

        dim::size2di TileCount_;
        dim::size2di GridSize_;

        u32 LightCount_;

};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
