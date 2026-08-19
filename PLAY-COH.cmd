@echo off
setlocal
cd /d "%~dp0"

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0agent\play-local.ps1" %*
set "EXITCODE=%ERRORLEVEL%"

if not "%EXITCODE%"=="0" (
    echo.
    echo PLAY-COH failed with exit code %EXITCODE%.
    pause
)

exit /b %EXITCODE%
