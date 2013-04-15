/*
 * Deferred renderer shader classbacks header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DEFERRED_RENDERER_SHADER_CALLBACKS_H__
#define __SP_DEFERRED_RENDERER_SHADER_CALLBACKS_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include <vector>


namespace sp
{

namespace scene
{
    class MaterialNode;
}

namespace video
{


class ShaderClass;
class TextureLayer;

/*
 * All deferred renderer (DfRn) shader callbacks
 */

void DfRnGBufferObjectShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object);
void DfRnGBufferObjectShaderCallbackCB(ShaderClass* ShdClass, const scene::MaterialNode* Object);

void DfRnGBufferSurfaceShaderCallback(ShaderClass* ShdClass, const std::vector<TextureLayer*> &TextureLayers);
void DfRnGBufferSurfaceShaderCallbackCB(ShaderClass* ShdClass, const std::vector<TextureLayer*> &TextureLayers);

void DfRnDeferredShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object);
void DfRnDeferredShaderCallbackCB(ShaderClass* ShdClass, const scene::MaterialNode* Object);

void DfRnShadowShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object);

void DfRnDebugVPLShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object);


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================