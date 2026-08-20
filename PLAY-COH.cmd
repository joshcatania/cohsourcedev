@echo off
setlocal
cd /d "%~dp0"

if /I "%~1"=="--webswing-dev" (
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0agent\play-local.ps1" -WebSwingDev %~2 %~3 %~4 %~5 %~6 %~7 %~8 %~9
) else if /I "%~1"=="--full" (
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0agent\play-local.ps1" -Full %~2 %~3 %~4 %~5 %~6 %~7 %~8 %~9
) else (
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0agent\play-local.ps1" %*
)
set "EXITCODE=%ERRORLEVEL%"

if not "%EXITCODE%"=="0" (
    echo.
    echo PLAY-COH failed with exit code %EXITCODE%.
    pause
)

exit /b %EXITCODE%
