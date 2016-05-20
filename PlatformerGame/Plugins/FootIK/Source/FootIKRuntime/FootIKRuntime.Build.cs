// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FootIKRuntime : ModuleRules
{
	public FootIKRuntime(TargetInfo Target)
	{
        PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine",
				"AnimGraphRuntime"
			}
			);
	}
}
