/*
 * Direct3D9 render system file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D9/spDirect3D9RenderSystem.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "Base/spInternalDeclarations.hpp"
#include "Base/spSharedObjects.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Framework/Cg/spCgShaderProgramD3D9.hpp"
#include "RenderSystem/Direct3D9/spDirect3D9VertexBuffer.hpp"
#include "RenderSystem/Direct3D9/spDirect3D9IndexBuffer.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern scene::SceneGraph* __spSceneManager;

namespace video
{


/*
 * ========== Internal members ==========
 */

const io::stringc d3dDllFileName = "d3dx9_" + io::stringc(static_cast<s32>(D3DX_SDK_VERSION)) + ".dll";

const s32 D3DCompareList[] =
{
    D3DCMP_NEVER, D3DCMP_EQUAL, D3DCMP_NOTEQUAL, D3DCMP_LESS, D3DCMP_LESSEQUAL,
    D3DCMP_GREATER, D3DCMP_GREATEREQUAL, D3DCMP_ALWAYS,
};

const s32 D3DMappingGenList[] =
{
    D3DTSS_TCI_PASSTHRU, D3DTSS_TCI_PASSTHRU, D3DTSS_TCI_CAMERASPACEPOSITION, D3DTSS_TCI_SPHEREMAP,
    D3DTSS_TCI_CAMERASPACENORMAL, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR,
};

const s32 D3DTextureEnvList[] =
{
    D3DTOP_MODULATE, D3DTOP_SELECTARG1, D3DTOP_ADD, D3DTOP_ADDSIGNED,
    D3DTOP_SUBTRACT, D3DTOP_LERP, D3DTOP_DOTPRODUCT3,
};

const s32 D3DBlendingList[] =
{
    D3DBLEND_ZERO, D3DBLEND_ONE, D3DBLEND_SRCCOLOR, D3DBLEND_INVSRCCOLOR, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA,
    D3DBLEND_DESTCOLOR, D3DBLEND_INVDESTCOLOR, D3DBLEND_DESTALPHA, D3DBLEND_INVDESTALPHA,
};

const s32 D3DTextureWrapModes[] =
{
    D3DTADDRESS_WRAP, D3DTADDRESS_MIRROR, D3DTADDRESS_CLAMP,
};

const D3DFORMAT D3DTexInternalFormatListUByte8[] = {
    D3DFMT_A8, D3DFMT_L8, D3DFMT_A8L8, D3DFMT_X8R8G8B8,
    D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_D24X8
};

const D3DFORMAT D3DTexInternalFormatListFloat16[] = {
    D3DFMT_R16F, D3DFMT_R16F, D3DFMT_G16R16F, D3DFMT_A16B16G16R16F,
    D3DFMT_A16B16G16R16F, D3DFMT_A16B16G16R16F, D3DFMT_A16B16G16R16F, D3DFMT_D24X8
};

const D3DFORMAT D3DTexInternalFormatListFloat32[] = {
    D3DFMT_R32F, D3DFMT_R32F, D3DFMT_G32R32F, D3DFMT_A32B32G32R32F,
    D3DFMT_A32B32G32R32F, D3DFMT_A32B32G32R32F, D3DFMT_A32B32G32R32F, D3DFMT_D24X8
};


/*
 * ========== Constructors & destructor ==========
 */

Direct3D9RenderSystem::Direct3D9RenderSystem() :
    RenderSystem                (RENDERER_DIRECT3D9 ),
    D3DInstance_                (0                  ),
    D3DDevice_                  (0                  ),
    D3DDefVertexBuffer_         (0                  ),
    D3DDefFlexibleVertexBuffer_ (0                  ),
    LastRenderTarget_           (0                  ),
    LastRTCount_                (0                  ),
    CurD3DTexture_              (0                  ),
    CurD3DCubeTexture_          (0                  ),
    CurD3DVolumeTexture_        (0                  ),
    ClearColor_                 (video::color::empty),
    ClearColorMask_             (1, 1, 1, 1         ),
    isFullscreen_               (false              ),
    isImageBlending_            (true               ),
    CurSamplerLevel_            (0                  )
{
    /* Create the Direct3D renderer */
    D3DInstance_ = Direct3DCreate9(D3D_SDK_VERSION);
    
    if (!D3DInstance_)
    {
        io::Log::error("Could not create Direct3D9 interface");
        return;
    }
}
Direct3D9RenderSystem::~Direct3D9RenderSystem()
{
    /* Release all Direct3D9 fonts */
    foreach (Font* FontObj, FontList_)
        releaseFontObject(FontObj);
    
    /* Close and release the standard- & flexible vertex buffer */
    releaseObject(D3DDefVertexBuffer_);
    releaseObject(D3DDefFlexibleVertexBuffer_);
    
    /* Close and release Direct3D */
    releaseObject(D3DInstance_);
}


/*
 * ======= Initialization functions =======
 */

void Direct3D9RenderSystem::setupConfiguration()
{
    /* Get all device capabilities */
    D3DDevice_->GetDeviceCaps(&DevCaps_);
    
    MaxClippingPlanes_ = DevCaps_.MaxUserClipPlanes;
    
    /*
     * Create the standard vertex buffer
     * used for 2d drawing operations
     * (drawing: rectangle, images etc.)
     */
    D3DDevice_->CreateVertexBuffer(
        sizeof(SPrimitiveVertex)*4,
        0,
        FVF_VERTEX2D,
        D3DPOOL_DEFAULT,
        &D3DDefVertexBuffer_,
        0
    );
    
    if (!D3DDefVertexBuffer_)
    {
        io::Log::error("Could not create Direct3D9 vertex buffer");
        return;
    }
    
    /*
     * Create the flexible vertex buffer
     * used for 2d drawing operations
     * (drawing: polygon & other objects with undefined sizes)
     */
    D3DDevice_->CreateVertexBuffer(
        sizeof(SPrimitiveVertex),
        0,
        FVF_VERTEX2D,
        D3DPOOL_DEFAULT,
        &D3DDefFlexibleVertexBuffer_,
        0
    );
    
    if (!D3DDefFlexibleVertexBuffer_)
    {
        io::Log::error("Could not create Direct3D9 vertex buffer");
        return;
    }
    
    /* Default settings */
    D3DDevice_->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    D3DDevice_->SetRenderState(D3DRS_ALPHATESTENABLE, true);
    D3DDevice_->SetRenderState(D3DRS_SPECULARENABLE, true);
    D3DDevice_->SetRenderState(D3DRS_NORMALIZENORMALS, true);
    
    /* Default queries */
    RenderQuery_[RENDERQUERY_SHADER]                = queryVideoSupport(QUERY_SHADER);
    RenderQuery_[RENDERQUERY_MULTI_TEXTURE]         = queryVideoSupport(QUERY_MULTI_TEXTURE);
    RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER]   = queryVideoSupport(QUERY_HARDWARE_MESHBUFFER);
    RenderQuery_[RENDERQUERY_RENDERTARGET]          = queryVideoSupport(QUERY_RENDERTARGET);
}


/*
 * ======= Renderer information =======
 */

io::stringc Direct3D9RenderSystem::getRenderer() const
{
    D3DADAPTER_IDENTIFIER9 Adapter;
    D3DInstance_->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &Adapter);
    return io::stringc(Adapter.Description);
}
io::stringc Direct3D9RenderSystem::getVersion() const
{
    if (queryVideoSupport(QUERY_VERTEX_SHADER_3_0) && queryVideoSupport(QUERY_PIXEL_SHADER_3_0))
        return "Direct3D 9.0c";
    return "Direct3D 9.0";
}
io::stringc Direct3D9RenderSystem::getVendor() const
{
    D3DADAPTER_IDENTIFIER9 Adapter;
    D3DInstance_->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &Adapter);
    return getVendorNameByID((u32)Adapter.VendorId);
}
io::stringc Direct3D9RenderSystem::getShaderVersion() const
{
    if (queryVideoSupport(QUERY_VERTEX_SHADER_3_0) && queryVideoSupport(QUERY_PIXEL_SHADER_3_0))
        return "HLSL Shader Model 3.0";
    return "HLSL Shader Model 2.0";
}

bool Direct3D9RenderSystem::queryVideoSupport(const EVideoFeatureQueries Query) const
{
    switch (Query)
    {
        case QUERY_ANTIALIASING:
            return true; // (todo)
        case QUERY_MULTI_TEXTURE:
            return getMultitexCount() > 1;
        case QUERY_HARDWARE_MESHBUFFER:
            return true;
        case QUERY_STENCIL_BUFFER:
            return DevCaps_.StencilCaps != 0;
        case QUERY_RENDERTARGET:
        case QUERY_MULTISAMPLE_RENDERTARGET:
            return true;
            
        case QUERY_BILINEAR_FILTER:
            return (DevCaps_.TextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT) != 0;
        case QUERY_TRILINEAR_FILTER:
            return (DevCaps_.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0;
        case QUERY_ANISOTROPY_FILTER:
            return (DevCaps_.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) != 0;
        case QUERY_MIPMAPS:
            return (DevCaps_.TextureCaps & D3DPTEXTURECAPS_MIPMAP) != 0;
        case QUERY_VOLUMETRIC_TEXTURE:
            return (DevCaps_.TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP) != 0;
            
        case QUERY_VETEX_PROGRAM:
        case QUERY_FRAGMENT_PROGRAM:
            return true; // (todo)
        case QUERY_SHADER:
        case QUERY_HLSL:
        case QUERY_VERTEX_SHADER_1_1:
            return DevCaps_.VertexShaderVersion >= D3DVS_VERSION(1, 1);
        case QUERY_VERTEX_SHADER_2_0:
            return DevCaps_.VertexShaderVersion >= D3DVS_VERSION(2, 0);
        case QUERY_VERTEX_SHADER_3_0:
            return DevCaps_.VertexShaderVersion >= D3DVS_VERSION(3, 0);
        case QUERY_PIXEL_SHADER_1_1:
            return DevCaps_.PixelShaderVersion >= D3DPS_VERSION(1, 1);
        case QUERY_PIXEL_SHADER_1_2:
            return DevCaps_.PixelShaderVersion >= D3DPS_VERSION(1, 2);
        case QUERY_PIXEL_SHADER_1_3:
            return DevCaps_.PixelShaderVersion >= D3DPS_VERSION(1, 3);
        case QUERY_PIXEL_SHADER_1_4:
            return DevCaps_.PixelShaderVersion >= D3DPS_VERSION(1, 4);
        case QUERY_PIXEL_SHADER_2_0:
            return DevCaps_.PixelShaderVersion >= D3DPS_VERSION(2, 0);
        case QUERY_PIXEL_SHADER_3_0:
            return DevCaps_.PixelShaderVersion >= D3DPS_VERSION(3, 0);
    }
    
    return false;
}

s32 Direct3D9RenderSystem::getMultitexCount() const
{
    return DevCaps_.MaxTextureBlendStages;
}
s32 Direct3D9RenderSystem::getMaxAnisotropicFilter() const
{
    return DevCaps_.MaxAnisotropy;
}
s32 Direct3D9RenderSystem::getMaxLightCount() const
{
    return DevCaps_.MaxActiveLights;
}


/*
 * ======= User controll functions (clear buffers, flip buffers, 2d drawing etc.) =======
 */

void Direct3D9RenderSystem::clearBuffers(const s32 ClearFlags)
{
    setViewport(0, dim::size2di(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight));
    
    const video::color ClearColor(ClearColor_ * ClearColorMask_);
    
    DWORD Mask = 0;
    
    if (ClearFlags & BUFFER_COLOR)
        Mask |= D3DCLEAR_TARGET;
    if (ClearFlags & BUFFER_DEPTH)
        Mask |= D3DCLEAR_ZBUFFER;
    if (ClearFlags & BUFFER_STENCIL)
        Mask |= D3DCLEAR_STENCIL;
    
    D3DDevice_->Clear(0, 0, Mask, ClearColor.getSingle(), 1.0f, 0);
    D3DDevice_->BeginScene();
}


/*
 * ======= Setting-/ getting functions =======
 */

void Direct3D9RenderSystem::setShadeMode(const EShadeModeTypes ShadeMode)
{
    switch (ShadeMode)
    {
        case SHADEMODE_SMOOTH:
            D3DDevice_->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
            break;
        case SHADEMODE_FLAT:
            D3DDevice_->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
            break;
    }
}

void Direct3D9RenderSystem::setClearColor(const color &Color)
{
    ClearColor_ = Color;
}

void Direct3D9RenderSystem::setColorMask(bool isRed, bool isGreen, bool isBlue, bool isAlpha)
{
    DWORD Mask = 0;
    ClearColorMask_ = video::color::empty;
    
    if (isRed)
    {
        Mask |= D3DCOLORWRITEENABLE_RED;
        ClearColorMask_.Red = 1;
    }
    if (isGreen)
    {
        Mask |= D3DCOLORWRITEENABLE_GREEN;
        ClearColorMask_.Green = 1;
    }
    if (isBlue)
    {
        Mask |= D3DCOLORWRITEENABLE_BLUE;
        ClearColorMask_.Blue = 1;
    }
    if (isAlpha)
    {
        Mask |= D3DCOLORWRITEENABLE_ALPHA;
        ClearColorMask_.Alpha = 1;
    }
    
    D3DDevice_->SetRenderState(D3DRS_COLORWRITEENABLE, Mask);
}

void Direct3D9RenderSystem::setDepthMask(bool isDepth)
{
    D3DDevice_->SetRenderState(D3DRS_ZWRITEENABLE, isDepth);
}

void Direct3D9RenderSystem::setAntiAlias(bool isAntiAlias)
{
    D3DDevice_->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, isAntiAlias);
}


