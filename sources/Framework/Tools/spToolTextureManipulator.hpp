/*
 * Texture manipulator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TOOL_TEXTUREMANIPULATOR_H__
#define __SP_TOOL_TEXTUREMANIPULATOR_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_TEXTUREMANIPULATOR


#include "Base/spDimension.hpp"
#include "Base/spMath.hpp"
#include "Base/spMaterialColor.hpp"
#include "RenderSystem/spTextureBase.hpp"


namespace sp
{
namespace tool
{


static const dim::rect2di DEF_TEXMANIP_RECT = dim::rect2di(-1, -1);


class SP_EXPORT TextureManipulator
{
    
    public:
        
        TextureManipulator();
        ~TextureManipulator();
        
        void drawMosaic(video::Texture* Tex, s32 PixelSize, dim::rect2di Rect = DEF_TEXMANIP_RECT);
        void drawBlur(video::Texture* Tex, s32 PixelSize, dim::rect2di Rect = DEF_TEXMANIP_RECT);
        
    private:
        
        void checkDimension(const video::Texture* Tex, dim::rect2di &Rect);
        
};


} // /namespace tool
    
} // /namespace sp


#endif

#endif



// ================================================================================
