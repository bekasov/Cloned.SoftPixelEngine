//
//  SoftPixelEngine_MacOSX_AppDelegate.h
//  SoftPixelEngine(MacOSX)
//
//  Created by Lukas Hermanns on 24.12.11.
//  Copyright 2011 TU  Darmstadt. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SoftPixelEngine_MacOSX_AppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
}

@property (assign) IBOutlet NSWindow *window;

@end
