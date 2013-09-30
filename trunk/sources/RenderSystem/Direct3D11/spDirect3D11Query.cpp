/*
 * Direct3D11 query file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11Query.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


static const D3D11_QUERY D3D11QueryTypes[] =
{
    D3D11_QUERY_OCCLUSION, D3D11_QUERY_OCCLUSION_PREDICATE, D3D11_QUERY_SO_STATISTICS, D3D11_QUERY_TIMESTAMP
};

#define D3D_DEVICE          static_cast<Direct3D11RenderSystem*>(GlbRenderSys)->getDirect3DDevice()
#define D3D_DEVICE_CONTEXT  static_cast<Direct3D11RenderSystem*>(GlbRenderSys)->getD3DDeviceContext()

Direct3D11Query::Direct3D11Query(const EQueryTypes Type) :
    Query       (Type   ),
    D3DQuery_   (0      )
{
    /* Create D3D11 query object */
    D3D11_QUERY_DESC QueryDesc;
    {
        QueryDesc.Query     = D3D11QueryTypes[getType()];
        QueryDesc.MiscFlags = 0;
    }
    D3D_DEVICE->CreateQuery(&QueryDesc, &D3DQuery_);
}
Direct3D11Query::~Direct3D11Query()
{
    /* Release query object */
    if (D3DQuery_)
        D3DQuery_->Release();
}

void Direct3D11Query::begin()
{
    if (D3DQuery_)
        D3D_DEVICE_CONTEXT->Begin(D3DQuery_);
}

void Direct3D11Query::end()
{
    if (D3DQuery_)
        D3D_DEVICE_CONTEXT->End(D3DQuery_);
}

u64 Direct3D11Query::result() const
{
    if (D3DQuery_)
    {
        UINT64 Result = 0;
        
        while (D3D_DEVICE_CONTEXT->GetData(D3DQuery_, &Result, sizeof(UINT64), 0) != S_OK)
        {
            // Empty loop
        }
        
        return static_cast<u64>(Result);
    }
    return 0;
}

#undef D3D_DEVICE
#undef D3D_DEVICE_CONTEXT


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