/*
 * ======= Rendering 3D scenes =======
 */

void Direct3D9RenderSystem::setupMaterialStates(const MaterialStates* Material)
{
    /* Check for equality to optimize render path */
    if (!Material || Material->compare(LastMaterial_))
        return;
    else
        LastMaterial_ = Material;
    
    /* Cull facing */
    switch (Material->getRenderFace())
    {
        case video::FACE_FRONT:
            D3DDevice_->SetRenderState(D3DRS_CULLMODE, isFrontFace_ ? D3DCULL_CCW : D3DCULL_CW);
            break;
        case video::FACE_BACK:
            D3DDevice_->SetRenderState(D3DRS_CULLMODE, isFrontFace_ ? D3DCULL_CW : D3DCULL_CCW);
            break;
        case video::FACE_BOTH:
            D3DDevice_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            break;
    }
    
    /* Fog effect */
    D3DDevice_->SetRenderState(D3DRS_FOGENABLE, __isFog && Material->getFog());
    
    /* Color material */
    D3DDevice_->SetRenderState(D3DRS_COLORVERTEX, Material->getColorMaterial());
    
    /* Lighting material */
    if (__isLighting && Material->getLighting())
    {
        D3DMATERIAL9 D3DMat;
        
        D3DDevice_->SetRenderState(D3DRS_LIGHTING, true);
        
        /* Diffuse, ambient, specular and emissive color */
        D3DMat.Diffuse = getD3DColor(Material->getDiffuseColor());
        D3DMat.Ambient = getD3DColor(Material->getAmbientColor());
        D3DMat.Specular = getD3DColor(Material->getSpecularColor());
        D3DMat.Emissive = getD3DColor(Material->getEmissionColor());
        
        /* Shininess */
        D3DMat.Power = Material->getShininessFactor();
        
        /* Set the material */
        D3DDevice_->SetMaterial(&D3DMat);
    }
    else
        D3DDevice_->SetRenderState(D3DRS_LIGHTING, false);
    
    /* Depth functions */
    if (Material->getDepthBuffer())
    {
        D3DDevice_->SetRenderState(D3DRS_ZENABLE, true);
        D3DDevice_->SetRenderState(D3DRS_ZFUNC, D3DCompareList[Material->getDepthMethod()]);
    }
    else
        D3DDevice_->SetRenderState(D3DRS_ZENABLE, false);
    
    /* Blending mode */
    if (Material->getBlending())
    {
        D3DDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
        D3DDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBlendingList[Material->getBlendSource()]);
        D3DDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBlendingList[Material->getBlendTarget()]);
    }
    else
        D3DDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
    
    /* Polygon offset */
    if (Material->getPolygonOffset())
    {
        D3DDevice_->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&Material->OffsetFactor_);
        D3DDevice_->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&Material->OffsetUnits_);
    }
    else
    {
        D3DDevice_->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, 0);
        D3DDevice_->SetRenderState(D3DRS_DEPTHBIAS, 0);
    }
    
    /* Alpha functions */
    D3DDevice_->SetRenderState(D3DRS_ALPHAFUNC, D3DCompareList[Material->getAlphaMethod()]);
    D3DDevice_->SetRenderState(D3DRS_ALPHAREF, s32(Material->getAlphaReference() * 255));
    
    /* Polygon mode */
    D3DDevice_->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT + Material->getWireframeFront());
    
    /* Flexible vertex format (FVF) */
    D3DDevice_->SetFVF(FVF_VERTEX3D);
}

void Direct3D9RenderSystem::drawPrimitiveList(
    const ERenderPrimitives Type,
    const scene::SMeshVertex3D* Vertices, u32 VertexCount, const void* Indices, u32 IndexCount,
    std::vector<SMeshSurfaceTexture>* TextureList)
{
    if (!Vertices || !VertexCount)
        return;
    
    /* Select the primitive type */
    D3DPRIMITIVETYPE Mode;
    UINT PrimitiveCount = 0;
    
    switch (Type)
    {
        case PRIMITIVE_POINTS:
            Mode = D3DPT_POINTLIST, PrimitiveCount = VertexCount; break;
        case PRIMITIVE_LINES:
            Mode = D3DPT_LINELIST, PrimitiveCount = VertexCount/2; break;
        case PRIMITIVE_LINE_STRIP:
            Mode = D3DPT_LINESTRIP, PrimitiveCount = VertexCount/2 + 1; break;
        case PRIMITIVE_TRIANGLES:
            Mode = D3DPT_TRIANGLELIST, PrimitiveCount = VertexCount/3; break;
        case PRIMITIVE_TRIANGLE_STRIP:
            Mode = D3DPT_TRIANGLESTRIP, PrimitiveCount = VertexCount - 2; break;
        case PRIMITIVE_TRIANGLE_FAN:
            Mode = D3DPT_TRIANGLEFAN, PrimitiveCount = VertexCount - 2; break;
        case PRIMITIVE_LINE_LOOP:
        case PRIMITIVE_QUADS:
        case PRIMITIVE_QUAD_STRIP:
        case PRIMITIVE_POLYGON:
        default:
            return;
    }
    
    /* Bind texture layers */
    if (TextureList && __isTexturing)
        bindTextureList(*TextureList);
    
    /* Render primitives */
    if (!Indices || !IndexCount)
    {
        D3DDevice_->DrawPrimitiveUP(
            Mode,
            PrimitiveCount,
            Vertices,
            sizeof(scene::SMeshVertex3D)
        );
    }
    else
    {
        D3DDevice_->DrawIndexedPrimitiveUP(
            Mode, 0,
            VertexCount,
            IndexCount/3, Indices,
            D3DFMT_INDEX32,
            Vertices,
            sizeof(scene::SMeshVertex3D)
        );
    }
    
    /* Unbind texture layers */
    if (TextureList && __isTexturing)
        unbindTextureList(*TextureList);
}

void Direct3D9RenderSystem::updateLight(
    u32 LightID, const scene::ELightModels LightType, bool isVolumetric,
    const dim::vector3df &Direction, f32 SpotInnerConeAngle, f32 SpotOuterConeAngle,
    f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic)
{
    if (LightID >= MAX_COUNT_OF_LIGHTS)
        return;
    
    /* Get the light source */
    D3DDevice_->GetLight(LightID, &D3DActiveLight_);
    
    /* Update type and direction */
    switch (LightType)
    {
        case scene::LIGHT_DIRECTIONAL:
            D3DActiveLight_.Type        = D3DLIGHT_DIRECTIONAL;
            D3DActiveLight_.Direction   = *D3D_VECTOR(scene::spWorldMatrix.getRotationMatrix() * Direction);
            break;
        case scene::LIGHT_POINT:
            D3DActiveLight_.Type        = D3DLIGHT_POINT;
            D3DActiveLight_.Direction   = *D3D_VECTOR(scene::spWorldMatrix.getRotationMatrix() * -Direction);
            break;
        case scene::LIGHT_SPOT:
            D3DActiveLight_.Type        = D3DLIGHT_SPOT;
            break;
    }
    
    /* Lighting location */
    D3DActiveLight_.Position = *D3D_VECTOR(scene::spWorldMatrix.getPosition());
    
    /* Spot light attributes */
    D3DActiveLight_.Theta   = SpotInnerConeAngle * 2.0f * math::DEG;
    D3DActiveLight_.Phi     = SpotOuterConeAngle * 2.0f * math::DEG;
    
    /* Volumetric light attenuations */
    if (isVolumetric)
    {
        D3DActiveLight_.Attenuation0 = AttenuationConstant;
        D3DActiveLight_.Attenuation1 = AttenuationLinear;
        D3DActiveLight_.Attenuation2 = AttenuationQuadratic;
    }
    else
    {
        D3DActiveLight_.Attenuation0 = 1.0f;
        D3DActiveLight_.Attenuation1 = 0.0f;
        D3DActiveLight_.Attenuation2 = 0.0f;
    }
    
    /* Set the light source */
    D3DDevice_->SetLight(LightID, &D3DActiveLight_);
}


/* === Hardware mesh buffers === */

void Direct3D9RenderSystem::createVertexBuffer(void* &BufferID)
{
    BufferID = new D3D9VertexBuffer();
}
void Direct3D9RenderSystem::createIndexBuffer(void* &BufferID)
{
    BufferID = new D3D9IndexBuffer();
}

