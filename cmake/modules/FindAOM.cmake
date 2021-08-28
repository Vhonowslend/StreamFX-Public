# Copyright 2021 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

################################################################################
# Options
################################################################################
set(AOM_PATH "" CACHE PATH "Path to AOM dynamic or static binaries")

################################################################################
# Find Code
################################################################################
find_package(PkgConfig QUIET)

math(EXPR _LIB_SUFFIX "8*${CMAKE_SIZEOF_VOID_P}")

if(PKG_CONFIG_FOUND)
	pkg_check_modules(PC_AOM QUIET aom)
endif()

################################################################################
# Include Dir
################################################################################
find_path(AOM_INCLUDE_DIR
	NAMES
		"aom/aom.h"
	HINTS
		ENV AOM_PATH
		${AOM_PATH}
		${PC_AOM_INCLUDE_DIRS}
	PATHS
		/usr/include
		/usr/local/include
		/opt/local/include
		/sw/include
	PATH_SUFFIXES
		include
)

################################################################################
# Static/Dynamic Library
################################################################################
if(WIN32)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
endif()
find_library(AOM_LIBRARY
	NAMES
		"aom" "libaom"
	HINTS
		ENV AOM_PATH
		${AOM_PATH}
		${PC_AOM_LIBRARY_DIRS}
	PATHS
		/usr/lib
		/usr/local/lib
		/opt/local/lib
		/sw/lib
	PATH_SUFFIXES
		lib${_LIB_SUFFIX} lib
		libs${_LIB_SUFFIX} libs
		bin${_LIB_SUFFIX} bin
)

# Try to find shared binary
if(WIN32)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
	find_library(AOM_BINARY
		NAMES
			"aom" "libaom"
		HINTS
			ENV AOM_PATH
			${AOM_PATH}
			${PC_AOM_LIBRARY_DIRS}
		PATH_SUFFIXES
			bin${_LIB_SUFFIX} bin
			lib${_LIB_SUFFIX} lib
			libs${_LIB_SUFFIX} libs
	)
endif()

################################################################################
# Validation
################################################################################
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AOM
	FOUND_VAR AOM_FOUND
	REQUIRED_VARS AOM_INCLUDE_DIR AOM_LIBRARY
	HANDLE_COMPONENTS)
