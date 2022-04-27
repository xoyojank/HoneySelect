// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
public class EditableAvatar : ModuleRules
{
	public EditableAvatar(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.AddRange(
		    new string[]
		{
			// ... add public include paths required here ...
			"EditableAvatar/Public",
		}
		);

		PrivateIncludePaths.AddRange(
		    new string[]
		{
			"EditableAvatar/Private",
			// ... add other private include paths required here ...
		}
		);

		PublicDependencyModuleNames.AddRange(
		    new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AnimGraphRuntime",
			// ... add other public dependencies that you statically link with here ...
		}
		);

		PrivateDependencyModuleNames.AddRange(
		    new string[]
		{
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
}