/*
 * iOS AppDelegate header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#import <UIKit/UIKit.h>

@class SoftPixelEngine_ViewController;

@interface SoftPixelEngine_AppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    SoftPixelEngine_ViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet SoftPixelEngine_ViewController *viewController;

@end



// ================================================================================
