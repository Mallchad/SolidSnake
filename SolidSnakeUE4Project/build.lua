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

platforms =
   {
      linux     = "Linux",
      windows   = "Windows",
      mac       = "Mac",
   }
target_platforms =
{
      linux        = "Linux",
      linux_game   = "LinuxNoEditor",
      linux_editor = "LinuxEditor",
      linux_server = "LinuxServer",
      linux_client = "LinuxClient"
}
configurations =
   {
      debug_stability       = "Debug",
      development_optmized  = "Development",
      shipping              = "Shipping"
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
platform                = platforms.linux
configuration           = configurations.development_optmized
build_type              = "-"..build_types.cooked
cook_target_platform    = target_platforms.linux_game

-- Paths
project_path            = "/local/repos/SolidSnake/SolidSnakeUE4Project/" -- hardcoded
engine_path             = "/local/repos/UnrealEngine4/"                   -- hardcoded
uproject_path           = project_path..project_name..".uproject"
uproject_stub_path      = project_path.."Stub.uproject"

editor_command_path     = engine_path.."Engine/Binaries/Linux/UE4Editor"
editor_cmdlet_path      = engine_path.."Engine/Binaries/Linux/UE4Editor-Cmd"
ubt_executable_path     = engine_path.."Engine/Binaries/DotNET/UnrealBuildTool.exe"
build_executable_path   =
   engine_path.."Engine/Build/BatchFiles/"..platform.."/Build.sh"

-- == Generate Build Commands ==

-- Generate Commands
editor_command          = editor_command_path
editor_cmdlet_command   = editor_cmdlet_path
build_script_command    = build_executable_path
ubt_executable_command  = "mono "..ubt_executable_path
engine_cmdlet_command   = engine_cmdlet_path
-- Generate Arguments
cook_target_platform_arg = "-targetplatform="..cook_target_platform

uproject_arg        = uproject_path
build_type_arg      = build_type
project_name_arg    = project_name
platform_arg        = "-Platform "..platform
cook_content_arg    = "-cookonthefly"
-- Optional args
iterative_cook_arg = b_iterative_cook and "-iterate" or "" -- Does this even do anything?
cook_on_fly_arg    = b_cook_on_fly and "-cookonthefly" or ""

-- Concatonate Arguments

-- You need a build editor target to cook with
cook_command = build_command(editor_cmdlet_command,
                             uproject_path,
                             "-run=cook",
                             cook_target_platform_arg,
                             "-iterate",  -- Reuse non-stale cooked content
                             "-compress"
                             ) -- Compress build

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
                              project_name_arg,
                              uproject_arg,
                              build_type_arg,
                              platform_arg,
                              configuration,
                              -- Operation args
                              cook_on_fly_arg, "-waitmutex")

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
