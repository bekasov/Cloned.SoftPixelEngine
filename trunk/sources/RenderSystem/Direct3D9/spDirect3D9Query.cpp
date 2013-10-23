/*
 * Direct3D9 query file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D9/spDirect3D9Query.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "RenderSystem/Direct3D9/spDirect3D9RenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


static const D3DQUERYTYPE D3D9QueryTypes[] =
{
    D3DQUERYTYPE_OCCLUSION, D3DQUERYTYPE_OCCLUSION, D3DQUERYTYPE_VERTEXSTATS , D3DQUERYTYPE_TIMESTAMP
};

Direct3D9Query::Direct3D9Query(const EQueryTypes Type) :
    Query       (Type   ),
    D3DQuery_   (0      ),
    Protocol_   (0      )
{
    createHWQuery();
}
Direct3D9Query::~Direct3D9Query()
{
    /* Release query object */
    if (D3DQuery_)
        D3DQuery_->Release();
}

void Direct3D9Query::begin()
{
    if (D3DQuery_ && Protocol_ == 0)
    {
        if (D3DQuery_->Issue(D3DISSUE_BEGIN) == D3D_OK)
            Protocol_ = 1;
        else
            Protocol_ = 0;
    }
}

void Direct3D9Query::end()
{
    if (D3DQuery_ && Protocol_ == 1)
    {
        if (D3DQuery_->Issue(D3DISSUE_END) == D3D_OK)
            Protocol_ = 2;
        else
            Protocol_ = 0;
    }
}

u64 Direct3D9Query::result() const
{
    if (!D3DQuery_ || Protocol_ != 2)
        return 0;
    
    DWORD Result = 0;
    
    while (D3DQuery_->GetData(&Result, sizeof(DWORD), D3DGETDATA_FLUSH) != S_OK)
    {
        // Empty loop
    }

    Protocol_ = 0;
    
    return static_cast<s32>(Result);
}


/*
 * ======= Private: =======
 */

void Direct3D9Query::createHWQuery()
{
    /* Create D3D9 query object */
    IDirect3DDevice9* D3DDevice = D3D9_DEVICE;
    
    D3DDevice->CreateQuery(D3D9QueryTypes[getType()], &D3DQuery_);
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