void Direct3D9RenderSystem::deleteVertexBuffer(void* &BufferID)
{
    if (BufferID)
    {
        D3D9VertexBuffer* Buffer = static_cast<D3D9VertexBuffer*>(BufferID);
        delete Buffer;
        
        BufferID = 0;
    }
}
void Direct3D9RenderSystem::deleteIndexBuffer(void* &BufferID)
{
    if (BufferID)
    {
        D3D9IndexBuffer* Buffer = static_cast<D3D9IndexBuffer*>(BufferID);
        delete Buffer;
        
        BufferID = 0;
    }
}

void Direct3D9RenderSystem::updateVertexBuffer(
    void* BufferID, const dim::UniversalBuffer &BufferData, const VertexFormat* Format, const EMeshBufferUsage Usage)
{
    if (BufferID && Format)
    {
        D3D9VertexBuffer* Buffer = static_cast<D3D9VertexBuffer*>(BufferID);
        Buffer->update(D3DDevice_, BufferData, Format, Usage);
    }
}
void Direct3D9RenderSystem::updateIndexBuffer(
    void* BufferID, const dim::UniversalBuffer &BufferData, const IndexFormat* Format, const EMeshBufferUsage Usage)
{
    if (BufferID && Format)
    {
        D3D9IndexBuffer* Buffer = static_cast<D3D9IndexBuffer*>(BufferID);
        Buffer->update(D3DDevice_, BufferData, Format, Usage);
    }
}

void Direct3D9RenderSystem::updateVertexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index)
{
    if (BufferID && BufferData.getSize())
    {
        D3D9VertexBuffer* Buffer = static_cast<D3D9VertexBuffer*>(BufferID);
        Buffer->update(D3DDevice_, BufferData, Index);
    }
}
void Direct3D9RenderSystem::updateIndexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index)
{
    if (BufferID && BufferData.getSize())
    {
        D3D9IndexBuffer* Buffer = static_cast<D3D9IndexBuffer*>(BufferID);
        Buffer->update(D3DDevice_, BufferData, Index);
    }
}

void Direct3D9RenderSystem::drawMeshBuffer(const MeshBuffer* MeshBuffer)
{
    /* Get reference mesh buffer */
    if (!MeshBuffer)
        return;
    
    const video::MeshBuffer* OrigMeshBuffer = MeshBuffer;
    MeshBuffer = MeshBuffer->getReference();
    
    if (!MeshBuffer->renderable())
        return;
    
    /* Surface shader callback */
    if (CurShaderClass_ && ShaderSurfaceCallback_)
        ShaderSurfaceCallback_(CurShaderClass_, MeshBuffer->getSurfaceTextureList());
    
    /* Get hardware vertex- and index buffers */
    D3D9VertexBuffer* VertexBuffer = static_cast<D3D9VertexBuffer*>(MeshBuffer->getVertexBufferID());
    D3D9IndexBuffer* IndexBuffer   = static_cast<D3D9IndexBuffer*>(MeshBuffer->getIndexBufferID());
    
    /* Bind textures */
    if (__isTexturing)
        bindTextureList(OrigMeshBuffer->getSurfaceTextureList());
    
    /* Setup vertex format */
    D3DDevice_->SetFVF(VertexBuffer->FormatFlags_);
    
    /* Get primitive count */
    D3DPRIMITIVETYPE PrimitiveType  = D3DPT_TRIANGLELIST;
    u32 PrimitiveCount              = MeshBuffer->getIndexCount();
    u32 ArrayIndexCount             = MeshBuffer->getVertexCount();
    
    switch (MeshBuffer->getPrimitiveType())
    {
        case PRIMITIVE_TRIANGLES:
            PrimitiveType   = D3DPT_TRIANGLELIST;
            PrimitiveCount  = PrimitiveCount / 3;
            ArrayIndexCount = ArrayIndexCount / 3;
            break;
        case PRIMITIVE_TRIANGLE_STRIP:
            PrimitiveType   = D3DPT_TRIANGLESTRIP;
            PrimitiveCount  = PrimitiveCount - 2;
            ArrayIndexCount = ArrayIndexCount - 2;
            break;
        case PRIMITIVE_TRIANGLE_FAN:
            PrimitiveType   = D3DPT_TRIANGLEFAN;
            PrimitiveCount  = PrimitiveCount - 2;
            ArrayIndexCount = ArrayIndexCount - 2;
            break;
        case PRIMITIVE_LINES:
            PrimitiveType   = D3DPT_LINELIST;
            PrimitiveCount  = PrimitiveCount / 2;
            ArrayIndexCount = ArrayIndexCount / 2;
            break;
        case PRIMITIVE_LINE_STRIP:
            PrimitiveType   = D3DPT_LINESTRIP;
            PrimitiveCount  = PrimitiveCount - 1;
            ArrayIndexCount = ArrayIndexCount - 1;
            break;
        case PRIMITIVE_POINTS:
            PrimitiveType   = D3DPT_POINTLIST;
            break;
        default:
            return;
    }
    
    /* Check if hardware buffers are available */
    if (VertexBuffer->HWBuffer_)
    {
        /* Bind hardware mesh buffer */
        D3DDevice_->SetStreamSource(
            0, VertexBuffer->HWBuffer_,
            0, MeshBuffer->getVertexFormat()->getFormatSize()
        );
        
        /* Draw the primitives */
        if (MeshBuffer->getIndexBufferEnable() && IndexBuffer)
        {
            D3DDevice_->SetIndices(IndexBuffer->HWBuffer_);
            
            D3DDevice_->DrawIndexedPrimitive(
                PrimitiveType, 0, 0, MeshBuffer->getVertexCount(), 0, PrimitiveCount
            );
        }
        else
            D3DDevice_->DrawPrimitive(PrimitiveType, 0, ArrayIndexCount);
        
        /* Unbind hardware mesh buffer */
        D3DDevice_->SetStreamSource(0, 0, 0, 0);
        D3DDevice_->SetIndices(0);
    }
    else
    {
        /* Draw the primitives */
        if (MeshBuffer->getIndexBufferEnable())
        {
            D3DDevice_->DrawIndexedPrimitiveUP(
                D3DPT_TRIANGLELIST, 0,
                MeshBuffer->getVertexCount(),
                PrimitiveCount,
                MeshBuffer->getIndexBuffer().getArray(),
                IndexBuffer->FormatFlags_,
                MeshBuffer->getVertexBuffer().getArray(),
                MeshBuffer->getVertexFormat()->getFormatSize()
            );
        }
        else
        {
            D3DDevice_->DrawPrimitiveUP(
                D3DPT_TRIANGLELIST,
                ArrayIndexCount,
                MeshBuffer->getVertexBuffer().getArray(),
                MeshBuffer->getVertexFormat()->getFormatSize()
            );
        }
    }
    
    /* Unbind textures */
    if (__isTexturing)
        unbindTextureList(OrigMeshBuffer->getSurfaceTextureList());
    
    #ifdef SP_DEBUGMODE
    ++RenderSystem::NummDrawCalls_;
    ++RenderSystem::NumMeshBufferBindings_;
    #endif
}


/* === Render states === */

void Direct3D9RenderSystem::setRenderState(const video::ERenderStates Type, s32 State)
{
    switch (Type)
    {
        case RENDER_ALPHATEST:
            D3DDevice_->SetRenderState(D3DRS_ALPHATESTENABLE, State); break;
        case RENDER_BLEND:
            D3DDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, State);
            isImageBlending_ = (State != 0);
            break;
        case RENDER_COLORMATERIAL:
            D3DDevice_->SetRenderState(D3DRS_COLORVERTEX, State); break;
        case RENDER_CULLFACE:
            D3DDevice_->SetRenderState(D3DRS_CULLMODE, State ? D3DCULL_CCW : D3DCULL_NONE); break;
        case RENDER_DEPTH:
            D3DDevice_->SetRenderState(D3DRS_ZENABLE, State); break;
        case RENDER_DITHER:
            D3DDevice_->SetRenderState(D3DRS_DITHERENABLE, State); break;
        case RENDER_FOG:
            D3DDevice_->SetRenderState(D3DRS_FOGENABLE, State); break;
        case RENDER_LIGHTING:
            D3DDevice_->SetRenderState(D3DRS_LIGHTING, State); break;
        case RENDER_LINESMOOTH:
            D3DDevice_->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, State); break;
        case RENDER_MULTISAMPLE:
            D3DDevice_->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, State); break;
        case RENDER_NORMALIZE:
        case RENDER_RESCALENORMAL:
            D3DDevice_->SetRenderState(D3DRS_NORMALIZENORMALS, State); break;
        case RENDER_POINTSMOOTH:
            break;
        case RENDER_SCISSOR:
            D3DDevice_->SetRenderState(D3DRS_SCISSORTESTENABLE, State); break;
        case RENDER_STENCIL:
            D3DDevice_->SetRenderState(D3DRS_STENCILENABLE, State); break;
        case RENDER_TEXTURE:
            __isTexturing = (State != 0); break;
    }
}

s32 Direct3D9RenderSystem::getRenderState(const video::ERenderStates Type) const
{
    DWORD State = 0;
    
    switch (Type)
    {
        case RENDER_ALPHATEST:
            D3DDevice_->GetRenderState(D3DRS_ALPHATESTENABLE, &State); break;
        case RENDER_BLEND:
            return static_cast<s32>(isImageBlending_);
        case RENDER_COLORMATERIAL:
            D3DDevice_->GetRenderState(D3DRS_COLORVERTEX, &State); break;
        case RENDER_CULLFACE:
            D3DDevice_->GetRenderState(D3DRS_CULLMODE, &State);
            State = (s32)(State == D3DCULL_CCW);
            break;
        case RENDER_DEPTH:
            D3DDevice_->GetRenderState(D3DRS_ZENABLE, &State); break;
        case RENDER_DITHER:
            D3DDevice_->GetRenderState(D3DRS_DITHERENABLE, &State); break;
        case RENDER_FOG:
            D3DDevice_->GetRenderState(D3DRS_FOGENABLE, &State); break;
        case RENDER_LIGHTING:
            D3DDevice_->GetRenderState(D3DRS_LIGHTING, &State); break;
        case RENDER_LINESMOOTH:
            D3DDevice_->GetRenderState(D3DRS_ANTIALIASEDLINEENABLE, &State); break;
        case RENDER_MULTISAMPLE:
            D3DDevice_->GetRenderState(D3DRS_MULTISAMPLEANTIALIAS, &State); break;
        case RENDER_NORMALIZE:
        case RENDER_RESCALENORMAL:
            D3DDevice_->GetRenderState(D3DRS_NORMALIZENORMALS, &State); break;
        case RENDER_POINTSMOOTH:
            break;
        case RENDER_SCISSOR:
            D3DDevice_->GetRenderState(D3DRS_SCISSORTESTENABLE, &State); break;
        case RENDER_STENCIL:
            D3DDevice_->GetRenderState(D3DRS_STENCILENABLE, &State); break;
        case RENDER_TEXTURE:
            return (s32)__isTexturing;
    }
    
    return State;
}

