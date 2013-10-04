/*
 * Render system header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERSYSTEM_H__
#define __SP_RENDERSYSTEM_H__


#include "Base/spStandard.hpp"
#include "Base/spMaterialStates.hpp"
#include "Base/spImageManagement.hpp"
#include "Base/spTransformation2D.hpp"
#include "Base/spMathSpline.hpp"
#include "Base/spVertexFormatList.hpp"
#include "Base/spIndexFormat.hpp"
#include "Base/spInputOutput.hpp"
#include "Base/spDimension.hpp"
#include "Base/spGeometryStructures.hpp"
#include "Base/spMaterialColor.hpp"
#include "FileFormats/Image/spImageFileFormats.hpp"
#include "RenderSystem/spShaderProgram.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "RenderSystem/spRenderSystemMovie.hpp"
#include "RenderSystem/spRenderSystemFont.hpp"
#include "RenderSystem/spQuery.hpp"
#include "SceneGraph/spSceneLight.hpp"

#include <map>
#include <boost/shared_ptr.hpp>


namespace sp
{

class SoftPixelDevice;

#if defined(SP_PLATFORM_WINDOWS)
class SoftPixelDeviceWin32;
#endif

#if defined(SP_PLATFORM_LINUX)
class SoftPixelDeviceLinux;
#endif

namespace scene
{

class Camera;
class Mesh;
class MaterialNode;
class SceneGraph;

} // /namespace scene

namespace video
{


class Query;
class TextureLayer;
class RenderContext;
class ShaderResource;

#ifdef SP_COMPILE_WITH_DIRECT3D11
class Direct3D11RenderContext;
#endif


//! Fog states structured.
struct SFogStates
{
    SFogStates() :
        Type    (FOG_NONE   ),
        Mode    (FOG_PALE   ),
        Range   (0.0f       ),
        Near    (0.0f       ),
        Far     (0.0f       )
    {
    }
    ~SFogStates()
    {
    }
    
    /* Members */
    EFogTypes Type;
    EFogModes Mode;
    f32 Range, Near, Far;
    video::color Color;
};

//! Depth range structure.
struct SDepthRange
{
    SDepthRange() :
        Enabled (true),
        Near    (0.0f),
        Far     (1.0f)
    {
    }
    ~SDepthRange()
    {
    }
    
    /* Members */
    bool Enabled;
    f32 Near, Far;
};


/**
RenderSystem class used for all graphic operations such as drawing 2D, rendering 3D, shader programs etc.
Since version 2.2 the SoftPixel Engine does no longer support a software renderer because of irrelevance.
The supported render systems are OpenGL (1.1 - 4.1), Direct3D 9 and Direct3D 11.
*/
class SP_EXPORT RenderSystem
{
    
    public:
        
        virtual ~RenderSystem();
        
        /* === Initialization functions === */
        
        virtual void setupConfiguration() = 0;
        
        /* === Renderer information === */
        
        static io::stringc getVendorNameByID(const u32 VendorID);
        
        virtual io::stringc getRenderer()       const = 0;  //!< Returns the renderer hardware (e.g. "NVIDIA GeForce 8800 GTX").
        virtual io::stringc getVersion()        const = 0;  //!< Returns the renderer API version (e.g. "OpenGL 3.2" or "Direct3D9.0c").
        virtual io::stringc getVendor()         const = 0;  //!< Returns the graphics hardware vendor (e.g. "NVIDIA Corporation" or "ATI Technologies Inc.").
        virtual io::stringc getShaderVersion()  const = 0;  //!< Returns the shader version (e.g. "1.50 NVIDIA via Cg compiler" or "HLSL Shader Model 2").
        
        virtual bool queryVideoSupport(const EVideoFeatureSupport Query) const = 0; //!< Returns true if the specified feature is supported by the renderer.
        
        virtual s32 getMultitexCount()          const = 0;  //!< Returns the count of maximal texture layers for the fixed-function-pipeline.
        virtual s32 getMaxAnisotropicFilter()   const = 0;  //!< Returns the count of maximal anisotropic texture filter sampling.
        virtual s32 getMaxLightCount()          const = 0;  //!< Returns the count of maximal light sources for the fixed-function-pipeline.
        
        virtual dim::EMatrixCoordinateSystmes getProjectionMatrixType() const;
        
        virtual void printWarning();
        
        /* === Video buffer control functions === */
        
        /**
        Clears the specified buffers.
        \param[in] ClearFlags Specifies which buffers are to be cleared.
        This can be a combination of the following types defined in the 'EClearBufferTypes' enumeration.
        If your 3D scene has a skybox or the background is not visible because of
        any other reason you should clear only the depth buffer to speed up your application.
        \see EClearBufferTypes
        */
        virtual void clearBuffers(const s32 ClearFlags = BUFFER_COLOR | BUFFER_DEPTH) = 0;
        
        /* === Configuration functions === */
        
        //! Sets the shading mode for primitives (where primitives are points, lines and triangles).
        virtual void setShadeMode(const EShadeModeTypes ShadeMode);
        
        //! Sets triangle rendering to CW (clock-wise -> false) or CCW (counter-clock-wise -> true).
        virtual void setFrontFace(bool isFrontFace);
        
        /**
        Sets the color with which the color-buffer is to be cleared.
        \note This affects every render context. So don't call this every frame when you have several render contexts!
        \see RenderContext
        */
        virtual void setClearColor(const color &Color);
        
        //! Enables or disables the color components which are to be written for rendering operations.
        virtual void setColorMask(bool isRed, bool isGreen, bool isBlue, bool isAlpha = true);
        
        //! Enables or disables the depth component which is to be written for rendering operations.
        virtual void setDepthMask(bool Enable);
        
        //! Enables or disables anti-aliasing or multi-sampling.
        virtual void setAntiAlias(bool isAntiAlias);
        
        /**
        Sets the depth range.
        \param Near: Specifies the near clipping plane. This must be in the range [0.0 .. 1.0]. By default 0.0.
        \param Far: Specifies the far clipping plane. This must be in the range [0.0 .. 1.0]. By default 1.0.
        \note Near and far parameters may also be inverse, i.e. Near > Far is also allowed.
        When these values are too near you will get z-fighting! So never set these values to the same value.
        */
        virtual void setDepthRange(f32 Near, f32 Far);
        /**
        Returns the depth range.
        \see setDepthRange
        */
        virtual void getDepthRange(f32 &Near, f32 &Far) const;
        
        //! Enables or disables depth clipping. By default enabled.
        virtual void setDepthClip(bool Enable);
        
        /* === Stencil buffer === */
        
        //! Sets the bit-mask for the stencil buffer. This enables or disables individual bits in the stencil buffer to be written.
        virtual void setStencilMask(u32 BitMask);
        
        /**
        Sets the stencil buffer method.
        \param[in] Method Specifies the size comparision type. This is similiar to the "setDepthMethod" function in the "MaterialStates" class.
        \param[in] Reference Specifies the stencil reference value which will be used for the "STENCIL_REPLACE" stencil operation. By default zero.
        \param[in] BitMask Specifies the stencil buffer bit mask. By default 0xFFFFFFFF.
        \see ESizeComparisionTypes
        */
        virtual void setStencilMethod(const ESizeComparisionTypes Method, s32 Reference = 0, u32 BitMask = ~0);
        
        /**
        Sets the stencil buffer operations.
        \param[in] FailOp Specifies the operation which is to be executed when the stencil test fails. The default value is STENCIL_KEEP.
        \param[in] ZFailOp Specifies the operation which is to be executed when the stencil test succeeds but the z-buffer test fails. The default value is STENCIL_KEEP.
        \param[in] ZPassOp Specifies the operation which is to be executed when the stencil test and the z-buffer test succeed. The default value is STENCIL_KEEP.
        \see EStencilOperations
        */
        virtual void setStencilOperation(const EStencilOperations FailOp, const EStencilOperations ZFailOp, const EStencilOperations ZPassOp);
        
