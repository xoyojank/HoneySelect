// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
public class EditableAvatarEditor : ModuleRules
{
	public EditableAvatarEditor(TargetInfo Target)
	{
		PublicIncludePaths.AddRange(
		    new string[]
		{
			// ... add public include paths required here ...
		}
		);

		PrivateIncludePaths.AddRange(
		    new string[]
		{
			// ... add other private include paths required here ...
			"EditableAvatarEditor/Private",
		}
		);

		PublicDependencyModuleNames.AddRange(
		    new string[]
		{
			"Core",
			"CoreUObject",
			// ... add other public dependencies that you statically link with here ...
		}
		);

		PrivateDependencyModuleNames.AddRange(
		    new string[]
		{
			// ... add private dependencies that you statically link with here ...
			"Core",
			"CoreUObject",
			"Engine",
			"AnimGraph",
			"BlueprintGraph",
			"EditableAvatar",
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