/*
 * GUI string gadget file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUIStringGadget.hpp"

#ifdef SP_COMPILE_WITH_GUI

#include "RenderSystem/spRenderSystem.hpp"
#include "GUI/spGUIManager.hpp"
#include "Base/spInternalDeclarations.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern gui::GUIManager* __spGUIManager;

namespace gui
{


GUIStringGadget::GUIStringGadget() :
    GUIGadget       (GADGET_STRING  ),
    CursorPos_      (0              ),
    ViewPos_        (0              ),
    SelectionStart_ (0              ),
    SelectionEnd_   (0              ),
    CursorBlinkTime_(0              ),
    BlinkState_     (false          ),
    isPasteMode_    (false          )
{
}
GUIStringGadget::~GUIStringGadget()
{
}

bool GUIStringGadget::update()
{
    if (hasFocus())
        updateInput();
    
    if (!checkDefaultUpdate())
        return false;
    
    FinalText_ = (Flags_ & GUIFLAG_PASSWORD ? io::stringc::space(Text_.size(), '*') : Text_);
    
    if (isEnabled_ && mouseOver(Rect_) && !foreignUsage())
    {
        if (mouseLeft())
        {
            useFocus(USAGE_SELECT);
            
            /* Update cursor position */
            CursorPos_ = getCursorPosition(__spGUIManager->CursorPos_.X);
            
            SelectionStart_ = CursorPos_;
            SelectionEnd_   = CursorPos_;
            
            CursorBlinkTime_    = __spGUIManager->Time_;
            BlinkState_         = true;
            
            /* Update view position */
            updateViewPos(true);
        }
        
        if (mouseLeftDown())
        {
            CursorPos_      = getCursorPosition(__spGUIManager->CursorPos_.X);
            SelectionEnd_   = CursorPos_;
            
            updateViewPos(false);
        }
    }
    
    return true;
}

void GUIStringGadget::draw()
{
    if (!isVisible_ || isValidated_ || !setupClipping())
        return;
    
    FinalText_ = (Flags_ & GUIFLAG_PASSWORD ? io::stringc::space(Text_.size(), '*') : Text_);
    
    __spVideoDriver->draw2DRectangle(Rect_, 255);
    
    if (SelectionStart_ != SelectionEnd_ && hasFocus())
    {
        /* Draw selection bar and text parts */
        s32 Start, End;
        getSelection(Start, End);
        
        __spVideoDriver->draw2DRectangle(
            dim::rect2di(
                Rect_.Left + 5 - ViewPos_ + getStringLen(Start), Rect_.Top + 3,
                Rect_.Left + 4 - ViewPos_ + getStringLen(End), Rect_.Bottom - 3
            ),
            video::color(64, 64, 255)
        );
        
        const s32 TextWidthStart    = getStringLen(Start);
        const s32 TextWidthEnd      = getStringLen(End);
        
        drawText(
            dim::point2di(Rect_.Left + 5 - ViewPos_, Rect_.Top + 2),
            FinalText_.left(Start), 0, 0
        );
        drawText(
            dim::point2di(Rect_.Left + 5 - ViewPos_ + TextWidthStart, Rect_.Top + 2),
            FinalText_.section(Start, End), 255, 0
        );
        drawText(
            dim::point2di(Rect_.Left + 5 - ViewPos_ + TextWidthEnd, Rect_.Top + 2),
            FinalText_.right(FinalText_.size() - End), 0, 0
        );
    }
    else
    {
        /* Draw normal simple text */
        drawText(
            dim::point2di(Rect_.Left + 5 - ViewPos_, Rect_.Top + 2), FinalText_, 0, 0
        );
    }
    
    if (hasFocus())
    {
        if (__spGUIManager->Time_ > CursorBlinkTime_ + 500)
        {
            CursorBlinkTime_    = __spGUIManager->Time_;
            BlinkState_         = !BlinkState_;
        }
        
        if (BlinkState_)
            drawCursor(Rect_.Left + 5 - ViewPos_ + getStringLen(CursorPos_));
    }
    
    if (!isEnabled_)
        drawHatchedFace(Rect_);
    
    if (!(Flags_ & GUIFLAG_BORDERLESS))
        drawFrame(Rect_, 0, false);
}


/*
 * ======= Private: =======
 */

void GUIStringGadget::drawCursor(s32 PosHorz)
{
    if (isPasteMode_)
    {
        __spVideoDriver->draw2DLine(
            dim::point2di(PosHorz, Rect_.Bottom - 4),
            dim::point2di(PosHorz + getStringLen(CursorPos_, CursorPos_ + 1), Rect_.Bottom - 4),
            0
        );
    }
    else
    {
        __spVideoDriver->draw2DLine(
            dim::point2di(PosHorz, Rect_.Top + 3), dim::point2di(PosHorz, Rect_.Bottom - 3), 0
        );
    }
}

