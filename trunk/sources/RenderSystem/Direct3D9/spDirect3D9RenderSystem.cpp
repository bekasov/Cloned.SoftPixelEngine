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


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern scene::SceneGraph* __spSceneManager;

namespace video
{


/*
 * ========== Internal members ==========
 */

const io::stringc d3dDllFileName = "d3dx9_" + io::stringc((s32)D3DX_SDK_VERSION) + ".dll";

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
    #if 1
    D3DFMT_P8, D3DFMT_D24S8, D3DFMT_L8, D3DFMT_L8, D3DFMT_L8, D3DFMT_L8, D3DFMT_A8, D3DFMT_L8, 
    D3DFMT_A8L8, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_D24X8, D3DFMT_A8R8G8B8, D3DFMT_A8R8G8B8
    #else
    D3DFMT_A8, D3DFMT_L8, D3DFMT_A8L8, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_D24X8
    #endif
};

const D3DFORMAT D3DTexInternalFormatListFloat16[] = {
    #if 1
    D3DFMT_P8, D3DFMT_D24S8, D3DFMT_L16, D3DFMT_R16F, D3DFMT_R16F, D3DFMT_R16F, D3DFMT_A8, D3DFMT_R16F, 
    D3DFMT_G16R16F, D3DFMT_A16B16G16R16F, D3DFMT_A16B16G16R16F, D3DFMT_D24X8, D3DFMT_A16B16G16R16F, D3DFMT_A16B16G16R16F
    #else
    D3DFMT_R16F, D3DFMT_R16F, D3DFMT_G16R16F, D3DFMT_A16B16G16R16F, D3DFMT_A16B16G16R16F, D3DFMT_A16B16G16R16F, D3DFMT_A16B16G16R16F, D3DFMT_D24X8
    #endif
};

const D3DFORMAT D3DTexInternalFormatListFloat32[] = {
    #if 1
    D3DFMT_P8, D3DFMT_D24S8, D3DFMT_L16, D3DFMT_R32F, D3DFMT_R32F, D3DFMT_R32F, D3DFMT_A8, D3DFMT_R32F, 
    D3DFMT_G32R32F, D3DFMT_A32B32G32R32F, D3DFMT_A32B32G32R32F, D3DFMT_D24X8, D3DFMT_A32B32G32R32F, D3DFMT_A32B32G32R32F
    #else
    D3DFMT_R32F, D3DFMT_R32F, D3DFMT_G32R32F, D3DFMT_A32B32G32R32F, D3DFMT_A32B32G32R32F, D3DFMT_A32B32G32R32F, D3DFMT_A32B32G32R32F, D3DFMT_D24X8
    #endif
};

extern s32 D3D9PixelFormatDataSize[];


/*
 * ========== Constructors & destructor ==========
 */

Direct3D9RenderSystem::Direct3D9RenderSystem()
    : RenderSystem(RENDERER_DIRECT3D9), D3DInstance_(0), D3DDevice_(0), LastRenderTarget_(0),
    CurD3DTexture_(0), CurD3DCubeTexture_(0), CurD3DVolumeTexture_(0)
{
    /* Initialization */
    init();
}
Direct3D9RenderSystem::~Direct3D9RenderSystem()
{
    /* Release all Direct3D9 device contexts */
    clear();
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
        &pDirect3DVertexBuffer_,
        0
    );
    
    if (!pDirect3DVertexBuffer_)
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
        &pDirect3DFlexibleVertexBuffer_,
        0
    );
    
    if (!pDirect3DFlexibleVertexBuffer_)
    {
        io::Log::error("Could not create Direct3D9 vertex buffer");
        return;
    }
    
    /* Default settings */
    D3DDevice_->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    D3DDevice_->SetRenderState(D3DRS_ALPHATESTENABLE, true);
    D3DDevice_->SetRenderState(D3DRS_SPECULARENABLE, true);
    D3DDevice_->SetRenderState(D3DRS_NORMALIZENORMALS, true);
    
    /* Default camera range */
    RangeNear_  = 1.0f;
    RangeFar_   = 1000.0f;
    
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
            return DevCaps_.StencilCaps;
        case QUERY_RENDERTARGET:
        case QUERY_MULTISAMPLE_RENDERTARGET:
            return true;
            
        case QUERY_BILINEAR_FILTER:
            return (DevCaps_.TextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT);
        case QUERY_TRILINEAR_FILTER:
            return (DevCaps_.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR);
        case QUERY_ANISOTROPY_FILTER:
            return (DevCaps_.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC);
        case QUERY_MIPMAPS:
            return (DevCaps_.TextureCaps & D3DPTEXTURECAPS_MIPMAP);
        case QUERY_VOLUMETRIC_TEXTURE:
            return (DevCaps_.TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP);
            
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
    ClearColorMask_ = video::emptycolor;
    
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

