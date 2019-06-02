@echo off
cd bin
start /min AuthServer.exe
timeout 2
start /min launcher.exe -noversioncheck
timeout 2
start /min dbserver.exe
REM moved the remaining servers into loadBalanceShardSpecific.cfg
