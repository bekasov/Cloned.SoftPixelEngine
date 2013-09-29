/*
 * Direct3D11 query header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIRECT3D11_QUERY_H__
#define __SP_DIRECT3D11_QUERY_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/spQuery.hpp"

#include <D3D11.h>


namespace sp
{
namespace video
{


class Direct3D11Query : public Query
{
    
    public:
        
        Direct3D11Query(const EQueryTypes Type);
        ~Direct3D11Query();
        
        /* === Functions === */
        
        void begin();
        void end();
        
        u64 result() const;
        
    private:
        
        /* === Members === */
        
        ID3D11Query* D3DQuery_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