void Direct3D9RenderSystem::setVsync(bool isVsync)
{
    //todo
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
    
    /* Temporary variables */
    D3DMATERIAL9 d3dMat;
    
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
        D3DDevice_->SetRenderState(D3DRS_LIGHTING, true);
        
        /* Shininess */
        d3dMat.Power = Material->getShininessFactor();
        
        /* Diffuse, ambient, specular and emissive color */
        d3dMat.Diffuse = getD3DColor(Material->getDiffuseColor());
        d3dMat.Ambient = getD3DColor(Material->getAmbientColor());
        d3dMat.Specular = getD3DColor(Material->getSpecularColor());
        d3dMat.Emissive = getD3DColor(Material->getEmissionColor());
        
        /* Set the material */
        D3DDevice_->SetMaterial(&d3dMat);
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
    D3DDevice_->GetLight(LightID, &CurLight_);
    
    switch (LightType)
    {
        case scene::LIGHT_DIRECTIONAL:
            CurLight_.Type      = D3DLIGHT_DIRECTIONAL;
            CurLight_.Direction = *D3D_VECTOR(scene::spWorldMatrix.getRotationMatrix() * Direction);
            break;
        case scene::LIGHT_POINT:
            CurLight_.Type      = D3DLIGHT_POINT;
            CurLight_.Direction = *D3D_VECTOR(scene::spWorldMatrix.getRotationMatrix() * -Direction);
            break;
        case scene::LIGHT_SPOT:
            CurLight_.Type      = D3DLIGHT_SPOT;
            break;
    }
    
    /* Update the direction */
    //Direction = scene::spWorldMatrix.getRotationMatrix() * Direction;
    
    /* Lighting location */
    CurLight_.Position  = *D3D_VECTOR(scene::spWorldMatrix.getPosition());
    
    /* Spot light attributes */
    CurLight_.Theta = SpotInnerConeAngle * 2.0f * math::RAD;
    CurLight_.Phi   = SpotOuterConeAngle * 2.0f * math::RAD;
    
    /* Volumetric light attenuations */
    if (isVolumetric)
    {
        CurLight_.Attenuation0  = AttenuationConstant;
        CurLight_.Attenuation1  = AttenuationLinear;
        CurLight_.Attenuation2  = AttenuationQuadratic;
    }
    else
    {
        CurLight_.Attenuation0  = 1.0;
        CurLight_.Attenuation1  = 0.0;
        CurLight_.Attenuation2  = 0.0;
    }
    
    /* Set the light source */
    D3DDevice_->SetLight(LightID, &CurLight_);
}


/* === Hardware mesh buffers === */

void Direct3D9RenderSystem::createVertexBuffer(void* &BufferID)
{
    BufferID = new SVertexBuffer();
}
void Direct3D9RenderSystem::createIndexBuffer(void* &BufferID)
{
    BufferID = new SIndexBuffer();
}

void Direct3D9RenderSystem::deleteVertexBuffer(void* &BufferID)
{
    if (BufferID)
    {
        SVertexBuffer* Buffer = static_cast<SVertexBuffer*>(BufferID);
        
        if (Buffer->HWVertexBuffer)
            Buffer->HWVertexBuffer->Release();
        
        delete Buffer;
        
        BufferID = 0;
    }
}
void Direct3D9RenderSystem::deleteIndexBuffer(void* &BufferID)
{
    if (BufferID)
    {
        SIndexBuffer* Buffer = static_cast<SIndexBuffer*>(BufferID);
        
        if (Buffer->HWIndexBuffer)
            Buffer->HWIndexBuffer->Release();
        
        delete Buffer;
        
        BufferID = 0;
    }
}

