# Nvidia AR SDK
# 
# Sets
# - NVAR_FOUND
# - NVAR_INCLUDE_DIRS
# - NVAR_SOURCE_DIRS
#
#

include(FindPackageHandleStandardArgs)
find_package(PkgConfig QUIET)

# Variables
set(NVAR_ROOT "" CACHE PATH "Path to NVidia AR SDK")

find_path(NVAR_INCLUDE_DIRS
	NAMES
		"nvAR.h" "nvAR_defs.h"
	HINTS
		ENV NVAR_ROOT
		${NVAR_ROOT}
	PATHS
		/usr/include
		/usr/local/include
		/opt/local/include
	PATH_SUFFIXES
		include
		nvar/include
)
find_path(NVAR_SOURCE_DIRS
	NAMES
		"nvARProxy.cpp"
	HINTS
		ENV NVAR_ROOT
		${NVAR_ROOT}
	PATHS
		/usr/include
		/usr/local/include
		/opt/local/include
	PATH_SUFFIXES
		src
		nvar/src
)

find_package_handle_standard_args(NVAR
	FOUND_VAR NVAR_FOUND
	REQUIRED_VARS NVAR_INCLUDE_DIRS NVAR_SOURCE_DIRS
	VERSION_VAR NVAR_VERSION
	HANDLE_COMPONENTS
)

if(NVAR_FOUND AND NOT TARGET nvARProxy)
	add_library(nvARProxy INTERFACE)
	target_include_directories(nvARProxy
		INTERFACE
			${NVAR_SOURCE_DIRS}
			${NVAR_INCLUDE_DIRS}
	)
endif()
