@ECHO OFF
SETLOCAL enabledelayedexpansion

:: This is the default root folder for Blender installs.
:: You need to update this if you have a non-standard install path
SET DEFAULT_BLENDER_INSTALL_PATH=%PROGRAMFILES%\Blender Foundation

SET SCRIPT_DIR=%~dp0
SET IMPORTANT_PYTHON_FILE="%SCRIPT_DIR%rig2blender.py"

:: We need to specify three module paths:
::  1) The path to the blender python scripts
::  2) The path to the rig2pyHelper script
::  3) The path to the rig2pyBlender binary module
SET MODULE_PATHS="%SCRIPT_DIR%;%SCRIPT_DIR%\..\rig2py\py;%SCRIPT_DIR%\..\bin/win64"

:: We require at least N arguments
IF "%1"=="" goto printUsage

if NOT EXIST %1 (
   ECHO "Input file not found"
   EXIT /B 1
)

:: Try and find the blender command:
::  - Try 'blender' at the command line (requires the Blender install directory to be in PATH)
::  - Try the standard installation directory
::  - TODO: Parse the registry - ugh
SET blenderCmd="blender"
WHERE !blenderCmd! >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (
   :: Pick the last (which should be the highest) subfolder
   FOR /d %%i in ("!DEFAULT_BLENDER_INSTALL_PATH!\*") do (SET blenderDir=%%i)
   SET blenderCmd=!blenderDir!\blender.exe
   IF NOT EXIST !blenderCmd! (
      ECHO "Cannot locate blender.exe. Update this script manually or add it to the PATH"
      EXIT /B 1
   )
)

:: Start Blender with python
"%blenderCmd%" --python "%IMPORTANT_PYTHON_FILE%" -- %* -s %MODULE_PATHS%

EXIT /B 0

:printUsage
   ECHO "Usage: %0 <input_json_file>"
   EXIT /B 1