void Direct3D9RenderSystem::updateVertexBuffer(
    void* BufferID, const dim::UniversalBuffer &BufferData, const VertexFormat* Format, const EMeshBufferUsage Usage)
{
    if (!BufferID || !Format)
        return;
    
    /* Setup format flags */
    s32 FormatFlags = 0;
    
    if (Format->getFlags() & VERTEXFORMAT_COORD)
        FormatFlags |= D3DFVF_XYZ;
    if (Format->getFlags() & VERTEXFORMAT_NORMAL)
        FormatFlags |= D3DFVF_NORMAL;
    if (Format->getFlags() & VERTEXFORMAT_COLOR)
        FormatFlags |= D3DFVF_DIFFUSE;
    
    FormatFlags |= (D3DFVF_TEX1 * Format->getTexCoords().size());
    
    if (Format->getFlags() & VERTEXFORMAT_TEXCOORDS)
    {
        for (s32 i = 0; i < Format->getTexCoords().size(); ++i)
        {
            switch (Format->getTexCoords()[i].Size)
            {
                case 1: FormatFlags |= D3DFVF_TEXCOORDSIZE1(i); break;
                case 2: FormatFlags |= D3DFVF_TEXCOORDSIZE2(i); break;
                case 3: FormatFlags |= D3DFVF_TEXCOORDSIZE3(i); break;
                case 4: FormatFlags |= D3DFVF_TEXCOORDSIZE4(i); break;
            }
        }
    }
    
    /* Temporary variables */
    SVertexBuffer* Buffer   = static_cast<SVertexBuffer*>(BufferID);
    
    const u32 VertexCount   = BufferData.getCount();
    const u32 BufferSize    = BufferData.getSize();
    
    if (!Buffer->HWVertexBuffer || VertexCount != Buffer->VertexCount || BufferSize != Buffer->BufferSize || FormatFlags != Buffer->FormatFlags)
    {
        /* Release old hardware vertex buffer */
        releaseObject(Buffer->HWVertexBuffer);
        
        Buffer->VertexCount = VertexCount;
        Buffer->FormatFlags = FormatFlags;
        Buffer->BufferSize  = BufferSize;
        
        /* Create hardware vertex buffer */
        HRESULT Result = D3DDevice_->CreateVertexBuffer(
            BufferSize,
            D3DUSAGE_WRITEONLY | (Usage == MESHBUFFER_DYNAMIC ? D3DUSAGE_DYNAMIC : 0),
            Buffer->FormatFlags,
            D3DPOOL_DEFAULT,
            &Buffer->HWVertexBuffer,
            0
        );
        
        if (Result != D3D_OK || !Buffer->HWVertexBuffer)
        {
            io::Log::error("Could not create hardware vertex buffer");
            return;
        }
    }
    
    if (VertexCount)
    {
        /* Update hardware vertex buffer */
        void* LockBuffer = 0;
        
        if (Buffer->HWVertexBuffer->Lock(0, 0, &LockBuffer, 0) == D3D_OK)
        {
            memcpy(LockBuffer, BufferData.getArray(), BufferSize);
            Buffer->HWVertexBuffer->Unlock();
        }
        else
            io::Log::error("Could not update hardware vertex buffer");
    }
}

void Direct3D9RenderSystem::updateIndexBuffer(
    void* BufferID, const dim::UniversalBuffer &BufferData, const IndexFormat* Format, const EMeshBufferUsage Usage)
{
    if (!BufferID || !Format)
        return;
    
    /* Setup format flags */
    D3DFORMAT FormatFlags = D3DFMT_INDEX16;
    
    if (Format->getDataType() == DATATYPE_UNSIGNED_INT)
        FormatFlags = D3DFMT_INDEX32;
    
    /* Temporary variables */
    SIndexBuffer* Buffer   = static_cast<SIndexBuffer*>(BufferID);
    
    const u32 IndexCount    = BufferData.getCount();
    const u32 BufferSize    = BufferData.getSize();
    
    if (!Buffer->HWIndexBuffer || IndexCount != Buffer->IndexCount || BufferSize != Buffer->BufferSize || FormatFlags != Buffer->FormatFlags)
    {
        /* Release old hardware index buffer */
        releaseObject(Buffer->HWIndexBuffer);
        
        Buffer->IndexCount  = IndexCount;
        Buffer->FormatFlags = FormatFlags;
        Buffer->BufferSize  = BufferSize;
        
        /* Create hardware index buffer */
        HRESULT Result = D3DDevice_->CreateIndexBuffer(
            BufferSize,
            D3DUSAGE_WRITEONLY | (Usage == MESHBUFFER_DYNAMIC ? D3DUSAGE_DYNAMIC : 0),
            Buffer->FormatFlags,
            D3DPOOL_DEFAULT,
            &Buffer->HWIndexBuffer,
            0
        );
        
        if (Result != D3D_OK || !Buffer->HWIndexBuffer)
        {
            io::Log::error("Could not create hardware index buffer");
            return;
        }
    }
    
    if (IndexCount)
    {
        /* Update hardware index buffer */
        void* LockBuffer = 0;
        
        if (Buffer->HWIndexBuffer->Lock(0, 0, &LockBuffer, 0) == D3D_OK)
        {
            memcpy(LockBuffer, BufferData.getArray(), BufferSize);
            Buffer->HWIndexBuffer->Unlock();
        }
        else
            io::Log::error("Could not update hardware index buffer");
    }
}

