// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FootIKEditor : ModuleRules
{
	public FootIKEditor(TargetInfo Target)
	{
        PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"Slate",
				"UnrealEd",
				"AnimGraph", 
				"BlueprintGraph", 
				"FootIKRuntime",
				"AnimGraphRuntime"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"SlateCore",
			}
		);
	}
}