void Direct3D9RenderSystem::disableTriangleListStates()
{
    /* Default render functions */
    D3DDevice_->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
    D3DDevice_->SetRenderState(D3DRS_ALPHAREF, 0);
    D3DDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    D3DDevice_->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    
    LastMaterial_ = 0;
}

void Direct3D9RenderSystem::disable3DRenderStates()
{
    /* Disable all used states */
    D3DDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
    D3DDevice_->SetRenderState(D3DRS_LIGHTING, false);
    D3DDevice_->SetRenderState(D3DRS_FOGENABLE, false);
}

void Direct3D9RenderSystem::disableTexturing()
{
    //???
}

void Direct3D9RenderSystem::setDefaultAlphaBlending()
{
    D3DDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    D3DDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void Direct3D9RenderSystem::enableBlending()
{
    D3DDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
}

void Direct3D9RenderSystem::disableBlending()
{
    D3DDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
}


/*
 * ======= Lighting =======
 */

void Direct3D9RenderSystem::addDynamicLightSource(
    u32 LightID, scene::ELightModels Type,
    video::color &Diffuse, video::color &Ambient, video::color &Specular,
    f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic)
{
    if (LightID >= MAX_COUNT_OF_LIGHTS)
        return;
    
    /* Set the lighting type */
    switch (Type)
    {
        case scene::LIGHT_DIRECTIONAL:
            D3DActiveLight_.Type = D3DLIGHT_DIRECTIONAL; break;
        case scene::LIGHT_POINT:
            D3DActiveLight_.Type = D3DLIGHT_POINT; break;
        case scene::LIGHT_SPOT:
            D3DActiveLight_.Type = D3DLIGHT_SPOT; break;
    }
    
    /* Default values */
    D3DActiveLight_.Range           = 1000.0f;
    D3DActiveLight_.Falloff         = 1.0f;
    D3DActiveLight_.Direction.z     = 1.0f;
    
    /* Lighting colors */
    D3DActiveLight_.Diffuse         = getD3DColor(Diffuse);
    D3DActiveLight_.Ambient         = getD3DColor(Ambient);
    D3DActiveLight_.Specular        = getD3DColor(Specular);
    
    /* Volumetric light attenuations */
    D3DActiveLight_.Attenuation0    = AttenuationConstant;
    D3DActiveLight_.Attenuation1    = AttenuationLinear;
    D3DActiveLight_.Attenuation2    = AttenuationQuadratic;
    
    /* Set the light attributes */
    D3DDevice_->SetLight(LightID, &D3DActiveLight_);
    
    /* Enable the light */
    D3DDevice_->LightEnable(LightID, true);
}

void Direct3D9RenderSystem::setLightStatus(u32 LightID, bool isEnable)
{
    D3DDevice_->LightEnable(LightID, isEnable);
}

void Direct3D9RenderSystem::setLightColor(
    u32 LightID, const video::color &Diffuse, const video::color &Ambient, const video::color &Specular)
{
    /* Get the light attributes */
    D3DDevice_->GetLight(LightID, &D3DActiveLight_);
    
    /* Lighting colors */
    D3DActiveLight_.Diffuse     = getD3DColor(Diffuse);
    D3DActiveLight_.Ambient     = getD3DColor(Ambient);
    D3DActiveLight_.Specular    = getD3DColor(Specular);
    
    /* Set the light attributes */
    D3DDevice_->SetLight(LightID, &D3DActiveLight_);
}


/*
 * ======= Fog effect =======
 */

void Direct3D9RenderSystem::setFog(const EFogTypes Type)
{
    /* Select the fog mode */
    switch (Fog_.Type = Type)
    {
        case FOG_NONE:
        {
            __isFog = false;
        }
        break;
        
        case FOG_STATIC:
        {
            __isFog = true;
            
            /* Set fog type */
            switch (Fog_.Mode)
            {
                case FOG_PALE:
                    D3DDevice_->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP); break;
                case FOG_THICK:
                    D3DDevice_->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP2); break;
            }
            
            /* Range settings */
            D3DDevice_->SetRenderState(D3DRS_FOGDENSITY, *(DWORD*)&Fog_.Range);
            D3DDevice_->SetRenderState(D3DRS_FOGSTART, *(DWORD*)&Fog_.Near);
            D3DDevice_->SetRenderState(D3DRS_FOGEND, *(DWORD*)&Fog_.Far);
        }
        break;
        
        case FOG_VOLUMETRIC:
        {
            __isFog = true;
            
            /* Renderer settings */
            D3DDevice_->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
            D3DDevice_->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR); // ???
            D3DDevice_->SetRenderState(D3DRS_FOGDENSITY, *(DWORD*)&Fog_.Range);
            D3DDevice_->SetRenderState(D3DRS_FOGSTART, 0);
            D3DDevice_->SetRenderState(D3DRS_FOGEND, 1);
        }
        break;
    }
}

void Direct3D9RenderSystem::setFogColor(const video::color &Color)
{
    D3DDevice_->SetRenderState(D3DRS_FOGCOLOR, Color.getSingle());
    Fog_.Color = Color;
}

void Direct3D9RenderSystem::setFogRange(f32 Range, f32 NearPlane, f32 FarPlane, const EFogModes Mode)
{
    RenderSystem::setFogRange(Range, NearPlane, FarPlane, Mode);
    
    if (Fog_.Type != FOG_VOLUMETRIC)
    {
        /* Set fog type */
        switch (Fog_.Mode)
        {
            case FOG_PALE:
                D3DDevice_->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP); break;
            case FOG_THICK:
                D3DDevice_->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP2); break;
        }
        
        /* Range settings */
        D3DDevice_->SetRenderState(D3DRS_FOGDENSITY, *(DWORD*)&Fog_.Range);
        D3DDevice_->SetRenderState(D3DRS_FOGSTART, *(DWORD*)&Fog_.Near);
        D3DDevice_->SetRenderState(D3DRS_FOGEND, *(DWORD*)&Fog_.Far);
    }
}


/*
 * ======= Stencil buffer =======
 */

void Direct3D9RenderSystem::drawStencilShadowVolume(
    const dim::vector3df* pTriangleList, s32 Count, bool ZFailMethod, bool VolumetricShadow)
{
    if (!pTriangleList || !Count)
        return;
    
    for (s32 i = 0; i < 4; ++i)
    {
        D3DDevice_->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
        D3DDevice_->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    
    D3DDevice_->SetFVF(FVF_POSITION);
    
    D3DDevice_->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    D3DDevice_->SetRenderState(D3DRS_STENCILENABLE, TRUE);
    
    if (ZFailMethod)
    {
        /* Set the render states */
        D3DDevice_->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
        D3DDevice_->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
        D3DDevice_->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
        D3DDevice_->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
        
        D3DDevice_->SetRenderState(D3DRS_STENCILREF, 0x0);
        D3DDevice_->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
        D3DDevice_->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
        
        D3DDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        D3DDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
        D3DDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        
        /* Draw the stencil shadow volume */
        D3DDevice_->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
        D3DDevice_->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCRSAT);
        D3DDevice_->DrawPrimitiveUP(D3DPT_TRIANGLELIST, Count / 3, pTriangleList, sizeof(dim::vector3df));
        
        if (!VolumetricShadow)
        {
            D3DDevice_->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
            D3DDevice_->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_DECRSAT);
            D3DDevice_->DrawPrimitiveUP(D3DPT_TRIANGLELIST, Count / 3, pTriangleList, sizeof(dim::vector3df));
        }
    }
    else
    {
        /* Set the render states */
        D3DDevice_->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
        D3DDevice_->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
        D3DDevice_->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
        
        D3DDevice_->SetRenderState(D3DRS_STENCILREF, 0x1);
        D3DDevice_->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
        D3DDevice_->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
        
        D3DDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        D3DDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
        D3DDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        
        /* Draw the stencil shadow volume */
        D3DDevice_->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
        D3DDevice_->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCRSAT);
        D3DDevice_->DrawPrimitiveUP(D3DPT_TRIANGLELIST, Count / 3, pTriangleList, sizeof(dim::vector3df));
        
        if (!VolumetricShadow)
        {
            D3DDevice_->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
            D3DDevice_->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECRSAT);
            D3DDevice_->DrawPrimitiveUP(D3DPT_TRIANGLELIST, Count / 3, pTriangleList, sizeof(dim::vector3df));
        }
    }
}

void Direct3D9RenderSystem::drawStencilShadow(const video::color &Color)
{
    /* Configure the stencil states */
    D3DDevice_->SetRenderState(D3DRS_STENCILREF, 0x1);
    D3DDevice_->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
    D3DDevice_->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
    D3DDevice_->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
    D3DDevice_->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
    D3DDevice_->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
    D3DDevice_->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    
    /* Draw the rectangle */
    beginDrawing2D();
    draw2DRectangle(dim::rect2di(0, 0, gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight), Color);
    endDrawing2D();
    
    /* Clear the stencil buffer */
    D3DDevice_->Clear(0, 0, D3DCLEAR_STENCIL, 0, 1.0, 0);
    
    /* Back settings */
    D3DDevice_->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    D3DDevice_->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}


/* === Clipping planes === */

void Direct3D9RenderSystem::setClipPlane(u32 Index, const dim::plane3df &Plane, bool Enable)
{
    if (Index >= MaxClippingPlanes_)
        return;
    
    D3DDevice_->SetClipPlane(Index, (const f32*)&Plane);
    
    DWORD State;
    
    D3DDevice_->GetRenderState(D3DRS_CLIPPLANEENABLE, &State);
    
    if (Enable)
        State |= (1 << Index);
    else
        State &= ~(1 << Index);
    
    D3DDevice_->SetRenderState(D3DRS_CLIPPLANEENABLE, State);
}



/*
 * ======= Shader programs =======
 */

ShaderClass* Direct3D9RenderSystem::createShaderClass(VertexFormat* VertexInputLayout)
{
    ShaderClass* NewShaderClass = new Direct3D9ShaderClass();
    
    ShaderClassList_.push_back(NewShaderClass);
    
    return NewShaderClass;
}

Shader* Direct3D9RenderSystem::createShader(
    ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
    const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint)
{
    Shader* NewShader = new Direct3D9Shader(ShaderClassObj, Type, Version);
    
    NewShader->compile(ShaderBuffer, EntryPoint);
    
    if (!ShaderClassObj)
        NewShader->getShaderClass()->link();
    
    ShaderList_.push_back(NewShader);
    
    return NewShader;
}

Shader* Direct3D9RenderSystem::createCgShader(
    ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
    const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint,
    const c8** CompilerOptions)
{
    Shader* NewShader = 0;
    
    #ifndef SP_COMPILE_WITH_CG
    io::Log::error("This engine was not compiled with the Cg toolkit");
    #else
    if (RenderQuery_[RENDERQUERY_SHADER])
        NewShader = new CgShaderProgramD3D9(ShaderClassObj, Type, Version);
    else
    #endif
        NewShader = new Shader(ShaderClassObj, Type, Version);
    
    NewShader->compile(ShaderBuffer, EntryPoint, CompilerOptions);
    
    if (!ShaderClassObj)
        NewShader->getShaderClass()->link();
    
    ShaderList_.push_back(NewShader);
    
    return NewShader;
}

