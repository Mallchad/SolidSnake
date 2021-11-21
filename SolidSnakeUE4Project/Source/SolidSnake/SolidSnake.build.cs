// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SolidSnake : ModuleRules
{
    public SolidSnake(ReadOnlyTargetRules Target) : base(Target)
        {
            DefaultBuildSettings = BuildSettingsVersion.V2;

            // Enables IWYU-style PCH model.
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            // -- Explit Build Settings after Engine Upgrade --
            // Omits subfolders from public include, reducing compiler command line length.
            bLegacyPublicIncludePaths = false;

            // Treats shadowed variable warnings as errors.
            ShadowVariableWarningLevel = WarningLevel.Error;

            PublicDependencyModuleNames.AddRange(new string[]
            {
                // Core Requirements
                "Core",
                "CoreUObject",
                "Engine",
                "RHI",

                // Extra Requirements
                "AIModule",
                "NavigationSystem"
                // "InputCore",
                // "HeadMountedDisplay",
            });
        }
}
