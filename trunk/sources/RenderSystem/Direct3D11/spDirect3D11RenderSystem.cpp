/*
 * Direct3D11 render system file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "Base/spInternalDeclarations.hpp"
#include "Base/spTimer.hpp"
#include "Base/spSharedObjects.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Framework/Cg/spCgShaderProgramD3D11.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11HardwareBuffer.hpp"

#include <boost/foreach.hpp>

#include <DXGI.h>
//#pragma comment(lib, "DXGI.lib")


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace video
{


/*
 * ======= Internal members =======
 */

const D3D11_COMPARISON_FUNC D3D11CompareList[] =
{
    D3D11_COMPARISON_NEVER, D3D11_COMPARISON_EQUAL, D3D11_COMPARISON_NOT_EQUAL, D3D11_COMPARISON_LESS,
    D3D11_COMPARISON_LESS_EQUAL, D3D11_COMPARISON_GREATER, D3D11_COMPARISON_GREATER_EQUAL, D3D11_COMPARISON_ALWAYS,
};

const D3D11_BLEND D3D11BlendingList[] =
{
    D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_SRC_COLOR, D3D11_BLEND_INV_SRC_COLOR,
    D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_DEST_COLOR,
    D3D11_BLEND_INV_DEST_COLOR, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_INV_DEST_ALPHA,
};


/*
 * ======= Constructors & destructor =======
 */

Direct3D11RenderSystem::Direct3D11RenderSystem() :
    RenderSystem            (RENDERER_DIRECT3D11),
    D3DDevice_              (0                  ),
    D3DDeviceContext_       (0                  ),
    RenderTargetView_       (0                  ),
    OrigRenderTargetView_   (0                  ),
    DepthStencil_           (0                  ),
    DepthStencilView_       (0                  ),
    OrigDepthStencilView_   (0                  ),
    RasterizerState_        (0                  ),
    DepthStencilState_      (0                  ),
    BlendState_             (0                  ),
    DxGIFactory_            (0                  ),
    NumBoundedSamplers_     (0                  ),
    Quad2DVertexBuffer_     (0                  ),
    isMultiSampling_        (false              ),
    UseDefaultBasicShader_  (true               ),
    DefaultBasicShader2D_   (0                  ),
    Draw2DVertFmt_          (0                  )
{
    /* Initialize memory buffers */
    memset(ShaderResourceViewList_, 0, sizeof(ID3D11ShaderResourceView*) * MAX_COUNT_OF_TEXTURES);
    memset(SamplerStateList_, 0, sizeof(ID3D11SamplerState*) * MAX_COUNT_OF_TEXTURES);
    
    /* Create DXGI factory */
    if (CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&DxGIFactory_)))
        io::Log::warning("Could not get DirectX factory interface");
}
Direct3D11RenderSystem::~Direct3D11RenderSystem()
{
    setRenderTarget(0);
    
    /* Delete objects and lists */
    MemoryManager::deleteList(ComputeShaderIOList_);
    
    delete Draw2DVertFmt_;
    
    /* Release extended interfaces */
    releaseObject(DepthStencilView_     );
    releaseObject(DepthStencil_         );
    releaseObject(RenderTargetView_     );
    releaseObject(Quad2DVertexBuffer_   );
    
    /* Release core interfaces */
    releaseObject(DxGIFactory_);
    //!TODO! -> deleting the device context should be inside the "Direct3D11RenderContext" class!!!
    releaseObject(D3DDeviceContext_ );
    releaseObject(D3DDevice_        );
}


/*
 * ======= Renderer information =======
 */

io::stringc Direct3D11RenderSystem::getRenderer() const
{
    if (!DxGIFactory_)
        return "";
    
    DWORD dwAdapter = 0; 
    IDXGIAdapter1* Adapter = 0;
    
    io::stringc RendererName;
    
    while (DxGIFactory_->EnumAdapters1(dwAdapter++, &Adapter) != DXGI_ERROR_NOT_FOUND) 
    {
        DXGI_ADAPTER_DESC1 AdapterDesc;
        Adapter->GetDesc1(&AdapterDesc);
        
        std::wstring ws(AdapterDesc.Description);
        RendererName += io::stringw(ws).toAscii();
        
        break;
    }
    
    releaseObject(Adapter);
    
    return RendererName;
}

io::stringc Direct3D11RenderSystem::getVersion() const
{
    switch (FeatureLevel_)
    {
        case D3D_FEATURE_LEVEL_11_0:
            return "Direct3D 11.0";
        case D3D_FEATURE_LEVEL_10_1:
            return "Direct3D 10.1";
        case D3D_FEATURE_LEVEL_10_0:
            return "Direct3D 10.0";
        case D3D_FEATURE_LEVEL_9_3:
            return "Direct3D 9.0c";
        case D3D_FEATURE_LEVEL_9_2:
            return "Direct3D 9.0b";
        case D3D_FEATURE_LEVEL_9_1:
            return "Direct3D 9.0a";
    }
    return "Direct3D";
}

io::stringc Direct3D11RenderSystem::getVendor() const
{
    if (!DxGIFactory_)
        return "";
    
    DWORD dwAdapter = 0; 
    IDXGIAdapter1* Adapter = 0;
    
    io::stringc RendererName;
    
    while (DxGIFactory_->EnumAdapters1(dwAdapter++, &Adapter) != DXGI_ERROR_NOT_FOUND) 
    {
        DXGI_ADAPTER_DESC1 AdapterDesc;
        Adapter->GetDesc1(&AdapterDesc);
        
        RendererName = getVendorNameByID(AdapterDesc.VendorId);
        
        break;
    }
    
    releaseObject(Adapter);
    
    return RendererName;
}

io::stringc Direct3D11RenderSystem::getShaderVersion() const
{
    switch (FeatureLevel_)
    {
        case D3D_FEATURE_LEVEL_11_0:
            return "HLSL Shader Model 5.0";
        case D3D_FEATURE_LEVEL_10_1:
            return "HLSL Shader Model 4.1";
        case D3D_FEATURE_LEVEL_10_0:
            return "HLSL Shader Model 4.0";
        case D3D_FEATURE_LEVEL_9_3:
            return "HLSL Shader Model 3.0";
        case D3D_FEATURE_LEVEL_9_2:
            return "HLSL Shader Model 2.0b";
        case D3D_FEATURE_LEVEL_9_1:
            return "HLSL Shader Model 2.0a";
    }
    return "";
}

bool Direct3D11RenderSystem::queryVideoSupport(const EVideoFeatureQueries Query) const
{
    switch (Query)
    {
        case QUERY_MULTI_TEXTURE:
        case QUERY_HARDWARE_MESHBUFFER:
        case QUERY_RENDERTARGET:
        case QUERY_MIPMAPS:
            return true;
            
        case QUERY_SHADER:
        case QUERY_HLSL:
        case QUERY_VERTEX_SHADER_1_1:
        case QUERY_VERTEX_SHADER_2_0:
        case QUERY_PIXEL_SHADER_1_1:
        case QUERY_PIXEL_SHADER_1_2:
        case QUERY_PIXEL_SHADER_1_3:
        case QUERY_PIXEL_SHADER_1_4:
        case QUERY_PIXEL_SHADER_2_0:
            return FeatureLevel_ >= D3D_FEATURE_LEVEL_9_1;
        case QUERY_VERTEX_SHADER_3_0:
        case QUERY_PIXEL_SHADER_3_0:
            return FeatureLevel_ >= D3D_FEATURE_LEVEL_9_3;
        case QUERY_VERTEX_SHADER_4_0:
        case QUERY_VERTEX_SHADER_4_1:
        case QUERY_PIXEL_SHADER_4_0:
        case QUERY_PIXEL_SHADER_4_1:
        case QUERY_GEOMETRY_SHADER:
            return FeatureLevel_ >= D3D_FEATURE_LEVEL_10_0;
        case QUERY_VERTEX_SHADER_5_0:
        case QUERY_PIXEL_SHADER_5_0:
        case QUERY_TESSELLATION_SHADER:
            return FeatureLevel_ >= D3D_FEATURE_LEVEL_11_0;
    }
    
    return false;
}

s32 Direct3D11RenderSystem::getMultitexCount() const
{
    return 8;
}
s32 Direct3D11RenderSystem::getMaxAnisotropicFilter() const
{
    return 16;
}
s32 Direct3D11RenderSystem::getMaxLightCount() const
{
    return 8;
}

void Direct3D11RenderSystem::printWarning()
{
    /* Check which feature level is supported */
    if (FeatureLevel_ < D3D_FEATURE_LEVEL_11_0)
        io::Log::warning("Direct3D version is lower then 11.0; feature level is limited");
}