void Direct3D9RenderSystem::unbindShaders()
{
    D3DDevice_->SetVertexShader(0);
    D3DDevice_->SetPixelShader(0);
}


/*
 * ======= Drawing 2D objects =======
 */

void Direct3D9RenderSystem::beginDrawing2D()
{
    /* Disable depth test (only 2d) */
    D3DDevice_->SetRenderState(D3DRS_ZENABLE, false);
    
    /* Alpha blending */
    D3DDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    D3DDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    
    /* Enable alpha ablending */
    D3DDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    
    /* Unit matrices */
    const dim::matrix4f IdentityMatrix;
    
    setViewMatrix(IdentityMatrix);
    setWorldMatrix(IdentityMatrix);
    
    Matrix2D_.make2Dimensional(
        gSharedObjects.ScreenWidth,
        -gSharedObjects.ScreenHeight,
        gSharedObjects.ScreenWidth,
        gSharedObjects.ScreenHeight
    );
    setProjectionMatrix(Matrix2D_);
    
    /* Disable 3d render states */
    D3DDevice_->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    D3DDevice_->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    D3DDevice_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    
    /* Use no texture layer */
    D3DDevice_->SetTexture(0, 0);
    
    setViewport(0, dim::size2di(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight));
}

void Direct3D9RenderSystem::endDrawing2D()
{
    /* Disable 2D render states */
    D3DDevice_->SetRenderState(D3DRS_ZENABLE, true);
    D3DDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
}

void Direct3D9RenderSystem::beginDrawing3D()
{
    /* Update camera view */
    __spSceneManager->getActiveCamera()->setupRenderView();
    
    /* 3D render states */
    D3DDevice_->SetRenderState(D3DRS_LIGHTING, false);
    D3DDevice_->SetRenderState(D3DRS_FOGENABLE, false);
}

void Direct3D9RenderSystem::endDrawing3D()
{
}

void Direct3D9RenderSystem::setBlending(const EBlendingTypes SourceBlend, const EBlendingTypes DestBlend)
{
    D3DDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBlendingList[SourceBlend]);
    D3DDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBlendingList[DestBlend]);
}

void Direct3D9RenderSystem::setClipping(bool Enable, const dim::point2di &Position, const dim::size2di &Dimension)
{
    D3DDevice_->SetRenderState(D3DRS_SCISSORTESTENABLE, Enable);
    
    RECT rc;
    {
        rc.left     = Position.X;
        rc.top      = Position.Y;
        rc.right    = Position.X + Dimension.Width;
        rc.bottom   = Position.Y + Dimension.Height;
    }
    D3DDevice_->SetScissorRect(&rc);
}

void Direct3D9RenderSystem::setViewport(const dim::point2di &Position, const dim::size2di &Dimension)
{
    D3DVIEWPORT9 d3dViewport;
    {
        d3dViewport.X       = Position.X;
        d3dViewport.Y       = Position.Y;
        d3dViewport.Width   = Dimension.Width;
        d3dViewport.Height  = Dimension.Height;
        d3dViewport.MinZ    = 0.0f;
        d3dViewport.MaxZ    = 1.0f;
    }
    D3DDevice_->SetViewport(&d3dViewport);
}

bool Direct3D9RenderSystem::setRenderTarget(Texture* Target)
{
    if (Target && Target->getRenderTarget())
    {
        if (!setRenderTargetSurface(0, Target))
            return false;
        
        const std::vector<Texture*>& MRTexList = Target->getMultiRenderTargets();
        
        for (u32 i = 0; i < MRTexList.size(); ++i)
        {
            if (!setRenderTargetSurface(i + 1, MRTexList[i]))
                return false;
        }
        
        RenderTarget_ = Target;
    }
    else if (RenderTarget_ && LastRenderTarget_)
    {
        /* Set the last render target */
        D3DDevice_->SetRenderTarget(0, LastRenderTarget_);
        releaseObject(LastRenderTarget_);
        
        const u32 RTCount = RenderTarget_->getMultiRenderTargets().size() + 1;
        
        for (u32 i = 1; i < RTCount && i < DevCaps_.NumSimultaneousRTs; ++i)
            D3DDevice_->SetRenderTarget(i, 0);
        
        RenderTarget_ = 0;
    }
    
    return true;
}

void Direct3D9RenderSystem::setLineSize(s32 Size)
{
    // do nothing -> Direct3D9 does not support line size
}
void Direct3D9RenderSystem::setPointSize(s32 Size)
{
    f32 tmp = (f32)Size;
    D3DDevice_->SetRenderState(D3DRS_POINTSIZE, *(DWORD*)(&tmp));
}


/*
 * ======= Image drawing =======
 */

void Direct3D9RenderSystem::draw2DImage(
    const Texture* Tex, const dim::point2di &Position, const color &Color)
{
    /* Bind the texture */
    Tex->bind();
    
    /* Set the texture attributes */
    D3DDevice_->SetTextureStageState(0, isImageBlending_ ? D3DTSS_ALPHAOP : D3DTSS_COLOROP, D3DTOP_MODULATE);
    
    /* Temporary variables */
    s32 Width   = Tex->getSize().Width;
    s32 Height  = Tex->getSize().Height;
    u32 Clr     = Color.getSingle();
    
    /* Set the vertex data */
    const f32 x = static_cast<f32>(Position.X);
    const f32 y = static_cast<f32>(Position.Y);
    
    SPrimitiveVertex VerticesList[4] = {
        SPrimitiveVertex(x,         y,          0.0f, Clr, 0.0f, 0.0f),
        SPrimitiveVertex(x + Width, y,          0.0f, Clr, 1.0f, 0.0f),
        SPrimitiveVertex(x + Width, y + Height, 0.0f, Clr, 1.0f, 1.0f),
        SPrimitiveVertex(x,         y + Height, 0.0f, Clr, 0.0f, 1.0f)
    };
    
    /* Set the render states */
    D3DDevice_->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    
    /* Update the primitive list */
    updatePrimitiveList(VerticesList, 4);
    
    /* Draw the rectangle */
    D3DDevice_->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
    
    /* Unbind the texture */
    Tex->unbind();
}

void Direct3D9RenderSystem::draw2DImage(
    const Texture* Tex, const dim::rect2di &Position, const dim::rect2df &Clipping, const color &Color)
{
    /* Bind the texture */
    Tex->bind();
    
    /* Set the texture attributes */
    D3DDevice_->SetTextureStageState(0, isImageBlending_ ? D3DTSS_ALPHAOP : D3DTSS_COLOROP, D3DTOP_MODULATE);
    
    /* Temporary variables */
    u32 Clr = Color.getSingle();
    
    /* Set the vertex data */
    SPrimitiveVertex VerticesList[4] = {
        SPrimitiveVertex((f32)Position.Left,                    (f32)Position.Top,                      0.0f, Clr, Clipping.Left, Clipping.Top),
        SPrimitiveVertex((f32)(Position.Left + Position.Right), (f32)Position.Top,                      0.0f, Clr, Clipping.Right, Clipping.Top),
        SPrimitiveVertex((f32)(Position.Left + Position.Right), (f32)(Position.Top + Position.Bottom),  0.0f, Clr, Clipping.Right, Clipping.Bottom),
        SPrimitiveVertex((f32)Position.Left,                    (f32)(Position.Top + Position.Bottom),  0.0f, Clr, Clipping.Left, Clipping.Bottom)
    };
    
    /* Set the render states */
    D3DDevice_->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    
    /* Update the primitive list */
    updatePrimitiveList(VerticesList, 4);
    
    /* Draw the rectangle */
    D3DDevice_->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
    
    /* Unbind the texture */
    Tex->unbind();
}

void Direct3D9RenderSystem::draw2DImage(
    const Texture* Tex, const dim::point2di &Position, f32 Rotation, f32 Radius, const color &Color)
{
    /* Set the texture attributes */
    D3DDevice_->SetTextureStageState(0, isImageBlending_ ? D3DTSS_ALPHAOP : D3DTSS_COLOROP, D3DTOP_MODULATE);
    
    /* Bind the texture */
    Tex->bind();
    
    /* Temporary variables */
    u32 Clr = Color.getSingle();
    
    /* Set the vertex data */
    Radius *= math::SQRT2F;
    SPrimitiveVertex VerticesList[4] = {
        SPrimitiveVertex( math::Sin(Rotation -  45.0f)*Radius + Position.X, -math::Cos(Rotation -  45)*Radius + Position.Y, 0.0f, Clr, 0.0f, 0.0f ),
        SPrimitiveVertex( math::Sin(Rotation +  45.0f)*Radius + Position.X, -math::Cos(Rotation +  45)*Radius + Position.Y, 0.0f, Clr, 1.0f, 0.0f ),
        SPrimitiveVertex( math::Sin(Rotation + 135.0f)*Radius + Position.X, -math::Cos(Rotation + 135)*Radius + Position.Y, 0.0f, Clr, 1.0f, 1.0f ),
        SPrimitiveVertex( math::Sin(Rotation - 135.0f)*Radius + Position.X, -math::Cos(Rotation - 135)*Radius + Position.Y, 0.0f, Clr, 0.0f, 1.0f )
    };
    
    /* Set the render states */
    D3DDevice_->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    
    /* Update the primitive list */
    updatePrimitiveList(VerticesList, 4);
    
    /* Draw the rectangle */
    D3DDevice_->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
    
    /* Unbind the texture */
    Tex->unbind();
}

void Direct3D9RenderSystem::draw2DImage(
    const Texture* Tex,
    const dim::point2di &lefttopPosition, const dim::point2di &righttopPosition,
    const dim::point2di &rightbottomPosition, const dim::point2di &leftbottomPosition,
    const dim::point2df &lefttopClipping, const dim::point2df &righttopClipping,
    const dim::point2df &rightbottomClipping, const dim::point2df &leftbottomClipping,
    const color &lefttopColor, const color &righttopColor,
    const color &rightbottomColor, const color &leftbottomColor)
{
    /* Set the texture attributes */
    D3DDevice_->SetTextureStageState(0, isImageBlending_ ? D3DTSS_ALPHAOP : D3DTSS_COLOROP, D3DTOP_MODULATE);
    
    /* Bind the texture */
    Tex->bind();
    
    /* Set the vertex data */
    SPrimitiveVertex VerticesList[4] = {
        SPrimitiveVertex( (f32)lefttopPosition.X, (f32)lefttopPosition.Y, 0.0f, lefttopColor.getSingle(), lefttopClipping.X, lefttopClipping.Y ),
        SPrimitiveVertex( (f32)righttopPosition.X, (f32)righttopPosition.Y, 0.0f, righttopColor.getSingle(), righttopClipping.X, righttopClipping.Y ),
        SPrimitiveVertex( (f32)rightbottomPosition.X, (f32)rightbottomPosition.Y, 0.0f, rightbottomColor.getSingle(), rightbottomClipping.X, rightbottomClipping.Y ),
        SPrimitiveVertex( (f32)leftbottomPosition.X, (f32)leftbottomPosition.Y, 0.0f, leftbottomColor.getSingle(), leftbottomClipping.X, leftbottomClipping.Y )
    };
    
    /* Set the render states */
    D3DDevice_->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    
    /* Update the primitive list */
    updatePrimitiveList(VerticesList, 4);
    
    /* Draw the rectangle */
    D3DDevice_->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
    
    /* Unbind the texture */
    Tex->unbind();
}


