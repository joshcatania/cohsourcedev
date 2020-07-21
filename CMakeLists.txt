# City of Heroes/City of Villains - Game client
#
# Build the game client without dependencies on Win32 or VisualStudio.
#
# Build instructions:
#
# 1) mkdir build
# 2) cd build
# 3) cmake -DCMAKE_BUILD_TYPE=Release ..
# 4) make
#
# Any build system supported by cmake should work, such as ninja.
#
# On my Linux Mint 20 system, I needed these additional dependencies:
#
# * apt-get install libxmu-dev (for freetype)
# * apt-get reinstall libglu1-mesa-dev (for glew)
#
# All of the other dependencies were already there, but I'm not sure which
# ones need to be added to a "bare" system.

cmake_minimum_required (VERSION 3.7)
cmake_policy (SET CMP0054 NEW)

project(cox_portable)

# The game client requires these libraries, which we'll statically link to
# prevent incompatibilities with the game servers.
#
# * 3rdparty\jpeg-9c
# * 3rdparty\jpgdlib-0.93b
# * 3rdparty\libcubemapgen-1.4
# * 3rdparty\lua-5.1.5
# * 3rdparty\nvparse
# * 3rdparty\ogg-1.1.1
# * 3rdparty\vorbis-1.1.0
# * 3rdparty\zlib-1.2.11
# * libs\UtilitiesLib
# * Utilities\StructParser

add_subdirectory(3rdparty/cryptopp-8.3/build/cmake)
add_subdirectory(3rdparty/freetype-2.1.9/build/cmake)
add_subdirectory(3rdparty/glew-2.1.0/build/cmake)

# Original Win64 Debug settings:
#
#   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
#     <ClCompile>
#       <WarningLevel>Level3</WarningLevel>
#       <Optimization>Disabled</Optimization>
#       <SDLCheck>true</SDLCheck>
#       <PreprocessorDefinitions>CLIENT;USEZLIB;_CRT_SECURE_NO_WARNINGS;_CRT_SECURE_NO_DEPRECATE;_CRT_NONSTDC_NO_DEPRECATE;SUBSTANCE_PLATFORM_OGLCG;_WINSOCK_DEPRECATED_NO_WARNINGS;FULLDEBUG;_CRTDBG_MAP_ALLOC;_DEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
#       <ConformanceMode>true</ConformanceMode>
#       <LanguageStandard>stdcpplatest</LanguageStandard>
#       <MultiProcessorCompilation>true</MultiProcessorCompilation>
#       <AdditionalIncludeDirectories>$(ProjectDir)..\..\src;$(ProjectDir)..\..\..\Common;$(ProjectDir)..\..\..\3rdparty\nvdxt;$(ProjectDir)..\..\..\3rdparty\nvcpl;$(ProjectDir)..\..\..\3rdparty\nvperf\inc;$(ProjectDir)..\..\..\3rdparty\cg\include;$(ProjectDir)..\..\..\libs\UtilitiesLib\include;$(ProjectDir)..\..\..\3rdparty\zlib-1.2.11\include;$(ProjectDir)..\..\..\3rdparty\ogg-1.1.1\include;$(ProjectDir)..\..\..\3rdparty\glew-2.1.0\include;$(ProjectDir)..\..\..\3rdparty\vorbis-1.1.0\include;$(ProjectDir)..\..\..\3rdparty\jpeg-9c\include;$(ProjectDir)..\..\..\3rdparty\jpgdlib-0.93b\include;$(ProjectDir)..\..\..\3rdparty\freetype-2.1.9\include;$(ProjectDir)..\..\..\3rdparty\libcubemapgen-1.4\include;$(ProjectDir)..\..\..\3rdparty\PhysX\SDKs\Physics\include;$(ProjectDir)..\..\..\3rdparty\PhysX\SDKs\Cooking\include;$(ProjectDir)..\..\..\3rdparty\PhysX\SDKs\Foundation\include;$(ProjectDir)..\..\..\3rdparty\PhysX\SDKs\PhysXLoader\include;$(ProjectDir)..\..\..\3rdparty\AlienFX SDK\Unmanaged\includes</AdditionalIncludeDirectories>
#     </ClCompile>
#     <Link>
#       <SubSystem>Windows</SubSystem>
#       <GenerateDebugInformation>true</GenerateDebugInformation>
#       <LargeAddressAware>true</LargeAddressAware>
#       <AdditionalDependencies>ddraw.lib;dinput8.lib;dsound.lib;winmm.lib;dxguid.lib;Imm32.lib;%(AdditionalDependencies)</AdditionalDependencies>
#     </Link>
#     <PreBuildEvent>
#       <Command>$(Projectdir)..\..\..\Utilities\StructParser\bin\$(PlatformTarget)\$(Configuration)\StructParser.exe PrjPath=$(ProjectPath);SrcDir=$(ProjectDir)..\..\src\;CommonDir=$(ProjectDir)..\..\..\Common\;OutDir=$(OutDir);IntDir=$(IntDir);Platform=$(Platform);Configuration=$(Configuration);SlnPath=$(SolutionPath)</Command>
#       <Message>Running StructParser</Message>
#     </PreBuildEvent>
#     <PostBuildEvent>
#       <Command>copy /Y $(TargetDir)$(ProjectName).exe $(ProjectDir)..\..\..\bin\Ouroboros_Debug.exe
# copy /Y $(TargetDir)$(ProjectName).pdb $(ProjectDir)..\..\..\bin\Ouroboros_Debug.pdb
# copy /Y $(ProjectDir)..\..\..\3rdparty\cg\bin.x64\cg.dll $(ProjectDir)..\..\..\bin
# copy /Y $(ProjectDir)..\..\..\3rdparty\cg\bin.x64\cgGL.dll $(ProjectDir)..\..\..\bin
# copy /Y $(ProjectDir)..\..\..\3rdparty\PhysX\bin\Shared\*.dll $(ProjectDir)..\..\..\bin
# copy /Y $(ProjectDir)..\..\..\3rdparty\PhysX\bin\Debug\*.dll $(ProjectDir)..\..\..\bin</Command>
#     </PostBuildEvent>
#   </ItemDefinitionGroup>

