cd .\build\msvc18x64

@echo off

for /f "usebackq delims=" %%i in (`
  vswhere -latest -property installationPath
`) do set VSINSTALL=%%i

call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

msbuild api_tests.vcxproj ^
    /p:Configuration=%1 ^
    /p:Platform=x64 ^
    /m