/*
 * ======= Primitive drawing =======
 */

color Direct3D9RenderSystem::getPixelColor(const dim::point2di &Position) const
{
    return color(0); // todo
}
f32 Direct3D9RenderSystem::getPixelDepth(const dim::point2di &Position) const
{
    return 0.0f; // todo
}

void Direct3D9RenderSystem::draw2DPoint(const dim::point2di &Position, const color &Color)
{
    /* Set the vertex data */
    SPrimitiveVertex VerticesList[1] = {
        SPrimitiveVertex( (f32)Position.X, (f32)Position.Y, 0.0f, Color.getSingle() ),
    };
    
    /* Update the primitive list */
    updatePrimitiveList(VerticesList, 1);
    
    /* Draw the rectangle */
    D3DDevice_->DrawPrimitive(D3DPT_POINTLIST, 0, 1);
}

void Direct3D9RenderSystem::draw2DLine(
    const dim::point2di &PositionA, const dim::point2di &PositionB, const color &Color)
{
    draw2DLine(PositionA, PositionB, Color, Color);
}

void Direct3D9RenderSystem::draw2DLine(
    const dim::point2di &PositionA, const dim::point2di &PositionB, const color &ColorA, const color &ColorB)
{
    /* Set the vertex data */
    SPrimitiveVertex VerticesList[2] = {
        SPrimitiveVertex( (f32)PositionA.X, (f32)PositionA.Y, 0.0f, ColorA.getSingle() ),
        SPrimitiveVertex( (f32)PositionB.X, (f32)PositionB.Y, 0.0f, ColorB.getSingle() ),
    };
    
    /* Update the primitive list */
    updatePrimitiveList(VerticesList, 2);
    
    /* Draw the rectangle */
    D3DDevice_->DrawPrimitive(D3DPT_LINELIST, 0, 1);
}

void Direct3D9RenderSystem::draw2DLine(
    const dim::point2di &PositionA, const dim::point2di &PositionB, const color &Color, s32 DotLength)
{
    draw2DLine(PositionA, PositionB, Color, Color); // !!!
}

void Direct3D9RenderSystem::draw2DRectangle(
    const dim::rect2di &Rect, const color &Color, bool isSolid)
{
    draw2DRectangle(Rect, Color, Color, Color, Color, isSolid);
}

void Direct3D9RenderSystem::draw2DRectangle(
    const dim::rect2di &Rect, const color &lefttopColor, const color &righttopColor,
    const color &rightbottomColor, const color &leftbottomColor, bool isSolid)
{
    /* Set the vertex data */
    SPrimitiveVertex VerticesList[4] = {
        SPrimitiveVertex( (f32)Rect.Left, (f32)Rect.Top, 0.0f, lefttopColor.getSingle() ),
        SPrimitiveVertex( (f32)Rect.Right, (f32)Rect.Top, 0.0f, righttopColor.getSingle() ),
        SPrimitiveVertex( (f32)Rect.Right, (f32)Rect.Bottom, 0.0f, rightbottomColor.getSingle() ),
        SPrimitiveVertex( (f32)Rect.Left, (f32)Rect.Bottom, 0.0f, leftbottomColor.getSingle() )
    };
    
    /* Set the render states */
    D3DDevice_->SetRenderState(D3DRS_FILLMODE, isSolid ? D3DFILL_SOLID : D3DFILL_WIREFRAME);
    
    /* Update the primitive list */
    updatePrimitiveList(VerticesList, 4);
    
    /* Draw the rectangle */
    D3DDevice_->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
}


/*
 * ======= Extra drawing functions =======
 */

void Direct3D9RenderSystem::draw2DPolygon(
    const ERenderPrimitives Type, const scene::SPrimitiveVertex2D* VerticesList, u32 Count)
{
    if (!VerticesList || !Count)
        return;
    
    /* Select the primitive type */
    D3DPRIMITIVETYPE Mode;
    UINT PrimitiveCount = 0;
    
    switch (Type)
    {
        case PRIMITIVE_POINTS:
            Mode = D3DPT_POINTLIST, PrimitiveCount = Count; break;
        case PRIMITIVE_LINES:
            Mode = D3DPT_LINELIST, PrimitiveCount = Count/2; break;
        case PRIMITIVE_LINE_STRIP:
            Mode = D3DPT_LINESTRIP, PrimitiveCount = Count/2 + 1; break;
        case PRIMITIVE_TRIANGLES:
            Mode = D3DPT_TRIANGLELIST, PrimitiveCount = Count/3; break;
        case PRIMITIVE_TRIANGLE_STRIP:
            Mode = D3DPT_TRIANGLESTRIP, PrimitiveCount = Count - 2; break;
        case PRIMITIVE_TRIANGLE_FAN:
            Mode = D3DPT_TRIANGLEFAN, PrimitiveCount = Count - 2; break;
        case PRIMITIVE_LINE_LOOP:
        case PRIMITIVE_QUADS:
        case PRIMITIVE_QUAD_STRIP:
        case PRIMITIVE_POLYGON:
        default:
            return;
    }
    
    /* Setup the FVF for 2D graphics */
    D3DDevice_->SetFVF(FVF_VERTEX2D);
    
    /* Render primitives */
    D3DDevice_->DrawPrimitiveUP(
        Mode,
        PrimitiveCount,
        VerticesList,
        sizeof(scene::SPrimitiveVertex2D)
    );
}

void Direct3D9RenderSystem::draw2DPolygonImage(
    const ERenderPrimitives Type, Texture* Tex, const scene::SPrimitiveVertex2D* VerticesList, u32 Count)
{
    Tex->bind();
    draw2DPolygon(Type, VerticesList, Count);
    Tex->unbind();
}


/*
 * ======= 3D drawing functions =======
 */

void Direct3D9RenderSystem::draw3DPoint(const dim::vector3df &Position, const color &Color)
{
    setDrawingMatrix3D();
    
    /* Set the FVF (Flexible Vertex Format) */
    D3DDevice_->SetFVF(FVF_VERTEX3D);
    
    /* Set the vertices data */
    scene::SMeshVertex3D VerticesList[1] = {
        scene::SMeshVertex3D( Position.X, Position.Y, Position.Z, Color.getSingle() )
    };
    
    /* Draw the primitive */
    D3DDevice_->DrawPrimitiveUP(D3DPT_POINTLIST, 1, VerticesList, sizeof(scene::SMeshVertex3D));
}

void Direct3D9RenderSystem::draw3DLine(
    const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &Color)
{
    setDrawingMatrix3D();
    
    /* Set the FVF (Flexible Vertex Format) */
    D3DDevice_->SetFVF(FVF_VERTEX3D);
    
    /* Set the vertices data */
    scene::SMeshVertex3D VerticesList[2] = {
        scene::SMeshVertex3D( PositionA.X, PositionA.Y, PositionA.Z, Color.getSingle() ),
        scene::SMeshVertex3D( PositionB.X, PositionB.Y, PositionB.Z, Color.getSingle() )
    };
    
    /* Draw the primitive */
    D3DDevice_->DrawPrimitiveUP(D3DPT_LINELIST, 1, VerticesList, sizeof(scene::SMeshVertex3D));
}

void Direct3D9RenderSystem::draw3DLine(
    const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &ColorA, const color &ColorB)
{
    setDrawingMatrix3D();
    
    /* Set the FVF (Flexible Vertex Format) */
    D3DDevice_->SetFVF(FVF_VERTEX3D);
    
    /* Set the vertices data */
    scene::SMeshVertex3D VerticesList[2] = {
        scene::SMeshVertex3D( PositionA.X, PositionA.Y, PositionA.Z, ColorA.getSingle() ),
        scene::SMeshVertex3D( PositionB.X, PositionB.Y, PositionB.Z, ColorB.getSingle() ),
    };
    
    /* Draw the primitive */
    D3DDevice_->DrawPrimitiveUP(D3DPT_LINELIST, 1, VerticesList, sizeof(scene::SMeshVertex3D));
}

void Direct3D9RenderSystem::draw3DEllipse(
    const dim::vector3df &Position, const dim::vector3df &Rotation, const dim::size2df &Radius, const color &Color)
{
    // todo
}

void Direct3D9RenderSystem::draw3DTriangle(
    Texture* hTexture, const dim::triangle3df &Triangle, const color &Color)
{
    // todo
}


/*
 * ======= Texture loading & creating =======
 */

Texture* Direct3D9RenderSystem::createTexture(const STextureCreationFlags &CreationFlags)
{
    Texture* NewTexture = 0;
    
    /* Direct3D9 texture configurations */
    dim::vector3di Size(CreationFlags.Size.Width, CreationFlags.Size.Height, CreationFlags.Depth);
    
    if (createRendererTexture(CreationFlags.MipMaps, TEXTURE_2D, Size, CreationFlags.Format, 0))
    {
        NewTexture = new Direct3D9Texture(
            CurD3DTexture_, CurD3DCubeTexture_, CurD3DVolumeTexture_, CreationFlags
        );
        
        if (CreationFlags.Anisotropy > 0)
            NewTexture->setAnisotropicSamples(CreationFlags.Anisotropy);
    }
    else
        NewTexture = new Direct3D9Texture();
    
    /* Add the texture to the texture list */
    TextureList_.push_back(NewTexture);
    
    return NewTexture;
}

Texture* Direct3D9RenderSystem::createScreenShot(const dim::point2di &Position, dim::size2di Size)
{
    Texture* NewTexture = RenderSystem::createTexture(Size);
    
    createScreenShot(NewTexture, Position);
    
    //!TODO!
    
    /* Return the texture & exit the function */
    return NewTexture;
}

void Direct3D9RenderSystem::createScreenShot(Texture* Tex, const dim::point2di &Position)
{
    /* Get the Direct3D texture handle */
    IDirect3DTexture9* d3dTex = static_cast<Direct3D9Texture*>(Tex)->D3D2DTexture_;
    
    if (!d3dTex)
        return;
    
    IDirect3DSurface9* Surface = 0;
    
    D3DDISPLAYMODE DisplayMode;
    D3DDevice_->GetDisplayMode(0, &DisplayMode);
    
    if (D3DDevice_->CreateOffscreenPlainSurface(
        DisplayMode.Width, DisplayMode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &Surface, 0))
    {
        io::Log::error("Could not create Direct3D9 offscreen plain surface");
        return;
    }
    
    if (D3DDevice_->GetFrontBufferData(0, Surface))
    {
        Surface->Release();
        io::Log::error("Could not get front buffer data from Direct3D9 surface");
        return;
    }
    
    Surface->Release();
}


