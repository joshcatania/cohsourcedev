@echo off
cd bin
start /min AuthServer.exe
timeout 2
start ServerMonitor.exe
