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


class RenderContext;

/*
 * ======= Macros =======
 */

static const s32 DEF_TEXTURE_SIZE           = 1;
static const s32 DEF_SCREENSHOT_SIZE        = 256;
static const s32 DEF_FONT_SIZE              = 15;
static const s32 DEF_SPLINE_DETAIL          = 100;


/*
 * ======= Enumerations =======
 */

//! Shade modes
enum EShadeModeTypes
{
    SHADEMODE_SMOOTH,   //!< Smooth shading for primitives, particular trianlges (by default).
    SHADEMODE_FLAT      //!< Flat shading where only the last primitive vertex's color is used (no color interpolation).
};

//! Buffer types which can be cleared
enum EClearBufferTypes
{
    BUFFER_COLOR    = 0x01, //!< Color buffer (RGBA format).
    BUFFER_DEPTH    = 0x02, //!< Depth buffer. This buffer is shared with the stencil buffer.
    BUFFER_STENCIL  = 0x04, //!< Stencil buffer. This buffer is shared with the depth buffer.
    BUFFER_ALL      = ~0,   //!< All available buffers are used.
};

//! Fog types
enum EFogTypes
{
    FOG_NONE = 0,   //!< Disable fog.
    FOG_STATIC,     //!< Static fog.
    FOG_VOLUMETRIC  //!< Volumetric fog. This mode is only available if the renderer supports vertex fog coordinates.
};

//! Fog modes
enum EFogModes
{
    FOG_PALE,   //!< Pale fog mode.
    FOG_THICK,  //!< Thick fog mode.
};

//! Texture generation falgs
enum ETextureGenFlags
{
    TEXGEN_FILTER,          //!< Magnification- and minification texture filter. Use a value of the ETextureFilters enumeration.
    TEXGEN_MAGFILTER,       //!< Magnification texture filter. Same values as TEXGEN_FILTER.
    TEXGEN_MINFILTER,       //!< Minification texture filter. Same values as TEXGEN_FILTER.
    
    TEXGEN_MIPMAPFILTER,    //!< MIP mapping filter. Use a value of the ETextureMipMapFilters enumeration.
    TEXGEN_MIPMAPS,         //!< MIP mapping enable/disable. Use a boolean.
    
    TEXGEN_WRAP,            //!< U, V and W wrap mode. Use a value of the ETextureWrapModes enumeration.
    TEXGEN_WRAP_U,          //!< U wrap mode (X axis). Same values as TEXGEN_WRAP.
    TEXGEN_WRAP_V,          //!< V wrap mode (Y axis). Same values as TEXGEN_WRAP.
    TEXGEN_WRAP_W,          //!< W wrap mode (Z axis). Same values as TEXGEN_WRAP.
    
    TEXGEN_ANISOTROPY,      //!< Anisotropy of the anisotropic MIP mapping filter. Use a power of two value (2, 4, 8, 16 etc.).
};

//! Graphics hardware vendor IDs
enum EGraphicsVendorIDs
{
    VENDOR_UNKNOWN  = 0x0000, //!< Unknown vendor.
    VENDOR_ATI      = 0x1002, //!< ATI Technologies Inc.
    VENDOR_NVIDIA   = 0x10DE, //!< NVIDIA Corporation.
    VENDOR_MATROX   = 0x102B, //!< Matrox Electronic Systems Ltd.
    VENDOR_3DFX     = 0x121A, //!< 3dfx Interactive Inc.
    VENDOR_S3GC     = 0x5333, //!< S3 Graphics Co., Ltd.
    VENDOR_INTEL    = 0x8086, //!< Intel Corporation.
};

//! Text drawing falgs.
enum ETextDrawingFlags
{
    TEXT_CENTER_HORZ    = 0x0001,                               //!< Centers the text horizontal.
    TEXT_CENTER_VERT    = 0x0002,                               //!< Centers the text vertical.
    TEXT_CENTER         = TEXT_CENTER_HORZ | TEXT_CENTER_VERT,  //!< Centers the text horizontal and vertical.
};


/*
 * ======= Structures =======
 */

