@echo off

for /f "usebackq delims=" %%i in (`
  vswhere -latest -property installationPath
`) do set VSINSTALL=%%i

call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

python generate_vs.py
