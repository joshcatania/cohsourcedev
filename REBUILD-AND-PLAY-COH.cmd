@echo off
setlocal
cd /d "%~dp0"

if /I "%~1"=="--fast-shard" (
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0agent\rebuild-and-play.ps1" -Scope FastDev
) else if /I "%~1"=="--full" (
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0agent\rebuild-and-play.ps1" -Scope Full
) else (
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0agent\rebuild-and-play.ps1" -Scope Client
)
set "EXITCODE=%ERRORLEVEL%"

if not "%EXITCODE%"=="0" (
    echo.
    echo REBUILD-AND-PLAY-COH failed during launch with exit code %EXITCODE%.
    pause
)

exit /b %EXITCODE%