//! Primitive vertex structure used for Direct3D9 2D drawing.
struct SPrimitiveVertex
{
    SPrimitiveVertex() :
        RHW     (0.0f       ),
        Color   (0xFF000000 )
    {
    }
    SPrimitiveVertex(f32 X, f32 Y, f32 Z, video::color Clr, f32 U = 0.0f, f32 V = 0.0f, f32 AspectRHW = 1.0f) :
        Coord   (X, Y, Z        ),
        RHW     (AspectRHW      ),
        Color   (Clr.getSingle()),
        TexCoord(U, V           )
    {
    }
    SPrimitiveVertex(f32 X, f32 Y, f32 Z, u32 Clr, f32 U = 0.0f, f32 V = 0.0f, f32 AspectRHW = 1.0f) :
        Coord   (X, Y, Z    ),
        RHW     (AspectRHW  ),
        Color   (Clr        ),
        TexCoord(U, V       )
    {
    }
    
    /* Members */
    dim::vector3df Coord;
    f32 RHW;
    u32 Color;
    dim::point2df TexCoord;
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
        
        //! Returns type of render system (OpenGL, Direct3D9 etc.).
        inline ERenderSystems getRendererType() const
        {
            return RendererType_;
        }
        
        #if defined(SP_PLATFORM_WINDOWS)
        virtual inline HDC getDeviceContext() const
        {
            return DeviceContext_;
        }
        #elif defined(SP_PLATFORM_LINUX)
        
        #endif
        
        static io::stringc getVendorNameByID(const u32 VendorID);
        
        virtual io::stringc getRenderer()           const = 0;  //!< Returns the renderer hardware (e.g. "NVIDIA GeForce 8800 GTX").
        virtual io::stringc getVersion()            const = 0;  //!< Returns the renderer API version (e.g. "OpenGL 3.2" or "Direct3D9.0c").
        virtual io::stringc getVendor()             const = 0;  //!< Returns the graphics hardware vendor (e.g. "NVIDIA Corporation" or "ATI Technologies Inc.").
        virtual io::stringc getShaderVersion()      const = 0;  //!< Returns the shader version (e.g. "1.50 NVIDIA via Cg compiler" or "HLSL Shader Model 2").
        
        virtual bool queryVideoSupport(const EVideoFeatureQueries Query) const = 0; //!< Returns true if the specified feature is supported by the renderer.
        
        /**
        Returns true if the specified extension is supported by the renderer (actually only used for OpenGL, especially for shader extensions).
        \param TargetExtension: Extension name (e.g. "GL_ARB_shader_texture_lod" when using "texture2DGradARB" function in GLSL).
        */
        virtual bool queryExtensionSupport(const io::stringc &TargetExtension) const;
        
        virtual s32 getMultitexCount()              const = 0;  //!< Returns the count of maximal texture layers for the fixed-function-pipeline.
        virtual s32 getMaxAnisotropicFilter()       const = 0;  //!< Returns the count of maximal anisotropic texture filter sampling.
        virtual s32 getMaxLightCount()              const = 0;  //!< Returns the count of maximal light sources for the fixed-function-pipeline.
        
        virtual dim::EMatrixCoordinateSystmes getProjectionMatrixType() const;
        
        virtual void printWarning();
        
        /* === Video buffer control functions === */
        
        /**
        Clears the specified buffers.
        \param ClearFlags: Specifies which buffers are to be cleared.
        This can be a combination of the following types:
        BUFFER_COLOR (Color or pixel buffer), BUFFER_DEPTH (Depth or Z buffer), BUFFER_STENCIL
        (Stencil or mask buffer). If your 3D scene has a skybox or the background is not visible because of
        any other reason you can clear only the depth buffer to speed up your application
        (This is particular reasonable when using the software renderer).
        */
        virtual void clearBuffers(const s32 ClearFlags = BUFFER_COLOR | BUFFER_DEPTH) = 0;
        
        /* === Configuration functions === */
        
        //! Sets the shading mode for primitives (where primitives are points, lines and triangles).
        virtual void setShadeMode(const EShadeModeTypes ShadeMode);
        
        //! Sets triangle rendering to CW (clock-wise -> false) or CCW (counter-clock-wise -> true).
        virtual void setFrontFace(bool isFrontFace);
        