/*
 * ======= Video buffer control functions =======
 */

void Direct3D11RenderSystem::clearBuffers(const s32 ClearFlags)
{
    /* Clear color buffer */
    if ((ClearFlags & BUFFER_COLOR) != 0)
    {
        D3DDeviceContext_->ClearRenderTargetView(RenderTargetView_, FinalClearColor_);
        
        if (RenderTarget_)
        {
            Direct3D11Texture* Tex = static_cast<Direct3D11Texture*>(RenderTarget_);
            
            if (!Tex->MultiRenderTargetList_.empty())
            {
                for (u32 i = 1; i < Tex->MultiRenderTargetList_.size(); ++i)
                    D3DDeviceContext_->ClearRenderTargetView(Tex->MRTRenderTargetViewList_[i], FinalClearColor_);
            }
        }
    }
    
    /* Clear depth- and stencil view */
    UINT DSFlags = 0;
    
    if (ClearFlags & BUFFER_DEPTH)
        DSFlags |= D3D11_CLEAR_DEPTH;
    if (ClearFlags & BUFFER_STENCIL)
        DSFlags |= D3D11_CLEAR_STENCIL;
    
    if (DSFlags != 0)
        D3DDeviceContext_->ClearDepthStencilView(DepthStencilView_, DSFlags, 1.0f, 0);
}


/*
 * ======= Configuration functions =======
 */

void Direct3D11RenderSystem::setShadeMode(const EShadeModeTypes ShadeMode)
{
    // !TODO!
}

void Direct3D11RenderSystem::setClearColor(const color &Color)
{
    const UINT8 Mask = BlendDesc_.RenderTarget[0].RenderTargetWriteMask;
    
    ClearColor_ = Color;
    
    if (Mask & D3D11_COLOR_WRITE_ENABLE_RED)
        FinalClearColor_[0] = static_cast<f32>(Color.Red) / 255;
    else
        FinalClearColor_[0] = 0.0f;
    
    if (Mask & D3D11_COLOR_WRITE_ENABLE_GREEN)
        FinalClearColor_[1] = static_cast<f32>(Color.Green) / 255;
    else
        FinalClearColor_[1] = 0.0f;
    
    if (Mask & D3D11_COLOR_WRITE_ENABLE_BLUE)
        FinalClearColor_[2] = static_cast<f32>(Color.Blue) / 255;
    else
        FinalClearColor_[2] = 0.0f;
    
    if (Mask & D3D11_COLOR_WRITE_ENABLE_ALPHA)
        FinalClearColor_[3] = static_cast<f32>(Color.Alpha) / 255;
    else
        FinalClearColor_[3] = 0.0f;
    
}

void Direct3D11RenderSystem::setColorMask(bool isRed, bool isGreen, bool isBlue, bool isAlpha)
{
    UINT8* Mask = &BlendDesc_.RenderTarget[0].RenderTargetWriteMask;
    *Mask = 0;
    
    if (isRed)
        *Mask |= D3D11_COLOR_WRITE_ENABLE_RED;
    if (isGreen)
        *Mask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
    if (isBlue)
        *Mask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
    if (isAlpha)
        *Mask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
    
    setClearColor(ClearColor_);
}

void Direct3D11RenderSystem::setDepthMask(bool isDepth)
{
    // !TODO! (DepthStencilDesc_.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_[ZERO/ALL])
}

void Direct3D11RenderSystem::setAntiAlias(bool isAntiAlias)
{
    isMultiSampling_ = isAntiAlias;
}


/*
 * ======= Context functions =======
 */

void Direct3D11RenderSystem::setupConfiguration()
{
    /* Default queries */
    RenderQuery_[RENDERQUERY_SHADER]                = queryVideoSupport(QUERY_SHADER);
    RenderQuery_[RENDERQUERY_MULTI_TEXTURE]         = queryVideoSupport(QUERY_MULTI_TEXTURE);
    RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER]   = queryVideoSupport(QUERY_RENDERTARGET);
    RenderQuery_[RENDERQUERY_RENDERTARGET]          = queryVideoSupport(QUERY_RENDERTARGET);
    
    /* Setup default blend states */
    BlendDesc_.AlphaToCoverageEnable    = FALSE;
    BlendDesc_.IndependentBlendEnable   = FALSE;
    
    for (u32 i = 0; i < 8; ++i)
    {
        D3D11_RENDER_TARGET_BLEND_DESC& Desc = BlendDesc_.RenderTarget[i];
        
        Desc.BlendEnable            = FALSE;
        Desc.SrcBlend               = D3D11_BLEND_ONE;
        Desc.DestBlend              = D3D11_BLEND_ZERO;
        Desc.BlendOp                = D3D11_BLEND_OP_ADD;
        Desc.SrcBlendAlpha          = D3D11_BLEND_ONE;
        Desc.DestBlendAlpha         = D3D11_BLEND_ZERO;
        Desc.BlendOpAlpha           = D3D11_BLEND_OP_ADD;
        Desc.RenderTargetWriteMask  = D3D11_COLOR_WRITE_ENABLE_ALL;
    }
    
    /* Setup default depth-stenicl state */
    DepthStencilDesc_.DepthEnable                   = TRUE;
    DepthStencilDesc_.DepthWriteMask                = D3D11_DEPTH_WRITE_MASK_ALL;
    DepthStencilDesc_.DepthFunc                     = D3D11_COMPARISON_LESS;
    DepthStencilDesc_.StencilEnable                 = FALSE;
    DepthStencilDesc_.StencilReadMask               = D3D11_DEFAULT_STENCIL_READ_MASK;
    DepthStencilDesc_.StencilWriteMask              = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    
    DepthStencilDesc_.FrontFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc_.FrontFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc_.FrontFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc_.FrontFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
    
    DepthStencilDesc_.BackFace.StencilFailOp        = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc_.BackFace.StencilDepthFailOp   = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc_.BackFace.StencilPassOp        = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc_.BackFace.StencilFunc          = D3D11_COMPARISON_ALWAYS;
}


/*
 * ======= Rendering functions =======
 */

bool Direct3D11RenderSystem::setupMaterialStates(const MaterialStates* Material, bool Forced)
{
    /* Check for equality to optimize render path */
    if ( !Material || ( !Forced && ( PrevMaterial_ == Material || Material->compare(PrevMaterial_) ) ) )
        return false;
    
    PrevMaterial_ = Material;
    
    /* Get the material state objects */
    RasterizerState_    = reinterpret_cast<ID3D11RasterizerState*   >(Material->RefRasterizerState_     );
    DepthStencilState_  = reinterpret_cast<ID3D11DepthStencilState* >(Material->RefDepthStencilState_   );
    BlendState_         = reinterpret_cast<ID3D11BlendState*        >(Material->RefBlendState_          );
    
    /* Set material states */
    D3DDeviceContext_->RSSetState(RasterizerState_);
    D3DDeviceContext_->OMSetDepthStencilState(DepthStencilState_, 0);
    D3DDeviceContext_->OMSetBlendState(BlendState_, 0, ~0);
    
    #ifdef SP_DEBUGMODE
    ++RenderSystem::NumMaterialUpdates_;
    #endif
    
    return true;
}

void Direct3D11RenderSystem::bindTextureLayers(const TextureLayerListType &TexLayers)
{
    /* Check if this texture layer list is already bound */
    if (PrevTextureLayers_ == (&TexLayers))
        return;
    
    /* Unbind previously bounded texture layers */
    unbindPrevTextureLayers();
    
    PrevTextureLayers_ = (&TexLayers);
    
    /* Bind all texture layers */
    NumBoundedSamplers_ = 0;
    
    foreach (TextureLayer* TexLayer, TexLayers)
        TexLayer->bind();
    
    /* Set shader resources */
    updateShaderResources();
    
    #ifdef SP_DEBUGMODE
    ++RenderSystem::NumTexLayerBindings_;
    #endif
}

void Direct3D11RenderSystem::unbindTextureLayers(const TextureLayerListType &TexLayers)
{
    /* Unbind all texture layers */
    foreach (TextureLayer* TexLayer, TexLayers)
    {
        TexLayer->unbind();
        
        /* Unbind shader resources */
        const u8 Layer = TexLayer->getIndex();
        
        ShaderResourceViewList_[Layer] = 0;
        SamplerStateList_[Layer] = 0;
    }
    
    /* Set shader resources */
    updateShaderResources();
    
    NumBoundedSamplers_ = 0;
}

