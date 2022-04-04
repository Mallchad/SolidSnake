// Copyright Epic Games, Inc. All Rights Reserved.

#include "SolidSnake.h"

#include <CoreMinimal.h>
#include <Modules/ModuleManager.h>

#include <Logging/LogMacros.h>
#include <DynamicRHI.h>
#include <GameFramework/GameUserSettings.h>
#include <SDL_hints.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSolidSnake, All, All);
DEFINE_LOG_CATEGORY(LogSolidSnake);

static TAutoConsoleVariable<int32> CVarNullSetting(
    TEXT("ctest.NullSetting"),
    420,
    TEXT("Does absolutely nothing, here as proof of concept \n"
         "This is more text to show off multi-line C++ strings"));

class UGameUserSettings;
void FSolidSnake::StartupModule()
{
    // Don't minimize on focus loss so its easier to return to fullscreen games
    SDL_SetHint("SDL_VIDEO_MINIMIZE_ON_FOCUS_LOSS", "false");

    // Frame capping and video settings for machine stability
    if (GEngine != nullptr)
    {
        UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
        UserSettings->SetFrameRateLimit(60);
        UserSettings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
    }
    else { UE_LOG(LogSolidSnake, Error, TEXT("GEngine is a null pointer")); }

    // Try to get available resolutions, unimplimented on Vulkan
    FScreenResolutionArray QueriedVersions = {};
    bool bQuerySuccess = (GDynamicRHI != nullptr) ?
        GDynamicRHI->RHIGetAvailableResolutions(QueriedVersions, false) :
        false;

    if (bQuerySuccess)
    {
        for (FScreenResolutionRHI xScreenInfo : QueriedVersions)
        {
            UE_LOG(LogSolidSnake, Log, TEXT("Detected available screen resolution %d"), xScreenInfo.RefreshRate);
        }
    }
    else { UE_LOG(LogSolidSnake, Log, TEXT("Failed to get screen resolutions from RHI")); }
}

void FSolidSnake::ShutdownModule()
{
    UE_LOG(LogSolidSnake, Error, TEXT("Shutting down module SolidSnake"));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FSolidSnake, SolidSnake, "SolidSnake");