        //! Sets the color with which the color-buffer is to be cleared.
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
        //! Returns whether depth clipping is enabled or disabled.
        virtual bool getDepthClip() const;
        
        /* === Rendering functions === */
        
        //! Configures the renderer with the specified material states.
        virtual void setupMaterialStates(const MaterialStates* Material) = 0;
        
        //! Configures the renderer with the specified shader class.
        virtual void setupShaderClass(const scene::MaterialNode* Object, ShaderClass* ShaderObject);
        
        //! Updates the material states' references.
        virtual void updateMaterialStates(MaterialStates* Material, bool isClear = false);
        
        /**
        Renders a list of renderer primitives (points, line, triangles etc.) with the given vertices.
        \param Type: Specifies the primitive type. Note that not each renderer supports all primitive types.
        The following types are only supported by OpenGL: quads, quads-strip, polyong.
        \param Vertices: Vertex data array.
        \param VertexCount: Specifies the count of vertices or rather the number of elements in the vertex data array.
        \param Indices: Index data array. Note that for the most renderers this needs to be an "unsigned 32 bit integer pointer".
        Only for OpenGL|ES this needs to be an "unsigned 16 bit integer pointer".
        \param TextureList: Texture list which is to be mapped onto the geometry object.
        */
        virtual void drawPrimitiveList(
            const ERenderPrimitives Type,
            const scene::SMeshVertex3D* Vertices, u32 VertexCount, const void* Indices, u32 IndexCount,
            std::vector<SMeshSurfaceTexture>* TextureList
        );
        
        //! Updates the light for the renderer
        virtual void updateLight(
            u32 LightID, const scene::ELightModels LightType, bool isVolumetric,
            const dim::vector3df &Direction, f32 SpotInnerConeAngle, f32 SpotOuterConeAngle,
            f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic
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
            void* BufferID, const dim::UniversalBuffer &BufferData, const VertexFormat* Format, const EMeshBufferUsage Usage
        ) = 0;
        //! Updates the specified hardware index buffer by uploading the specified buffer data into VRAM.
        virtual void updateIndexBuffer(
            void* BufferID, const dim::UniversalBuffer &BufferData, const IndexFormat* Format, const EMeshBufferUsage Usage
        ) = 0;
        
