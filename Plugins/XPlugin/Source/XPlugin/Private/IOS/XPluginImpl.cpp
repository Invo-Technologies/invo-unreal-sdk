#include "XPluginImpl.h"
#include "Engine/Engine.h"

//#import <CommonCrypto/CommonDigest.h>
#import <AdSupport/AdSupport.h>
#import <Foundation/Foundation.h>
//#import <AVFoundation/AVFoundation.h>
//#import <WebKit/WebKit.h> Removing UIWebKitView

#import <CoreGraphics/CoreGraphics.h>
#import <UserNotifications/UserNotifications.h>
//#import <StoreKit/StoreKit.h>
#import <Foundation/Foundation.h>

//Facebook
//#import <FBSDKCoreKit/FBSDKCoreKit.h>


#import <UIKit/UIkit.h>
#include "IOSAppDelegate.h"

#define SYSTEM_VERSION_GRATERTHAN_OR_EQUALTO(v)  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)


//#if PLATFORM_IOS
////For Push Notifcation
//#define SYSTEM_VERSION_GRATERTHAN_OR_EQUALTO(v)  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)
//#endif


void FXPluginImpl::TestXPlugin()
{
// Commenting out code jesus 
// #if PLATFORM_IOS
    
//    // static IOSAppDelegate *appDelegate = (IOSAppDelegate*)[[UIApplication sharedApplication] delegate];
    
//     //if(IOSAppDelegate){

//     dispatch_async(dispatch_get_main_queue(), ^{

//     //UIViewController* vc =  [[UIApplication sharedApplication]delegate].window.rootViewController;
// //     UIViewController* vc  =    [IOSAppDelegate GetDelegate].window.rootViewController;
// //    
// //
// //        //Testing App Delegate from imported class
// //    IOSAppDelegate* appDelegate = (IOSAppDelegate*)[[UIApplication sharedApplication] delegate];
// //    
// //      
// //        
// //       
// //    
// //      //  IOSAppDelegate* CustomAppDel = [[IOSAppdelegate alloc] init];
// //      //  NSDictionary* lu = CustomAppDel.launchOptions;
// //        
// //    UILabel* myLabel = [[UILabel alloc]initWithFrame:CGRectMake(100, 420, 100, 100)];
// //    [myLabel setText:@"Facebook"];
// //    
// //    vc.view.backgroundColor = UIColor.whiteColor;
// //    
// //    UIView* newV = [[UIView alloc]initWithFrame:CGRectMake(0.0f, 370, 1000, 200)];
// //    newV.backgroundColor = UIColor.whiteColor;
// //    UIView* newM = vc.view;
// //    [newM addSubview:newV];
// //    [newM addSubview:myLabel];

        
//     GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::White, TEXT("TextPlugin launched from IOS Platform"));
        
// //    //Facebook Impleminations
//     [[FBSDKApplicationDelegate sharedInstance] application:[UIApplication sharedApplication]
//                                  didFinishLaunchingWithOptions:[IOSAppDelegate GetDelegate].launchOptions];

//     //[FBSDKSettings setAutoLogAppEventsEnabled:TRUE];
//     [FBSDKAppEvents logEvent:@"InstalliOSRia"];

//     //- (void)applicationDidBecomeActive:(UIApplication *)application {
//     [FBSDKAppEvents activateApp];

        
        
        
        
        
        
        
//    // [[UIApplication sharedApplication] registerForRemoteNotifications];
        
        
        
//         if(SYSTEM_VERSION_GRATERTHAN_OR_EQUALTO(@"10.0"))
//         {
//             UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
//             center.delegate =  [IOSAppDelegate GetDelegate];
//             [center requestAuthorizationWithOptions:(UNAuthorizationOptionSound | UNAuthorizationOptionAlert | UNAuthorizationOptionBadge) completionHandler:^(BOOL granted, NSError * _Nullable error){
             
//              if( !error)
//              {
             
//              dispatch_async(dispatch_get_main_queue(), ^{
//                 [[UIApplication sharedApplication] registerForRemoteNotifications];
                
//                 int32 types = (int32)(UNAuthorizationOptionSound | UNAuthorizationOptionAlert | UNAuthorizationOptionBadge);
//                 FFunctionGraphTask::CreateAndDispatchWhenReady([types]()
//                                                                {
//                                                                    FCoreDelegates::ApplicationRegisteredForUserNotificationsDelegate.Broadcast(types);
//                                                                }, TStatId(), NULL, ENamedThreads::GameThread);
                
                
//             });
             
//              }
             
             
             
//              }];
//         }

    
    
//      });

//   //  }
// #endif

}