        //! Sets the stencil clear value. The default value is zero.
        virtual void setClearStencil(s32 Stencil);
        
        /* === Rendering functions === */
        
        /**
        Configures the renderer with the specified material states.
        \param[in] Material Pointer to the MaterialStates object.
        \param[in] Forced Specifies whether the setup is to be forced or not.
        If true the material states will be updated guaranteed. Otherwise the render system
        checks if the material states are different to the previously set states.
        \return True if the new material states have changed. Otherwise there is no need
        for an update. This function always returns true if 'Forced' is set to true and 'Material' is not null.
        */
        virtual bool setupMaterialStates(const MaterialStates* Material, bool Forced = false) = 0;
        
        //! Sets the global material state which will be used instead of each object's individual material state.
        void setGlobalMaterialStates(const MaterialStates* GlobalMaterialStates);
        
        //! Configures the renderer with the specified texture layer states.
        virtual void setupTextureLayer(
            u8 LayerIndex, const dim::matrix4f &TexMatrix, const ETextureEnvTypes EnvType,
            const EMappingGenTypes GenType, s32 MappingCoordsFlags
        );
        
        //! Binds all given texture layers.
        virtual void bindTextureLayers(const TextureLayerListType &TexLayers);
        //! Unbinds all given texture layers.
        virtual void unbindTextureLayers(const TextureLayerListType &TexLayers);
        
        //! Configures the renderer with the specified shader class.
        virtual void setupShaderClass(const scene::MaterialNode* Object, ShaderClass* ShaderObject);
        
        //! Updates the material states' references.
        virtual void updateMaterialStates(MaterialStates* Material, bool isClear = false);
        
        /**
        Updates the light for the renderer
        \todo Refactor this function!
        */
        virtual void updateLight(
            u32 LightID, const scene::ELightModels LightType, bool IsVolumetric,
            const dim::vector3df &Direction, const scene::SLightCone &SpotCone, const scene::SLightAttenuation &Attn
        );
        
        /* === Hardware mesh buffers === */
        
        //! Creates a new hardware vertex buffer.
        virtual void createVertexBuffer(void* &BufferID) = 0;
        //! Creates a new hardware index buffer.
        virtual void createIndexBuffer(void* &BufferID) = 0;
        
        //! Deletes the specified hardware vertex buffer.
        virtual void deleteVertexBuffer(void* &BufferID) = 0;
        //! Deletes the specified hardware index buffer.
        virtual void deleteIndexBuffer(void* &BufferID) = 0;
        
        //! Updates the specified hardware vertex buffer by uploading the specified buffer data into VRAM.
        virtual void updateVertexBuffer(
            void* BufferID, const dim::UniversalBuffer &BufferData, const VertexFormat* Format, const EHWBufferUsage Usage
        ) = 0;
        //! Updates the specified hardware index buffer by uploading the specified buffer data into VRAM.
        virtual void updateIndexBuffer(
            void* BufferID, const dim::UniversalBuffer &BufferData, const IndexFormat* Format, const EHWBufferUsage Usage
        ) = 0;
        
