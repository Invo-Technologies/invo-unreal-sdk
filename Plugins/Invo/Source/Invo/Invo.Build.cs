// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Invo : ModuleRules
{
	public Invo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        //Type = ModuleRules.ModuleType.External;
       // PrecompileForTargets = PrecompileTargetsType.Any;

        // Include the libpqxx headers
        //PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "libpqxx"));

        // Link against the libpqxx static library
        //PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ThirdParty", "libpqxx", "pqxx.lib"));

        //PrivateIncludePaths.Add("ThirdParty/libpqxx/include");
        //PublicAdditionalLibraries.Add("ThirdParty/libpqxx/ibpqxx.lib");
        //PublicLibraryPaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "libpqxx", "lib"));
        //.Add("pqxx");
        //PublicAdditionalLibraries.Add("libpq");


        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				//Path.Combine(ModuleDirectory, "ThirdParty/libpqxx/include"),
               // Path.Combine(ModuleDirectory, "ThirdParty/libpqxx/build/include")
                //Path.Combine(ModuleDirectory, "C:\\Program Files\\PostgreSQL\\15\\include")

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
				"WebBrowserWidget"
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
				"WebBrowserWidget"

				// ... add private dependencies that you statically link with here ...	
			}
            );

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
    }
}
