@ECHO OFF
SETLOCAL

SET SCRIPT_DIR=%~dp0
SET IMPORTANT_PYTHON_FILE="%SCRIPT_DIR%rig2pyTest.py"
SET MODULE_PATHS="%SCRIPT_DIR%;%SCRIPT_DIR%..\..\bin\win64"

:: We require at least N arguments
IF "%1"=="" goto printUsage

if NOT EXIST %1 (
   ECHO "Input file not found"
   EXIT /B 1
)

:: Start python using the Windows python launcher
py %IMPORTANT_PYTHON_FILE% -- %* -s %MODULE_PATHS%

EXIT /B 0

:printUsage
   ECHO "Usage: %0 <input_json_file>"
   EXIT /B 1