void Direct3D11RenderSystem::setupShaderClass(const scene::MaterialNode* Object, ShaderClass* ShaderObject)
{
    /* Shader */
    if (GlobalShaderClass_)
    {
        GlobalShaderClass_->bind(Object);
        UseDefaultBasicShader_ = false;
    }
    else if (ShaderObject)
    {
        ShaderObject->bind(Object);
        UseDefaultBasicShader_ = false;
    }
    else
    {
        DefaultShader_.ShaderClass_->bind(Object);
        UseDefaultBasicShader_ = true;
    }
}

void Direct3D11RenderSystem::updateMaterialStates(MaterialStates* Material, bool isClear)
{
    RenderSystem::updateMaterialStates(Material, isClear);
    
    /* Get the material state objects */
    RasterizerState_    = reinterpret_cast<ID3D11RasterizerState*   >(Material->RefRasterizerState_     );
    DepthStencilState_  = reinterpret_cast<ID3D11DepthStencilState* >(Material->RefDepthStencilState_   );
    BlendState_         = reinterpret_cast<ID3D11BlendState*        >(Material->RefBlendState_          );
    
    /* Rlease the old objects */
    releaseObject(RasterizerState_);
    releaseObject(DepthStencilState_);
    releaseObject(BlendState_);
    
    /* Check if the objects only shall be deleted */
    if (isClear)
        return;
    
    /* === Configure material state descriptions === */
    
    /* Cull facing */
    switch (Material->getRenderFace())
    {
        case video::FACE_FRONT:
            RasterizerDesc_.CullMode = D3D11_CULL_BACK; break;
        case video::FACE_BACK:
            RasterizerDesc_.CullMode = D3D11_CULL_FRONT; break;
        case video::FACE_BOTH:
            RasterizerDesc_.CullMode = D3D11_CULL_NONE; break;
    }
    
    /* Depth functions */
    if (Material->getDepthBuffer())
    {
        DepthStencilDesc_.DepthEnable       = true;
        DepthStencilDesc_.DepthWriteMask    = D3D11_DEPTH_WRITE_MASK_ALL;
        DepthStencilDesc_.DepthFunc         = D3D11CompareList[Material->getDepthMethod()];
    }
    else
        DepthStencilDesc_.DepthEnable       = false;
    
    /* Blending mode */
    if (Material->getBlending())
    {
        BlendDesc_.RenderTarget[0].BlendEnable      = true;
        
        BlendDesc_.RenderTarget[0].BlendOp          = D3D11_BLEND_OP_ADD;
        BlendDesc_.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
        
        BlendDesc_.RenderTarget[0].SrcBlend         = D3D11BlendingList[Material->getBlendSource()];
        BlendDesc_.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_SRC_ALPHA;
        
        BlendDesc_.RenderTarget[0].DestBlend        = D3D11BlendingList[Material->getBlendTarget()];
        BlendDesc_.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_DEST_ALPHA;
    }
    else
        BlendDesc_.RenderTarget[0].BlendEnable      = false;
    
    /* Polygon mode */
    switch (Material->getWireframeFront()) // !!!
    {
        case WIREFRAME_SOLID:
            RasterizerDesc_.FillMode = D3D11_FILL_SOLID; break;
        case WIREFRAME_LINES:
            RasterizerDesc_.FillMode = D3D11_FILL_WIREFRAME; break;
        case WIREFRAME_POINTS:
            RasterizerDesc_.FillMode = D3D11_FILL_WIREFRAME; break;
    }
    
    /* Polygon offset */
    RasterizerDesc_.SlopeScaledDepthBias    = Material->getPolygonOffsetFactor();
    RasterizerDesc_.DepthBias               = static_cast<s32>(Material->getPolygonOffsetUnits());
    RasterizerDesc_.DepthBiasClamp          = 0.0f;
    RasterizerDesc_.DepthClipEnable         = DepthRange_.Enabled;
    
    /* Other rasterizer states */
    RasterizerDesc_.FrontCounterClockwise   = false;
    RasterizerDesc_.ScissorEnable           = false;
    
    /* Anti-aliasing */
    RasterizerDesc_.MultisampleEnable       = isMultiSampling_;
    RasterizerDesc_.AntialiasedLineEnable   = isMultiSampling_;
    
    /* Recreate the material states */
    D3DDevice_->CreateRasterizerState   (&RasterizerDesc_,      &RasterizerState_   );
    D3DDevice_->CreateDepthStencilState (&DepthStencilDesc_,    &DepthStencilState_ );
    D3DDevice_->CreateBlendState        (&BlendDesc_,           &BlendState_        );
    
    /* Update the material state objects */
    Material->RefRasterizerState_   = RasterizerState_;
    Material->RefDepthStencilState_ = DepthStencilState_;
    Material->RefBlendState_        = BlendState_;
}

void Direct3D11RenderSystem::updateLight(
    u32 LightID, const scene::ELightModels LightType, bool isVolumetric,
    const dim::vector3df &Direction, f32 SpotInnerConeAngle, f32 SpotOuterConeAngle,
    f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic)
{
    if (UseDefaultBasicShader_)
    {
        DefaultShader_.setupLight(
            LightID, LightType, isVolumetric,
            Direction, SpotInnerConeAngle, SpotOuterConeAngle,
            AttenuationConstant, AttenuationLinear, AttenuationQuadratic
        );
    }
}


/*
 * ======= Hardware mesh buffers =======
 */

void Direct3D11RenderSystem::createVertexBuffer(void* &BufferID)
{
    BufferID = new D3D11HardwareBuffer();
}
void Direct3D11RenderSystem::createIndexBuffer(void* &BufferID)
{
    BufferID = new D3D11HardwareBuffer();
}

void Direct3D11RenderSystem::deleteVertexBuffer(void* &BufferID)
{
    if (BufferID)
    {
        delete static_cast<D3D11HardwareBuffer*>(BufferID);
        BufferID = 0;
    }
}
void Direct3D11RenderSystem::deleteIndexBuffer(void* &BufferID)
{
    deleteVertexBuffer(BufferID);
}

void Direct3D11RenderSystem::updateVertexBuffer(
    void* BufferID, const dim::UniversalBuffer &BufferData, const VertexFormat* Format, const EHWBufferUsage Usage)
{
    if (BufferID && Format)
    {
        D3D11HardwareBuffer* Buffer = static_cast<D3D11HardwareBuffer*>(BufferID);
        Buffer->update(
            D3DDevice_, D3DDeviceContext_, BufferData,
            DATATYPE_UNSIGNED_INT, Usage, D3D11_BIND_VERTEX_BUFFER, "vertex"
        );
    }
}

void Direct3D11RenderSystem::updateIndexBuffer(
    void* BufferID, const dim::UniversalBuffer &BufferData, const IndexFormat* Format, const EHWBufferUsage Usage)
{
    if (BufferID && Format)
    {
        D3D11HardwareBuffer* Buffer = static_cast<D3D11HardwareBuffer*>(BufferID);
        Buffer->update(
            D3DDevice_, D3DDeviceContext_, BufferData,
            Format->getDataType(), Usage, D3D11_BIND_INDEX_BUFFER, "index"
        );
    }
}

void Direct3D11RenderSystem::updateVertexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index)
{
    if (BufferID && BufferData.getSize())
    {
        D3D11HardwareBuffer* Buffer = static_cast<D3D11HardwareBuffer*>(BufferID);
        Buffer->update(D3DDeviceContext_, BufferData, Index);
    }
}

void Direct3D11RenderSystem::updateIndexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index)
{
    if (BufferID && BufferData.getSize())
    {
        D3D11HardwareBuffer* Buffer = static_cast<D3D11HardwareBuffer*>(BufferID);
        Buffer->update(D3DDeviceContext_, BufferData, Index);
    }
}