/*
 * ======= Font loading and text drawing =======
 */

Font* Direct3D9RenderSystem::createBitmapFont(const io::stringc &FontName, s32 FontSize, s32 Flags)
{
    /* Temporary variables */
    HRESULT Result = S_OK;
    
    if (FontSize <= 0)
        FontSize = DEF_FONT_SIZE;
    
    const s32 Width     = 0;
    const s32 Height    = FontSize;
    
    /* Create the Direct3D font */
    
    ID3DXFont* DxFont = 0;
    
    #if D3DX_SDK_VERSION < 24
    
    #ifdef _MSC_VER
    #pragma comment (lib, "d3dx9.lib")
    #endif
    
    Result = D3DXCreateFont(
        D3DDevice_, Height, Width,
        isBold ? FW_BOLD : 0, 0, isItalic,
        isSymbolUsing ? SYMBOL_CHARSET : ANSI_CHARSET,
        OUT_TT_ONLY_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH,
        FontName.c_str(), &DxFont
    );
    
    #else
    
    typedef HRESULT (WINAPI *PFND3DXCREATEFONTW)(
        IDirect3DDevice9* pDevice, INT Height, UINT Width, UINT Weight, UINT MipLevels, BOOL Italic,
        DWORD CharSet, DWORD OutputPrecision, DWORD Quality, DWORD PitchAndFamily, LPCWSTR pFacename, LPD3DXFONT * ppFont
    );
    typedef HRESULT (WINAPI *PFND3DXCREATEFONTA)(
        IDirect3DDevice9* pDevice, INT Height, UINT Width, UINT Weight, UINT MipLevels, BOOL Italic,
        DWORD CharSet, DWORD OutputPrecision, DWORD Quality, DWORD PitchAndFamily, LPCTSTR pFacename, LPD3DXFONT * ppFont
    );
    
    static PFND3DXCREATEFONTW pFncCreateFontW = 0;
    static PFND3DXCREATEFONTA pFncCreateFontA = 0;
    
    if (!pFncCreateFontW)
    {
        HMODULE hModule = LoadLibrary(d3dDllFileName.c_str());
        
        if (hModule)
        {
            pFncCreateFontW = (PFND3DXCREATEFONTW)GetProcAddress(hModule, "D3DXCreateFontW");
            
            if (!pFncCreateFontW)
            {
                io::Log::warning(
                    "Could not load function \"D3DXCreateFontW\" from Direct3D9 library file: \"" +
                    d3dDllFileName + "\", unicode is not supported"
                );
                
                pFncCreateFontA = (PFND3DXCREATEFONTA)GetProcAddress(hModule, "D3DXCreateFontA");
                
                if (!pFncCreateFontA)
                {
                    io::Log::error(
                        "Could not load function \"D3DXCreateFontA\" from Direct3D9 library file: \"" +
                        d3dDllFileName + "\""
                    );
                }
            }
        }
    }
    
    if (pFncCreateFontW)
    {
        Result = pFncCreateFontW(
            D3DDevice_,
            Height,
            Width,
            (Flags & FONT_BOLD) != 0 ? FW_BOLD : FW_NORMAL,
            0,
            (Flags & FONT_ITALIC) != 0,
            (Flags & FONT_SYMBOLS) != 0 ? SYMBOL_CHARSET : ANSI_CHARSET,
            OUT_TT_ONLY_PRECIS,
            ANTIALIASED_QUALITY,
            FF_DONTCARE | DEFAULT_PITCH,
            FontName.toUnicode().c_str(),
            &DxFont
        );
    }
    else if (pFncCreateFontA)
    {
        Result = pFncCreateFontA(
            D3DDevice_,
            Height,
            Width,
            (Flags & FONT_BOLD) != 0 ? FW_BOLD : FW_NORMAL,
            0,
            (Flags & FONT_ITALIC) != 0,
            (Flags & FONT_SYMBOLS) != 0 ? SYMBOL_CHARSET : ANSI_CHARSET,
            OUT_TT_ONLY_PRECIS,
            ANTIALIASED_QUALITY,
            FF_DONTCARE | DEFAULT_PITCH,
            FontName.c_str(),
            &DxFont
        );
    }
    
    #endif
    
    /* Check for errors */
    if (Result != S_OK)
        io::Log::error("Could not load font: \"" + FontName + "\"");
    
    /* Create device font */
    HFONT FontObject = 0;
    createDeviceFont(&FontObject, FontName, dim::size2di(Width, Height), Flags);
    
    //if (DxFont)
    //    DeviceContext_ = DxFont->GetDC();
    
    /* Create new font */
    Font* NewFont = new Font(DxFont, FontName, dim::size2di(Width, Height), getCharWidths(&FontObject));
    FontList_.push_back(NewFont);
    
    /* Delete device font object */
    DeleteObject(FontObject);
    
    return NewFont;
}


/*
 * ======= Matrix controll =======
 */

void Direct3D9RenderSystem::updateModelviewMatrix()
{
    D3DDevice_->SetTransform(D3DTS_VIEW, D3D_MATRIX(scene::spViewMatrix));
    D3DDevice_->SetTransform(D3DTS_WORLD, D3D_MATRIX(scene::spWorldMatrix));
}

void Direct3D9RenderSystem::setProjectionMatrix(const dim::matrix4f &Matrix)
{
    scene::spProjectionMatrix = Matrix;
    D3DDevice_->SetTransform(D3DTS_PROJECTION, D3D_MATRIX(Matrix));
}
void Direct3D9RenderSystem::setViewMatrix(const dim::matrix4f &Matrix)
{
    RenderSystem::setViewMatrix(Matrix);
    D3DDevice_->SetTransform(D3DTS_VIEW, D3D_MATRIX(Matrix));
}
void Direct3D9RenderSystem::setWorldMatrix(const dim::matrix4f &Matrix)
{
    scene::spWorldMatrix = Matrix;
    D3DDevice_->SetTransform(D3DTS_WORLD, D3D_MATRIX(Matrix));
}
void Direct3D9RenderSystem::setTextureMatrix(const dim::matrix4f &Matrix, u8 TextureLayer)
{
    scene::spTextureMatrix[TextureLayer] = Matrix;
    D3DDevice_->SetTransform(
        (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + TextureLayer), D3D_MATRIX(Matrix)//.getTextureMatrix())
    );
}
void Direct3D9RenderSystem::setColorMatrix(const dim::matrix4f &Matrix)
{
    scene::spColorMatrix = Matrix;
}


/*
 * ======= Private functions =======
 */

void Direct3D9RenderSystem::updatePrimitiveList(SPrimitiveVertex* pVerticesList, u32 Size)
{
    /* Fill the standard vertex buffer */
    VOID* pVoid;
    D3DDefVertexBuffer_->Lock(0, sizeof(SPrimitiveVertex)*Size, (void**)&pVoid, 0);
    memcpy(pVoid, pVerticesList, sizeof(SPrimitiveVertex)*Size);
    D3DDefVertexBuffer_->Unlock();
    
    /* Setup the FVF for 2D graphics */
    D3DDevice_->SetFVF(FVF_VERTEX2D);
    
    /* Set the stream souce */
    D3DDevice_->SetStreamSource(0, D3DDefVertexBuffer_, 0, sizeof(SPrimitiveVertex));
}

void Direct3D9RenderSystem::updatePrimitiveListFlexible(SPrimitiveVertex* pVerticesList, u32 Count)
{
    /* Delete the old vertex buffer */
    releaseObject(D3DDefFlexibleVertexBuffer_);
    
    /* Create a new vertex buffer */
    D3DDevice_->CreateVertexBuffer(
        sizeof(SPrimitiveVertex)*Count,
        0,
        FVF_VERTEX2D,
        D3DPOOL_DEFAULT,
        &D3DDefFlexibleVertexBuffer_,
        0
    );
    
    if (!D3DDefFlexibleVertexBuffer_)
    {
        io::Log::error("Could not create Direct3D9 vertex buffer");
        return;
    }
    
    /* Fill the standard vertex buffer */
    VOID* pVoid;
    D3DDefFlexibleVertexBuffer_->Lock(0, sizeof(SPrimitiveVertex)*Count, (void**)&pVoid, 0);
    memcpy(pVoid, pVerticesList, sizeof(SPrimitiveVertex)*Count);
    D3DDefFlexibleVertexBuffer_->Unlock();
    
    /* Setup the FVF for 2D graphics */
    D3DDevice_->SetFVF(FVF_VERTEX2D);
    
    /* Set the stream souce */
    D3DDevice_->SetStreamSource(0, D3DDefFlexibleVertexBuffer_, 0, sizeof(SPrimitiveVertex));
}

void Direct3D9RenderSystem::setupTextureFormats(
    const EPixelFormats Format, const EHWTextureFormats HWFormat, D3DFORMAT &D3DFormat, DWORD &Usage)
{
    if (Format >= PIXELFORMAT_ALPHA && Format <= PIXELFORMAT_DEPTH)
    {
        switch (HWFormat)
        {
            case HWTEXFORMAT_UBYTE8:
                D3DFormat = D3DTexInternalFormatListUByte8[Format]; break;
            case HWTEXFORMAT_FLOAT16:
                D3DFormat = D3DTexInternalFormatListFloat16[Format]; break;
            case HWTEXFORMAT_FLOAT32:
                D3DFormat = D3DTexInternalFormatListFloat32[Format]; break;
        }
    }
    
    switch (Format)
    {
        case PIXELFORMAT_DEPTH:
            Usage = D3DUSAGE_DEPTHSTENCIL; break;
        default:
            break;
    }
}

