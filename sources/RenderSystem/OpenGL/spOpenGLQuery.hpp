/*
 * OpenGL query header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENGL_QUERY_H__
#define __SP_OPENGL_QUERY_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/spQuery.hpp"
#include "RenderSystem/OpenGL/spOpenGLCoreHeader.hpp"


namespace sp
{
namespace video
{


class OpenGLQuery : public Query
{
    
    public:
        
        OpenGLQuery(const EQueryTypes Type);
        ~OpenGLQuery();
        
        /* === Functions === */
        
        void begin();
        void end();
        
        u64 result() const;
        
    private:
        
        /* === Members === */
        
        GLuint QueryObject_;
        
        mutable s8 Protocol_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