void Direct3D11RenderSystem::drawMeshBuffer(const MeshBuffer* MeshBuffer)
{
    /* Get reference mesh buffer */
    if (!MeshBuffer)
        return;
    
    const video::MeshBuffer* OrigMeshBuffer = MeshBuffer;
    MeshBuffer = MeshBuffer->getReference();
    
    if (!MeshBuffer->renderable())
        return;
    
    /* Surface callback */
    if (CurShaderClass_ && ShaderSurfaceCallback_)
        ShaderSurfaceCallback_(CurShaderClass_, MeshBuffer->getTextureLayerList());
    
    /* Update the default basic shader's constant buffers */
    if (UseDefaultBasicShader_)
    {
        DefaultShader_.updateObject(__spSceneManager->getActiveMesh()); //!TODO! <- this should be called only once for a mesh object
        DefaultShader_.updateTextureLayers(MeshBuffer->getTextureLayerList());
    }
    
    /* Get hardware vertex- and index buffers */
    D3D11HardwareBuffer* VertexBuffer   = static_cast<D3D11HardwareBuffer*>(MeshBuffer->getVertexBufferID());
    D3D11HardwareBuffer* IndexBuffer    = static_cast<D3D11HardwareBuffer*>(MeshBuffer->getIndexBufferID());
    
    /* Setup triangle topology */
    D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    
    if (CurShaderClass_->getHullShader() && CurShaderClass_->getDomainShader() &&
        CurShaderClass_->getHullShader()->valid() && CurShaderClass_->getDomainShader()->valid())
    {
        Topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    }
    else
    {
        switch (MeshBuffer->getPrimitiveType())
        {
            case PRIMITIVE_POINTS:
                Topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
                break;
            case PRIMITIVE_LINES:
                Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
                break;
            case PRIMITIVE_LINE_STRIP:
                Topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
                break;
            case PRIMITIVE_TRIANGLE_STRIP:
                Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                break;
            default:
                break;
        }
    }
    
    D3DDeviceContext_->IASetPrimitiveTopology(Topology);
    
    /* Bind textures */
    if (__isTexturing)
        bindTextureLayers(OrigMeshBuffer->getTextureLayerList());
    else
        unbindPrevTextureLayers();
    
    const u32 Stride = MeshBuffer->getVertexFormat()->getFormatSize();
    const u32 Offset = 0;
    
    /* Draw the primitives */
    if (MeshBuffer->getIndexBufferEnable())
    {
        /* Bind the mesh buffer */
        D3DDeviceContext_->IASetIndexBuffer(IndexBuffer->HWBuffer_, IndexBuffer->FormatFlags_, 0);
        D3DDeviceContext_->IASetVertexBuffers(0, 1, &VertexBuffer->HWBuffer_, &Stride, &Offset);
        
        /* Render the triangles */
        if (MeshBuffer->getHardwareInstancing() > 1)
        {
            D3DDeviceContext_->DrawIndexedInstanced(
                MeshBuffer->getIndexCount(),
                MeshBuffer->getHardwareInstancing(),
                0, 0, 0
            );
        }
        else
            D3DDeviceContext_->DrawIndexed(MeshBuffer->getIndexCount(), 0, 0);
    }
    else
    {
        /* Bind the vertex buffer */
        D3DDeviceContext_->IASetVertexBuffers(0, 1, &VertexBuffer->HWBuffer_, &Stride, &Offset);
        
        /* Render the triangles */
        if (MeshBuffer->getHardwareInstancing() > 1)
        {
            D3DDeviceContext_->DrawInstanced(
                MeshBuffer->getVertexCount(),
                MeshBuffer->getHardwareInstancing(),
                0, 0
            );
        }
        else
            D3DDeviceContext_->Draw(MeshBuffer->getVertexCount(), 0);
    }
    
    #ifdef SP_DEBUGMODE
    ++RenderSystem::NumDrawCalls_;
    ++RenderSystem::NumMeshBufferBindings_;
    #endif
}


/*
 * ======= Render states =======
 */

void Direct3D11RenderSystem::setRenderState(const video::ERenderStates Type, s32 State)
{
    switch (Type)
    {
        case RENDER_TEXTURE:
            __isTexturing = (State != 0);
            break;
        default:
            break;
    }
}

s32 Direct3D11RenderSystem::getRenderState(const video::ERenderStates Type) const
{
    switch (Type)
    {
        case RENDER_TEXTURE:
            return __isTexturing ? 1 : 0;
        default:
            break;
    }
    return 0;
}


/*
 * ======= Lighting =======
 */

void Direct3D11RenderSystem::addDynamicLightSource(
    u32 LightID, scene::ELightModels Type,
    video::color &Diffuse, video::color &Ambient, video::color &Specular,
    f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic)
{
    DefaultShader_.setupLightStatus(LightID, true);
    DefaultShader_.setupLight(
        LightID, Type, false, dim::vector3df(0, 0, 1), 30.0f, 60.0f,
        AttenuationConstant, AttenuationLinear, AttenuationQuadratic
    );
    DefaultShader_.setupLightColor(LightID, Diffuse, Ambient, Specular);
}

void Direct3D11RenderSystem::setLightStatus(u32 LightID, bool Enable, bool UseAllRCs)
{
    DefaultShader_.setupLightStatus(LightID, Enable);
}

void Direct3D11RenderSystem::setLightColor(
    u32 LightID, const video::color &Diffuse, const video::color &Ambient, const video::color &Specular, bool UseAllRCs)
{
    DefaultShader_.setupLightColor(LightID, Diffuse, Ambient, Specular);
}


/*
 * ======= Fog effect =======
 */

void Direct3D11RenderSystem::setFog(const EFogTypes Type)
{
    Fog_.Type = Type;
    __isFog = (Fog_.Type != FOG_NONE);
    
    DefaultShader_.setupFog(Type, Fog_.Mode, Fog_.Range, Fog_.Near, Fog_.Far);
}

void Direct3D11RenderSystem::setFogColor(const video::color &Color)
{
    DefaultShader_.setupFogColor(Color);
}

void Direct3D11RenderSystem::setFogRange(f32 Range, f32 NearPlane, f32 FarPlane, const EFogModes Mode)
{
    RenderSystem::setFogRange(Range, NearPlane, FarPlane, Mode);
    DefaultShader_.setupFog(Fog_.Type, Mode, Range, NearPlane, FarPlane);
}


/*
 * ======= Clipping planes =======
 */

void Direct3D11RenderSystem::setClipPlane(u32 Index, const dim::plane3df &Plane, bool Enable)
{
    DefaultShader_.setupClipPlane(Index, Plane, Enable);
}


/*
 * ======= Shader programs =======
 */

ShaderClass* Direct3D11RenderSystem::createShaderClass(VertexFormat* VertexInputLayout)
{
    ShaderClass* NewShaderClass = new Direct3D11ShaderClass(VertexInputLayout);
    ShaderClassList_.push_back(NewShaderClass);
    return NewShaderClass;
}

Shader* Direct3D11RenderSystem::createShader(
    ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
    const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint)
{
    Shader* NewShader = new Direct3D11Shader(ShaderClassObj, Type, Version);
    
    NewShader->compile(ShaderBuffer, EntryPoint);
    
    if (!ShaderClassObj)
        NewShader->getShaderClass()->link();
    
    ShaderList_.push_back(NewShader);
    
    return NewShader;
}

Shader* Direct3D11RenderSystem::createCgShader(
    ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
    const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint,
    const c8** CompilerOptions)
{
    Shader* NewShader = 0;
    
    #ifndef SP_COMPILE_WITH_CG
    io::Log::error("This engine was not compiled with the Cg toolkit");
    #else
    if (RenderQuery_[RENDERQUERY_SHADER])
        NewShader = new CgShaderProgramD3D11(ShaderClassObj, Type, Version);
    else
    #endif
        return 0;
    
    NewShader->compile(ShaderBuffer, EntryPoint, CompilerOptions);
    
    if (!ShaderClassObj)
        NewShader->getShaderClass()->link();
    
    ShaderList_.push_back(NewShader);
    
    return NewShader;
}

void Direct3D11RenderSystem::unbindShaders()
{
    D3DDeviceContext_->VSSetShader(0, 0, 0);
    D3DDeviceContext_->PSSetShader(0, 0, 0);
    D3DDeviceContext_->GSSetShader(0, 0, 0);
    D3DDeviceContext_->HSSetShader(0, 0, 0);
    D3DDeviceContext_->DSSetShader(0, 0, 0);
}