void Direct3D9RenderSystem::updateTextureAttributes(
    const ETextureDimensions Dimension, const ETextureFilters MagFilter, const ETextureFilters MinFilter,
    const ETextureMipMapFilters MipMapFilter, f32 MaxAnisotropy, bool MipMaps, const dim::vector3d<ETextureWrapModes> &WrapMode)
{
    /* Wrap modes (reapeat, mirror, clamp) */
    D3DDevice_->SetSamplerState(CurSamplerLevel_, D3DSAMP_ADDRESSU, D3DTextureWrapModes[WrapMode.X]);
    D3DDevice_->SetSamplerState(CurSamplerLevel_, D3DSAMP_ADDRESSV, D3DTextureWrapModes[WrapMode.Y]);
    D3DDevice_->SetSamplerState(CurSamplerLevel_, D3DSAMP_ADDRESSW, D3DTextureWrapModes[WrapMode.Z]);
    
    /* Anisotropy */
    D3DDevice_->SetSamplerState(CurSamplerLevel_, D3DSAMP_MAXANISOTROPY, (DWORD)MaxAnisotropy);
    
    /* Texture filter */
    if (MipMaps)
    {
        switch (MipMapFilter)
        {
            case FILTER_BILINEAR:
                D3DDevice_->SetSamplerState(CurSamplerLevel_, D3DSAMP_MIPFILTER, D3DTEXF_POINT); break;
            case FILTER_TRILINEAR:
                D3DDevice_->SetSamplerState(CurSamplerLevel_, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR); break;
            case FILTER_ANISOTROPIC:
                D3DDevice_->SetSamplerState(CurSamplerLevel_, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC); break;
        }
    }
    else
        D3DDevice_->SetSamplerState(CurSamplerLevel_, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    
    /* Magnification filter */
    D3DDevice_->SetSamplerState(
        CurSamplerLevel_, D3DSAMP_MAGFILTER, (MagFilter == FILTER_SMOOTH ? D3DTEXF_LINEAR : D3DTEXF_POINT)
    );
    
    /* Minification filter */
    D3DTEXTUREFILTERTYPE d3dFilter = D3DTEXF_NONE;
    
    if (MipMapFilter == FILTER_ANISOTROPIC)
        d3dFilter = D3DTEXF_ANISOTROPIC;
    else
        d3dFilter = (MinFilter == FILTER_SMOOTH ? D3DTEXF_LINEAR : D3DTEXF_POINT);
    
    D3DDevice_->SetSamplerState(CurSamplerLevel_, D3DSAMP_MINFILTER, d3dFilter);
}

bool Direct3D9RenderSystem::createRendererTexture(
    bool MipMaps, const ETextureDimensions Dimension, dim::vector3di Size, const EPixelFormats Format,
    const u8* ImageData, const EHWTextureFormats HWFormat, bool isRenderTarget)
{
    /* Direct3D9 texture format setup */
    D3DFORMAT d3dFormat = D3DFMT_A8R8G8B8;
    DWORD d3dUsage      = 0;
    HRESULT d3dError    = 0;
    D3DPOOL d3dPool     = D3DPOOL_MANAGED;
    
    setupTextureFormats(Format, HWFormat, d3dFormat, d3dUsage);
    
    CurD3DTexture_          = 0;
    CurD3DCubeTexture_      = 0;
    CurD3DVolumeTexture_    = 0;
    
    /* Check for render target */
    if (isRenderTarget)
    {
        d3dUsage |= D3DUSAGE_RENDERTARGET;
        d3dPool = D3DPOOL_DEFAULT;
    }
    
    /* Register a new Direct3D9 texture */
    switch (Dimension)
    {
        case TEXTURE_1D:
        {
            d3dError = D3DDevice_->CreateTexture(
                Size.X,
                1,
                MipMaps ? 0 : 1,
                d3dUsage | (MipMaps ? D3DUSAGE_AUTOGENMIPMAP : 0),
                d3dFormat,
                d3dPool,
                &CurD3DTexture_,
                0
            );
        }
        break;
        
        case TEXTURE_2D:
        {
            d3dError = D3DDevice_->CreateTexture(
                Size.X,
                Size.Y,
                MipMaps ? 0 : 1,
                d3dUsage | (MipMaps ? D3DUSAGE_AUTOGENMIPMAP : 0),
                d3dFormat,
                d3dPool,
                &CurD3DTexture_,
                0
            );
        }
        break;
        
        case TEXTURE_3D:
        {
            d3dError = D3DDevice_->CreateVolumeTexture(
                Size.X,
                Size.Y,
                Size.Z,
                MipMaps ? 0 : 1,
                d3dUsage | (MipMaps ? D3DUSAGE_AUTOGENMIPMAP : 0),
                d3dFormat,
                d3dPool,
                &CurD3DVolumeTexture_,
                0
            );
        }
        break;
        
        case TEXTURE_CUBEMAP:
        {
            d3dError = D3DDevice_->CreateCubeTexture(
                Size.X,
                MipMaps ? 0 : 1,
                d3dUsage | (MipMaps ? D3DUSAGE_AUTOGENMIPMAP : 0),
                d3dFormat,
                d3dPool,
                &CurD3DCubeTexture_,
                0
            );
        }
        break;
    }
    
    /* Check if an error has been detected */
    if (d3dError)
    {
        io::Log::error("Could not create Direct3D9 texture");
        return false;
    }
    
    return true;
}

bool Direct3D9RenderSystem::setRenderTargetSurface(const s32 Index, Texture* Target)
{
    if (!LastRenderTarget_ && !Index)
        D3DDevice_->GetRenderTarget(0, &LastRenderTarget_);
    
    IDirect3DSurface9* Surface = 0;
    HRESULT Error = 0;
    
    if (Target->getDimension() == TEXTURE_CUBEMAP)
    {
        IDirect3DCubeTexture9* d3dTexture = static_cast<Direct3D9Texture*>(Target)->D3DCubeTexture_;
        Error = d3dTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)Target->getCubeMapFace(), 0, &Surface);
    }
    else if (Target->getDimension() == TEXTURE_3D)
    {
        io::Log::error("Volume texture render targets are not supported for Direct3D9 yet");
        return false;
    }
    else
    {
        IDirect3DTexture9* d3dTexture = static_cast<Direct3D9Texture*>(Target)->D3D2DTexture_;
        Error = d3dTexture->GetSurfaceLevel(0, &Surface);
    }
    
    if (Error)
    {
        io::Log::error("Could not get first surface level");
        return false;
    }
    
    /* Set the render target */
    if (D3DDevice_->SetRenderTarget(Index, Surface) == D3DERR_INVALIDCALL)
    {
        io::Log::error("Could not set render target");
        return false;
    }
    
    return true;
}

void Direct3D9RenderSystem::bindTextureList(const std::vector<SMeshSurfaceTexture> &TextureList)
{
    s32 TextureLayer = 0;
    
    /* Loop for each texture */
    for (std::vector<SMeshSurfaceTexture>::const_iterator itTex = TextureList.begin();
         itTex != TextureList.end(); ++itTex, ++TextureLayer)
    {
        if (!itTex->TextureObject)
            continue;
        
        /* Bind the current texture */
        itTex->TextureObject->bind(TextureLayer);
        
        /* Load texture matrix */
        D3DDevice_->SetTransform(
            (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + TextureLayer), D3D_MATRIX(itTex->Matrix)
        );
        
        /* Texture coordinate generation */
        D3DDevice_->SetTextureStageState(
            TextureLayer,
            D3DTSS_TEXCOORDINDEX,
            itTex->TexMappingCoords != MAPGEN_NONE ? D3DMappingGenList[itTex->TexMappingGen] : TextureLayer
        );
        
        /* Texture stage states */
        D3DDevice_->SetTextureStageState(TextureLayer, D3DTSS_COLOROP, D3DTextureEnvList[itTex->TexEnvType]);
        D3DDevice_->SetTextureStageState(TextureLayer, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
    }
}

void Direct3D9RenderSystem::unbindTextureList(const std::vector<SMeshSurfaceTexture> &TextureList)
{
    s32 TextureLayer = 0;
    
    /* Unbind the textures */
    for (std::vector<SMeshSurfaceTexture>::const_iterator itTex = TextureList.begin();
         itTex != TextureList.end(); ++itTex, ++TextureLayer)
    {
        if (itTex->TextureObject)
            itTex->TextureObject->unbind(TextureLayer);
    }
}

void Direct3D9RenderSystem::releaseFontObject(Font* FontObj)
{
    if (FontObj && FontObj->getBufferRawData())
    {
        if (FontObj->getTexture())
        {
            //todo
        }
        else
        {
            /* Release the Direct3D9 font */
            ID3DXFont* DxFont = reinterpret_cast<ID3DXFont*>(FontObj->getBufferRawData());
            releaseObject(DxFont);
        }
    }
}

void Direct3D9RenderSystem::drawTexturedFont(
    const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color)
{
    /* Get vertex buffer and glyph list */
    D3D9VertexBuffer* VertexBuffer = reinterpret_cast<D3D9VertexBuffer*>(FontObj->getBufferRawData());
    
    const SFontGlyph* GlyphList = &(FontObj->getGlyphList()[0]);
    
    /* Setup render- and texture states */
    D3DDevice_->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    
    #if 0
    D3DMATERIAL9 D3DMat;
    ZeroMemory(&D3DMat, sizeof(D3DMat));
    D3DMat.Ambient = getD3DColor(Color);
    D3DDevice_->SetMaterial(&D3DMat);
    
    D3DDevice_->SetRenderState(D3DRS_COLORVERTEX, true);
    D3DDevice_->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_RGBA(Color.Red, Color.Green, Color.Blue, Color.Alpha));
    D3DDevice_->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
    #endif
    
    D3DDevice_->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    
    /* Setup vertex buffer source */
    D3DDevice_->SetFVF(FVF_VERTEX_FONT);
    D3DDevice_->SetStreamSource(0, VertexBuffer->HWBuffer_, 0, sizeof(dim::vector3df) + sizeof(dim::point2df));
    
    /* Bind texture */
    FontObj->getTexture()->bind(0);
    
    /* Initialize transformation */
    dim::matrix4f Transform;
    Transform.translate(dim::vector3df(static_cast<f32>(Position.X), static_cast<f32>(Position.Y), 0.0f));
    Transform *= FontTransform_;
    
    /* Draw each character */
    for (u32 i = 0, c = Text.size(); i < c; ++i)
    {
        /* Get character glyph from string */
        const u32 CurChar = static_cast<u32>(static_cast<u8>(Text[i]));
        const SFontGlyph* Glyph = &(GlyphList[CurChar]);
        
        /* Offset movement */
        Transform.translate(dim::vector3df(static_cast<f32>(Glyph->StartOffset), 0.0f, 0.0f));
        
        /* Draw current character with current transformation */
        D3DDevice_->SetTransform(D3DTS_WORLD, D3D_MATRIX(Transform));
        D3DDevice_->DrawPrimitive(D3DPT_TRIANGLESTRIP, CurChar*4, 2);
        
        /* Character width and white space movement */
        Transform.translate(dim::vector3df(static_cast<f32>(Glyph->DrawnWidth + Glyph->WhiteSpace), 0.0f, 0.0f));
    }
    
    /* Reset world matrix */
    D3DDevice_->SetTransform(D3DTS_WORLD, D3D_MATRIX(scene::spWorldMatrix));
    
    /* Unbind vertex buffer */
    D3DDevice_->SetStreamSource(0, 0, 0, 0);
    
    /* Unbind texture */
    FontObj->getTexture()->unbind(0);
}

void Direct3D9RenderSystem::drawBitmapFont(
    const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color)
{
    ID3DXFont* DxFont = reinterpret_cast<ID3DXFont*>(FontObj->getBufferRawData());
    
    if (!DxFont)
        return;
    
    /* Setup drawing area */
    RECT rc;
    rc.left     = Position.X;
    rc.top      = Position.Y;
    rc.right    = gSharedObjects.ScreenWidth;
    rc.bottom   = gSharedObjects.ScreenHeight;
    
    /* Draw bitmap text */
    DxFont->DrawText(
        0, Text.c_str(), Text.size(), &rc, DT_LEFT | DT_TOP | DT_SINGLELINE, Color.getSingle()
    );
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