        //! Updates the specified hardware vertex buffer only for the specified element.
        virtual void updateVertexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index) = 0;
        //! Updates the specified hardware index buffer only for the specified element.
        virtual void updateIndexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index) = 0;
        
        //! Renders the given hardware mesh buffer.
        virtual void drawMeshBuffer(const MeshBuffer* MeshBuffer) = 0;
        
        /**
        Renders only the plain geometry of the given hardware mesh buffer. No normals for lighting, no tex-coords for texture mapping,
        no fog-coord etc. This can be used to render fast shadow- or depth maps.
        \param MeshBuffer: Specifies the hardware mesh buffer which is to be drawn.
        \param useFirstTextureLayer: Specifies whether the first texture layer is to be bound or not.
        This can be used when the mesh buffer has a transparent color texture and you want to perform alpha-testing.
        \note If the active render system does not support this optimized function the default function "drawMeshBuffer" will be called.
        */
        virtual void drawMeshBufferPlain(const MeshBuffer* MeshBuffer, bool useFirstTextureLayer = false);
        
        /* === Render states === */
        
        /**
        Sets the render state manual. Can be used do enabled or disable effects directly.
        \param Type: Render state type (lighting, fog effect etc.).
        \param State: New state (mostly only true or false).
        */
        virtual void setRenderState(const video::ERenderStates Type, s32 State) = 0;
        //! Returns the current render state.
        virtual s32 getRenderState(const video::ERenderStates Type) const = 0;
        
        virtual void disableTriangleListStates();
        virtual void disable3DRenderStates();
        virtual void disableTexturing();
        
        virtual void setDefaultAlphaBlending();
        
        virtual void enableBlending();
        virtual void disableBlending();
        
        virtual void updateWireframeMode(s32 &ModeFront, s32 &ModeBack);
        
        /* === Lighting === */
        
        virtual void addDynamicLightSource(
            u32 LightID, scene::ELightModels Type,
            video::color &Diffuse, video::color &Ambient, video::color &Specular,
            f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic
        );
        
        virtual void setLightStatus(u32 LightID, bool isEnable);
        virtual void setLightColor(
            u32 LightID, const video::color &Diffuse, const video::color &Ambient, const video::color &Specular
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
        
        /* === Stencil buffering === */
        
        virtual void clearStencilBuffer();
        
        virtual void drawStencilShadowVolume(
            const dim::vector3df* pTriangleList, s32 Count, bool ZFailMethod, bool VolumetricShadow
        );
        virtual void drawStencilShadow(const video::color &Color);
        
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
        virtual ShaderClass* createShaderClass(VertexFormat* VertexInputLayout = 0);
        /**
        Deletes the giben shader class.
        \param ShaderClassObj: Pointer to the shader class which is to be deleted.
        \param DeleteAppendantShaders: Specifies whether the appendant shaders are also to be deleted or not. By default false.
        */
        virtual void deleteShaderClass(ShaderClass* ShaderClassObj, bool DeleteAppendantShaders = false);
        
        /**
        Loads a shader from the disk.
        \param ShaderClassObj: Pointer to a ShaderClass object. Needed to link several shaders (Vertex-, Pixel shaders etc.).
        \param Type: Shader type (Vertex-, Pixel shader etc.).
        \param Version: Shader version (GLSL 1.20/ HLSL Vertex 1.1 etc.).
        \param Filename: Shader filename.
        \param EntryPoint: Shader main function name (only used for DirectX).
        */
        virtual Shader* loadShader(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const io::stringc &Filename, const io::stringc &EntryPoint = ""
        );
        
        /**
        Creates a shader with the specified buffer strings.
        \param ShaderBuffer: List of strings which specify the shader program lines.
        */
        virtual Shader* createShader(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = ""
        );
        
        //! Loads a vertex- and pixel shader, creates a shader class and links the program
        virtual ShaderClass* loadShaderClass(
            const io::stringc &FilenameVertex, const io::stringc &FilenamePixel,
            const EShaderVersions VersionVertex, const EShaderVersions VersionPixel,
            const io::stringc &EntryPointVertex = "", const io::stringc &EntryPointPixel = ""
        );
        
        //! Creates a new Cg shader class. If the engine was compiled without the Cg toolkit this function returns null.
        virtual ShaderClass* createCgShaderClass(VertexFormat* VertexInputLayout = 0);
        
        /**
        Creates a Cg shader.
        \param CompilerOptions: Specifies the compiler options. Invalid options may occur a runtime crash!
        So be careful with this option. Look at the Cg documentation to see the full list of supported compiler options.
        The last options in the array must be a null.
        \return Pointer to the new shader or null if the engine was not compiled with the Cg toolkit.
        */
        virtual Shader* createCgShader(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "",
            const c8** CompilerOptions = 0
        );
        
        //! Deletes the specified shader object.
        virtual void deleteShader(Shader* ShaderObj);
        
        //! Unbinds (or rather disables) all shader types.
        virtual void unbindShaders();
        
        /**
        Runs a compute shader (Only available for Direct3D11).
        \param ShaderObj: Compute shader object which is to be executed.
        \param OutputData: Pointer to the memory where the output data is to be stored.
        \param OutputDataSize: Size of the memory in bytes.
        \param GroupSize: 3 dimensional group size. Minimal size is (1, 1, 1).
        For shader model 4.0 and 4.1 'GroupSize.Z' needs to be 1.
        \return True if the compute shader execution has been succeeded otherwise false.
        */
        virtual bool runComputeShader(
            Shader* ShaderObj, ComputeShaderIO* IOInterface, const dim::vector3di &GroupSize
        );
        
        /**
        Creates a new computer shader input/ouput interface. This is used to set data to and
        to get data from the GPU. Input- and output buffers are created with this interface.
        \return Pointer to a new ComputeShaderIO object.
        */
        virtual ComputeShaderIO* createComputeShaderIO();
        virtual void deleteComputeShaderIO(ComputeShaderIO* &IOInterface);
        
        /* Get list - shader */
        inline std::list<Shader*> getShaderList() const
        {
            return ShaderList_;
        }
        inline std::list<ShaderClass*> getShaderClassList() const
        {
            return ShaderClassList_;
        }
        inline std::list<ComputeShaderIO*> getComputeShaderIOList() const
        {
            return ComputeShaderIOList_;
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
        inline ShaderClass* getGlobalShaderClass() const
        {
            return GlobalShaderClass_;
        }
        
        /* === Simple drawing functions === */
        
        //! Configures the renderer to draw further in 2D. This needs to be called before drawing in 2D (draw2DLine, draw2DImage etc.).
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
        Sets the render target.
        \param Target: Texture which is to be used as the new render target. If the render target shall be reset to the
        actual pixel-buffer set the render target to 0. When using a render target texture the texture needs be created
        to a render target (use "makeRenderTarget" from the Texture class).
        */
        virtual bool setRenderTarget(Texture* Target);
        virtual Texture* getRenderTarget() const;
        
        //! Sets the size for each line which is to be rendered. By default 1. This is not available when using Direct3D.
        virtual void setLineSize(s32 Size = 1);
        
        //! Sets the size for each point which is to be rendered. By default 1.
        virtual void setPointSize(s32 Size = 1);
        
        //! Sets the solid mode. If enabled all 2D images will be drawn for both sides (or rather culling is disabled). By default true.
        inline void setSolidMode(bool isSolidMode)
        {
            isSolidMode_ = isSolidMode;
        }
        inline bool getSolidMode() const
        {
            return isSolidMode_;
        }
        
        /* === Image drawing === */
        
        /**
        Draws a normal 2D image using the specified texture.
        \param Tex: Texture which is to be used. No settings are needed to be done
        to draw 2 dimensional with normal Texture objects.
        \param Position: 2D position in screen space where the image is to be drawn.
        \param Color: Color which the image shall get.
        */
        virtual void draw2DImage(
            Texture* Tex, const dim::point2di &Position, const color &Color = color(255)
        );
        
        /**
        Draws a strechted and mapped 2D image.
        \param Position: 2D area in screen space. In this case the two parameters of
        rect2di ("Right" and "Bottom") specifie the size (width and height).
        \param Clipping: 2D mapping or clipping area. This specifies the texture coordinates for the quad.
        */
        virtual void draw2DImage(
            Texture* Tex, const dim::rect2di &Position,
            const dim::rect2df &Clipping = dim::rect2df(0.0f, 0.0f, 1.0f, 1.0f),
            const color &Color = color(255)
        );
        
        /**
        Draws a rotatable 2D image.
        \param Rotation: Angle (in degrees) for the centered image rotation.
        \param Radius: Radius for the square image.
        */
        virtual void draw2DImage(
            Texture* Tex, const dim::point2di &Position, f32 Rotation, f32 Radius, const color &Color = color(255)
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
            Texture* Tex,
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
            Texture* Tex,
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
        
        //! \return Color of the pixel buffer (color buffer) at the specified position in screen space.
        virtual color getPixelColor(const dim::point2di &Position) const;
        
        //! \return Depth of the pixel buffer (depth buffer) at the specified position in screen space.
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
        Returns the specified texture file. This function loads a texture file only one times.
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
        CubeMap->setDimension(video::TEXTURE_CUBEMAP);
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
        \param FontName: Specifies the type of font. By default the first font which could be found.
        Under MS/Windows this is often "Arial".
        \param FontSize: Specifies the font size. By default the standard OS font size.
        \param Flags: Additional options for the font. This can be a combination of the following values:
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
            Font* FontObject, const dim::point2di &Position, const io::stringc &Text,
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
            Font* FontObject, const dim::matrix4f &Transformation, const io::stringc &Text, const color &Color = color(255)
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
        
        virtual dim::matrix4f getProjectionMatrix() const;
        virtual dim::matrix4f getViewMatrix      () const;
        virtual dim::matrix4f getWorldMatrix     () const;
        virtual dim::matrix4f getTextureMatrix   (u8 TextureLayer = 0) const;
        virtual dim::matrix4f getColorMatrix     () const;
        
        /* === Inline functions === */
        
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
        inline std::list<Texture*> getTextureList() const
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
        
        inline const std::list<VertexFormat*>& getVertexFormatList() const
        {
            return VertexFormatList_;
        }
        
    protected:
        
        /* === Friends === */
        
        friend class Font;
        friend class GLTextureBase;
        friend class OpenGLTexture;
        friend class OpenGLES1Texture;
        friend class GLFramebufferObject;
        friend class sp::SoftPixelDevice;
        friend class VertexFormat;
        friend class VertexFormatUniversal;
        friend class Texture;
        friend class MeshBuffer;
        friend class SoftPixelDevice;
        
        #if defined(SP_PLATFORM_WINDOWS)
        friend class sp::SoftPixelDeviceWin32;
        #endif
        
        #if defined(SP_PLATFORM_LINUX)
        friend class sp::SoftPixelDeviceLinux;
        #endif
        
        /* === Enumerations === */
        
        enum ERenderQueries
        {
            RENDERQUERY_SHADER = 0,
            RENDERQUERY_GEOMETRY_SHADER,
            RENDERQUERY_TESSELLATION_SHADER,
            
            RENDERQUERY_RENDERTARGET,
            RENDERQUERY_MULTISAMPLE_RENDERTARGET,
            RENDERQUERY_MULTI_TEXTURE,
            RENDERQUERY_CUBEMAP_ARRAY,
            
            RENDERQUERY_HARDWARE_MESHBUFFER,
            RENDERQUERY_HARDWARE_INSTANCING,
            
            RENDERQUERY_FOG_COORD,
            
            RENDERQUERY_COUNT,
        };
        
        /* === Structures === */
        
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
        
        /* === Functions === */
        
        RenderSystem(const ERenderSystems Type);
        
        virtual void setDrawingMatrix2D();
        virtual void setDrawingMatrix3D();
        
        virtual Shader* createEmptyShaderWithError(
            const io::stringc &Message, ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version
        );
        
        virtual void createDeviceFont(
            void* FontObject, const io::stringc &FontName, const dim::size2di &FontSize,
            s32 Flags = 0, s32 CharSet = 0
        ) const;
        
        virtual std::vector<SFontGlyph> getCharWidths(void* FontObject) const;
        
        virtual void updateVertexInputLayout(VertexFormat* Format, bool isCreate);
        
        void createDefaultVertexFormats();
        
        virtual void releaseFontObject(Font* FontObj);
        
        virtual void drawTexturedFont(Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color);
        virtual void drawBitmapFont(Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color);
        
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
        std::list<ComputeShaderIO*> ComputeShaderIOList_;
        std::list<Font*>            FontList_;
        std::list<Movie*>           MovieList_;
        
        std::map<std::string, Texture*> TextureMap_;
        
        std::vector<RenderContext*> ContextList_;
        
        /* Semaphores */
        CriticalSection TextureListSemaphore_;
        
        /* States and flags */
        u8 StdFillColor_[4];
        
        SFogStates Fog_;
        STextureCreationFlags TexGenFlags_;
        
        /* Render states */
        dim::matrix4f Matrix2D_;
        dim::matrix4f FontTransform_;
        u32 MaxClippingPlanes_;
        
        bool isFrontFace_;
        bool isSolidMode_;
        
        /* Render target */
        Texture* RenderTarget_;
        
        /* Shader programs */
        ShaderClass* CurShaderClass_;
        ShaderClass* GlobalShaderClass_;
        ShaderSurfaceCallback ShaderSurfaceCallback_;
        
        const MaterialStates* LastMaterial_;
        
        /* Vertex formats */
        std::list<VertexFormat*> VertexFormatList_;
        
        VertexFormatDefault*    VertexFormatDefault_;
        VertexFormatReduced*    VertexFormatReduced_;
        VertexFormatExtended*   VertexFormatExtended_;
        VertexFormatFull*       VertexFormatFull_;
        
        /* Queries */
        bool RenderQuery_[RENDERQUERY_COUNT];
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
