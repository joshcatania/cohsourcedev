@echo off
setlocal
cd /d "%~dp0"

echo Building Release/x86...
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0agent\build.ps1" -Configuration Release -Platform x86
if errorlevel 1 (
    echo.
    echo BUILD FAILED. The local client was not launched.
    pause
    exit /b 1
)

echo.
echo Build succeeded. Starting local City of Heroes...
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0agent\play-local.ps1" %*
set "EXITCODE=%ERRORLEVEL%"

if not "%EXITCODE%"=="0" (
    echo.
    echo REBUILD-AND-PLAY-COH failed during launch with exit code %EXITCODE%.
    pause
)

exit /b %EXITCODE%
