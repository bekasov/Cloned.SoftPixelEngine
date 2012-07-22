/*
 * GUI string gadget header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_GUI_STRINGGADGET_H__
#define __SP_GUI_STRINGGADGET_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "GUI/spGUIGadget.hpp"


namespace sp
{
namespace gui
{


enum EStringFlags
{
    GUIFLAG_READONLY    = 0x00000001,
    GUIFLAG_UPPERCASE   = 0x00000002,
    GUIFLAG_LOWERCASE   = 0x00000004,
    GUIFLAG_PASSWORD    = 0x00000008,
    GUIFLAG_NUMERIC     = 0x00000010,
    GUIFLAG_NUMERICINT  = 0x00000020,
};


class SP_EXPORT GUIStringGadget : public GUIGadget
{
    
    public:
        
        GUIStringGadget();
        ~GUIStringGadget();
        
        bool update();
        void draw();
        
        /* Inline fucntions */
        
        inline void setPasteMode(bool isPasteMode)
        {
            isPasteMode_ = isPasteMode;
        }
        inline bool getPasteMode() const
        {
            return isPasteMode_;
        }
        
    private:
        
        /* Enumerations */
        
        enum EFocusUsages
        {
            USAGE_SELECT,
        };
        
        /* Functions */
        
        void init();
        
        void drawCursor(s32 PosHorz);
        
        s32 getCursorPosition(s32 GlobalCursorPosHorz) const;
        void setCursorPosition(s32 LocalCursorPosHorz);
        
        void getSelection(s32 &Start, s32 &End) const;
        
        s32 getStringLen(u32 Pos) const;
        s32 getStringLen(u32 Start, u32 End) const;
        
        void clampViewPos();
        void updateViewPos(bool isSingleClick);
        void updateViewPosCursor();
        
        void updateInput();
        void updateText();
        
        bool isValidChar(c8 Character) const;
        
        /* Members */
        
        io::stringc FinalText_;
        s32 CursorPos_, ViewPos_;
        s32 SelectionStart_, SelectionEnd_;
        
        u64 CursorBlinkTime_;
        bool BlinkState_;
        bool isPasteMode_;
        
};


} // /namespace gui

} // /namespace sp


#endif

#endif



// ================================================================================
