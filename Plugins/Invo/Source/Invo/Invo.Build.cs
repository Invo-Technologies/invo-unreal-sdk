// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;


public class Invo : ModuleRules
{
	public Invo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Settings",
                "Json",
                "JsonUtilities",
                "HTTP",
                "UMG",
                "WebBrowser",
                "WebBrowserWidget",
                "Slate",
                "Engine",
                "SlateCore",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Core",
                "InputCore",
                "Engine",
                "Slate",
                "SlateCore",
                "JsonUtilities",
                "Json",
                "HTTP",
                "WebBrowser",
                "UMG",
                "WebBrowserWidget",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        //string LibrariesPath = Path.Combine(ModuleDirectory, "../../Source/ThirdParty/jsoncpp/lib");
        //string ThirdPartyPath = Path.Combine(ModuleDirectory, "../../Source/ThirdParty");
		//string OpenSSLPath = Path.Combine(ModuleDirectory, "../../Source/ThirdParty/OpenSSL");

        // Include path
        //PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "SQLite", "include"));
        //PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "OpenSSL", "include"));



        // Library path
        //string LibraryPath = Path.Combine(ThirdPartyPath, "SQLite", "lib", "SQLiteStaticLib.lib");
        //string JsonCppLibraryPath = Path.Combine(ThirdPartyPath, "OpenSSL", "lib", "libssl_static.lib");


        // Link the static library
        //PublicAdditionalLibraries.Add(LibraryPath);

        //PublicAdditionalLibraries.Add(JsonCppLibraryPath);

        //PublicAdditionalLibraries.Add(Path.Combine(OpenSSLPath, "lib", "libcrypto_static.lib"));
        //PublicAdditionalLibraries.Add(Path.Combine(OpenSSLPath, "lib", "libssl_static.lib"));

    }
}
