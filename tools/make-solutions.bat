@ECHO OFF
SETLOCAL EnableDelayedExpansion
mkdir %~dp0\..\build
CD /D %~dp0\..\build

SET "DEPSPATH=%CD%/deps/obs"

:: Compilers
SET COMPILER#=0
SET /A COMPILER#=COMPILER#+1
SET "COMPILER[%COMPILER#%]=Visual Studio 15 2017"
SET "DISTRIB[%COMPILER#%]=%CD%/vs2017-distrib"
SET "PATH[%COMPILER#%]=%CD%/vs2017-32"
SET "TOOLSET[%COMPILER#%]="
SET /A COMPILER#=COMPILER#+1
SET "COMPILER[%COMPILER#%]=Visual Studio 15 2017 Win64"
SET "DISTRIB[%COMPILER#%]=%CD%/vs2017-distrib"
SET "PATH[%COMPILER#%]=%CD%/vs2017-64"
SET "TOOLSET[%COMPILER#%]=host=x64"

FOR /L %%i IN (1,1,%COMPILER#%) DO (
	ECHO -- BUILD FOR "!COMPILER[%%i]!" --
	mkdir "!PATH[%%i]!"
	pushd "!PATH[%%i]!"
	cmake -G "!COMPILER[%%i]!" -T "!TOOLSET[%%i]!" --target INSTALL -DPATH_OBSStudio="%DEPSPATH%" -DINSTALL_DIR="!DISTRIB[COMPILER[%%i]!" ../../
	popd
)
PAUSE