/*
 * GUI web gadget header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_WEBGADGET_H__
#define __SP_GUI_WEBGADGET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIGadget.hpp"
#include "GUI/spGUIScrollViewBased.hpp"
#include "Framework/Tools/ScriptParser/spToolXMLParser.hpp"

#include <map>


namespace sp
{
namespace gui
{


/**
Web view controller gadget.
\todo This is incomplete
\ingroup group_gui
*/
class SP_EXPORT GUIWebGadget : public GUIGadget, public GUIScrollViewBased
{
    
    public:
        
        GUIWebGadget();
        ~GUIWebGadget();
        
        /* Functions */
        
        bool update();
        void draw();
        
        dim::rect2di getLocalViewArea(const GUIController* Obj = 0) const;
        
        /**
        Loads the XML content into the web gadget. To get this XML data you can use the tool::XMLParser class.
        \param XMLMainBlock: Main block of the XML data. You can get this data by the
        "getMainBlock" function of the tool::XMLParser class.
        */
        void loadContent(const tool::SXMLTag &XMLMainBlock, const s32 ContentWidth = 512);
        
        //! Loads an XML (or rather HTML) file's content into the web gadget.
        void loadContent(const io::stringc &Filename, const s32 ContentWidth = 512);
        
        /* Inline functions */
        
        //! Returns the content texture where the website is graphically stored.
        inline video::Texture* getContentTexture() const
        {
            return ContentTex_;
        }
        
    private:
        
        /* === Macros === */
        
        static const s32 TEXT_DISTANCE;// = 5;
        
        /* === Structures === */
        
        struct SXMLFontKey
        {
            SXMLFontKey() :
                Size    (0),
                Flags   (0)
            {
            }
            ~SXMLFontKey()
            {
            }
            
            /* Operators */
            bool operator < (const SXMLFontKey &other) const
            {
                return
                    Size        < other.Size        ||
                    Flags       < other.Flags       ||
                    Color.Red   < other.Color.Red   ||
                    Color.Green < other.Color.Green ||
                    Color.Blue  < other.Color.Blue  ||
                    Color.Alpha < other.Color.Alpha ||
                    Name.str()  < other.Name.str();
            }
            
            /* Members */
            io::stringc Name;
            video::color Color;
            s32 Size;
            s32 Flags;
        };
        
        struct SXMLFont
        {
            SXMLFont() :
                Object  (0),
                Color   (0),
                Size    (0),
                Flags   (0)
            {
            }
            
            /* Members */
            video::Font* Object;
            io::stringc Name;
            video::color Color;
            s32 Size;
            s32 Flags;
        };
        
        /* === Functions === */
        
        void createWebsiteContent(const tool::SXMLTag &Block);
        
        void deleteLoadedResources();
        
        void setFont(const io::stringc &Name, s32 Size, const video::color &Color, s32 Flags = 0);
        
        video::color getHexColor(const io::stringc &HexStr) const;
        u8 getHexComponent(c8 c0, c8 c1) const;
        
        /* === Members === */
        
        video::Texture* ContentTex_;
        
        std::map<SXMLFontKey, SXMLFont> FontMap_;
        SXMLFont CurFont_;
        s32 DrawnFontSize_;
        
        s32 ContentWidth_;
        dim::point2di DrawPos_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
