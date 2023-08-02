using UnrealBuildTool;
using System;
using System.IO;


public class XPlugin : ModuleRules
{
    public XPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        // This parameter enabled the Include-What-You-Use principle in the Engine for this plugin.
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;





        // Add basic public and private include paths so the plugin and game will be able to locate include files.
        //PublicIncludePaths.Add("XPlugin/Public");
        PrivateIncludePaths.Add("XPlugin/Private");

        // Add Settings to Private Include Path Module Names to safely use Settings module.
        PrivateIncludePathModuleNames.Add("Settings");

        // Add basic dependency modules. You can add some more modules here if you need them.
        PublicDependencyModuleNames.AddRange(new string[] { "Core","CoreUObject" });
        PrivateDependencyModuleNames.AddRange(new string[] { "CoreUObject", "Engine", "HTTP" ,"Json","ApplicationCore"});

        // For every supported platform add the proper directory to private include paths so the plugin will know where to look for platform specific includes.
        // Also it is a good practice to add a Definition that tells us if the plugin is implemented, so we can disable or enable parts of code in the game dependent on if the plugin is on or off.
        
		
		if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            PrivateIncludePaths.Add("XPlugin/Private/IOS");
            PublicDefinitions.Add("WITH_XPLUGIN=1");


        	PublicFrameworks.AddRange(
            new string[]
            {
				"EventKit",
				"MediaPlayer",
				"AdSupport",
				"CoreLocation",
				"SystemConfiguration",
				"MessageUI",
				"Security",
				"CoreTelephony",
				"CoreGraphics",
				"AVFoundation",
				"Foundation",
				"CFNetwork",
				"UIKit",
				//"StoreKit",
				"UserNotifications",
				"MobileCoreServices",
				"CoreData",
				"CoreFoundation"
                //"Bolts" //removing WebKit becausse its depreciated in SDK13 for iOS
            }
            );
        
		// works but iunccomenting 

	/*
      PublicAdditionalFrameworks.Add(
				new Framework(
					"FBSDKCoreKit",
					"../../ThirdParty/iOS/FacebookSDK/FBSDKCoreKit.embeddedframework.zip",
					"FBSDKCoreKit.framework/Resources/FacebookSDKStrings.bundle"
				)
			);

			// Access to Facebook login
		PublicAdditionalFrameworks.Add(
				new Framework(
					"FBSDKLoginKit",
					"../../ThirdParty/iOS/FacebookSDK/FBSDKLoginKit.embeddedframework.zip"
				)
			);

			*/



 // PublicAdditionalFrameworks.Add(
		// 		new Framework(
		// 			"Bolts",
		// 			"../../ThirdParty/iOS/FacebookSDK/Bolts.embeddedframework.zip"
		// 		)
		// 	);

			// Access to Facebook core
		// PublicAdditionalFrameworks.Add(
		// 		new Framework(
		// 			"FBSDKCoreKit",
		// 			"IOS/FacebookSDK/FBSDKCoreKit.embeddedframework.zip",
		// 			"FBSDKCoreKit.framework/Resources/FacebookSDKStrings.bundle"
		// 		)
		// 	);


        //Test for framwork porting
        // PublicAdditionalFrameworks.Add(
        //         new Framework(
        //             "IronSource",
        //             "../../ThirdParty/iOS/IronSource.embeddedframework.zip"
        //         )
                        
        //     );


		// PublicAdditionalFrameworks.Add(
		// 			new Framework(
		// 				"Pushwoosh",
		// 				"../../lib/iOS/Pushwoosh.embeddedframework.zip"
		// 			)
		// 		);

		// 	PublicAdditionalLibraries.Add("z");
		// 	PublicAdditionalLibraries.Add("stdc++");
		
		// else if(Target.Platform == UnrealTargetPlatform.Android)
		// {
		// 	string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
		// 	AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "Pushwoosh_APL.xml"));
		// }



        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateIncludePaths.Add("XPlugin/Private/Android");
            PublicDefinitions.Add("WITH_XPLUGIN=1");

			// string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, BuildConfiguration.RelativeEnginePath);
			// AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin", Path.Combine(PluginPath, "FacebookSDK_APL.xml")));
			// AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "FacebookSDK_APL.xml"));


			// string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath); // works
			// AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "FacebookSDK_APL.xml"));

			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath); //  tessting 
			//AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "FacebookSDK_APL.xml"));


        }
        else if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PrivateIncludePaths.Add("XPlugin/Private/Windows");
            PublicDefinitions.Add("WITH_XPLUGIN=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PrivateIncludePaths.Add("XPlugin/Private/Mac");
            PublicDefinitions.Add("WITH_XPLUGIN=1");
        }
        else
        {
            PublicDefinitions.Add("WITH_XPLUGIN=0");
        }

    }
}
