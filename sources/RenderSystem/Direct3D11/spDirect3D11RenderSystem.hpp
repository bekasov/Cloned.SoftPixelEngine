/*
 * Direct3D11 render system header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERSYSTEM_DIRECT3D11_H__
#define __SP_RENDERSYSTEM_DIRECT3D11_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11Texture.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11Shader.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11DefaultShader.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11HardwareBuffer.hpp"

#include <D3D11.h>
#include <D3DX11.h>


namespace sp
{
namespace video
{


//! Direct3D11 render system. This renderer supports Direct3D 11.0.
class SP_EXPORT Direct3D11RenderSystem : public RenderSystem
{
    
    public:
        
        Direct3D11RenderSystem();
        ~Direct3D11RenderSystem();
        
        /* === Initialization functions === */
        
        void setupConfiguration();
        
        /* === Renderer information === */
        
        io::stringc getRenderer()           const;
        io::stringc getVersion()            const;
        io::stringc getVendor()             const;
        io::stringc getShaderVersion()      const;
        
        s32 getMultitexCount()              const;
        s32 getMaxAnisotropicFilter()       const;
        s32 getMaxLightCount()              const;
        
        bool queryVideoSupport(const EVideoFeatureQueries Query) const;
        
        void printWarning();
        
        /* === Video buffer control functions === */
        
        void clearBuffers(const s32 ClearFlags = BUFFER_COLOR | BUFFER_DEPTH);
        
        /* === Configuration functions === */
        
        void setShadeMode(const EShadeModeTypes ShadeMode);
        void setClearColor(const color &Color);
        void setColorMask(bool isRed, bool isGreen, bool isBlue, bool isAlpha = true);
        void setDepthMask(bool isDepth);
        void setAntiAlias(bool isAntiAlias);
        
        /* === Rendering functions === */
        
        bool setupMaterialStates(const MaterialStates* Material, bool Forced);
        
        void bindTextureLayers(const TextureLayerListType &TexLayers);
        void unbindTextureLayers(const TextureLayerListType &TexLayers);
        
        void setupShaderClass(const scene::MaterialNode* Object, ShaderClass* ShaderObject);
        
        void updateMaterialStates(MaterialStates* Material, bool isClear = false);
        
        void updateLight(
            u32 LightID, const scene::ELightModels LightType, bool isVolumetric,
            const dim::vector3df &Direction, f32 SpotInnerConeAngle, f32 SpotOuterConeAngle,
            f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic
        );
        
        /* === Hardware mesh buffers === */
        
        void createVertexBuffer(void* &BufferID);
        void createIndexBuffer(void* &BufferID);
        
        void deleteVertexBuffer(void* &BufferID);
        void deleteIndexBuffer(void* &BufferID);
        
        void updateVertexBuffer(
            void* BufferID, const dim::UniversalBuffer &BufferData, const VertexFormat* Format, const EHWBufferUsage Usage
        );
        void updateIndexBuffer(
            void* BufferID, const dim::UniversalBuffer &BufferData, const IndexFormat* Format, const EHWBufferUsage Usage
        );
        
        void updateVertexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index);
        void updateIndexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index);
        
        void drawMeshBuffer(const MeshBuffer* MeshBuffer);
        
        /* === Render states === */
        
        void setRenderState(const video::ERenderStates Type, s32 State);
        s32 getRenderState(const video::ERenderStates Type) const;
        
        /* === Lighting === */
        
        void addDynamicLightSource(
            u32 LightID, scene::ELightModels Type,
            video::color &Diffuse, video::color &Ambient, video::color &Specular,
            f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic
        );
        
        void setLightStatus(u32 LightID, bool Enable, bool UseAllRCs = false);
        
        void setLightColor(
            u32 LightID,
            const video::color &Diffuse, const video::color &Ambient, const video::color &Specular,
            bool UseAllRCs
        );
        
        /* === Fog effect === */
        
        void setFog(const EFogTypes Type);
        void setFogColor(const video::color &Color);
        void setFogRange(f32 Range, f32 NearPlane = 1.0f, f32 FarPlane = 1000.0f, const EFogModes Mode = FOG_PALE);
        
        /* === Clipping planes === */
        
        void setClipPlane(u32 Index, const dim::plane3df &Plane, bool Enable);
        
        /* === Shader programs === */
        
        ShaderClass* createShaderClass(const VertexFormat* VertexInputLayout = 0);
        
        Shader* createShader(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = ""
        );
        
        Shader* createCgShader(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "",
            const c8** CompilerOptions = 0
        );
        
        void unbindShaders();
        
        bool runComputeShader(Shader* ShaderObj, const dim::vector3di &GroupSize);
        
        bool runComputeShader(
            Shader* ShaderObj, ComputeShaderIO* IOInterface, const dim::vector3di &GroupSize
        );
        
        ComputeShaderIO* createComputeShaderIO();
        
        /* === Simple drawing functions === */
        
        void beginDrawing2D();
        
        void setBlending(const EBlendingTypes SourceBlend, const EBlendingTypes DestBlend);
        void setClipping(bool Enable, const dim::point2di &Position, const dim::size2di &Dimension);
        void setViewport(const dim::point2di &Position, const dim::size2di &Dimension);
        
        bool setRenderTarget(Texture* Target);
        
        /* === Image drawing === */
        
        void draw2DImage(
            const Texture* Tex, const dim::point2di &Position, const color &Color = color(255)
        );
        
        void draw2DImage(
            const Texture* Tex, const dim::rect2di &Position,
            const dim::rect2df &Clipping = dim::rect2df(0.0f, 0.0f, 1.0f, 1.0f),
            const color &Color = color(255)
        );
        
        /* === Primitive drawing === */
        
        void draw2DRectangle(
            const dim::rect2di &Rect, const color &Color = 255, bool isSolid = true
        );
        void draw2DRectangle(
            const dim::rect2di &Rect, const color &lefttopColor, const color &righttopColor,
            const color &rightbottomColor, const color &leftbottomColor, bool isSolid = true
        );
        
        /* === Texture loading and creating === */
        
        Texture* createTexture(const STextureCreationFlags &CreationFlags);
        
        /* === Matrix controll === */
        
        void updateModelviewMatrix();
        
        /* === Special renderer functions === */
        
        inline ID3D11Device* getDirect3DDevice() const
        {
            return D3DDevice_;
        }
        
        /* === Inline functions === */
        
        template <class T> static inline void releaseObject(T* &Object)
        {
            if (Object)
            {
                Object->Release();
                Object = 0;
            }
        }
        
    private:
        
        friend class Direct3D11ShaderClass;
        friend class Direct3D11Shader;
        friend class Direct3D11Texture;
        friend class Direct3D11ComputeShaderIO;
        friend class Direct3D11RenderContext;
        friend class Direct3D11ConstantBuffer;
        friend class Direct3D11ShaderResource;
        friend class D3D11HardwareBuffer;
        friend class VertexFormat;
        
        /* === Macros === */
        
        static const u32 MAX_SHADER_RESOURCES   = 16;
        static const u32 MAX_SAMPLER_STATES     = 16;
        
        /* === Structures === */
        
        struct SConstBuffer2DMain
        {
            dim::matrix4f ProjectionMatrix;
            dim::vector4df Color;
            s32 UseTexture;
        };
        
        struct SConstBuffer2DMapping
        {
            dim::point2df Position;
            dim::point2df TexPosition;
            dim::matrix4f WorldMatrix;
            dim::matrix4f TextureMatrix;
        };
        
        struct SQuad2DVertex
        {
            dim::point2df Position;
            dim::point2df TexCoord;
        };
        
        /* === Functions === */
        
        void createDefaultResources();
        void createRendererStates();
        void createQuad2DVertexBuffer();
        
        void updateShaderResources();
        
        //! \deprecated Use "Direct3D11ShaderResource" class instead.
        ID3D11Buffer* createStructuredBuffer(u32 ElementSize, u32 ElementCount, void* InitData = 0);
        //! \deprecated Use "Direct3D11ShaderResource" class instead.
        ID3D11Buffer* createCPUAccessBuffer(ID3D11Buffer* GPUOutputBuffer);
        //! \deprecated Use "Direct3D11ShaderResource" class instead.
        ID3D11UnorderedAccessView* createUnorderedAccessView(ID3D11Buffer* StructuredBuffer);
        //! \deprecated Use "Direct3D11ShaderResource" class instead.
        ID3D11ShaderResourceView* createShaderResourceView(ID3D11Buffer* StructuredBuffer);
        
        void updateVertexInputLayout(VertexFormat* Format, bool isCreate);
        void addVertexInputLayoutAttribute(std::vector<D3D11_INPUT_ELEMENT_DESC>* InputDesc, const SVertexAttribute &Attrib);
        
        void drawTexturedFont(const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color);
        
        void createTexturedFontVertexBuffer(dim::UniversalBuffer &VertexBuffer, VertexFormatUniversal &VertFormat);
        void setupTexturedFontGlyph(void* &RawVertexData, const SFontGlyph &Glyph, const dim::rect2df &Mapping);
        
        void setupShaderResourceView(u32 Index, ID3D11ShaderResourceView* ResourceView);
        void setupSamplerState(u32 Index, ID3D11SamplerState* SamplerState);
        
        /* === Private members === */
        
        /* Direct3D members */
        
        ID3D11Device* D3DDevice_;
        ID3D11DeviceContext* D3DDeviceContext_;
        
        ID3D11RenderTargetView* RenderTargetView_;
        ID3D11RenderTargetView* OrigRenderTargetView_;
        
        ID3D11Texture2D* DepthStencil_;
        ID3D11DepthStencilView* DepthStencilView_;
        ID3D11DepthStencilView* OrigDepthStencilView_;
        
        ID3D11RasterizerState* RasterizerState_;
        ID3D11DepthStencilState* DepthStencilState_;
        ID3D11BlendState* BlendState_;
        
        IDXGIFactory1* DxGIFactory_;
        
        /* Descriptions */
        
        //D3D11_INPUT_ELEMENT_DESC* VertexLayout2D_;
        
        D3D11_RASTERIZER_DESC RasterizerDesc_;
        D3D11_DEPTH_STENCIL_DESC DepthStencilDesc_;
        D3D11_BLEND_DESC BlendDesc_;
        
        /* Containers */
        
        u32 NumBoundedResources_;
        ID3D11ShaderResourceView* ShaderResourceViewList_[MAX_SHADER_RESOURCES];
        
        u32 NumBoundedSamplers_;
        ID3D11SamplerState* SamplerStateList_[MAX_SAMPLER_STATES];
        
        D3D11VertexBuffer* Quad2DVertexBuffer_;
        
        /* Other members */
        
        bool isMultiSampling_;
        f32 FinalClearColor_[4];
        video::color ClearColor_;
        
        D3D_FEATURE_LEVEL FeatureLevel_;
        
        /* Default basic shader objects */
        
        D3D11DefaultShader DefaultShader_;
        bool UseDefaultBasicShader_;
        
        ShaderClass* DefaultBasicShader2D_;
        VertexFormatUniversal* Draw2DVertFmt_;
        
        SConstBuffer2DMain ConstBuffer2DMain_;
        SConstBuffer2DMapping ConstBuffer2DMapping_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
