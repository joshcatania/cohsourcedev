@echo off
cd bin
start /min AuthServer.exe
timeout 2
start /min dbserver.exe
timeout 10
start /min launcher.exe -noversioncheck
timeout 2
start /min ChatServer.exe -noreserved
timeout 2
start /min AccountServer.exe -nolastauthor
timeout 2
start /min auctionserver.exe
timeout 2
start /min MissionServer.exe
timeout 2
start /min arenaserver.exe -db 127.0.0.1
timeout 2
start /min raidserver.exe -db 127.0.0.1
timeout 2
start /min statserver.exe -db 127.0.0.1
timeout 2
start /min TurnstileServer.exe
timeout 2
start /min QueueServer.exe -db 127.0.0.1
timeout 2
