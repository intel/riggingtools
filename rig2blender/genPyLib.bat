@ECHO OFF

SET defFile=python3.def
SET libOutFile=python3.lib
:: Assumes the def file has already been declared with the necessary functions
lib /def:%defFile% /MACHINE:X64 /OUT:%libOutFile%