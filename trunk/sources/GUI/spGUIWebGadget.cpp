/*
 * GUI web gadget file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUIWebGadget.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/OpenGL/spOpenGLRenderSystem.hpp"
#include "Base/spSharedObjects.hpp"
#include "GUI/spGUIManager.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;
extern gui::GUIManager* GlbGUIMngr;

namespace gui
{


GUIWebGadget::GUIWebGadget() :
    GUIGadget           (GADGET_WEB ),
    GUIScrollViewBased  (           ),
    ContentTex_         (0          ),
    DrawnFontSize_      (0          ),
    ContentWidth_       (0          )
{
    HorzScroll_.setParent(this);
    VertScroll_.setParent(this);
}
GUIWebGadget::~GUIWebGadget()
{
    GlbRenderSys->deleteTexture(ContentTex_);
}

bool GUIWebGadget::update()
{
    updateScrollBars(&HorzScroll_, &VertScroll_);
    
    if (hasFocus() && GlbGUIMngr->MouseWheel_)
        VertScroll_.scroll(-GlbGUIMngr->MouseWheel_ * 30);
    
    if (!checkDefaultUpdate())
        return false;
    
    if (isEnabled_ && mouseOver(Rect_) && !foreignUsage())
    {
        if (mouseLeft())
        {
            focus();
            
            // ... !!!
        }
    }
    
    updateChildren();
    
    return true;
}

void GUIWebGadget::draw()
{
    if (!isVisible_ || isValidated_ || !setupClipping())
        return;
    
    GlbRenderSys->draw2DRectangle(Rect_, Color_);
    
    if (ContentTex_)
    {
        GlbRenderSys->draw2DImage(
            ContentTex_,
            dim::rect2di(
                Rect_.Left + ScrollPos_.X, Rect_.Top + ScrollPos_.Y,
                ContentTex_->getSize().Width, ContentTex_->getSize().Height
            ),
            dim::rect2df(0, 1, 1, 0)
        );
    }
    
    drawChildren();
    
    GlbRenderSys->setClipping(true, dim::point2di(VisRect_.Left, VisRect_.Top), VisRect_.getSize());
    
    drawFrame(Rect_, 0, false);
}

dim::rect2di GUIWebGadget::getLocalViewArea(const GUIController* Obj) const
{
    dim::rect2di Rect(Rect_);
    
    if (Obj != &HorzScroll_ && Obj != &VertScroll_)
    {
        if (HorzScroll_.getVisible())
            Rect.Bottom -= SCROLLBAR_SIZE;
        if (VertScroll_.getVisible())
            Rect.Right -= SCROLLBAR_SIZE;
    }
    
    return Rect; 
}

void GUIWebGadget::loadContent(const tool::SXMLTag &XMLMainBlock, const s32 ContentWidth)
{
    ContentWidth_   = ContentWidth;
    DrawPos_        = dim::point2di(TEXT_DISTANCE);
    
    /* Recreate a new content texture */
    GlbRenderSys->deleteTexture(ContentTex_);
    
    ContentTex_ = GlbRenderSys->createTexture(dim::size2di(ContentWidth_, ContentWidth_*3)); // !!!
    //ContentTex_->setWrapMode(video::TEXWRAP_CLAMP);
    ContentTex_->setRenderTarget(true);
    
    HorzScroll_.setRange(ContentTex_->getSize().Width);
    VertScroll_.setRange(ContentTex_->getSize().Height);
    
    /* Fill texture with the website content */
    video::Texture* LastRenderTarget = GlbRenderSys->getRenderTarget();
    
    setFont("times new roman", 17, 0);
    
    #ifdef SP_COMPILE_WITH_OPENGL
    s32 LastScreenHeight = 0;
    
    if (GlbRenderSys->getRendererType() == video::RENDERER_OPENGL)
    {
        LastScreenHeight = gSharedObjects.ScreenHeight;
        gSharedObjects.ScreenHeight = ContentTex_->getSize().Height;
    }
    
    bool EnableBalanceRT = video::OpenGLRenderSystem::getBalanceRT();
    video::OpenGLRenderSystem::setBalanceRT(false);
    #endif
    
    GlbRenderSys->setRenderTarget(ContentTex_);
    GlbRenderSys->beginDrawing2D();
    
    /* Create final website content */
    createWebsiteContent(XMLMainBlock);
    deleteLoadedResources();
    
    GlbRenderSys->endDrawing2D();
    GlbRenderSys->setRenderTarget(LastRenderTarget);
    
    #ifdef SP_COMPILE_WITH_OPENGL
    video::OpenGLRenderSystem::setBalanceRT(EnableBalanceRT);
    
    if (GlbRenderSys->getRendererType() == video::RENDERER_OPENGL)
        gSharedObjects.ScreenHeight = LastScreenHeight;
    #endif
}

void GUIWebGadget::loadContent(const io::stringc &Filename, const s32 ContentWidth)
{
    tool::XMLParser xmlParser;
    xmlParser.loadFile(Filename);
    loadContent(xmlParser.getRootTag(), ContentWidth);
}


/*
 * ======= Private: =======
 */