# Original Win64 Release settings:
#
#   <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
#     <ClCompile>
#       <WarningLevel>Level3</WarningLevel>
#       <Optimization>MaxSpeed</Optimization>
#       <FunctionLevelLinking>true</FunctionLevelLinking>
#       <IntrinsicFunctions>true</IntrinsicFunctions>
#       <SDLCheck>true</SDLCheck>
#       <PreprocessorDefinitions>CLIENT;USEZLIB;_CRT_SECURE_NO_WARNINGS;_CRT_SECURE_NO_DEPRECATE;_CRT_NONSTDC_NO_DEPRECATE;SUBSTANCE_PLATFORM_OGLCG;_WINSOCK_DEPRECATED_NO_WARNINGS;FINAL;NDEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
#       <ConformanceMode>true</ConformanceMode>
#       <LanguageStandard>stdcpplatest</LanguageStandard>
#       <MultiProcessorCompilation>true</MultiProcessorCompilation>
#       <AdditionalIncludeDirectories>$(ProjectDir)..\..\src;$(ProjectDir)..\..\..\Common;$(ProjectDir)..\..\..\3rdparty\nvdxt;$(ProjectDir)..\..\..\3rdparty\nvcpl;$(ProjectDir)..\..\..\3rdparty\nvperf\inc;$(ProjectDir)..\..\..\3rdparty\cg\include;$(ProjectDir)..\..\..\libs\UtilitiesLib\include;$(ProjectDir)..\..\..\3rdparty\zlib-1.2.11\include;$(ProjectDir)..\..\..\3rdparty\ogg-1.1.1\include;$(ProjectDir)..\..\..\3rdparty\glew-2.1.0\include;$(ProjectDir)..\..\..\3rdparty\vorbis-1.1.0\include;$(ProjectDir)..\..\..\3rdparty\jpeg-9c\include;$(ProjectDir)..\..\..\3rdparty\jpgdlib-0.93b\include;$(ProjectDir)..\..\..\3rdparty\freetype-2.1.9\include;$(ProjectDir)..\..\..\3rdparty\libcubemapgen-1.4\include;$(ProjectDir)..\..\..\3rdparty\PhysX\SDKs\Physics\include;$(ProjectDir)..\..\..\3rdparty\PhysX\SDKs\Cooking\include;$(ProjectDir)..\..\..\3rdparty\PhysX\SDKs\Foundation\include;$(ProjectDir)..\..\..\3rdparty\PhysX\SDKs\PhysXLoader\include;$(ProjectDir)..\..\..\3rdparty\AlienFX SDK\Unmanaged\includes</AdditionalIncludeDirectories>
#     </ClCompile>
#     <Link>
#       <SubSystem>Windows</SubSystem>
#       <EnableCOMDATFolding>true</EnableCOMDATFolding>
#       <OptimizeReferences>true</OptimizeReferences>
#       <GenerateDebugInformation>true</GenerateDebugInformation>
#       <LargeAddressAware>true</LargeAddressAware>
#       <AdditionalDependencies>ddraw.lib;dinput8.lib;dsound.lib;winmm.lib;dxguid.lib;Imm32.lib;%(AdditionalDependencies)</AdditionalDependencies>
#     </Link>
#     <PreBuildEvent>
#       <Command>$(Projectdir)..\..\..\Utilities\StructParser\bin\$(PlatformTarget)\$(Configuration)\StructParser.exe PrjPath=$(ProjectPath);SrcDir=$(ProjectDir)..\..\src\;CommonDir=$(ProjectDir)..\..\..\Common\;OutDir=$(OutDir);IntDir=$(IntDir);Platform=$(Platform);Configuration=$(Configuration);SlnPath=$(SolutionPath)</Command>
#       <Message>Running StructParser</Message>
#     </PreBuildEvent>
#     <PostBuildEvent>
#       <Command>copy /Y $(TargetDir)$(ProjectName).exe $(ProjectDir)..\..\..\bin\Ouroboros.exe
# copy /Y $(TargetDir)$(ProjectName).pdb $(ProjectDir)..\..\..\bin\Ouroboros.pdb
# copy /Y $(ProjectDir)..\..\..\3rdparty\cg\bin.x64\cg.dll $(ProjectDir)..\..\..\bin
# copy /Y $(ProjectDir)..\..\..\3rdparty\cg\bin.x64\cgGL.dll $(ProjectDir)..\..\..\bin
# copy /Y $(ProjectDir)..\..\..\3rdparty\PhysX\bin\Shared\*.dll $(ProjectDir)..\..\..\bin
# copy /Y $(ProjectDir)..\..\..\3rdparty\PhysX\bin\Release\*.dll $(ProjectDir)..\..\..\bin</Command>
#     </PostBuildEvent>
#   </ItemDefinitionGroup>