        //! Updates the specified hardware vertex buffer only for the specified element.
        virtual void updateVertexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index) = 0;
        //! Updates the specified hardware index buffer only for the specified element.
        virtual void updateIndexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index) = 0;
        
        /**
         * Binds the specified mesh buffer.
         * \param[in] Buffer Constant pointer to the mesh buffer which is to be bound.
         * \return True if the mesh buffer could be bound successful.
         * \note Only use this when you want to draw the same mesh buffer several times
         * consecutively with the 'drawMeshBufferPart' function.
         * \note This function does not support mesh buffer referencing, so use the following code before you bind and draw the mesh buffer:
         * \code
         * video::MeshBuffer* Buffer = ...
         * Buffer = Buffer->getReference();
         * spRenderer->bindMeshBuffer(Buffer);
         * spRenderer->drawMeshBufferPart(Buffer, 0, Buffer->getVertexCount());
         * \endcode
         * \see drawMeshBufferPart
         * \see MeshBuffer
         * \see unbindMeshBuffer
         * \since Version 3.3
        */
        virtual bool bindMeshBuffer(const MeshBuffer* Buffer) = 0;
        
        /**
        Unbinds the previously bound mesh buffer.
        \see bindMeshBuffer
        \note Some render systems (Such as the OpenGL render system) can unbind
        the previous mesh buffer automatically when a different new mesh buffer is bound.
        But always use this function to make your code portable with other render systems.
        \since Version 3.3
        */
        virtual void unbindMeshBuffer() = 0;
        
        /**
        Draws a part from the specified mesh buffer. Indexed vertex rendering can not be used,
        i.e. the vertices must be ordered in ascending sequence.
        \param[in] Buffer Constant pointer to the mesh buffer which is to be rendered.
        \param[in] StartOffset Specifies the index of the first vertex.
        \param[in] NumVertices Specifies the number of vertices which are to be rendered.
        \note This requires that the specified mesh buffer was previously bound manually with the 'bindMeshBuffer' function.
        \see bindMeshBuffer
        \see MeshBuffer
        \since Version 3.3
        */
        virtual void drawMeshBufferPart(const MeshBuffer* Buffer, u32 StartOffset, u32 NumVertices) = 0;
        
        /**
        Draws the specified mesh buffer.
        \note You don't need to bind the mesh buffer by yourself. This is done in this function automatically.
        \see MeshBuffer
        */
        virtual void drawMeshBuffer(const MeshBuffer* Buffer) = 0;
        
        /**
        Draws only the plain geometry of the given hardware mesh buffer. No normals for lighting,
        no tex-coords for texture mapping, no fog-coord etc. This can be used to render fast shadow- or depth maps.
        \param[in] MeshBuffer Constant pointer to the mesh buffer which is to be drawn.
        \param[in] UseFirstTextureLayer Specifies whether the first texture layer is to be bound or not.
        This can be used when the mesh buffer has a transparent color texture and you want to perform alpha-testing.
        \note If the active render system does not support this optimized function the default function "drawMeshBuffer" will be called.
        \see MeshBuffer
        */
        virtual void drawMeshBufferPlain(const MeshBuffer* MeshBuffer, bool UseFirstTextureLayer = false);
        
        /* === Render states === */
        
        /**
        Sets the new render mode when it's a different type than before.
        This is used internally to change the render mode between drawing 2D, drawing 3D and scene rendering.
        This function was introduced to remove the old "begin/endDrawing2D/3D" functions.
        \see ERenderModes
        \since Version 3.2
        */
        virtual void setRenderMode(const ERenderModes Mode);
        
        /**
        Sets the render state manual. Can be used do enabled or disable effects directly.
        \param[in] Type Specifies the render state which is to be changed (lighting, blending, fog effect etc.).
        \param[in] State Specifies the new state (commonly TRUE/ FALSE or rather 1/ 0).
        \note Since version 3.2 the engine itself changes the internal render mode.
        Thus when drawing a 2D primitive (such as a 2D image) and the render mode will be changed all your previously
        changed render states will be overwritten. Therefore you have to provide the next render mode,
        e.g. when you are drawing in 2D and you want to change a special render state do it this way:
        \code
        // Provide the next render mode
        spRenderer->setRenderMode(video::RENDERMODE_DRAWING_2D);
        
        // Store the previous render state
        const s32 PrevRenderState = spRenderer->getRenderState(video::RENDER_BLEND);
        
        // Set an individual render state
        spRenderer->setRenderState(video::RENDERSTATE_BLEND, false);
        
        // Draw the primitive
        spRenderer->draw2DImage(Img, 0);
        
        // Reset the render state
        spRenderer->setRenderState(video::RENDER_BLEND, PrevRenderState);
        \endcode
        \see setRenderMode
        \see ERenderStates
        */
        virtual void setRenderState(const ERenderStates Type, s32 State) = 0;
        //! Returns the current render state.
        virtual s32 getRenderState(const ERenderStates Type) const = 0;
        
        /* === Lighting === */
        
        virtual void addDynamicLightSource(
            u32 LightID, scene::ELightModels Type,
            video::color &Diffuse, video::color &Ambient, video::color &Specular,
            f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic
        );
        
        /**
        Enabled or disables the specified fixed-function light source.
        \param[in] LightID Specifies the light's ID number (beginning with 0).
        \param[in] Enable Specifies whether the light source is to be enabled or disabled.
        \param[in] UseAllRCs Specifies whether all render-contexts are to be used or only the active one.
        By default only the active render context is affected.
        */
        virtual void setLightStatus(u32 LightID, bool Enable, bool UseAllRCs = false);
        
        /**
        Sets the light color for the specified fixed-function light source.
        \param[in] LightID Specifies the light's ID number (beginning with 0).
        \param[in] Diffuse Specifies the diffuse light color. By default (200, 200, 200, 255).
        \param[in] Ambient Specifies the ambient light color. By default (255, 255, 255, 255).
        \param[in] Specular Specifies the specular light color. By default (0, 0, 0, 255).
        \param[in] UseAllRCs Specifies whether all render-contexts are to be used or only the active one.
        By default only the active render context is affected.
        */
        virtual void setLightColor(
            u32 LightID,
            const video::color &Diffuse, const video::color &Ambient, const video::color &Specular,
            bool UseAllRCs = false
        );
        
        /* === Fog effect === */
        
        /**
        Sets the fog's type if used.
        \param Type: Type of the fog. If fog shall be disabled use (video::FOG_NONE).
        */
        virtual void setFog(const EFogTypes Type);
        virtual EFogTypes getFog() const;
        
        //! Sets the fog color
        virtual void setFogColor(const video::color &Color);
        virtual video::color getFogColor() const;
        
        virtual void setFogRange(f32 Range, f32 NearPlane = 1.0f, f32 FarPlane = 1000.0f, const EFogModes Mode = FOG_PALE);
        virtual void getFogRange(f32 &Range, f32 &NearPlane, f32 &FarPlane, EFogModes &Mode);
        
        /* === Clipping planes === */
        
        /**
        Sets and enables or disables a clipping plane. These clipping-planes are additional clipping planes
        to the normal 6 view-frustum-clipping-planes. Normally 8 of those planes should be available.
        \param Index: Index number of the clipping plane.
        \param Plane: Plane which holds the plane equation.
        \param Enable: Specifies if the plane is shall be enabled or disabled. If false "Plane" can just be "dim::plane3df()".
        */
        virtual void setClipPlane(u32 Index, const dim::plane3df &Plane, bool Enable);
        
        /* === Shader programs === */
        
        /**
        Creates a new shader class.
        \param VertexInputLayout: Specifies the vertex format which is used for the objects which
        will use this shader class. This needs only to be set for Direct3D11 but it's a good
        programming style to set it always.
        \return Pointer to a ShaderClass object.
        */
        virtual ShaderClass* createShaderClass(const VertexFormat* VertexInputLayout = 0);
        /**
        Deletes the giben shader class.
        \param ShaderClassObj: Pointer to the shader class which is to be deleted.
        \param DeleteAppendantShaders: Specifies whether the appendant shaders are also to be deleted or not. By default true.
        */
        virtual void deleteShaderClass(ShaderClass* ShaderClassObj, bool DeleteAppendantShaders = true);
        
        /**
        Loads a shader from the disk.
        \param[in] ShaderClassObj Pointer to a valid shader-class object.
        Create a valid shader-class object with the "createShaderClass" function.
        \param[in] Type Specifies the shader type (Vertex-, Pixel shader etc.).
        \param[in] Version Specifies the shader version (GLSL 1.20/ HLSL Vertex 1.1 etc.).
        \param[in] Filename Specifies the shader source filename.
        \param[in] EntryPoint Specifies the shader entry point or rather the main function name.
        This is not required for GLSL. But for HLSL and Cg! The default name is empty, in this case
        the following names will be choosen automatically for the respective shader type (depends on the
        parameter 'Type'): "VertexMain", "PixelMain", "HullMain", "DomainMain", "GeometryMain" or "ComputeMain".
        \param[in] Flags Specifies the compilation and loading flags. This can be a combination of the
        bit masks specified in the EShaderLoadingFlags enumeration. By default 0.
        \param[in] PreShaderCode Specifies additional pre-shader source code. This can be used to
        add some macros to your code. Here is a small example:
        \code
        std::list<io::stringc> PreShaderCode;
        PreShaderCode.push_back("#define ENABLE_EFFECT_XY\n"); // Don't forget the end-of-line character '\n'!
        \endcode
        \return Pointer to the new shader object
        \see Shader
        \see ShaderClass
        \see EShaderLoadingFlags
        */
        virtual Shader* loadShader(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const io::stringc &Filename, io::stringc EntryPoint = "", u32 Flags = 0,
            const std::list<io::stringc> &PreShaderCode = std::list<io::stringc>()
        );
        
        /**
        Creates a shader with the specified buffer strings.
        \param[in] ShaderBuffer List of strings which specify the shader program lines.
        \see EShaderLoadingFlags
        */
        virtual Shader* createShader(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "", u32 Flags = 0
        );
        
        //! Loads a vertex- and pixel shader, creates a shader class and links the program
        virtual ShaderClass* loadShaderClass(
            const io::stringc &FilenameVertex, const io::stringc &FilenamePixel,
            const EShaderVersions VersionVertex, const EShaderVersions VersionPixel,
            const io::stringc &EntryPointVertex = "", const io::stringc &EntryPointPixel = ""
        );
        
        //! Creates a new Cg shader class. If the engine was compiled without the Cg toolkit this function returns null.
        virtual ShaderClass* createCgShaderClass(const VertexFormat* VertexInputLayout = 0);
        
        /**
        Creates a Cg shader.
        \param CompilerOptions: Specifies the compiler options. Invalid options may occur a runtime crash!
        So be careful with this option. Look at the Cg documentation to see the full list of supported compiler options.
        The last options in the array must be a null.
        \return Pointer to the new shader or null if the engine was not compiled with the Cg toolkit.
        */
        virtual Shader* createCgShader(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "",
            const c8** CompilerOptions = 0
        );
        
        //! Deletes the specified shader object.
        virtual void deleteShader(Shader* ShaderObj);
        
        //! Unbinds (or rather disables) all shader types.
        virtual void unbindShaders();

        /**
        Creates a new shader resource. For HLSL a shader resource represents a general purpose shader buffer
        and for GLSL a shader resource represents a "Shader Storange Buffer Object" (SSBO).
        \note Currently only supported for HLSL in Direct3D 11.
        \see ShaderResource
        \since Version 3.3
        */
        virtual ShaderResource* createShaderResource();

        //! Deletes the specified shader resource object.
        virtual void deleteShaderResource(ShaderResource* &Resource);
        
        /**
        Runs the specified compute shader.
        \param[in] ShdClass Pointer to the compute shader class which is to be executed (also called 'dispatched').
        \param[in] GroupSize Specifies the thread group size. Each component must be less than 64,000.
        If the feature level for Direct3D 11 is limited to 10 (i.e. Direct3D 10 hardware) the Z component must be 1.
        \since Version 3.3
        */
        virtual bool dispatch(ShaderClass* ShdClass, const dim::vector3d<u32> &GroupSize);
        
        /* === Queries === */
        
        /**
        Creates a new query object.
        \param[in] Type Specifies the query object type.
        \return Pointer to the new Query object or null if the render system does not support query objects.
        \see Query
        \see EQueryTypes
        */
        virtual Query* createQuery(const EQueryTypes Type);
        //! Deletes the specified query object.
        virtual void deleteQuery(Query* &QueryObj);
        
        /* === Simple drawing functions === */
        
        //! Begins with scene rendering. Always call this before rendering mesh buffers.
        virtual void beginSceneRendering();
        //! Ends with scene rendering. Always call this after rendering mesh buffers.
        virtual void endSceneRendering();
        
        /**
        Configures the renderer to draw further in 2D.
        \note Since version 3.2 this is no longer required to be called before drawing in 2D.
        The engine itself changes the mode from 2D to 3D and vice versa if necessary.
        \see setRenderMode
        */
        virtual void beginDrawing2D();
        virtual void endDrawing2D();
        
        /**
        Configures the renderer to draw further in 3D. This only needs to be called before drawing
        in 3D (draw3DLine etc.) but not to render 3D geometry using "SceneGraph::renderScene.
        */
        virtual void beginDrawing3D();
        virtual void endDrawing3D();
        
        /**
        Sets the blending mode. Before you have to enable blending by calling "setRenderState(RENDER_BLEND, true)."
        \param SourceBlend: Specifies the source pixel blending mode.
        \param TargetBlend: Specifies the target (or rather destination) pixel blending mode.
        \note You can download the \htmlonly <a href="http://softpixelengine.sourceforge.net/downloads.html">BlendingChart</a> \endhtmlonly
        tool to have a visualization of all blending combinations.
        */
        virtual void setBlending(const EBlendingTypes SourceBlend, const EBlendingTypes DestBlend);
        
        /**
        Sets the clipping mode (or rather scissors).
        \param Enable: Specifies whether clipping is to be enabled or disabled.
        \param Position: Specifies the position (in screen space).
        \param Size: Specifies the size (in screen space).
        */
        virtual void setClipping(bool Enable, const dim::point2di &Position, const dim::size2di &Size);
        
        /**
        Sets the viewport. This is actually only used for 3D drawing. For 2D drawing you can use the "setClipping" function.
        \param Position: Specifies the position (in screen space).
        \param Size: Specifies the size (in screen space).
        */
        virtual void setViewport(const dim::point2di &Position, const dim::size2di &Size);
        
        /**
        Sets the new render target.
        \param[in] Target Pointer to the Texture object which is to be used as the new render target.
        If the render target shall be reset to the actual frame buffer set the render target to null.
        This texture must be a valid render target.
        \return True if the specified render target could be set. Otherwise render targets are not supported.
        \see Texture::setRenderTarget
        */
        virtual bool setRenderTarget(Texture* Target);
        
        /**
        Sets the new render target and binds all shader resources for the pixel shader.
        \param[in] Target Pointer to the Texture object which is to be used as the new render target.
        If the render target shall be reset to the actual frame buffer set the render target to null.
        This texture must be a valid render target.
        \param[in] ShdClass Pointer to the shader class whose shader resources are to be bound for the pixel shader stage.
        If this parameter is null the other "setRenderTarget" function will be used.
        \note The start slot for the UAVs (Unordered Access Views) for the shader resources (such as RWTexture3D
        or RWStructuredBuffer in HLSL) begin with the count of render targets you pass (including multi render targets).
        If the default frame buffer is used the start slot is 1.
        \see Texture::setRenderTarget
        \since Version 3.3
        */
        virtual bool setRenderTarget(Texture* Target, ShaderClass* ShdClass);
        
        //! Returns a pointer to the active render target. If no render target is used, the return value is null.
        virtual Texture* getRenderTarget() const;
        
        //! Sets the size for each line which is to be rendered. By default 1. This is not available when using Direct3D.
        virtual void setLineSize(s32 Size = 1);
        
        //! Sets the size for each point which is to be rendered. By default 1.
        virtual void setPointSize(s32 Size = 1);
        
        /**
        Sets the line stipple pattern.
        \param[in] Factor Specifies the multiplication factor. Must be in the range [1 .. 256]. By default 1.
        \param[in] Pattern Specifies the bit pattern. By default 0xFFFF.
        \note Currently only supported for OpenGL render system.
        \since Version 3.3
        */
        virtual void setLineStipple(s32 Factor = 1, u16 Pattern = 0xFFFF);
        
        /**
        Sets the polygon stipple pattern.
        \param[in] Pattern Constant pointer to an array of 32 unsigned integers.
        By default all bits 1. Set this to null to disable polygon stipple.
        \note Currently only supported for OpenGL render system.
        \since Version 3.3
        */
        virtual void setPolygonStipple(const u32* Pattern);
        
        /**
        Sets the logical pixel operation for rendering.
        \param[in] Op Specifies the logical pixel operation. By default LOGICOP_NONE.
        \see ELogicOperations
        \note Currently only supported for OpenGL and OpenGL|ES 1 render system.
        \since Version 3.3
        */
        virtual void setLogicOp(const ELogicOperations Op);
        
        /* === Image drawing === */
        
        /**
        Draws a normal 2D image using the specified texture.
        \param Tex: Texture which is to be used. No settings are needed to be done
        to draw 2 dimensional with normal Texture objects.
        \param Position: 2D position in screen space where the image is to be drawn.
        \param Color: Color which the image shall get.
        */
        virtual void draw2DImage(
            const Texture* Tex, const dim::point2di &Position, const color &Color = color(255)
        );
        
        /**
        Draws a strechted and mapped 2D image.
        \param Position: 2D area in screen space. In this case the two parameters of
        rect2di ("Right" and "Bottom") specifie the size (width and height).
        \param Clipping: 2D mapping or clipping area. This specifies the texture coordinates for the quad.
        */
        virtual void draw2DImage(
            const Texture* Tex, const dim::rect2di &Position,
            const dim::rect2df &Clipping = dim::rect2df(0.0f, 0.0f, 1.0f, 1.0f),
            const color &Color = color(255)
        );
        
        /**
        Draws a rotatable 2D image.
        \param Rotation: Angle (in degrees) for the centered image rotation.
        \param Radius: Radius for the square image.
        */
        virtual void draw2DImage(
            const Texture* Tex, const dim::point2di &Position, f32 Rotation, f32 Radius, const color &Color = color(255)
        );
        
        /**
        Draws an individual rotatable 2D image.
        \param Rotation: Angle (in degrees) for the image rotation.
        \param RotationPoint: Origin of the rotation in percent. By default 0.5 (50%) so it is centerd.
        \param lefttopColor: Color of the left top image corner.
        \param righttopColor: Color of the right top image corner.
        \param rightbottomColor: Color of the right bottom image corner.
        \param leftbottomColor: Color of the left bottom image corner.
        */
        virtual void draw2DImage(
            const Texture* Tex,
            dim::rect2di Position,
            const dim::rect2df &Clipping,
            f32 Rotation,
            const dim::point2df &RotationPoint = dim::point2df(0.5f, 0.5f),
            const color &lefttopColor = color(255),
            const color &righttopColor = color(255),
            const color &rightbottomColor = color(255),
            const color &leftbottomColor = color(255)
        );
        
        //! Draws an individual positionable 2D image.
        virtual void draw2DImage(
            const Texture* Tex,
            const dim::point2di &lefttopPosition,
            const dim::point2di &righttopPosition,
            const dim::point2di &rightbottomPosition,
            const dim::point2di &leftbottomPosition,
            const dim::point2df &lefttopClipping = dim::point2df(0.0f, 0.0f),
            const dim::point2df &righttopClipping = dim::point2df(1.0f, 0.0f),
            const dim::point2df &rightbottomClipping = dim::point2df(1.0f, 1.0f),
            const dim::point2df &leftbottomClipping = dim::point2df(0.0f, 1.0f),
            const color &lefttopColor = color(255),
            const color &righttopColor = color(255),
            const color &rightbottomColor = color(255),
            const color &leftbottomColor = color(255)
        );
        
        /* === Primitive drawing === */
        
        /**
        Returns the color of the pixel buffer (color buffer) at the specified position in screen space.
        \deprecated Reading directly from the frame buffer can be very slow. Use render targets instead.
        */
        virtual color getPixelColor(const dim::point2di &Position) const;
        
        /**
        Returns the depth of the pixel buffer (depth buffer) at the specified position in screen space.
        \deprecated Reading directly from the depth buffer can be very slow. Use render targets instead.
        */
        virtual f32 getPixelDepth(const dim::point2di &Position) const;
        
        //! Draws a 2D point at the specified position in screen space.
        virtual void draw2DPoint(const dim::point2di &Position, const color &Color = color(255));
        
        //! Draws a 2D line at the specified position (PositionA to PositionB).
        virtual void draw2DLine(
            const dim::point2di &PositionA, const dim::point2di &PositionB, const color &Color = color(255)
        );
        
        //! Draws a 2D line with a color change-over (ColorA to ColorB).
        virtual void draw2DLine(
            const dim::point2di &PositionA, const dim::point2di &PositionB, const color &ColorA, const color &ColorB
        );
        
        /**
        Draws a 2D rectangle or quad.
        \param Rect: 2D area in screen space.
        \param Color: Color which is to be used for the rectangle.
        \param isSolid: If true the rectangle is filled. Otherwise four lines will be drawn.
        \see draw2DBox
        */
        virtual void draw2DRectangle(
            const dim::rect2di &Rect, const color &Color = 255, bool isSolid = true
        );
        
        //! Draws a 2D rectangle or quad with color change-overs.
        virtual void draw2DRectangle(
            const dim::rect2di &Rect, const color &lefttopColor, const color &righttopColor,
            const color &rightbottomColor, const color &leftbottomColor, bool isSolid = true
        );
        
        /**
        Draws a centered 2D box.
        \see draw2DRectangle
        \since 3.2
        */
        virtual void draw2DBox(
            const dim::point2di &Position, const dim::size2di &Size, const color &Color = 255, bool isSolid = true
        );
        
        /**
        Draws a 2D circle using the "Bresenham Algorithm".
        \note This is a very time consuming drawing operation because your 3D graphics hardware is optimized
        for rendering triangles. The circle will be drawn in software mode.
        */
        virtual void draw2DCircle(const dim::point2di &Position, s32 Radius, const color &Color = 255);
        
        //! Draws a 2D ellipse (time consuming drawing operation). \see draw2DCircle
        virtual void draw2DEllipse(
            const dim::point2di &Position, const dim::size2di &Radius, const color &Color = 255
        );
        
        /* === Extra drawing functions === */
        
        virtual void draw2DPolygon(
            const ERenderPrimitives Type, const scene::SPrimitiveVertex2D* VerticesList, u32 Count
        );
        virtual void draw2DPolygonImage(
            const ERenderPrimitives Type, Texture* Tex, const scene::SPrimitiveVertex2D* VerticesList, u32 Count
        );
        
        /* === 3D drawing functions === */
        
        virtual void draw3DPoint(
            const dim::vector3df &Position, const color &Color = 255
        );
        virtual void draw3DLine(
            const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &Color = 255
        );
        virtual void draw3DLine(
            const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &ColorA, const color &ColorB
        );
        virtual void draw3DEllipse(
            const dim::vector3df &Position, const dim::vector3df &Rotation = 0.0f,
            const dim::size2df &Radius = 0.5f, const color &Color = 255
        );
        virtual void draw3DTriangle(
            Texture* Tex, const dim::triangle3df &Triangle, const color &Color = 255
        );
        
        /**
        Draws a 3D axis-aligned bounding-box. This is relatively time consuming. A faster way to draw complex line construction is to create
        a mesh and change the primitive type from triangles to lines, line-strip or line-fan.
        */
        virtual void draw3DBox(
            const dim::aabbox3df &BoundBox, const dim::matrix4f &Transformation, const color &Color = color(255)
        );
        
        //! Draws a 3D oriented bounding-box.
        virtual void draw3DBox(const dim::obbox3df &BoundBox, const color &Color = color(255));
        
        /* === Texture loading and creating === */
        
        /**
        Sets the texture generation flags. Use this function to configure textures which were not even loaded.
        \param Flag: Texture generation flag which is to be set.
        \param Value: Value for the specified flag. e.g. if TEXGEN_MIPMAPS is set as flag set this value to 'true' of 'false'.
        */
        void setTextureGenFlags(const ETextureGenFlags Flag, const s32 Value);
        
        //! Returns the image format of the specified file.
        EImageFileFormats getImageFileFormat(io::File* TexFile) const;
        //! Returns the image format of the specified file given by the filename.
        EImageFileFormats getImageFileFormat(const io::stringc &Filename) const;
        
        /**
        Loads a texture (or rather image) used for 3D geometry and 2D drawing.
        \param Filename: Image filename. The engine can load different file formats like BMP, JPG or PNG etc.
        The engine also detects the true file format i.e. if an image file ends with ".bmp" but it is truly a JPEG
        image it will be loaded correctly. This is because the engine looks inside the files and reads the magic number
        to identify the actual file format. To load WAD files use the "loadTextureList" function.
        \return Pointer to the new loaded Texture object.
        */
        virtual Texture* loadTexture(const io::stringc &Filename);
        
        //! Loads a texture using the specified image loader.
        virtual Texture* loadTexture(ImageLoader* Loader);
        
        /**
        Returns the specified texture file. This function loads a texture file only once.
        If you call this function several times with the same file the engine will use the same Texture object
        to save memory and time.
        */
        virtual Texture* getTexture(const io::stringc &Filename);
        
        /**
        Loads a texture list. This is only used for WAD files (from the Quake and Half-Life maps).
        \param Filename: Image list filename. In WAD (Where is All the Data) files several 100 images may be stored.
        \return List with all loaded Texture object.
        */
        virtual std::list<Texture*> loadTextureList(const io::stringc &Filename);
        
        /**
        Saves the specified texture to the disk.
        \param Tex: Pointer to the Texture object which is to be saved.
        \param Filename: Image filename.
        \param FileFormat: Image format. Currently only the BMP file format is supported.
        \return True if saving the image was successful.
        */
        bool saveTexture(const Texture* Tex, io::stringc Filename, const EImageFileFormats FileFormat = IMAGEFORMAT_BMP);
        
        //! Deletes the specifed Texture object.
        void deleteTexture(Texture* &Tex);
        
        //! Returns true if the specifies texture is a valid hardware texture.
        virtual bool isTexture(const Texture* Tex) const;
        
        //! Copies the specified texture and returns the new instance.
        virtual Texture* copyTexture(const Texture* Tex);
        
        /**
        Creates a new texture.
        \param Size: Specifies the initial texture size.
        \param Format: Specifies the image buffer format. By default RGB.
        \param ImageBuffer: Specifies the image buffer which is to be used. Internally only a copy of this
        image buffer will be used. Thus you need to handle the memory by yourself.
        By default 0 which means that the texture will be filled by a default color which can be set with "setFillColor".
        \returns Pointer to the new Texture object.
        */
        virtual Texture* createTexture(
            const dim::size2di &Size, const EPixelFormats Format = PIXELFORMAT_RGB, const u8* ImageBuffer = 0
        );
        
        //! Creates a new float texture.
        virtual Texture* createTexture(
            const dim::size2di &Size, const EPixelFormats Format, const f32* ImageBuffer
        );
        
        //! Creates a new texture with the specified creation flags.
        virtual Texture* createTexture(const STextureCreationFlags &CreationFlags) = 0;
        
        /**
        Creates a texture cubemap.
        The functionality of this method is equivalent to the following code:
        \code
        Texture* CubeMap = spRenderer->createTexture(dim::size2di(Size.Width, Size.Height*6));
        CubeMap->setType(video::TEXTURE_CUBEMAP);
        CubeMap->setWrapMode(video::TEXWRAP_CLAMP);
        CubeMap->setRenderTarget(isRenderTarget);
        \endcode
        \param Size: Specifies the size for each cubemap face.
        \param isRenderTarget: Specifies whether this cubemap should be a render target or not.
        By default true.
        */
        virtual Texture* createCubeMap(const dim::size2di &Size, bool isRenderTarget = true);
        
        /**
        Takes a screenshot of the current frame.
        \param Position: Specifies the 2D position (in screen space) of the screenshot.
        \param Size: Specifies the size (in screen space) of the screenshot.
        \return Pointer to the new Texture object.
        \note This is a time consuming pixel reading operation. The best way to make a 'screenshot' of the current frame
        is to render into a render target and than get the image buffer from the video RAM (VRAM).
        */
        virtual Texture* createScreenShot(const dim::point2di &Position = 0, dim::size2di Size = 0);
        
        //! Takes a screenshot of the current frame and stores the data in the specified Texture object.
        virtual void createScreenShot(Texture* Tex, const dim::point2di &Position = 0);
        
        //! Sets the standard texture creation fill color.
        virtual void setFillColor(const video::color &Color);
        
        virtual void clearTextureList();
        virtual void reloadTextureList();
        
        /**
        Creates a new texture from the given device bitmap and its device-context.
        This is currently only supported for MS/Windows. The two pointers must reference to
        a HDC and a HBITMAP object.
        \code
        // Code example for MS/Windows:
        HDC hDC = GetDC(hWnd);
        
        HBITMAP hBmp = CreateCompatibleBitmap(hDC, Width, Height);
        HDC hBmpDC = CreateCompatibleDC(hDC);
        
        video::Texture* Tex = spRenderer->createTextureFromDeviceBitmap(&hBmpDC, &hBmp);
        \endcode
        */
        Texture* createTextureFromDeviceBitmap(void* BitmapDC, void* Bitmap);
        
        //! Deletes the specified VertexFormat object.
        void deleteVertexFormat(VertexFormat* Format);
        
        /* === Movie loading === */
        
        /**
        Loads an AVI movie file.
        \param Filename: Specifies the AVI movie file which is to be loaded.
        \param Resolution: Specifies the movie resolution. By default 256. This should be a "power of two" value.
        \returns Pointer to the new Movie object.
        */
        Movie* loadMovie(const io::stringc &Filename, s32 Resolution = 256);
        void deleteMovie(Movie* &MovieObject);
        
        /* === Font loading and text drawing === */
        
        /**
        Loads a new font.
        \param[in] FontName Specifies the type of font. By default the first font which could be found.
        Under MS/Windows this is often "Arial".
        \param[in] FontSize Specifies the font size. By default the standard OS font size.
        \param[in] Flags Additional options for the font. This can be a combination of the following values:
        FONT_BOLD, FONT_ITALIC, FONT_UNDERLINED, FONT_STRIKEOUT, FONT_SYMBOLS.
        */
        virtual Font* createFont(const io::stringc &FontName = "", s32 FontSize = 0, s32 Flags = 0);
        
        virtual Font* createTexturedFont(const io::stringc &FontName = "", s32 FontSize = 0, s32 Flags = 0);
        virtual Font* createBitmapFont(const io::stringc &FontName = "", s32 FontSize = 0, s32 Flags = 0);
        
        //! \deprecated
        virtual Font* createFont(video::Texture* FontTexture);
        //! \deprecated
        virtual Font* createFont(video::Texture* FontTexture, const io::stringc &FontXMLFile);
        virtual Font* createFont(video::Texture* FontTexture, const std::vector<SFontGlyph> &GlyphList, s32 FontHeight);
        
        /**
        Creates a font texture with the given font type and styles.
        \param GlyphList: Specifies the resulting glyph list. Assign this to the final font object.
        \param FontName: Specifies the font name. Under MS/Windows there are default fonts like "Arial", "Courier New" or "Times New Roman".
        \param FontSize: Specifies the font size. This is actually the font's height. The width will be computed automatically.
        \param Flags: Specifies the font style flags. This can be a combination of the following values:
        FONT_BOLD, FONT_ITALIC, FONT_UNDERLINED, FONT_STRIKEOUT, FONT_SYMBOLS.
        \see EFontFlags
        */
        virtual Texture* createFontTexture(
            std::vector<SFontGlyph> &GlyphList, const io::stringc &FontName = "", s32 FontSize = 0, s32 Flags = 0
        );
        
        //! Deletes the specified font.
        virtual void deleteFont(Font* FontObj);
        
        /**
        Draws a 2D text with the specified font.
        \param FontObject: Specifies the font which is to be used for text drawing.
        \param Position: Specifies the 2D position (in screen space).
        \param Text: Specifies the text which is to be drawn.
        \param Color: Specifies the color in which the text is to be drawn.
        \param Flags: Specifies the drawing flags.
        \see ETextDrawingFlags
        */
        virtual void draw2DText(
            const Font* FontObject, const dim::point2di &Position, const io::stringc &Text,
            const color &Color = color(255), s32 Flags = 0
        );
        
        /**
        Draws a 3D text.
        \param FontObject: Specifies the font which is to be used for text drawing.
        \param Transformation: Specifies the transformation matrix (in global space).
        \param Text: Specifies the text which is to be drawn.
        \param Color: Specifies the color in which the text is to be drawn.
        */
        virtual void draw3DText(
            const Font* FontObject, const dim::matrix4f &Transformation, const io::stringc &Text, const color &Color = color(255)
        );
        
        /* === Matrix controll === */
        
        /* Update the render matrix (with the SoftPixel-modelview matrix) */
        virtual void updateModelviewMatrix() = 0;
        
        virtual void matrixWorldViewReset();
        
        virtual void setProjectionMatrix(const dim::matrix4f &Matrix);
        virtual void setViewMatrix      (const dim::matrix4f &Matrix);
        virtual void setWorldMatrix     (const dim::matrix4f &Matrix);
        virtual void setTextureMatrix   (const dim::matrix4f &Matrix, u8 TextureLayer = 0);
        virtual void setColorMatrix     (const dim::matrix4f &Matrix);
        
        //! Returns the current projection matrix.
        virtual dim::matrix4f getProjectionMatrix() const;
        //! Returns the current view matrix.
        virtual dim::matrix4f getViewMatrix() const;
        //! Returns the current world matrix.
        virtual dim::matrix4f getWorldMatrix() const;
        //! Returns the current texture matrix for the specified texture layer.
        virtual dim::matrix4f getTextureMatrix(u8 TextureLayer = 0) const;
        //! \deprecated This is no longer used.
        virtual dim::matrix4f getColorMatrix() const;
        
        /**
        Sets up the current world-view-projection matrix.
        \see getWVPMatrix
        \since Version 3.3
        */
        void setupWVPMatrix(dim::matrix4f &Mat) const;
        /**
        Sets up the current view-projection matrix.
        \see getVPMatrix
        \since Version 3.3
        */
        void setupVPMatrix(dim::matrix4f &Mat) const;
        /**
        Sets up the current world-view matrix.
        \see getWVMatrix
        \since Version 3.3
        */
        void setupWVMatrix(dim::matrix4f &Mat) const;
        /**
        Sets up the current inverse view-projection matrix.
        \note This matrix does not contain a position in the elements [12, 13, 14]!
        \see getInvVPMatrix
        \since Version 3.3
        */
        void setupInvVPMatrix(dim::matrix4f &Mat) const;
        
        /**
        Returns the current world-view-projection matrix.
        \see setupWVPMatrix
        \since Version 3.3
        */
        dim::matrix4f getWVPMatrix() const;
        /**
        Returns the current view-projection matrix.
        \see setupVPMatrix
        \since Version 3.3
        */
        dim::matrix4f getVPMatrix() const;
        /**
        Returns the current world-view matrix.
        \see setupWVMatrix
        \since Version 3.3
        */
        dim::matrix4f getWVMatrix() const;
        /**
        Returns the current inverse view-projection matrix.
        \note This matrix does not contain a position in the elements [12, 13, 14]!
        \see setupInvVPMatrix
        \since Version 3.3
        */
        dim::matrix4f getInvVPMatrix() const;
        
        /* === Static functions === */
        
        /**
        Returns the current number of MeshBuffer draw calls or zero if the engine was not compiled
        with the 'SP_COMPILE_WITH_RENDERSYS_QUERIES' option.
        \see MeshBuffer
        */
        static u32 getNumDrawCalls();
        /**
        Returns the current number of MeshBuffer bindings or zero if the engine was not compiled
        with the 'SP_COMPILE_WITH_RENDERSYS_QUERIES' option.
        \see MeshBuffer
        */
        static u32 getNumMeshBufferBindings();
        /**
        Returns the current number of TextureLayer list bindings or zero if the engine was not compiled
        with the 'SP_COMPILE_WITH_RENDERSYS_QUERIES' option.
        \see TextureLayer
        \see TextureLayerListType
        */
        static u32 getNumTextureLayerBindings();
        /**
        Returns the current number of material updates or zero if the engine was not compiled
        with the 'SP_COMPILE_WITH_RENDERSYS_QUERIES' option.
        \see MaterialStates
        */
        static u32 getNumMaterialUpdates();
        
        /* === Inline functions === */
        
        /**
        Returns type of render system (OpenGL, Direct3D9 etc.).
        \see ERenderSystems
        */
        inline ERenderSystems getRendererType() const
        {
            return RendererType_;
        }
        
        #if defined(SP_PLATFORM_WINDOWS)
        inline HDC getDeviceContext() const
        {
            return DeviceContext_;
        }
        #endif
        
        //! Returns whether depth clipping is enabled or disabled.
        inline bool getDepthClip() const
        {
            return DepthRange_.Enabled;
        }
        
        /**
        Returns the global shader list.
        \see Shader
        */
        inline const std::list<Shader*>& getShaderList() const
        {
            return ShaderList_;
        }
        /**
        Returns the global shader class list
        \see ShaderClass
        */
        inline const std::list<ShaderClass*>& getShaderClassList() const
        {
            return ShaderClassList_;
        }
        /**
        Returns the global shader resource list.
        \see ShaderResource
        \since Version 3.3
        */
        inline const std::list<ShaderResource*>& getShaderResourceList() const
        {
            return ShaderResourceList_;
        }
        
        inline void setSurfaceCallback(const ShaderSurfaceCallback &CallbackProc)
        {
            ShaderSurfaceCallback_ = CallbackProc;
        }
        
        //! Sets the global shader class which will be used instead of each object's individual shader class.
        inline void setGlobalShaderClass(ShaderClass* GlobalShaderClass)
        {
            GlobalShaderClass_ = GlobalShaderClass;
        }
        //! Returns the global shader class. By default null.
        inline ShaderClass* getGlobalShaderClass() const
        {
            return GlobalShaderClass_;
        }
        
        //! Returns the global material state. By default null.
        inline const MaterialStates* getGlobalMaterialStates() const
        {
            return GlobalMaterialStates_;
        }
        
        //! Sets the new texture generation flags. You can also set each flag individual by calling the other "setTextureGenFlags" function.
        inline void setTextureGenFlags(const STextureCreationFlags &Flags)
        {
            TexGenFlags_ = Flags;
        }
        //! Returns the texture generation flags structure.
        inline STextureCreationFlags getTextureGenFlags() const
        {
            return TexGenFlags_;
        }
        
        //! Returns the whole texture list.
        inline const std::list<Texture*>& getTextureList() const
        {
            return TextureList_;
        }
        
        /**
        Sets the font transformation for 2D text drawing.
        \note This only affects textured font!
        */
        inline void setFontTransformation(const dim::matrix4f &Transform)
        {
            FontTransform_ = Transform;
        }
        //! Returns the font transformation for 2D text drawing.
        inline dim::matrix4f getFontTransformation() const
        {
            return FontTransform_;
        }
        
        /**
        Sets the texture-layer visibility bit mask.
        \see TextureLayer
        \see TextureLayer::setVisibleMask.
        */
        inline void setTexLayerVisibleMask(s32 VisibleMask)
        {
            TexLayerVisibleMask_ = VisibleMask;
        }
        //! Returns the texture-layer visibility bit mask. By default 0xFFFFFFFF.
        inline s32 getTexLayerVisibleMask() const
        {
            return TexLayerVisibleMask_;
        }
        
        /**
        Only enabled or disables the rectangular clipping. This is actually only used to disable the
        clipping because it's equivalent to the following code:
        \code
        setClipping(Enable, 0, 0);
        \endcode
        */
        inline void setClipping(bool Enable)
        {
            setClipping(Enable, 0, 0);
        }
        
        /**
        Sets the default blending mode: source = BLEND_SRCALPHA, destination = BLEND_INVSRCALPHA.
        \see setBlending
        */
        inline void setupDefaultBlending()
        {
            setBlending(BLEND_SRCALPHA, BLEND_INVSRCALPHA);
        }
        
        /**
        Returns the active render mode. This is used internally to switch render modes dynamically.
        \see ERenderModes
        */
        inline ERenderModes getRenderMode() const
        {
            return RenderMode_;
        }
        
        /**
        Creates a new vertex format. This is a template function, thus you can create your own custom vertex formats.
        Just write a class which inherits from the VertexFormat base class.
        \return Pointer to the new VertexFormat object.
        */
        template <class T> T* createVertexFormat()
        {
            T* NewFormat = new T();
            VertexFormatList_.push_back(NewFormat);
            return NewFormat;
        }
        
        //! Returns the pre-defined "default" vertex format.
        inline VertexFormatDefault* getVertexFormatDefault() const
        {
            return VertexFormatDefault_;
        }
        //! Returns the pre-defined "reduced" vertex format.
        inline VertexFormatReduced* getVertexFormatReduced() const
        {
            return VertexFormatReduced_;
        }
        //! Returns the pre-defined "extended" vertex format.
        inline VertexFormatExtended* getVertexFormatExtended() const
        {
            return VertexFormatExtended_;
        }
        //! Returns the pre-defined "full" vertex format.
        inline VertexFormatFull* getVertexFormatFull() const
        {
            return VertexFormatFull_;
        }
        /**
        Returns the pre-defined "empty" vertex format. This can be used when the vertex input layout has only the vertex ID.
        \since Version 3.3
        */
        inline VertexFormatUniversal* getVertexFormatEmpty() const
        {
            return VertexFormatEmpty_;
        }
        
        //! Returns a constant pointer to the 2D drawing material.
        inline const MaterialStates* getMaterialDrawing2D() const
        {
            return Material2DDrawing_;
        }
        //! Returns a constant pointer to the 3D drawing material.
        inline const MaterialStates* getMaterialDrawing3D() const
        {
            return Material3DDrawing_;
        }
        
        //! Returns the vertex format list.
        inline const std::list<VertexFormat*>& getVertexFormatList() const
        {
            return VertexFormatList_;
        }
        
        /**
        Returns a constant default texture.
        \param[in] Type Specifies the default texture type.
        \return Constant pointer to the given default texture.
        \note Default textures can not be deleted by the client programer and they are not listed in the global texture container.
        \see EDefaultTextures
        */
        inline const video::Texture* getDefaultTexture(const EDefaultTextures Type)
        {
            return Type < DEFAULT_TEXTURE_COUNT ? DefaultTextures_[Type] : 0;
        }
        
    protected:
        
        /* === Friends === */
        
        friend class Font;
        friend class GLTextureBase;
        friend class OpenGLTexture;
        friend class OpenGLES1Texture;
        friend class GLFrameBufferObject;
        friend class VertexFormat;
        friend class VertexFormatUniversal;
        friend class Texture;
        friend class TextureLayer;
        friend class MeshBuffer;
        friend class sp::SoftPixelDevice;
        //friend class SoftPixelDevice;
        
        #ifdef SP_COMPILE_WITH_DIRECT3D11
        friend class Direct3D11RenderContext;
        #endif
        
        #ifdef SP_PLATFORM_WINDOWS
        friend class sp::SoftPixelDeviceWin32;
        #endif
        
        #ifdef SP_PLATFORM_LINUX
        friend class sp::SoftPixelDeviceLinux;
        #endif
        
        #ifdef SP_PLATFORM_ANDROID
        friend class sp::SoftPixelDeviceAndroid;
        #endif
        
        /* === Enumerations === */
        
        enum ERenderQueries
        {
            RENDERQUERY_SHADER = 0,
            RENDERQUERY_GEOMETRY_SHADER,
            RENDERQUERY_TESSELLATION_SHADER,
            RENDERQUERY_COMPUTE_SHADER,
            RENDERQUERY_CONSTANT_BUFFER,

            RENDERQUERY_RENDERTARGET,
            RENDERQUERY_MULTISAMPLE_RENDERTARGET,
            RENDERQUERY_MULTI_TEXTURE,
            RENDERQUERY_CUBEMAP_ARRAY,
            RENDERQUERY_TEXTURE_BUFFER,
            RENDERQUERY_SHADER_RESOURCE,
            
            RENDERQUERY_HARDWARE_MESHBUFFER,
            RENDERQUERY_HARDWARE_INSTANCING,
            
            RENDERQUERY_FOG_COORD,
            RENDERQUERY_QUERIES,
            
            RENDERQUERY_COUNT,
        };
        
        /* === Functions === */
        
        RenderSystem(const ERenderSystems Type);
        
        virtual void setup2DDrawing();
        virtual void setup3DDrawing();
        
        virtual Shader* createEmptyShaderWithError(
            const io::stringc &Message, ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version
        );
        
        virtual void createDeviceFont(
            void* FontObject, const io::stringc &FontName, const dim::size2di &FontSize,
            s32 Flags = 0, s32 CharSet = 0
        ) const;
        
        virtual std::vector<SFontGlyph> getCharWidths(void* FontObject) const;
        
        virtual void updateVertexInputLayout(VertexFormat* Format, bool isCreate);
        
        virtual void createDefaultResources();
        virtual void deleteDefaultResources();
        
        virtual void releaseFontObject(Font* FontObj);
        
        virtual void drawTexturedFont(const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color);
        virtual void drawBitmapFont(const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color);
        
        virtual void createTexturedFontVertexBuffer(dim::UniversalBuffer &VertexBuffer, VertexFormatUniversal &VertFormat);
        virtual void setupTexturedFontGlyph(void* &RawVertexData, const SFontGlyph &Glyph, const dim::rect2df &Mapping);
        
        void unbindPrevTextureLayers();
        void noticeTextureLayerChanged(const TextureLayer* TexLayer);
        
        /* === Templates === */
        
        template <typename T> Shader* createShaderObject(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint, u32 Flags)
        {
            /* Create shader object */
            Shader* NewShader = 0;
            
            try
            {
                NewShader = new T(ShaderClassObj, Type, Version);
            }
            catch (std::exception &Err)
            {
                io::Log::error(Err.what());
                return 0;
            }
            
            /* Compile shader and add to the list */
            NewShader->compile(ShaderBuffer, EntryPoint, 0, Flags);
            ShaderList_.push_back(NewShader);
            
            return NewShader;
        }
        
        /* === Static functions === */
        
        static void resetQueryCounters();
        
        /* === Members === */
        
        /* Identity member */
        ERenderSystems RendererType_;
        
        /* Context members */
        #if defined(SP_PLATFORM_WINDOWS)
        HDC DeviceContext_;
        UINT PixelFormat_;
        #elif defined(SP_PLATFORM_LINUX)
        Display* Display_;
        Window* Window_;
        #endif
        
        /* Object lists */
        std::list<Texture*>         TextureList_;
        std::list<Shader*>          ShaderList_;
        std::list<ShaderClass*>     ShaderClassList_;
        std::list<ShaderResource*>  ShaderResourceList_;
        std::list<Font*>            FontList_;
        std::list<Movie*>           MovieList_;             //!< \deprecated Movie system should be recreated completely!
        std::list<Query*>           QueryList_;
        
        std::map<std::string, Texture*> TextureMap_;
        
        std::vector<RenderContext*> ContextList_;
        
        /* Semaphores */
        CriticalSection TextureListSemaphore_;
        
        /* States and flags */
        u8 StdFillColor_[4];
        
        SFogStates Fog_;
        STextureCreationFlags TexGenFlags_;
        SDepthRange DepthRange_;
        
        /* Render states */
        ERenderModes RenderMode_;
        
        dim::matrix4f Matrix2D_;
        dim::matrix4f FontTransform_;
        u32 MaxClippingPlanes_;
        
        bool isFrontFace_;
        
        s32 TexLayerVisibleMask_;
        
        Texture* RenderTarget_;
        
        const MaterialStates* GlobalMaterialStates_;
        
        /* Shader programs */
        ShaderClass* CurShaderClass_;
        ShaderClass* GlobalShaderClass_;
        ShaderSurfaceCallback ShaderSurfaceCallback_;
        
        const MaterialStates* PrevMaterial_;
        const TextureLayerListType* PrevTextureLayers_;
        
        MaterialStates* Material2DDrawing_;
        MaterialStates* Material3DDrawing_;
        
        /* Vertex formats */
        std::list<VertexFormat*> VertexFormatList_;
        
        VertexFormatDefault*    VertexFormatDefault_;
        VertexFormatReduced*    VertexFormatReduced_;
        VertexFormatExtended*   VertexFormatExtended_;
        VertexFormatFull*       VertexFormatFull_;
        VertexFormatUniversal*  VertexFormatEmpty_;
        
        /* Queries */
        bool RenderQuery_[RENDERQUERY_COUNT];
        
        #ifdef SP_COMPILE_WITH_RENDERSYS_QUERIES
        
        static u32 NumDrawCalls_;           //!< Draw call counter. This counter will always be incremented when "drawMeshBuffer" has been called.
        static u32 NumMeshBufferBindings_;  //!< Mesh buffer binding counter.
        static u32 NumTexLayerBindings_;    //!< Texture layer list binding counter.
        static u32 NumMaterialUpdates_;     //!< Material states update counter.
        
        #endif
        
    private:
        
        /* === Functions === */
        
        void createDefaultVertexFormats();
        void createDefaultTextures();
        void createDrawingMaterials();
        
        /* === Members === */
        
        video::Texture* DefaultTextures_[DEFAULT_TEXTURE_COUNT];
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
