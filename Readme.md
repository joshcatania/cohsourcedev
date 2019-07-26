# Project: Ouroboros

![Build Status](https://dev.azure.com/OuroDev/Source/_apis/build/status/Volume%202%20Source?branchName=develop)

## Prerequisite Software

Building requires [Visual Studio 2019](https://visualstudio.microsoft.com/vs/). The Community edition will work fine.

If you just to run a server, you will also need [SQL Server 2017](https://www.microsoft.com/en-us/sql-server/sql-server-2017). Any version at least >= SQL Server 2008 will work fine.  
[SQL Server Management Studio](https://docs.microsoft.com/en-us/sql/ssms/download-sql-server-management-studio-ssms?view=sql-server-2017) should be installed as well to aid in setting up the database.

Download the necessary data archives from [links posted on the wiki](https://wiki.ourodev.com/Magnet_Links). Specifically "Volume 2 Issue 1.1 Server Data" and "Volume 2 Issue 1.1 Server Piggs".

## Downloading a Pre-built Archive

Build pipelines are triggered upon every commit to `master` and `develop`. You can find their output at [https://build.ourodev.com](build.ourodev.com).  
Skip to #2 if you use this option.

## 1) Building from Source

- Open `build/vs2019/master.sln`. Build against your chosen target platform and configuration.

## 2) Setting up SQL Server

- Enable TCP/IP in SQL Configuration Manager.
- Restart the SQL Server service.
- Create the necessary databases in SSMS using the schemas from `Assets/DBSchemas/`.
- If using an AuthServer, set up a File DSN with permissions to `cohauth`.

## 3) Running the Servers

Navigate to the `bin/` directory.
- Copy `Assets/ConfigFiles/*` here.
- Extract `data-v2i1.1.7z` and `piggs-v2i1.1.7z` here.
- Edit all server configuration files in `data/server/db/`. You should update any SqlLogin entries and define any server-specific variables you wish to use
- Run `MapServer.exe -productionmode -templates`
- If using an AuthServer, run `AuthServer.exe`
- Run `DBServer.exe -startall`.
- Run `Launcher.exe`.

Once all the services are up you should be able to connect using `Ouroboros.exe`. Keep an eye on things using ServerMonitor.

And most importantly, defer to [the wiki](https://wiki.ourodev.com) if you need in-depth instructions :)

<a href="https://wiki.ourodev.com/OuroDev_Discord"><img style="vertical-align:middle" src="https://discordapp.com/assets/fc0b01fe10a0b8c602fb0106d8189d9b.png" alt="Discord" height="30px" /></a>
