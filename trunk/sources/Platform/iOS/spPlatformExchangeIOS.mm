/*
 * iOS platform exchange file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Platform/iOS/spPlatformExchangeIOS.h"
#include "Base/spInternalDeclarations.hpp"

#import <UIKit/UIKit.h>


SiOS_TouchEvent iOS_TouchEvents[5];
int iOS_TouchEventCount = 0;


void iOS_AlertView(const char* Title, const char* Message)
{
    UIAlertView* Alert = [
        [UIAlertView alloc]
        initWithTitle:[NSString stringWithUTF8String: Title]
        message:[NSString stringWithUTF8String: Message]
        delegate:nil
        cancelButtonTitle:@"Ok"
        otherButtonTitles:nil
    ];
    
	[Alert show];
	[Alert release];
}

void iOS_SetTouchBegin(unsigned int i, int x, int y)
{
    if (i < 5 && !iOS_TouchEvents[i].Active)
    {
        iOS_TouchEvents[i].LastX = iOS_TouchEvents[i].X = x;
        iOS_TouchEvents[i].LastY = iOS_TouchEvents[i].Y = y;
        iOS_TouchEvents[i].Active = true;
    }
}

void iOS_SetTouchMove(unsigned int i, int x, int y)
{
    if (i < 5)
    {
        iOS_TouchEvents[i].Active = true;
        
        iOS_TouchEvents[i].LastX = iOS_TouchEvents[i].X;
        iOS_TouchEvents[i].LastY = iOS_TouchEvents[i].Y;
        
        iOS_TouchEvents[i].X = x;
        iOS_TouchEvents[i].Y = y;
    }
}

void iOS_SetTouchEnd(unsigned int i)
{
    #if 0
    if (i < 5 && iOS_TouchEvents[i].Active)
        iOS_TouchEvents[i].Active = false;
    #else
    for (int i = 0; i < 5; ++i)
        iOS_TouchEvents[i].Active = false;
    #endif
}

void iOS_SetupScreenSize()
{
    CGRect rect = [[UIScreen mainScreen] bounds];
    CGSize size = rect.size;
    sp::__Screen_Width = size.width;
    sp::__Screen_Height = size.height;
}



// ================================================================================
