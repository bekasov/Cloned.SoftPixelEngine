/*
 * Cg core header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_CG_COREHEADER_H__
#define __SP_CG_COREHEADER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_CG)


#include <Cg/cg.h>

#if defined(SP_COMPILE_WITH_OPENGL)
#   include "RenderSystem/OpenGL/spOpenGLShader.hpp"
#   include <Cg/cgGL.h>
#endif
#if defined(SP_COMPILE_WITH_DIRECT3D9)
#   include "RenderSystem/Direct3D9/spDirect3D9Shader.hpp"
#   include <Cg/cgD3D9.h>
#endif
#if defined(SP_COMPILE_WITH_DIRECT3D11)
#   include "RenderSystem/Direct3D11/spDirect3D11Shader.hpp"
#   include <Cg/cgD3D11.h>
#endif


#endif

#endif



// ================================================================================
