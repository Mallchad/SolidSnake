// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

// Game Specific Target
public class SolidSnakeTarget : TargetRules
{
    public SolidSnakeTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        /*
          Try to improve iterative build times
          Unity build reduces iterative build times at the advantage of massively
          improved recompile times for untouched headers using monolithic files
          Source files in the git working set are excluded from Unity builds
        */
        bUseUnityBuild = true;
        bUseAdaptiveUnityBuild = true;
        bAdaptiveUnityDisablesOptimizations = true;
        /*
          PCH are 'Pre-Compiled-Headers' which allows faster iteraiton on
          cpp-only headers
        */
        bAdaptiveUnityCreatesDedicatedPCH = true;
        bAdaptiveUnityEnablesEditAndContinue = true;
        bDisableDebugInfo = true;
        bDisableDebugInfoForGeneratedCode = true;
        // Hot Reload is buggy and risks blueprint corruption
        bAllowHotReload = false;

        ExtraModuleNames.AddRange(new string[]
        {
            "SolidSnake"
        });
    }
}
