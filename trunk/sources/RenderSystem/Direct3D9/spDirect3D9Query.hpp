/*
 * Direct3D9 query header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIRECT3D9_QUERY_H__
#define __SP_DIRECT3D9_QUERY_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "RenderSystem/spQuery.hpp"

#include <d3d9.h>


namespace sp
{
namespace video
{


class Direct3D9Query : public Query
{
    
    public:
        
        Direct3D9Query(const EQueryTypes Type);
        ~Direct3D9Query();
        
        /* === Functions === */
        
        void begin();
        void end();
        
        u64 result() const;
        
    private:
        
        friend class Direct3D9RenderSystem;

        /* === Functions === */

        void createHWQuery();

        /* === Members === */
        
        IDirect3DQuery9* D3DQuery_;
        
        mutable s8 Protocol_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
