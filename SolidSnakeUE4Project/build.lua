#!/usr/bin/env lua

--- Helper Build Script from Compiling a Standalone Game

--- Join arguments into one long whitespace seperated shell command
-- The primary purpose is a syntactically cleaner
function build_command(str, ...)
   concat_targets = {...}
   tmp = str

   for i_str = 1, #concat_targets do
      local x_str = concat_targets[i_str]
      assert(type(x_str) == "string", "arguments must be a string, arg: "..tostring(x_str))

      b_not_emptystring = x_str ~= ""
      if b_not_emptystring then
         tmp = tmp.." "..x_str
      end
   end

   return tmp
end

--- Distinct modes that the UnrealbuildTool can operate in
build_mode =
   {
      build =                       "Build",
      clean =                       "Clean",
      deploy =                      "Deploy",
      export_compile_commands =     "GenerateClangDatabase",
      query_targets =               "QueryTargets",
      execute =                     "Execute",
      json_export =                 "JsonExport",
      generate_project_files =      "GenerateProjectFiles",
      validate_platforms =          "ValidatePlatforms",
      write_documentation =         "WriteDocumentation",
      write_metadata =              "WriteMetaData",
      ios_post_build_sync =         "IOSPostBuildSync",
      aggregate_parse_timing_info = "AggregatedParsedTImingInfo",
      parse_msvc_timing_info =      "ParseMsvcTimingInfo",
      pvs_gather =                  "PVSGather"
   }

platforms =
   {
      linux     = "Linux",
      windows   = "Windows",
      mac       = "Mac",
   }
target_platforms =
{
      android = "Android",
      android_astc = "Android_ASTC",
      android_dxt = "Android_DXT",
      android_etc2 = "Android_ETC2",
      android_client, "AndroidClient",
      android_astc_client = "Android_ASTCClient",
      android_dxt_client = "AndroidDXTCLient",
      android_etc2_client = "Android_ETC2Client",
      android_multi = "Android_Multi",
      android_multi_client = "Android_MultiClient",
      linux        = "Linux",
      linux_game   = "LinuxNoEditor",
      linux_client = "LinuxClient",
      linux_server = "LinuxServer",
      linux_aarch64_game = "LinuxAArch64NoEditor",
      linux_aarch64_client = "LinuxAArch64Client",
      linux_aarch64_server = "LinuxAAarch64Server"
}
configurations =
   {
      debug_stability       = "Debug",
      optmized_development  = "Development",
      shipping_ready        = "Shipping"
   }
build_types =
   {
      -- Cooked monolithic game executable (GameName.exe).
      --- Also used for a game-agnostic engine executable (UE4Game.exe or RocketGame.exe)
      cooked = "Game",
      -- Uncooked modular editor executable and DLLs
      -- (UE4Editor.exe, UE4Editor*.dll, GameName*.dll)
      uncooked = "Editor",
      -- Cooked monolithic game client executable (GameNameClient.exe, but no server code)
      monolithic = "Client",
      -- Cooked monolithic game server executable (GameNameServer.exe, but no client
      server = "Server",
      -- Program (standalone program, e.g. ShaderCompileWorker.exe,
      -- can be modular or monolithic depending on the program)
      standalone = "Program",
   }

project_name            = "SolidSnake"
target_project          = "SolidSnake"

-- == Paths ==
-- Care here must be taken, because you cannot assume the platform binary path like you might think
project_path            = "/local/repos/SolidSnake/SolidSnakeUE4Project/" -- hardcoded
engine_path             = "/local/repos/UnrealEngine4/"                   -- hardcoded
uproject_path           = project_path..project_name..".uproject"
uproject_stub_path      = project_path.."Stub.uproject"

editor_command_path     = engine_path.."Engine/Binaries/Linux/UE4Editor"
editor_cmdlet_path      = engine_path.."Engine/Binaries/Linux/UE4Editor-Cmd"
ubt_executable_path     = engine_path.."Engine/Binaries/DotNET/UnrealBuildTool.exe"
build_executable_path   =
   engine_path.."Engine/Build/BatchFiles/Linux/Build.sh"

-- == Generate Build Commands ==
-- User Editable

-- Generate Commands
editor_command          = editor_command_path
editor_cmdlet_command   = editor_cmdlet_path
build_script_command    = build_executable_path
ubt_executable_command  = "mono "..ubt_executable_path
engine_cmdlet_command   = engine_cmdlet_path

-- Generate Arguments
-- Note: linux_editor cannot be a cook target
cook_target_platform_arg = target_platforms.linux_game

uproject_arg        = uproject_path
build_type_arg      = build_types.uncooked
configuration_arg   = configurations.optmized_development
project_name_arg    = "SolidSnake"
module_name_arg     = "SolidSnakeEditor"
platform_arg        = platforms.linux
cook_content_arg    = "-cookonthefly"
build_mode_arg      = build_mode.build

-- No touching from user
-- Should only run once
cook_target_platform_arg = "-targetplatform="..cook_target_platform_arg

uproject_arg        = uproject_arg
build_type_arg      = build_type_arg    -- Appears to be irrelevant command options
configuration_arg   = configuration_arg
project_name_arg    = project_name_arg
module_name_arg     = module_name_arg
platform_arg        = "-Platform "..platform_arg
cook_content_arg    = cook_content_arg
build_mode_arg = "-mode="..build_mode_arg

-- Optional args
iterative_cook_arg = b_iterative_cook and "-iterate" or "" -- Does this even do anything?
cook_on_fly_arg    = b_cook_on_fly and "-cookonthefly" or ""

-- Concatonate Arguments

-- An editor target must be built to cook or run from the editor
cook_command = build_command(editor_cmdlet_command,
                             uproject_path,
                             "-run=cook",
                             cook_target_platform_arg,
                             "-iterate",  -- Reuse non-stale cooked content
                             "-compress") -- Compress build

-- "Cook On the Fly" Cook Server
cook_server_command = build_command(editor_cmdlet_command,
                                    uproject_stub_path,
                                    "-run=cook",
                                    cook_target_platform_arg,
                                    "-iterate",  -- Reuse non-stale cooked content
                                    "-compress",
                                    "-cookonthefly")

-- Don't forget that you need to rebuild the build system if buils.cs files are stale
build_command = build_command("ccache",
                              build_script_command,
                              module_name_arg,
                              uproject_arg,
                              platform_arg,
                              configuration_arg,
                              cook_on_fly_arg, "-waitmutex",
                              build_mode_arg)

-- Run with '-Game' to run uncooked project
editor_command = "/local/repos/UnrealEngine4/Engine/Binaries/Linux/UE4Editor-Cmd -generateprojectfiles $(readlink -f SolidSnake.uproject) Development znull &"

print("[Cook Command]", cook_command)
print("[Build Command]", build_command)
print("Note that assets must be cooked to update all levels and assets")
print("[Building Source]")

if os.execute() == nil then
   print("No shell found")
end

if string.find(arg[0], "build.lua") then
   local build_succes, code_type, code =  os.execute(build_command)
   -- Pass through exit code for scripts
   os.exit(code)
end