bool Direct3D11RenderSystem::runComputeShader(
    Shader* ShaderObj, ComputeShaderIO* IOInterface, const dim::vector3di &GroupSize)
{
    /* Check for wrong arguments */
    if (!ShaderObj || !IOInterface || GroupSize.X < 1 || GroupSize.Y < 1 || GroupSize.Z < 1)
    {
        io::Log::error("Invalid arguments for compute shader execution");
        return false;
    }
    
    if (ShaderObj->getType() != video::SHADER_COMPUTE)
    {
        io::Log::error("Specified object is not a compute shader");
        return false;
    }
    
    if (GroupSize.Z > 1 && FeatureLevel_ < D3D_FEATURE_LEVEL_11_0)
    {
        io::Log::error("Compute shader execution with group size Z greater than 1 is only supported since shader model 5.0");
        return false;
    }
    
    Direct3D11Shader* ComputeShader = static_cast<Direct3D11Shader*>(ShaderObj);
    Direct3D11ComputeShaderIO* D3D11IOInterface = static_cast<Direct3D11ComputeShaderIO*>(IOInterface);
    
    /* Bind the compute shader and shader resources- and unordered access views */
    D3DDeviceContext_->CSSetShader(ComputeShader->ComputeShaderObject_, 0, 0);
    
    if (!D3D11IOInterface->InputBuffers_.empty())
        D3DDeviceContext_->CSSetShaderResources(0, D3D11IOInterface->InputBuffers_.size(), &D3D11IOInterface->InputBuffers_[0]);
    
    if (!D3D11IOInterface->OutputBuffers_.empty())
        D3DDeviceContext_->CSSetUnorderedAccessViews(0, D3D11IOInterface->OutputBuffers_.size(), &D3D11IOInterface->OutputBuffers_[0], 0);
    
    if (!ComputeShader->ConstantBuffers_.empty())
        D3DDeviceContext_->CSSetConstantBuffers(0, ComputeShader->ConstantBuffers_.size(), &ComputeShader->ConstantBuffers_[0]);
    
    /* Start the dispatch pipeline */
    D3DDeviceContext_->Dispatch(GroupSize.X, GroupSize.Y, GroupSize.Z);
    
    /* Reset all compute shader settings */
    D3DDeviceContext_->CSSetShader(0, 0, 0);
    D3DDeviceContext_->CSSetShaderResources(0, 0, 0);
    D3DDeviceContext_->CSSetUnorderedAccessViews(0, 0, 0, 0);
    D3DDeviceContext_->CSSetConstantBuffers(0, 0, 0);
    
    return true;
}

ComputeShaderIO* Direct3D11RenderSystem::createComputeShaderIO()
{
    ComputeShaderIO* NewIOInterface = new Direct3D11ComputeShaderIO();
    ComputeShaderIOList_.push_back(NewIOInterface);
    return NewIOInterface;
}


/*
 * ======= Simple drawing functions =======
 */

void Direct3D11RenderSystem::beginDrawing2D()
{
    /* Unit matrices */
    setViewMatrix(dim::matrix4f::IDENTITY);
    setWorldMatrix(dim::matrix4f::IDENTITY);
    
    Matrix2D_.make2Dimensional(
        gSharedObjects.ScreenWidth,
        -gSharedObjects.ScreenHeight,
        gSharedObjects.ScreenWidth,
        gSharedObjects.ScreenHeight
    );
    setProjectionMatrix(Matrix2D_);
    
    setViewport(0, dim::size2di(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight));
    
    RenderSystem::beginDrawing2D();
}

void Direct3D11RenderSystem::setBlending(const EBlendingTypes SourceBlend, const EBlendingTypes DestBlend)
{
    // !TODO!
}

void Direct3D11RenderSystem::setClipping(bool Enable, const dim::point2di &Position, const dim::size2di &Dimension)
{
    D3D11_RECT Rect;
    {
        Rect.left   = Position.X;
        Rect.top    = Position.Y;
        Rect.right  = Position.X + Dimension.Width;
        Rect.bottom = Position.Y + Dimension.Height;
    }
    D3DDeviceContext_->RSSetScissorRects(1, &Rect);
}

void Direct3D11RenderSystem::setViewport(const dim::point2di &Position, const dim::size2di &Dimension)
{
    D3D11_VIEWPORT Viewport;
    {
        Viewport.TopLeftX   = static_cast<f32>(Position.X);
        Viewport.TopLeftY   = static_cast<f32>(Position.Y);
        Viewport.Width      = static_cast<f32>(Dimension.Width);
        Viewport.Height     = static_cast<f32>(Dimension.Height);
        Viewport.MinDepth   = DepthRange_.Near;
        Viewport.MaxDepth   = DepthRange_.Far;
    }
    D3DDeviceContext_->RSSetViewports(1, &Viewport);
}

bool Direct3D11RenderSystem::setRenderTarget(Texture* Target)
{
    if (RenderTarget_ != Target && RenderTarget_ && RenderTarget_->getMipMapping())
        D3DDeviceContext_->GenerateMips(static_cast<Direct3D11Texture*>(RenderTarget_)->ShaderResourceView_);
    
    if (Target && Target->getRenderTarget())
    {
        Direct3D11Texture* Tex = static_cast<Direct3D11Texture*>(Target);
        
        if (Tex->DepthStencilView_)
            DepthStencilView_ = Tex->DepthStencilView_;
        
        if (Target->getDimension() == TEXTURE_CUBEMAP)
            RenderTargetView_ = Tex->RenderTargetViewCubeMap_[static_cast<s32>(Target->getCubeMapFace())];
        else
            RenderTargetView_ = Tex->RenderTargetView_;
        
        if (!Tex->MultiRenderTargetList_.empty())
        {
            D3DDeviceContext_->OMSetRenderTargets(
                Tex->MRTRenderTargetViewList_.size(),
                &Tex->MRTRenderTargetViewList_[0],
                DepthStencilView_
            );
        }
        else
            D3DDeviceContext_->OMSetRenderTargets(1, &RenderTargetView_, DepthStencilView_);
        
        RenderTarget_ = Target;
    }
    else if (RenderTarget_)
    {
        RenderTargetView_ = OrigRenderTargetView_;
        DepthStencilView_ = OrigDepthStencilView_;
        
        D3DDeviceContext_->OMSetRenderTargets(1, &RenderTargetView_, DepthStencilView_);
        
        RenderTarget_ = 0;
    }
    
    return true;
}


/*
 * ======= Image drawing =======
 */

void Direct3D11RenderSystem::draw2DImage(
    const Texture* Tex, const dim::point2di &Position, const color &Color)
{
    draw2DImage(Tex, dim::rect2di(Position.X, Position.Y, Tex->getSize().Width, Tex->getSize().Height));
}

void Direct3D11RenderSystem::draw2DImage(
    const Texture* Tex, const dim::rect2di &Position, const dim::rect2df &Clipping, const color &Color)
{
    /* Setup 2D drawing */
    setup2DDrawing();
    
    if (!Quad2DVertexBuffer_)
        return;
    
    /* Setup default 2D drawing shader when no one is used */
    if (UseDefaultBasicShader_) //!TODO! -> does not work correctly!!!
    {
        /* Setup main constant buffer */
        ConstBuffer2DMain_.ProjectionMatrix = getProjectionMatrix();
        
        ConstBuffer2DMain_.Color = dim::vector4df(Color.getVector(true), static_cast<f32>(Color.Alpha) / 255.0f);
        
        ConstBuffer2DMain_.UseTexture = (Tex != 0 ? 1 : 0);
        
        DefaultBasicShader2D_->getVertexShader()->setConstantBuffer(0, &ConstBuffer2DMain_);
        DefaultBasicShader2D_->getPixelShader()->setConstantBuffer(0, &ConstBuffer2DMain_);
        
        /* Setup mapping constant buffer */
        ConstBuffer2DMapping_.Position.X = static_cast<f32>(Position.Left);
        ConstBuffer2DMapping_.Position.Y = static_cast<f32>(Position.Top);
        
        ConstBuffer2DMapping_.TexPosition.X = Clipping.Left;
        ConstBuffer2DMapping_.TexPosition.Y = Clipping.Top;
        
        const dim::point2df Scale(Position.getRBPoint().cast<f32>());
        ConstBuffer2DMapping_.WorldMatrix.reset();
        ConstBuffer2DMapping_.WorldMatrix[0] = Scale.X;
        ConstBuffer2DMapping_.WorldMatrix[5] = Scale.Y;
        
        ConstBuffer2DMapping_.TextureMatrix.reset();
        ConstBuffer2DMapping_.TextureMatrix[0] = Clipping.Right - Clipping.Left;
        ConstBuffer2DMapping_.TextureMatrix[5] = Clipping.Bottom - Clipping.Top;
        
        DefaultBasicShader2D_->getVertexShader()->setConstantBuffer(1, &ConstBuffer2DMapping_);
        DefaultBasicShader2D_->getPixelShader()->setConstantBuffer(1, &ConstBuffer2DMapping_);
        
        /* Bind default drawing shader */
        DefaultBasicShader2D_->bind();
    }
    
    /* Bind texture */
    if (Tex)
        Tex->bind(0);
    
    /* Update shader resources for texture samplers */
    updateShaderResources();
    
    D3DDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    
    /* Temporary values */
    const u32 Stride = sizeof(SQuad2DVertex);
    const u32 Offset = 0;
    
    /* Draw the 2D quad */
    D3DDeviceContext_->IASetVertexBuffers(0, 1, &Quad2DVertexBuffer_, &Stride, &Offset);
    D3DDeviceContext_->Draw(4, 0);
    
    /* Unbind texture */
    if (Tex)
        Tex->unbind(0);
}


