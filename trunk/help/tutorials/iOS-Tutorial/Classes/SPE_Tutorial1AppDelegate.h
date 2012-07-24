//
//  SPE_Tutorial1AppDelegate.h
//  SPE-Tutorial1
//
//  Created by Lukas Hermanns on 03.01.12.
//  Copyright 2012 TU  Darmstadt. All rights reserved.
//

#import <UIKit/UIKit.h>

@class SPE_Tutorial1ViewController;

@interface SPE_Tutorial1AppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    SPE_Tutorial1ViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet SPE_Tutorial1ViewController *viewController;

@end

