/*
 * Web page renderer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spUtilityWebPageRenderer.hpp"

#ifdef SP_COMPILE_WITH_WEBPAGERENDERER


#include "RenderSystem/spRenderSystem.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace tool
{


WebPageRenderer::WebPageRenderer()
{
}
WebPageRenderer::~WebPageRenderer()
{
}

void WebPageRenderer::renderWebPage(video::Texture* Tex, const SXMLTag &RootTag)
{
    if (!Tex)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("WebPageRenderer::renderWebPage");
        #endif
        return;
    }
    
    /* Initialize rendering */
    ViewSize_ = Tex->getSize();
    
    
    
    
    
}


/*
 * ======= Private: =======
 */

void WebPageRenderer::parseTag(const SXMLTag &Tag)
{
    
}

void WebPageRenderer::parseFontTag(const SXMLTag &Tag)
{
    
}

void WebPageRenderer::parseImageTag(const SXMLTag &Tag)
{
    
}

void WebPageRenderer::pushFont(const io::stringc &Face, const video::color &Color, s32 Size)
{
    
}

void WebPageRenderer::pushFontBold()
{
    
}

void WebPageRenderer::pushFontItalic()
{
    
}

void WebPageRenderer::pushFontUnderlined()
{
    
}

void WebPageRenderer::popFont()
{
    
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