/*
 * ======= Primitive drawing =======
 */

void Direct3D11RenderSystem::draw2DRectangle(const dim::rect2di &Rect, const color &Color, bool isSolid)
{
    draw2DImage(
        0, dim::rect2di(Rect.Left, Rect.Top, Rect.getWidth(), Rect.getHeight()),
        dim::rect2df(0, 0, 1, 1), Color
    );
}

void Direct3D11RenderSystem::draw2DRectangle(
    const dim::rect2di &Rect, const color &lefttopColor, const color &righttopColor,
    const color &rightbottomColor, const color &leftbottomColor, bool isSolid)
{
    //todo -> this is incomplete
    draw2DRectangle(Rect, lefttopColor, isSolid);
}


/*
 * ======= Texture loading and creating =======
 */

Texture* Direct3D11RenderSystem::createTexture(const STextureCreationFlags &CreationFlags)
{
    /* Create Direct3D11 texture */
    Texture* NewTexture = new Direct3D11Texture(D3DDevice_, D3DDeviceContext_, CreationFlags);
    
    /* Add the texture to the texture list */
    TextureListSemaphore_.lock();
    TextureList_.push_back(NewTexture);
    TextureListSemaphore_.unlock();
    
    return NewTexture;
}


/*
 * ======= Matrix controll =======
 */

void Direct3D11RenderSystem::updateModelviewMatrix()
{
}


/*
 * ======= Private: =======
 */

void Direct3D11RenderSystem::createDefaultResources()
{
    /* Create basic default resources */
    RenderSystem::createDefaultResources();
    
    const u64 TmpTime = io::Timer::millisecs();
    
    /* Create default drawing vertex format */
    Draw2DVertFmt_ = new VertexFormatUniversal();
    
    Draw2DVertFmt_->addCoord(DATATYPE_FLOAT, 2);
    Draw2DVertFmt_->addTexCoord();
    
    /* Create default shaders */
    io::Log::message("Compiling Default Shaders (Shader Model 4.0) ... ", io::LOG_NONEWLINE);
    
    if (!DefaultShader_.createShader())
        return;
    
    DefaultBasicShader2D_ = createShaderClass(Draw2DVertFmt_);
    
    if (queryVideoSupport(QUERY_VERTEX_SHADER_4_0))
    {
        std::list<io::stringc> ShaderBuffer;
        ShaderBuffer.push_back(
            #include "Resources/spDefaultDrawingShaderStr.hlsl"
        );
        
        createShader(
            DefaultBasicShader2D_, SHADER_VERTEX, HLSL_VERTEX_4_0, ShaderBuffer, "VertexMain"
        );
        createShader(
            DefaultBasicShader2D_, SHADER_PIXEL, HLSL_PIXEL_4_0, ShaderBuffer, "PixelMain"
        );
    }
    else
    {
        io::Log::error("Could not create default basic shaders because shader model is less than 4.0");
        return;
    }
    
    DefaultBasicShader2D_->link();
    
    io::Log::message(
        io::stringc(static_cast<u32>(io::Timer::millisecs() - TmpTime)) + " ms.", 0
    );
    
    createRendererStates();
}

void Direct3D11RenderSystem::createRendererStates()
{
    /* Create renderer states */
    ZeroMemory(&RasterizerDesc_,    sizeof(D3D11_RASTERIZER_DESC    ));
    ZeroMemory(&DepthStencilDesc_,  sizeof(D3D11_DEPTH_STENCIL_DESC ));
    ZeroMemory(&BlendDesc_,         sizeof(D3D11_BLEND_DESC         ));
    
    for (s32 i = 0; i < 8; ++i)
        BlendDesc_.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    
    /* Initialize clipping planes */
    MaxClippingPlanes_ = 8;
    DefaultShader_.updateExtensions();
    
    createQuad2DVertexBuffer();
}

void Direct3D11RenderSystem::updateShaderResources()
{
    if (CurShaderClass_)
    {
        if (CurShaderClass_->getVertexShader())
        {
            D3DDeviceContext_->VSSetShaderResources(0, NumBoundedSamplers_, ShaderResourceViewList_);
            D3DDeviceContext_->VSSetSamplers(0, NumBoundedSamplers_, SamplerStateList_);
        }
        if (CurShaderClass_->getPixelShader())
        {
            D3DDeviceContext_->PSSetShaderResources(0, NumBoundedSamplers_, ShaderResourceViewList_);
            D3DDeviceContext_->PSSetSamplers(0, NumBoundedSamplers_, SamplerStateList_);
        }
        if (CurShaderClass_->getGeometryShader())
        {
            D3DDeviceContext_->GSSetShaderResources(0, NumBoundedSamplers_, ShaderResourceViewList_);
            D3DDeviceContext_->GSSetSamplers(0, NumBoundedSamplers_, SamplerStateList_);
        }
        if (CurShaderClass_->getHullShader())
        {
            D3DDeviceContext_->HSSetShaderResources(0, NumBoundedSamplers_, ShaderResourceViewList_);
            D3DDeviceContext_->HSSetSamplers(0, NumBoundedSamplers_, SamplerStateList_);
        }
        if (CurShaderClass_->getDomainShader())
        {
            D3DDeviceContext_->DSSetShaderResources(0, NumBoundedSamplers_, ShaderResourceViewList_);
            D3DDeviceContext_->DSSetSamplers(0, NumBoundedSamplers_, SamplerStateList_);
        }
    }
}

