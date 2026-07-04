@echo off

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo ERROR: No se encontro vswhere.exe
    exit /b 1
)

for /f "usebackq delims=" %%i in (`
  "%VSWHERE%" -latest -property installationPath
`) do set VSINSTALL=%%i

call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

python generate_vs.py
