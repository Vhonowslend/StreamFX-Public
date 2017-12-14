mkdir build
mkdir build\32
mkdir build\64

IF "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2013" (
	cd build\32
	cmake -G "Visual Studio 12 2013" --target INSTALL -DPATH_OBSStudio="%CD%/deps/obs" -DINSTALL_DIR="../distrib" -DPACKAGE_SUFFIX=".%APPVEYOR_BUILD_VERSION%" -DPACKAGE_PREFIX=""  ../..
	cd ..\64
	cmake -G "Visual Studio 12 2013 Win64" -T host=x64 --target INSTALL -DPATH_OBSStudio="%CD%/deps/obs" -DINSTALL_DIR="../distrib" -DPACKAGE_SUFFIX=".%APPVEYOR_BUILD_VERSION%" -DPACKAGE_PREFIX="" ../..
) ELSE IF "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2015" (
	cd build\32
	cmake -G "Visual Studio 14 2015" --target INSTALL -DPATH_OBSStudio="%CD%/deps/obs" -DINSTALL_DIR="../distrib" -DPACKAGE_SUFFIX=".%APPVEYOR_BUILD_VERSION%" -DPACKAGE_PREFIX="vs2015_"  ../..
	cd ..\64
	cmake -G "Visual Studio 14 2015 Win64" -T host=x64 --target INSTALL -DPATH_OBSStudio="%CD%/deps/obs" -DINSTALL_DIR="../distrib" -DPACKAGE_SUFFIX=".%APPVEYOR_BUILD_VERSION%" -DPACKAGE_PREFIX="vs2015_"  ../..
) ELSE IF "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2017" (
	cd build\32
	cmake -G "Visual Studio 15 2017" --target INSTALL -DPATH_OBSStudio="%CD%/deps/obs" -DINSTALL_DIR="../distrib" -DPACKAGE_SUFFIX=".%APPVEYOR_BUILD_VERSION%" -DPACKAGE_PREFIX="vs2017_"  ../..
	cd ..\64
	cmake -G "Visual Studio 15 2017 Win64" -T host=x64 --target INSTALL -DPATH_OBSStudio="%CD%/deps/obs" -DINSTALL_DIR="../distrib" -DPACKAGE_SUFFIX=".%APPVEYOR_BUILD_VERSION%" -DPACKAGE_PREFIX="vs2017_"  ../..
)
	
cd ..\..