void Direct3D11RenderSystem::createQuad2DVertexBuffer()
{
    const SQuad2DVertex VertexList[] =
    {
        { dim::point2df(0, 0), dim::point2df(0, 0) },
        { dim::point2df(1, 0), dim::point2df(1, 0) },
        { dim::point2df(0, 1), dim::point2df(0, 1) },
        { dim::point2df(1, 1), dim::point2df(1, 1) }
    };
    
    D3D11_SUBRESOURCE_DATA ResourceData;
    ZeroMemory(&ResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    
    ResourceData.pSysMem = VertexList;
    
    /* Create the vertex buffer */
    D3D11_BUFFER_DESC VertexDesc;
    ZeroMemory(&VertexDesc, sizeof(D3D11_BUFFER_DESC));
    
    VertexDesc.Usage                = D3D11_USAGE_DEFAULT;
    VertexDesc.ByteWidth            = sizeof(SQuad2DVertex) * 4;
    VertexDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER;
    VertexDesc.CPUAccessFlags       = 0;
    VertexDesc.StructureByteStride  = sizeof(SQuad2DVertex);
    
    HRESULT Result = D3DDevice_->CreateBuffer(&VertexDesc, &ResourceData, &Quad2DVertexBuffer_);
    
    if (Result || !Quad2DVertexBuffer_)
        io::Log::error("Could not create vertex buffer for 2D-quad");
}

ID3D11Buffer* Direct3D11RenderSystem::createStructuredBuffer(u32 ElementSize, u32 ElementCount, void* InitData)
{
    /* Configure buffer description */
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    
    BufferDesc.BindFlags            = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    BufferDesc.ByteWidth            = ElementSize * ElementCount;
    BufferDesc.StructureByteStride  = ElementSize;
    BufferDesc.MiscFlags            = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    
    /* Create structured buffer */
    HRESULT Result = 0;
    ID3D11Buffer* Buffer = 0;
    
    if (InitData)
    {
        D3D11_SUBRESOURCE_DATA ResourceData;
        ResourceData.pSysMem = InitData;
        Result = D3DDevice_->CreateBuffer(&BufferDesc, &ResourceData, &Buffer);
    }
    else
        Result = D3DDevice_->CreateBuffer(&BufferDesc, 0, &Buffer);
    
    if (Result)
    {
        io::Log::error("Could not create structured buffer");
        return 0;
    }
    
    return Buffer;
}

ID3D11Buffer* Direct3D11RenderSystem::createCPUAccessBuffer(ID3D11Buffer* GPUOutputBuffer)
{
    if (!GPUOutputBuffer)
        return 0;
    
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    GPUOutputBuffer->GetDesc(&BufferDesc);
    
    BufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_READ;
    BufferDesc.Usage            = D3D11_USAGE_STAGING;
    BufferDesc.BindFlags        = 0;
    BufferDesc.MiscFlags        = 0;
    
    ID3D11Buffer* AccessBuffer  = 0;
    
    if (D3DDevice_->CreateBuffer(&BufferDesc, 0, &AccessBuffer))
    {
        io::Log::error("Could not create CPU access buffer");
        return 0;
    }
    
    D3DDeviceContext_->CopyResource(AccessBuffer, GPUOutputBuffer);
    
    return AccessBuffer;
}

ID3D11UnorderedAccessView* Direct3D11RenderSystem::createUnorderedAccessView(ID3D11Buffer* StructuredBuffer)
{
    if (!StructuredBuffer)
        return 0;
    
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    StructuredBuffer->GetDesc(&BufferDesc);
    
    /* Configure access view description */
    D3D11_UNORDERED_ACCESS_VIEW_DESC AccessViewDesc;
    ZeroMemory(&AccessViewDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
    
    AccessViewDesc.ViewDimension        = D3D11_UAV_DIMENSION_BUFFER;
    AccessViewDesc.Buffer.FirstElement  = 0;
    
    if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
        AccessViewDesc.Format               = DXGI_FORMAT_R32_TYPELESS;
        AccessViewDesc.Buffer.Flags         = D3D11_BUFFER_UAV_FLAG_RAW;
        AccessViewDesc.Buffer.NumElements   = BufferDesc.ByteWidth / 4;
    }
    else if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
    {
        AccessViewDesc.Format               = DXGI_FORMAT_UNKNOWN;
        AccessViewDesc.Buffer.NumElements   = BufferDesc.ByteWidth / BufferDesc.StructureByteStride;
    }
    else
    {
        io::Log::error("Unsupported buffer for unordered access view");
        return 0;
    }
    
    /* Create unordered access view */
    ID3D11UnorderedAccessView* AccessView = 0;
    
    if (D3DDevice_->CreateUnorderedAccessView(StructuredBuffer, &AccessViewDesc, &AccessView))
    {
        io::Log::error("Could not create unordered access view");
        return 0;
    }
    
    return AccessView;
}

ID3D11ShaderResourceView* Direct3D11RenderSystem::createShaderResourceView(ID3D11Buffer* StructuredBuffer)
{
    if (!StructuredBuffer)
        return 0;
    
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    StructuredBuffer->GetDesc(&BufferDesc);
    
    /* Configure resource view description */
    D3D11_SHADER_RESOURCE_VIEW_DESC ResourceViewDesc;
    ZeroMemory(&ResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    
    ResourceViewDesc.ViewDimension          = D3D11_SRV_DIMENSION_BUFFEREX;
    ResourceViewDesc.Buffer.FirstElement    = 0;
    
    if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
        ResourceViewDesc.Format                 = DXGI_FORMAT_R32_TYPELESS;
        ResourceViewDesc.BufferEx.Flags         = D3D11_BUFFEREX_SRV_FLAG_RAW;
        ResourceViewDesc.BufferEx.NumElements   = BufferDesc.ByteWidth / 4;
    }
    else if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
    {
        ResourceViewDesc.Format                 = DXGI_FORMAT_UNKNOWN;
        ResourceViewDesc.BufferEx.NumElements   = BufferDesc.ByteWidth / BufferDesc.StructureByteStride;
    }
    else
    {
        io::Log::error("Unsupported buffer for unordered access view");
        return 0;
    }
    
    /* Create unordered access view */
    ID3D11ShaderResourceView* ResoruceView = 0;
    
    if (D3DDevice_->CreateShaderResourceView(StructuredBuffer, &ResourceViewDesc, &ResoruceView))
    {
        io::Log::error("Could not create shader resource view");
        return 0;
    }
    
    return ResoruceView;
}

void Direct3D11RenderSystem::updateVertexInputLayout(VertexFormat* Format, bool isCreate)
{
    if (!Format)
        return;
    
    std::vector<D3D11_INPUT_ELEMENT_DESC>* InputDesc = reinterpret_cast<std::vector<D3D11_INPUT_ELEMENT_DESC>*>(Format->InputLayout_);
    
    if (InputDesc)
    {
        /* Delete semantic names */
        for (std::vector<D3D11_INPUT_ELEMENT_DESC>::iterator it = InputDesc->begin(); it != InputDesc->end(); ++it)
            MemoryManager::deleteBuffer(it->SemanticName);
        InputDesc->clear();
    }
    
    if (!isCreate)
    {
        /* Delete attribute container */
        delete InputDesc;
        Format->InputLayout_ = 0;
        return;
    }
    
    if (!InputDesc)
    {
        /* Allocate new attribute container */
        InputDesc = new std::vector<D3D11_INPUT_ELEMENT_DESC>();
    }
    
    if (Format->getFlags() & VERTEXFORMAT_COORD)
        addVertexInputLayoutAttribute(InputDesc, Format->getCoord());
    if (Format->getFlags() & VERTEXFORMAT_COLOR)
        addVertexInputLayoutAttribute(InputDesc, Format->getColor());
    if (Format->getFlags() & VERTEXFORMAT_NORMAL)
        addVertexInputLayoutAttribute(InputDesc, Format->getNormal());
    if (Format->getFlags() & VERTEXFORMAT_BINORMAL)
        addVertexInputLayoutAttribute(InputDesc, Format->getBinormal());
    if (Format->getFlags() & VERTEXFORMAT_TANGENT)
        addVertexInputLayoutAttribute(InputDesc, Format->getTangent());
    if (Format->getFlags() & VERTEXFORMAT_FOGCOORD)
        addVertexInputLayoutAttribute(InputDesc, Format->getFogCoord());
    if (Format->getFlags() & VERTEXFORMAT_TEXCOORDS)
    {
        for (u32 i = 0; i < Format->getTexCoords().size(); ++i)
            addVertexInputLayoutAttribute(InputDesc, Format->getTexCoords()[i]);
    }
    if (Format->getFlags() & VERTEXFORMAT_UNIVERSAL)
    {
        for (u32 i = 0; i < Format->getUniversals().size(); ++i)
            addVertexInputLayoutAttribute(InputDesc, Format->getUniversals()[i]);
    }
    
    Format->InputLayout_ = InputDesc;
}

void Direct3D11RenderSystem::addVertexInputLayoutAttribute(std::vector<D3D11_INPUT_ELEMENT_DESC>* InputDesc, const SVertexAttribute &Attrib)
{
    if (!InputDesc)
        return;
    
    /* Add new attribute */
    InputDesc->resize(InputDesc->size() + 1);
    D3D11_INPUT_ELEMENT_DESC* DescAttrib = &InputDesc->back();
    
    /* General attribute settings */
    DescAttrib->InputSlot               = 0;
    DescAttrib->InputSlotClass          = D3D11_INPUT_PER_VERTEX_DATA;
    DescAttrib->AlignedByteOffset       = Attrib.Offset;
    DescAttrib->InstanceDataStepRate    = 0;
    
    /* Setup semantic name and index */
    u32 Index = 0;
    io::stringc Name(Attrib.Name);
    
    u32 Len = Name.size();
    
    for (; Len > 0; --Len)
    {
        if (Name[Len - 1] < '0' || Name[Len - 1] > '9')
            break;
    }
    
    if (Len < Name.size())
    {
        Index   = Name.right(Name.size() - Len).val<u32>();
        Name    = Name.left(Len);
    }
    
    c8* SemanticName = new c8[Name.size() + 1];
    memcpy(SemanticName, Name.c_str(), Name.size());
    SemanticName[Name.size()] = 0;
    
    DescAttrib->SemanticName    = SemanticName;
    DescAttrib->SemanticIndex   = Index;
    
    /* Setup attribute format */
    DescAttrib->Format = DXGI_FORMAT_UNKNOWN;
    
    switch (Attrib.Type)
    {
        case DATATYPE_FLOAT:
            switch (Attrib.Size)
            {
                case 1: DescAttrib->Format = DXGI_FORMAT_R32_FLOAT; break;
                case 2: DescAttrib->Format = DXGI_FORMAT_R32G32_FLOAT; break;
                case 3: DescAttrib->Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
                case 4: DescAttrib->Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
            }
            break;
            
        case DATATYPE_BYTE:
            if (Attrib.Normalize)
            {
                switch (Attrib.Size)
                {
                    case 1: DescAttrib->Format = DXGI_FORMAT_R8_SNORM; break;
                    case 2: DescAttrib->Format = DXGI_FORMAT_R8G8_SNORM; break;
                    case 4: DescAttrib->Format = DXGI_FORMAT_R8G8B8A8_SNORM; break;
                }
            }
            else
            {
                switch (Attrib.Size)
                {
                    case 1: DescAttrib->Format = DXGI_FORMAT_R8_SINT; break;
                    case 2: DescAttrib->Format = DXGI_FORMAT_R8G8_SINT; break;
                    case 4: DescAttrib->Format = DXGI_FORMAT_R8G8B8A8_SINT; break;
                }
            }
            break;
            
        case DATATYPE_UNSIGNED_BYTE:
            if (Attrib.Normalize)
            {
                switch (Attrib.Size)
                {
                    case 1: DescAttrib->Format = DXGI_FORMAT_R8_UNORM; break;
                    case 2: DescAttrib->Format = DXGI_FORMAT_R8G8_UNORM; break;
                    case 4: DescAttrib->Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
                }
            }
            else
            {
                switch (Attrib.Size)
                {
                    case 1: DescAttrib->Format = DXGI_FORMAT_R8_UINT; break;
                    case 2: DescAttrib->Format = DXGI_FORMAT_R8G8_UINT; break;
                    case 4: DescAttrib->Format = DXGI_FORMAT_R8G8B8A8_UINT; break;
                }
            }
            break;
            
        case DATATYPE_SHORT:
            if (Attrib.Normalize)
            {
                switch (Attrib.Size)
                {
                    case 1: DescAttrib->Format = DXGI_FORMAT_R16_SNORM; break;
                    case 2: DescAttrib->Format = DXGI_FORMAT_R16G16_SNORM; break;
                    case 4: DescAttrib->Format = DXGI_FORMAT_R16G16B16A16_SNORM; break;
                }
            }
            else
            {
                switch (Attrib.Size)
                {
                    case 1: DescAttrib->Format = DXGI_FORMAT_R16_SINT; break;
                    case 2: DescAttrib->Format = DXGI_FORMAT_R16G16_SINT; break;
                    case 4: DescAttrib->Format = DXGI_FORMAT_R16G16B16A16_SINT; break;
                }
            }
            break;
            
        case DATATYPE_UNSIGNED_SHORT:
            if (Attrib.Normalize)
            {
                switch (Attrib.Size)
                {
                    case 1: DescAttrib->Format = DXGI_FORMAT_R16_UNORM; break;
                    case 2: DescAttrib->Format = DXGI_FORMAT_R16G16_UNORM; break;
                    case 4: DescAttrib->Format = DXGI_FORMAT_R16G16B16A16_UNORM; break;
                }
            }
            else
            {
                switch (Attrib.Size)
                {
                    case 1: DescAttrib->Format = DXGI_FORMAT_R16_UINT; break;
                    case 2: DescAttrib->Format = DXGI_FORMAT_R16G16_UINT; break;
                    case 4: DescAttrib->Format = DXGI_FORMAT_R16G16B16A16_UINT; break;
                }
            }
            break;
            
        case DATATYPE_INT:
            switch (Attrib.Size)
            {
                case 1: DescAttrib->Format = DXGI_FORMAT_R32_SINT; break;
                case 2: DescAttrib->Format = DXGI_FORMAT_R32G32_SINT; break;
                case 3: DescAttrib->Format = DXGI_FORMAT_R32G32B32_SINT; break;
                case 4: DescAttrib->Format = DXGI_FORMAT_R32G32B32A32_SINT; break;
            }
            break;
            
        case DATATYPE_UNSIGNED_INT:
            switch (Attrib.Size)
            {
                case 1: DescAttrib->Format = DXGI_FORMAT_R32_UINT; break;
                case 2: DescAttrib->Format = DXGI_FORMAT_R32G32_UINT; break;
                case 3: DescAttrib->Format = DXGI_FORMAT_R32G32B32_UINT; break;
                case 4: DescAttrib->Format = DXGI_FORMAT_R32G32B32A32_UINT; break;
            }
            break;
    }
    
    if (DescAttrib->Format == DXGI_FORMAT_UNKNOWN)
        io::Log::error("Unknown attribute format in vertex input layout");
}

void Direct3D11RenderSystem::drawTexturedFont(
    const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color)
{
    /* Setup 2D drawing */
    setup2DDrawing();
    
    /* Get vertex buffer and glyph list */
    D3D11HardwareBuffer* VertexBuffer = reinterpret_cast<D3D11HardwareBuffer*>(FontObj->getBufferRawData());
    
    const SFontGlyph* GlyphList = &(FontObj->getGlyphList()[0]);
    
    /* Setup vertex buffer */
    const u32 Stride = sizeof(SQuad2DVertex);
    const u32 Offset = 0;
    
    D3DDeviceContext_->IASetVertexBuffers(0, 1, &VertexBuffer->HWBuffer_, &Stride, &Offset);
    D3DDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    
    /* Initialize main constant buffer */
    ConstBuffer2DMain_.ProjectionMatrix = getProjectionMatrix();
    
    ConstBuffer2DMain_.Color = dim::vector4df(Color.getVector(true), static_cast<f32>(Color.Alpha) / 255.0f);
    
    ConstBuffer2DMain_.UseTexture = 1;
    
    DefaultBasicShader2D_->getVertexShader()->setConstantBuffer(0, &ConstBuffer2DMain_);
    DefaultBasicShader2D_->getPixelShader()->setConstantBuffer(0, &ConstBuffer2DMain_);
    
    /* Initialize mapping constant buffer */
    ConstBuffer2DMapping_.Position = Position.cast<f32>();
    ConstBuffer2DMapping_.TexPosition = 0.0f;
    
    ConstBuffer2DMapping_.WorldMatrix.reset();
    ConstBuffer2DMapping_.TextureMatrix.reset();
    
    DefaultBasicShader2D_->getVertexShader()->setConstantBuffer(1, &ConstBuffer2DMapping_);
    DefaultBasicShader2D_->getPixelShader()->setConstantBuffer(1, &ConstBuffer2DMapping_);
    
    /* Bind default drawing shader */
    DefaultBasicShader2D_->bind();
    
    /* Bind texture and update shader resources for texture samplers */
    FontObj->getTexture()->bind(0);
    
    updateShaderResources();
    
    /* Draw each character */
    for (u32 i = 0, c = Text.size(); i < c; ++i)
    {
        /* Get character glyph from string */
        const u32 CurChar = static_cast<u32>(static_cast<u8>(Text[i]));
        const SFontGlyph* Glyph = &(GlyphList[CurChar]);
        
        /* Offset movement */
        ConstBuffer2DMapping_.Position.X += static_cast<f32>(Glyph->StartOffset);
        
        /* Update constant buffer */
        DefaultBasicShader2D_->getVertexShader()->setConstantBuffer(1, &ConstBuffer2DMapping_);
        
        /* Draw current character */
        D3DDeviceContext_->Draw(4, CurChar*4);
        
        /* Character width and white space movement */
        ConstBuffer2DMapping_.Position.X += static_cast<f32>(Glyph->DrawnWidth + Glyph->WhiteSpace);
    }
    
    /* Unbind texture */
    FontObj->getTexture()->unbind(0);
}

void Direct3D11RenderSystem::createTexturedFontVertexBuffer(dim::UniversalBuffer &VertexBuffer, VertexFormatUniversal &VertFormat)
{
    /* D3D11 vertex buffer for textured font glyphs */
    VertexBuffer.setStride(sizeof(SQuad2DVertex));
    
    VertFormat.addCoord(DATATYPE_FLOAT, 2);
    VertFormat.addTexCoord();
}

void Direct3D11RenderSystem::setupTexturedFontGlyph(
    void* &RawVertexData, const SFontGlyph &Glyph, const dim::rect2df &Mapping)
{
    SQuad2DVertex* VertexData = reinterpret_cast<SQuad2DVertex*>(RawVertexData);
    
    VertexData[0].Position = 0.0f;
    VertexData[1].Position = dim::point2di(Glyph.Rect.Right - Glyph.Rect.Left, 0).cast<f32>();
    VertexData[2].Position = dim::point2di(0, Glyph.Rect.Bottom - Glyph.Rect.Top).cast<f32>();
    VertexData[3].Position = dim::point2di(Glyph.Rect.Right - Glyph.Rect.Left, Glyph.Rect.Bottom - Glyph.Rect.Top).cast<f32>();
    
    VertexData[0].TexCoord = dim::point2df(Mapping.Left, Mapping.Top);
    VertexData[1].TexCoord = dim::point2df(Mapping.Right, Mapping.Top);
    VertexData[2].TexCoord = dim::point2df(Mapping.Left, Mapping.Bottom);
    VertexData[3].TexCoord = dim::point2df(Mapping.Right, Mapping.Bottom);
    
    VertexData += 4;
    
    RawVertexData = VertexData;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
