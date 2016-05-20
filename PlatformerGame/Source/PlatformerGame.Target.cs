// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class PlatformerGameTarget : TargetRules
{
    public PlatformerGameTarget(TargetInfo Target)
	{
		Type = TargetType.Game;
	}

	//
	// TargetRules interface.
	//

	public override void SetupBinaries(
		TargetInfo Target,
		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
		ref List<string> OutExtraModuleNames
		)
	{
        OutExtraModuleNames.Add("PlatformerGame");
	}
    public override List<UnrealTargetPlatform> GUBP_GetPlatforms_MonolithicOnly(UnrealTargetPlatform HostPlatform)
    {
        List<UnrealTargetPlatform> Platforms = null;

        switch (HostPlatform)
        {
            default:
                Platforms = new List<UnrealTargetPlatform>();
                break;
        }

        return Platforms;
    }

    public override List<UnrealTargetPlatform> GUBP_GetBuildOnlyPlatforms_MonolithicOnly(UnrealTargetPlatform HostPlatform)
    {
		List<UnrealTargetPlatform> Platforms = null;

		switch (HostPlatform)
		{
			case UnrealTargetPlatform.Mac:
				Platforms = new List<UnrealTargetPlatform> { HostPlatform, UnrealTargetPlatform.IOS };
				break;

			case UnrealTargetPlatform.Win64:
				Platforms = new List<UnrealTargetPlatform> { HostPlatform, UnrealTargetPlatform.Win32, UnrealTargetPlatform.Android };
				break;

			default:
				Platforms = new List<UnrealTargetPlatform>();
				break;
		}

		return Platforms;
    }

    public override List<UnrealTargetConfiguration> GUBP_GetConfigs_MonolithicOnly(UnrealTargetPlatform HostPlatform, UnrealTargetPlatform Platform)
    {
        return new List<UnrealTargetConfiguration> { UnrealTargetConfiguration.Development, UnrealTargetConfiguration.Test };
    }
}