void GUIWebGadget::createWebsiteContent(const tool::SXMLTag &Block)
{
    /* Check for usage */
    const io::stringc Name = Block.Name.lower();
    
    SXMLFont LastFont = CurFont_;
    
    /* Process block features */
    if (Name == "i")
        setFont(CurFont_.Name, CurFont_.Size, CurFont_.Color, CurFont_.Flags | video::FONT_ITALIC);
    else if (Name == "u")
        setFont(CurFont_.Name, CurFont_.Size, CurFont_.Color, CurFont_.Flags | video::FONT_UNDERLINED);
    else if (Name == "b")
        setFont(CurFont_.Name, CurFont_.Size, CurFont_.Color, CurFont_.Flags | video::FONT_BOLD);
    else if (Name == "font")
    {
        io::stringc Attrib;
        
        for (std::list<tool::SXMLAttribute>::const_iterator it = Block.Attributes.begin(); it != Block.Attributes.end(); ++it)
        {
            Attrib = it->Name.lower();
            
            if (Attrib == "face")
                CurFont_.Name = it->Value;
            else if (Attrib == "size")
                CurFont_.Size = it->Value.val<s32>() * 7 + 5;
            else if (Attrib == "color")
                CurFont_.Color = getHexColor(it->Value.lower());
        }
        
        setFont(CurFont_.Name, CurFont_.Size, CurFont_.Color, CurFont_.Flags);
    }
    else if (Name == "p" || Name == "br")
    {
        DrawPos_.X = TEXT_DISTANCE;
        DrawPos_.Y += DrawnFontSize_ + TEXT_DISTANCE;
    }
    
    /* Draw content */
    if (Block.Text.trim().size())
    {
        io::stringc Text = Block.Text.ltrim();
        DrawnFontSize_ = CurFont_.Size;
        
        /*if (CurFont_.Object->getStringSize(Text).Width + DrawPos_.X > ContentWidth_ - TEXT_DISTANCE*2)
        {
            s32 Len, TmpLen, Pos;
            
            while (Text.trim().size())
            {
                TmpLen = Len = 0;
                Pos = -1;
                
                while (CurFont_.Object->getStringSize(Text.left(TmpLen)).Width + DrawPos_.X <= ContentWidth_ - TEXT_DISTANCE*2)
                {
                    Len = TmpLen;
                    
                    if ( ( Pos = Text.find(" ", Pos + 1) ) == -1 )
                        Pos = Text.find("\t", Pos + 1);
                    
                    if (Pos == -1)
                        ++TmpLen;
                    else
                        TmpLen = Pos;
                }
                
                GlbRenderSys->draw2DText(CurFont_.Object, DrawPos_, Text.left(Len), CurFont_.Color);
                
                DrawPos_.Y += DrawnFontSize_ + TEXT_DISTANCE;
                DrawPos_.X = TEXT_DISTANCE;
                
                Text = Text.right(Text.size() - Len);
            }
        }
        else*/
        #if 0
        if (CurFont_.Object->getStringSize(Text).Width + DrawPos_.X > ContentWidth_ - TEXT_DISTANCE*2)
        {
            DrawPos_.X = TEXT_DISTANCE;
            DrawPos_.Y += DrawnFontSize_ + TEXT_DISTANCE;
        }
        #endif
            GlbRenderSys->draw2DText(CurFont_.Object, DrawPos_, Text, CurFont_.Color);
        
        DrawPos_.X += CurFont_.Object->getStringWidth(Text) + TEXT_DISTANCE;
    }
    
    /* Create children blocks */
    foreach (tool::SXMLTag Tag, Block.Tags)
        createWebsiteContent(Tag);
    
    CurFont_ = LastFont;
}

void GUIWebGadget::deleteLoadedResources()
{
    for (std::map<SXMLFontKey, SXMLFont>::iterator it = FontMap_.begin(); it != FontMap_.end(); ++it)
        GlbRenderSys->deleteFont(it->second.Object);
    FontMap_.clear();
}

void GUIWebGadget::setFont(
    const io::stringc &Name, s32 Size, const video::color &Color, s32 Flags)
{
    SXMLFontKey FontKey;
    
    FontKey.Name    = Name;
    FontKey.Color   = Color;
    FontKey.Size    = Size;
    FontKey.Flags   = Flags;
    
    /*CurFont_ = FontMap_[FontKey];
    
    if (CurFont_.Object)
        return;*/
    
    CurFont_.Object = GlbRenderSys->createFont(Name, Size, Flags);
    CurFont_.Name   = Name;
    CurFont_.Color  = Color;
    CurFont_.Size   = Size;
    CurFont_.Flags  = Flags;
    
    FontMap_[FontKey] = CurFont_;
}

video::color GUIWebGadget::getHexColor(const io::stringc &HexStr) const
{
    video::color Color;
    
    if (HexStr.size() >= 6)
    {
        Color.Red   = getHexComponent(HexStr[0], HexStr[1]);
        Color.Green = getHexComponent(HexStr[2], HexStr[3]);
        Color.Blue  = getHexComponent(HexStr[4], HexStr[5]);
    }
    
    return Color;
}

u8 GUIWebGadget::getHexComponent(c8 c0, c8 c1) const
{
    return
        (c0 <= '9' ? c0 - '0' : c0 - 'a' + 10) * 16 +
        (c1 <= '9' ? c1 - '0' : c1 - 'a' + 10);
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
