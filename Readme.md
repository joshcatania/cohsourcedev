# City of Heroes
A consolidated effort to get City of Heroes in development once again!

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. The setup guide describes what we call a fakeAuth setup, meaning no authentication server will be spun up but instead the database server (for short dbserver) will be configured such as to allow any user to log in. 

## Prerequisites

* [Visual Studio 2010 professional](https://visualstudio.microsoft.com/vs/older-downloads/) or Visual Studio 2019 with the 2010 toolchain configured
* [Git for Windows](https://git-scm.com/download/win)
* [SQL Server 2012](https://www.microsoft.com/en-us/download/details.aspx?id=29062) or later (Express versions are good enough for development)
* [SQL Server Management Studio](https://docs.microsoft.com/en-us/sql/ssms/download-sql-server-management-studio-ssms?view=sql-server-2017)

## Quick Start Guide

> Verify SQL Server is up and running by opening the SQL Server Configuration Manager and check if the MSSQLSERVER Icon in the left panel shows a green arrow overlay.
> Ensure MSBuild.exe has been added to the PATH environment variable. To test if MSBuild is properly set up, open a command line and type
```
msbuild
``` 
> if you receive following error message

```
Microsoft (R) Build Engine version 16.0.461+g6ff56ef63c for .NET Framework
Copyright (C) Microsoft Corporation. All rights reserved.

MSBUILD : error MSB1003: Specify a project or solution file. The current working directory does not contain a project or solution file.
```
> then MSBuild is set up correctly.


> Set up PowerShell's execution policy by opening a PowerShell window as Administrator and type

```
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope LocalMachine
```

> Ensure git.exe is in the search path. Validate it by opening a command prompt and type
```
git
```
> If you see a response similar to
```
usage: git [--version] [--help] [-C <path>] [-c <name>=<value>]
           [--exec-path[=<path>]] [--html-path] [--man-path] [--info-path]
           [-p | --paginate | --no-pager] [--no-replace-objects] [--bare]
           [--git-dir=<path>] [--work-tree=<path>] [--namespace=<name>]
           <command> [<args>]

These are common Git commands used in various situations:

start a working area (see also: git help tutorial)
   clone      Clone a repository into a new directory
   init       Create an empty Git repository or reinitialize an existing one

work on the current change (see also: git help everyday)
   add        Add file contents to the index
   mv         Move or rename a file, a directory, or a symlink
   reset      Reset current HEAD to the specified state
   rm         Remove files from the working tree and from the index

examine the history and state (see also: git help revisions)
   bisect     Use binary search to find the commit that introduced a bug
   grep       Print lines matching a pattern
   log        Show commit logs
   show       Show various types of objects
   status     Show the working tree status

grow, mark and tweak your common history
   branch     List, create, or delete branches
   checkout   Switch branches or restore working tree files
   commit     Record changes to the repository
   diff       Show changes between commits, commit and working tree, etc
   merge      Join two or more development histories together
   rebase     Reapply commits on top of another base tip
   tag        Create, list, delete or verify a tag object signed with GPG

collaborate (see also: git help workflows)
   fetch      Download objects and refs from another repository
   pull       Fetch from and integrate with another repository or a local branch
   push       Update remote refs along with associated objects

'git help -a' and 'git help -g' list available subcommands and some
concept guides. See 'git help <command>' or 'git help <concept>'
to read about a specific subcommand or concept.
```
> then your git is set up proper

> Clone the repository to a local location of your choice by opening a command prompt and create the root directory in which to clone the repository. For example, if you like to clone the repository into D:\Ouroboros\ create the root directory executing following command on the command line:
```
mkdir D:\Ouroboros
```
> then change into the newly created directory using following commands
```
D:
cd Ouroboros
```
> and execute to clone command like
```
git clone --recursive https://git.ourodev.com/CoX/Source.git
```

##### TBD: Add guide on how to install the assets

> After cloning the repository open a PowerShell window if you are not already in one, switch into the MasterSolution subdirectory and execute the build script by typing
```
Build.ps1
```
A build from scratch takes around 40 minutes on an average machine so please be patient.

> Set up a shortcut to start up the City of Heroes client by creating a new shortcut to Ouroboros.exe in D:\\Ouroboros\Source\bin (assuming you used D:\\Ouroboros as you clone root folder)
and add following arguments to Target filed in the shortcut dialog
```
-db 127.0.0.1 -console
```
> so the whole command would look like
```
D:\Ouroboros\Source\bin\Ouroboros.exe -db 127.0.0.1 -console
```

> Start the dbserver by typing or setting up a shortcut to dbserver.exe in D:\\Ouroboros\Source\bin like
```
dbserver
```

> Start the launcher by typing or setting up a shortcut launcher.exe in D:\\Ouroboros\Source\bin like
```
launcher
```

> Start the City of Heroes client by double clicking the shortcut to CityOfHeroes previously set up

> Log in by typing whatever you like when prompted for account name and password

> Set up your test character and enter the game

### Welcome home!

