#!/usr/bin/env lua

require("build")

print("[Cooking Assets]")

local build_succes, code_type, code =  os.execute(cook_command)
-- Pass through exit code for scripts
os.exit(code)