s32 GUIStringGadget::getCursorPosition(s32 GlobalCursorPosHorz) const
{
    const s32 PixelPos = GlobalCursorPosHorz - Rect_.Left - 5 + ViewPos_;
    s32 Width = 0, CurWidth;
    
    for (u32 i = 0; i < Text_.size(); ++i)
    {
        CurWidth = Font_->getStringWidth(FinalText_.mid(i, 1));
        
        if (Width > PixelPos - CurWidth/2)
            return static_cast<s32>(i);
        
        Width += CurWidth;
    }
    
    return Text_.size();
}
void GUIStringGadget::setCursorPosition(s32 LocalCursorPosHorz)
{
    CursorPos_ = LocalCursorPosHorz;
    updateViewPosCursor();
    
    SelectionEnd_ = CursorPos_;
    
    if (!__isKey[io::KEY_SHIFT])
        SelectionStart_ = CursorPos_;
}

void GUIStringGadget::getSelection(s32 &Start, s32 &End) const
{
    Start   = SelectionStart_;
    End     = SelectionEnd_;
    
    if (Start > End)
        std::swap(Start, End);
}

s32 GUIStringGadget::getStringLen(u32 Pos) const
{
    return Font_->getStringWidth(FinalText_.left(Pos));
}
s32 GUIStringGadget::getStringLen(u32 Start, u32 End) const
{
    return Font_->getStringWidth(FinalText_.section(Start, End));
}

void GUIStringGadget::clampViewPos()
{
    math::clamp(ViewPos_, 0, Font_->getStringWidth(FinalText_) - (Rect_.Right - Rect_.Left) + 10);
}
void GUIStringGadget::updateViewPos(bool isSingleClick)
{
    const s32 Step = (isSingleClick ? (Rect_.Right - Rect_.Left)/2 : 5);
    
    if (__spGUIManager->CursorPos_.X > Rect_.Right - 10)
    {
        ViewPos_ += Step;
        clampViewPos();
    }
    if (__spGUIManager->CursorPos_.X < Rect_.Left + 10)
    {
        ViewPos_ -= Step;
        clampViewPos();
    }
}
void GUIStringGadget::updateViewPosCursor()
{
    ViewPos_ = getStringLen(CursorPos_) - (Rect_.Right - Rect_.Left)/2;
    clampViewPos();
}

void GUIStringGadget::updateInput()
{
    const io::stringc InputStr = __spGUIManager->InputStr_;
    bool isPlaySound = false;
    s32 Start, End;
    
    if (__isKey[io::KEY_CONTROL])
    {
        if (__hitKey[io::KEY_A])
        {
            SelectionStart_ = 0;
            SelectionEnd_   = Text_.size();
        }
    }
    else if (InputStr.size())
    {
        if (InputStr[0] == c8(8)) // Backspace
        {
            if (SelectionStart_ != SelectionEnd_)
            {
                getSelection(Start, End);
                Text_ = Text_.left(Start) + Text_.right(Text_.size() - End);
                setCursorPosition(Start);
            }
            else if (CursorPos_ > 0)
            {
                Text_ = Text_.left(CursorPos_ - 1) + Text_.right(Text_.size() - CursorPos_);
                setCursorPosition(--CursorPos_);
            }
            else
                isPlaySound = true;
        }
        else if (InputStr[0] == c8(127)) // Delete
        {
            if (CursorPos_ < static_cast<s32>(Text_.size()) - 1)
                Text_ = Text_.left(CursorPos_) + Text_.right(Text_.size() - CursorPos_ - 1);
            else
                isPlaySound = true;
        }
        else
        {
            if (isValidChar(InputStr[0]))
            {
                Text_ = Text_.left(CursorPos_) + InputStr[0] + Text_.right(Text_.size() - CursorPos_ - (isPasteMode_ ? 1 : 0));
                setCursorPosition(++CursorPos_);
            }
            else
                isPlaySound = true;
        }
    }
    else
    {
        if (__hitKey[io::KEY_HOME] || __hitKey[io::KEY_DOWN])
        {
            if (CursorPos_)
                setCursorPosition(0);
            else
                isPlaySound = true;
        }
        else if (__hitKey[io::KEY_END] || __hitKey[io::KEY_UP])
        {
            if (CursorPos_ < static_cast<s32>(Text_.size()))
                setCursorPosition(Text_.size());
            else
                isPlaySound = true;
        }
        else if (__hitKey[io::KEY_LEFT])
        {
            if (CursorPos_)
                setCursorPosition(--CursorPos_);
            else
                isPlaySound = true;
        }
        else if (__hitKey[io::KEY_RIGHT])
        {
            if (CursorPos_ < static_cast<s32>(Text_.size()))
                setCursorPosition(++CursorPos_);
            else
                isPlaySound = true;
        }
        else if (__hitKey[io::KEY_INSERT])
            isPasteMode_ = !isPasteMode_;
    }
    
    #ifdef SP_PLATFORM_WINDOWS
    if (isPlaySound)
        MessageBeep(0);
    #endif
}

void GUIStringGadget::updateText()
{
    for (u32 i = 0, j = 0; i < Text_.size(); ++i)
    {
        if (!isValidChar(Text_[j]))
            Text_ = Text_.left(j) + Text_.right(Text_.size() - j - 1);
        else
            ++j;
    }
}

bool GUIStringGadget::isValidChar(c8 Character) const
{
    if ( (Flags_ & GUIFLAG_NUMERIC) && Character != '.' && ( Character < '0' || Character > '9' ) )
        return false;
    if ( (Flags_ & GUIFLAG_NUMERICINT) && ( Character < '0' || Character > '9' ) )
        return false;
    return true;
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