void Direct3D9RenderSystem::updateVertexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index)
{
    if (!BufferID || !BufferData.getSize())
        return;
    
    /* Temporary variables */
    SVertexBuffer* Buffer = static_cast<SVertexBuffer*>(BufferID);
    
    void* LockBuffer = 0;
    const u32 BufferStride = BufferData.getStride();
    
    /* Update hardware vertex buffer element */
    if (Buffer->HWVertexBuffer->Lock(Index * BufferStride, BufferStride, &LockBuffer, 0) == D3D_OK)
    {
        memcpy(LockBuffer, BufferData.getArray(Index, 0), BufferStride);
        Buffer->HWVertexBuffer->Unlock();
    }
    else
        io::Log::error("Could not updater hardware vertex buffer element");
}

void Direct3D9RenderSystem::updateIndexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index)
{
    if (!BufferID || !BufferData.getSize())
        return;
    
    /* Temporary variables */
    SIndexBuffer* Buffer = static_cast<SIndexBuffer*>(BufferID);
    
    void* LockBuffer = 0;
    const u32 BufferStride = BufferData.getStride();
    
    /* Update hardware index buffer element */
    if (Buffer->HWIndexBuffer->Lock(Index * BufferStride, BufferStride, &LockBuffer, 0) == D3D_OK)
    {
        memcpy(LockBuffer, BufferData.getArray(Index, 0), BufferStride);
        Buffer->HWIndexBuffer->Unlock();
    }
    else
        io::Log::error("Could not updater hardware index buffer element");
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
    if (CurShaderTable_ && ShaderSurfaceCallback_)
        ShaderSurfaceCallback_(CurShaderTable_, &MeshBuffer->getSurfaceTextureList());
    
    /* Get hardware vertex- and index buffers */
    SVertexBuffer* VertexBuffer = static_cast<SVertexBuffer*>(MeshBuffer->getVertexBufferID());
    SIndexBuffer* IndexBuffer   = static_cast<SIndexBuffer*>(MeshBuffer->getIndexBufferID());
    
    /* Bind textures */
    if (__isTexturing)
        bindTextureList(OrigMeshBuffer->getSurfaceTextureList());
    
    /* Setup vertex format */
    D3DDevice_->SetFVF(VertexBuffer->FormatFlags);
    
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
    if (VertexBuffer->HWVertexBuffer)
    {
        /* Bind hardware mesh buffer */
        D3DDevice_->SetStreamSource(
            0, VertexBuffer->HWVertexBuffer,
            0, MeshBuffer->getVertexFormat()->getFormatSize()
        );
        
        /* Draw the primitives */
        if (MeshBuffer->getIndexBufferEnable() && IndexBuffer)
        {
            D3DDevice_->SetIndices(IndexBuffer->HWIndexBuffer);
            
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
                IndexBuffer->FormatFlags,
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
            CurLight_.Type = D3DLIGHT_DIRECTIONAL; break;
        case scene::LIGHT_POINT:
            CurLight_.Type = D3DLIGHT_POINT; break;
        case scene::LIGHT_SPOT:
            CurLight_.Type = D3DLIGHT_SPOT; break;
    }
    
    /* Default values */
    CurLight_.Range         = 1000.0f;
    CurLight_.Falloff       = 1.0f;
    CurLight_.Direction.z   = 1.0f;
    
    /* Lighting colors */
    CurLight_.Diffuse   = getD3DColor(Diffuse);
    CurLight_.Ambient   = getD3DColor(Ambient);
    CurLight_.Specular  = getD3DColor(Specular);
    
    /* Volumetric light attenuations */
    CurLight_.Attenuation0  = AttenuationConstant;
    CurLight_.Attenuation1  = AttenuationLinear;
    CurLight_.Attenuation2  = AttenuationQuadratic;
    
    /* Set the light attributes */
    D3DDevice_->SetLight(LightID, &CurLight_);
    
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
    D3DDevice_->GetLight(LightID, &CurLight_);
    
    /* Lighting colors */
    CurLight_.Diffuse   = getD3DColor(Diffuse);
    CurLight_.Ambient   = getD3DColor(Ambient);
    CurLight_.Specular  = getD3DColor(Specular);
    
    /* Set the light attributes */
    D3DDevice_->SetLight(LightID, &CurLight_);
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
    Fog_.Range  = Range;
    Fog_.Near   = NearPlane;
    Fog_.Far    = FarPlane;
    Fog_.Mode   = Mode;
    
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

ShaderTable* Direct3D9RenderSystem::createShaderTable(VertexFormat* VertexInputLayout)
{
    ShaderTable* NewShaderTable = new Direct3D9ShaderTable();
    
    ShaderTableList_.push_back(NewShaderTable);
    
    return NewShaderTable;
}

Shader* Direct3D9RenderSystem::createShader(
    ShaderTable* ShaderTableObj, const EShaderTypes Type, const EShaderVersions Version,
    const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint)
{
    Shader* NewShader = new Direct3D9Shader(ShaderTableObj, Type, Version);
    
    NewShader->compile(ShaderBuffer, EntryPoint);
    
    if (!ShaderTableObj)
        NewShader->getShaderTable()->link();
    
    ShaderList_.push_back(NewShader);
    
    return NewShader;
}

Shader* Direct3D9RenderSystem::createCgShader(
    ShaderTable* ShaderTableObj, const EShaderTypes Type, const EShaderVersions Version,
    const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint)
{
    Shader* NewShader = 0;
    
    #ifndef SP_COMPILE_WITH_CG
    io::Log::error("This engine was not compiled with the Cg toolkit");
    #else
    if (RenderQuery_[RENDERQUERY_SHADER])
        NewShader = new CgShaderProgramD3D9(ShaderTableObj, Type, Version);
    else
    #endif
        NewShader = new Shader(ShaderTableObj, Type, Version);
    
    NewShader->compile(ShaderBuffer, EntryPoint);
    
    if (!ShaderTableObj)
        NewShader->getShaderTable()->link();
    
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
    setProjectionMatrix(IdentityMatrix);
    setViewMatrix(IdentityMatrix);
    setWorldMatrix(IdentityMatrix);
    
    #ifdef __DRAW2DXYZ__
    Matrix2D_.make2Dimensional(
        gSharedObjects.ScreenWidth,
        -gSharedObjects.ScreenHeight,
        gSharedObjects.ScreenWidth,
        gSharedObjects.ScreenHeight
    );
    setProjectionMatrix(Matrix2D_);
    #endif
    
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
        
        std::vector<Texture*> MRTexList = Target->getMultiRenderTargets();
        
        for (s32 i = 0; i < MRTexList.size(); ++i)
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
        
        const s32 RTCount = RenderTarget_->getMultiRenderTargets().size() + 1;
        
        for (s32 i = 1; i < RTCount && i < DevCaps_.NumSimultaneousRTs; ++i)
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
    Texture* Tex, const dim::point2di &Position, const color &Color)
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
    SPrimitiveVertex VerticesList[4] = {
        SPrimitiveVertex( (f32)Position.X, (f32)Position.Y, 0.0f, Clr, 0.0f, 0.0f ),
        SPrimitiveVertex( (f32)Position.X + Width, (f32)Position.Y, 0.0f, Clr, 1.0f, 0.0f ),
        SPrimitiveVertex( (f32)Position.X + Width, (f32)Position.Y + Height, 0.0f, Clr, 1.0f, 1.0f ),
        SPrimitiveVertex( (f32)Position.X, (f32)Position.Y + Height, 0.0f, Clr, 0.0f, 1.0f )
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
    Texture* Tex, const dim::rect2di &Position, const dim::rect2df &Clipping, const color &Color)
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
    Texture* Tex, const dim::point2di &Position, f32 Rotation, f32 Radius, const color &Color)
{
    /* Set the texture attributes */
    D3DDevice_->SetTextureStageState(0, isImageBlending_ ? D3DTSS_ALPHAOP : D3DTSS_COLOROP, D3DTOP_MODULATE);
    
    /* Bind the texture */
    Tex->bind();
    
    /* Temporary variables */
    u32 Clr = Color.getSingle();
    
    /* Set the vertex data */
    Radius *= math::SQRT2;
    SPrimitiveVertex VerticesList[4] = {
        SPrimitiveVertex( (f32)Position.X + SIN(Rotation -  45)*Radius, (f32)Position.Y - COS(Rotation -  45)*Radius, 0.0f, Clr, 0.0f, 0.0f ),
        SPrimitiveVertex( (f32)Position.X + SIN(Rotation +  45)*Radius, (f32)Position.Y - COS(Rotation +  45)*Radius, 0.0f, Clr, 1.0f, 0.0f ),
        SPrimitiveVertex( (f32)Position.X + SIN(Rotation + 135)*Radius, (f32)Position.Y - COS(Rotation + 135)*Radius, 0.0f, Clr, 1.0f, 1.0f ),
        SPrimitiveVertex( (f32)Position.X + SIN(Rotation - 135)*Radius, (f32)Position.Y - COS(Rotation - 135)*Radius, 0.0f, Clr, 0.0f, 1.0f )
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
    Texture* Tex,
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

Texture* Direct3D9RenderSystem::loadTexture(ImageLoader* Loader)
{
    if (!Loader)
        return RenderSystem::createTexture(DEF_TEXTURE_SIZE);
    
    /* Load image data */
    SImageDataRead* ImageData = Loader->loadImageData();
    
    Texture* NewTexture = 0;
    
    if (ImageData)
    {
        /* Direct3D9 texture configurations */
        createRendererTexture(
            TexGenMipMapping_, TEXTURE_2D, dim::vector3di(ImageData->Width, ImageData->Height, 1),
            ImageData->Format, ImageData->ImageBuffer
        );
        
        /* Setup texture creation flags */
        STextureCreationFlags CreationFlags(TexGenFlags_);
        
        CreationFlags.Filename      = Loader->getFilename();
        CreationFlags.Size          = dim::size2di(ImageData->Width, ImageData->Height);
        CreationFlags.ImageBuffer   = ImageData->ImageBuffer;
        CreationFlags.Format        = ImageData->Format;
        
        /* Create the engine texture */
        NewTexture = new Direct3D9Texture(
            CurD3DTexture_, CurD3DCubeTexture_, CurD3DVolumeTexture_, CreationFlags
        );
        
        NewTexture->setAnisotropicSamples(TexGenAnisotropy_);
        
        if (!NewTexture->isSizePOT() && NewTexture->getFormatSize() != 4)
            NewTexture->setFormat(PIXELFORMAT_RGBA);
        
        /* Delete image data */
        MemoryManager::deleteMemory(ImageData);
    }
    else
    {
        /* Create an empty texture */
        NewTexture = RenderSystem::createTexture(DEF_TEXTURE_SIZE);
        
        io::Log::lowerTab();
        return NewTexture;
    }
    
    /* Add texture to the list */
    TextureList_.push_back(NewTexture);
    
    return NewTexture;
}

Texture* Direct3D9RenderSystem::copyTexture(const Texture* Tex)
{
    if (!Tex)
        return RenderSystem::createTexture(DEF_TEXTURE_SIZE);
    
    /* Temporary varibales */
    const dim::size2di Size = Tex->getSize();
    Texture* NewTexture     = 0;
    
    /* Direct3D9 texture configurations */
    if (createRendererTexture(
        TexGenMipMapping_, Tex->getDimension(), dim::vector3di(Size.Width, Size.Height, Tex->getDepth()),
        Tex->getFormat(), Tex->getImageBuffer()))
    {
        /* Setup texture creation flags */
        STextureCreationFlags CreationFlags;
        
        CreationFlags.Filename      = Tex->getFilename();
        CreationFlags.Size          = Size;
        CreationFlags.ImageBuffer   = Tex->getImageBuffer();
        CreationFlags.MagFilter     = Tex->getMagFilter();
        CreationFlags.MinFilter     = Tex->getMinFilter();
        CreationFlags.MipMapFilter  = Tex->getMipMapFilter();
        CreationFlags.Format        = Tex->getFormat();
        CreationFlags.MipMaps       = Tex->getMipMapping();
        CreationFlags.WrapMode      = Tex->getWrapMode();
        
        /* Allocate the new texture */
        NewTexture = new Direct3D9Texture(
            CurD3DTexture_, CurD3DCubeTexture_, CurD3DVolumeTexture_, CreationFlags
        );
    }
    else
    {
        /* Error message */
        io::Log::error("Could not create Direct3D9 texture");
        
        /* Allocate an empty texture */
        NewTexture = new Direct3D9Texture();
    }
    
    /* Add the texture to the texture list */
    TextureList_.push_back(NewTexture);
    
    /* Return the texture & exit the function */
    return NewTexture;
}

Texture* Direct3D9RenderSystem::createTexture(const STextureCreationFlags &CreationFlags)
{
    /* Temporary variables */
    Texture* LoadedTexture = 0;
    
    /* Direct3D9 texture configurations */
    if (createRendererTexture(TexGenMipMapping_, TEXTURE_2D, CreationFlags.getSizeVector(), CreationFlags.Format, 0))
    {
        LoadedTexture = new Direct3D9Texture(
            CurD3DTexture_, CurD3DCubeTexture_, CurD3DVolumeTexture_, CreationFlags
        );
    }
    else
        LoadedTexture = new Direct3D9Texture();
    
    /* Add the texture to the texture list */
    TextureList_.push_back(LoadedTexture);
    
    /* Return the texture & exit the function */
    return LoadedTexture;
}

Texture* Direct3D9RenderSystem::createScreenShot(const dim::point2di &Position, dim::size2di Size)
{
    Texture* NewTexture = RenderSystem::createTexture(Size);
    
    createScreenShot(NewTexture, Position);
    
    #if 0
    // !TODO!
    
    /* Allocate the new texture */
    Texture* NewTexture = new Direct3D9Texture();
    
    /* Add the texture to the texture list */
    TextureList_.push_back(NewTexture);
    #endif
    
    /* Return the texture & exit the function */
    return NewTexture;
}

void Direct3D9RenderSystem::createScreenShot(Texture* Tex, const dim::point2di &Position)
{
    /* Get the Direct3D texture handle */
    IDirect3DTexture9* d3dTex = static_cast<Direct3D9Texture*>(Tex)->pDirect3DTexture_;
    
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

Font* Direct3D9RenderSystem::loadFont(const io::stringc &FontName, dim::size2di FontSize, s32 Flags)
{
    /* Temporary variables */
    HRESULT Result;
    HFONT FontObject;
    
    const bool isBold       = (Flags & FONT_BOLD);
    const bool isItalic     = (Flags & FONT_ITALIC);
    const bool isUnderlined = (Flags & FONT_UNDERLINED);
    const bool isStrikeout  = (Flags & FONT_STRIKEOUT);
    const bool isSymbols    = (Flags & FONT_SYMBOLS);
    
    if (!FontSize.Height)
        FontSize.Height = DEF_FONT_SIZE;
    
    /* Create the Direct3D font */
    
    #if D3DX_SDK_VERSION < 24
    
    #ifdef _MSC_VER
    #pragma comment (lib, "d3dx9.lib")
    #endif
    
    Result = D3DXCreateFont(
        D3DDevice_, FontSize.Height, FontSize.Width,
        isBold ? FW_BOLD : 0, 0, isItalic,
        isSymbolUsing ? SYMBOL_CHARSET : ANSI_CHARSET,
        OUT_TT_ONLY_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH,
        FontName.c_str(), &CurFont_
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
            D3DDevice_, FontSize.Height, FontSize.Width,
            isBold ? FW_BOLD : FW_NORMAL, 0, isItalic,
            isSymbols ? SYMBOL_CHARSET : ANSI_CHARSET,
            OUT_TT_ONLY_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH,
            FontName.toUnicode().c_str(), &CurFont_
        );
    }
    else if (pFncCreateFontA)
    {
        Result = pFncCreateFontA(
            D3DDevice_, FontSize.Height, FontSize.Width,
            isBold ? FW_BOLD : FW_NORMAL, 0, isItalic,
            isSymbols ? SYMBOL_CHARSET : ANSI_CHARSET,
            OUT_TT_ONLY_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH,
            FontName.c_str(), &CurFont_
        );
    }
    
    #endif
    
    /* Check for errors */
    if (Result != S_OK)
        io::Log::error("Could not load font: \"" + FontName + "\"");
    
    /* Create device font */
    createDeviceFont(
        &FontObject, FontName, FontSize, isBold,
        isItalic, isUnderlined, isStrikeout, isSymbols
    );
    
    if (CurFont_)
        DeviceContext_ = CurFont_->GetDC();
    
    /* Create new font */
    Font* NewFont = new Font(CurFont_, FontName, FontSize, getCharWidths(&FontObject));
    FontList_.push_back(NewFont);
    
    /* Delete device font object */
    DeleteObject(FontObject);
    
    return NewFont;
}

void Direct3D9RenderSystem::deleteFont(Font* FontObject)
{
    if (FontObject)
    {
        /* Release the Direct3D9 font */
        CurFont_ = (ID3DXFont*)FontObject->getID();
        releaseObject(CurFont_);
        
        RenderSystem::deleteFont(FontObject);
    }
}

void Direct3D9RenderSystem::draw2DText(
    Font* FontObject, const dim::point2di &Position, const io::stringc &Text, const color &Color)
{
    if (!FontObject)
        return;
    
    CurFont_ = (ID3DXFont*)FontObject->getID();
    
    if (!CurFont_)
        return;
    
    const dim::size2di FontSize(FontObject->getSize());
    
    if (Position.X > gSharedObjects.ScreenWidth || Position.Y > gSharedObjects.ScreenHeight || Position.Y < -FontSize.Height)
        return;
    
    /* Temporary variabels */
    RECT rc;
    rc.left     = Position.X;
    rc.top      = Position.Y;
    rc.right    = gSharedObjects.ScreenWidth;
    rc.bottom   = gSharedObjects.ScreenHeight;
    
    /* Draw the text */
    CurFont_->DrawText(
        0, Text.c_str(), Text.size(), &rc, DT_LEFT | DT_TOP | DT_SINGLELINE, Color.getSingle()
    );
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

void Direct3D9RenderSystem::init()
{
    /* Create the Direct3D renderer */
    D3DInstance_ = Direct3DCreate9(D3D_SDK_VERSION);
    
    if (!D3DInstance_)
    {
        io::Log::error("Could not create Direct3D9 interface");
        return;
    }
    
    createDefaultVertexFormats();
    
    /* General settings */
    CurSamplerLevel_    = 0;
    ClearColor_         = video::emptycolor;
    ClearColorMask_     = video::color(1, 1, 1, 1);
    isImageBlending_    = true;
}
void Direct3D9RenderSystem::clear()
{
    /* Release all Direct3D9 fonts */
    for (std::list<Font*>::iterator it = FontList_.begin(); it != FontList_.end(); ++it)
    {
        /* Release the Direct3D9 font */
        CurFont_ = (ID3DXFont*)(*it)->getID();
        releaseObject(CurFont_);
    }
    
    /* Close and release the standard- & flexible vertex buffer */
    releaseObject(pDirect3DVertexBuffer_);
    releaseObject(pDirect3DFlexibleVertexBuffer_);
    
    /* Close and release Direct3D */
    releaseObject(D3DInstance_);
}

void Direct3D9RenderSystem::updatePrimitiveList(SPrimitiveVertex* pVerticesList, u32 Size)
{
    /* Fill the standard vertex buffer */
    VOID* pVoid;
    pDirect3DVertexBuffer_->Lock(0, sizeof(SPrimitiveVertex)*Size, (void**)&pVoid, 0);
    memcpy(pVoid, pVerticesList, sizeof(SPrimitiveVertex)*Size);
    pDirect3DVertexBuffer_->Unlock();
    
    /* Setup the FVF for 2D graphics */
    D3DDevice_->SetFVF(FVF_VERTEX2D);
    
    /* Set the stream souce */
    D3DDevice_->SetStreamSource(0, pDirect3DVertexBuffer_, 0, sizeof(SPrimitiveVertex));
}

void Direct3D9RenderSystem::updatePrimitiveListFlexible(SPrimitiveVertex* pVerticesList, u32 Count)
{
    /* Delete the old vertex buffer */
    releaseObject(pDirect3DFlexibleVertexBuffer_);
    
    /* Create a new vertex buffer */
    D3DDevice_->CreateVertexBuffer(
        sizeof(SPrimitiveVertex)*Count,
        0,
        FVF_VERTEX2D,
        D3DPOOL_DEFAULT,
        &pDirect3DFlexibleVertexBuffer_,
        0
    );
    
    if (!pDirect3DFlexibleVertexBuffer_)
    {
        io::Log::error("Could not create Direct3D9 vertex buffer");
        return;
    }
    
    /* Fill the standard vertex buffer */
    VOID* pVoid;
    pDirect3DFlexibleVertexBuffer_->Lock(0, sizeof(SPrimitiveVertex)*Count, (void**)&pVoid, 0);
    memcpy(pVoid, pVerticesList, sizeof(SPrimitiveVertex)*Count);
    pDirect3DFlexibleVertexBuffer_->Unlock();
    
    /* Setup the FVF for 2D graphics */
    D3DDevice_->SetFVF(FVF_VERTEX2D);
    
    /* Set the stream souce */
    D3DDevice_->SetStreamSource(0, pDirect3DFlexibleVertexBuffer_, 0, sizeof(SPrimitiveVertex));
}

void Direct3D9RenderSystem::setupTextureFormats(
    const EPixelFormats Format, const EHWTextureFormats HWFormat, D3DFORMAT &D3DFormat, DWORD &Usage)
{
    if (Format >= PIXELFORMAT_INDEX && Format <= PIXELFORMAT_BGRA)
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
        case PIXELFORMAT_STENCIL:
            Usage = D3DUSAGE_DEPTHSTENCIL; break;
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
    
    if (Size.Z > 1)
        Size.Y /= Size.Z;
    
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
        IDirect3DCubeTexture9* d3dTexture = static_cast<Direct3D9Texture*>(Target)->pDirect3DCubeTexture_;
        Error = d3dTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)Target->getCubeMapFace(), 0, &Surface);
    }
    else if (Target->getDimension() == TEXTURE_3D)
    {
        io::Log::error("Volume texture render targets are not supported for Direct3D9 yet");
        return false;
    }
    else
    {
        IDirect3DTexture9* d3dTexture = static_cast<Direct3D9Texture*>(Target)->pDirect3DTexture_;
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


/*
 * SMeshBufferData structure
 */

Direct3D9RenderSystem::SMeshBufferData::SMeshBufferData()
    : pBufferID(0), pVerticesList(0), VerticesCount(0), pVertexBuffer(0), IndicesCount(0), pIndexBuffer(0)
{
}
Direct3D9RenderSystem::SMeshBufferData::~SMeshBufferData()
{
    if (pVertexBuffer)
        pVertexBuffer->Release();
    if (pIndexBuffer)
        pIndexBuffer->Release();
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
