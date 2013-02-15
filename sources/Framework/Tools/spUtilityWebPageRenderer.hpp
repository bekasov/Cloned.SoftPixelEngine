/*
 * Web page renderer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILITY_WEBPAGE_RENDERER_H__
#define __SP_UTILITY_WEBPAGE_RENDERER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_WEBPAGERENDERER


#include "Framework/Tools/spToolXMLParser.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "RenderSystem/spRenderSystemFont.hpp"

#include <stack>
#include <map>


namespace sp
{
namespace tool
{


/**
\todo This is absolutely incomplete
\since Version 3.2 
*/
class SP_EXPORT WebPageRenderer
{
    
    public:
        
        WebPageRenderer();
        ~WebPageRenderer();
        
        /* === Functions === */
        
        void renderWebPage(video::Texture* Tex, const SXMLTag &RootTag);
        
    private:
        
        /* === Structures === */
        
        struct SFontDesc
        {
            SFontDesc() :
                Face        ("arial"),
                Size        (12     ),
                Bold        (false  ),
                Italic      (false  ),
                Underlined  (false  )
            {
            }
            ~SFontDesc()
            {
            }

            /* Member */
            io::stringc Face;
            video::color Color;
            s32 Size;
            bool Bold;
            bool Italic;
            bool Underlined;
        };
        
        struct SRenderStates
        {
            SRenderStates() :
                ActiveFont(0)
            {
            }
            ~SRenderStates()
            {
            }
            
            /* Members */
            dim::point2di TextPos;
            video::Font* ActiveFont;
        };
        
        /* === Functions === */
        
        void parseTag(const SXMLTag &Tag);
        void parseFontTag(const SXMLTag &Tag);
        void parseImageTag(const SXMLTag &Tag);
        
        void pushFont(const io::stringc &Face, const video::color &Color, s32 Size); //!< <font> tag
        void pushFontBold();        //!< <b> tag
        void pushFontItalic();      //!< <i> tag
        void pushFontUnderlined();  //!< <u> tag
        void popFont();             //!< </font>, </b>, </i>, </u> tags
        
        /* === Members === */
        
        dim::size2di ViewSize_; //!< Render target texture size.
        
        std::stack<video::Font*> FontStack_;
        
        std::map<SFontDesc, video::Font*> FontMap_;
        std::map<std::string, video::Texture*> ImageMap_;
        
        SRenderStates States_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
