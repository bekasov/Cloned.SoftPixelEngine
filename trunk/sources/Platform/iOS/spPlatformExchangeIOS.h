/*
 * iOS platform exchange header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_PLATFORMEXCHANGE_IOS_H__
#define __SP_PLATFORMEXCHANGE_IOS_H__


struct SiOS_TouchEvent
{
    SiOS_TouchEvent()
        : X(0), Y(0), LastX(0), LastY(0), SpeedX(0), SpeedY(0), Reset(0), Active(false)
    {
    }
    ~SiOS_TouchEvent()
    {
    }
    
    /* Members */
    int X, Y, LastX, LastY, SpeedX, SpeedY, Reset;
    bool Active;
};
typedef struct SiOS_TouchEvent SiOS_TouchEvent;


extern SiOS_TouchEvent iOS_TouchEvents[5];
extern int iOS_TouchEventCount;


void iOS_AlertView(const char* Title, const char* Message);

void iOS_SetTouchBegin(unsigned int i, int x, int y);
void iOS_SetTouchMove(unsigned int i, int x, int y);
void iOS_SetTouchEnd(unsigned int i);

void iOS_SetupScreenSize();


#endif



// ================================================================================
