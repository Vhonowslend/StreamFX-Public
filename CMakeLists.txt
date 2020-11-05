# Experimental new Sources, Filters and Transitions for OBS Studio
# Copyright (C) 2017 - 2018 Michael Fabian Dirks
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

# CMake Setup
cmake_minimum_required(VERSION 3.8...3.12)

# Automatic Versioning
set(VERSION_MAJOR 0)
set(VERSION_MINOR 9)
set(VERSION_PATCH 0)
set(VERSION_TWEAK 0)
set(VERSION_SUFFIX "")
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
	set(GIT_RESULT "")
	set(GIT_OUTPUT "")

	# Get commit index
	execute_process(
		COMMAND git rev-list --count --topo-order ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX}..HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		RESULT_VARIABLE GIT_RESULT
		OUTPUT_VARIABLE GIT_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	if(GIT_RESULT EQUAL 0)
		set(VERSION_TWEAK ${GIT_OUTPUT})
	endif()
	
	# Get commit hash
	execute_process(
		COMMAND git rev-parse --short=8 HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		RESULT_VARIABLE GIT_RESULT
		OUTPUT_VARIABLE GIT_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	if(GIT_RESULT EQUAL 0)
		set(VERSION_COMMIT ${GIT_OUTPUT})
	else()
		set(VERSION_COMMIT "")
	endif()
endif()
if(VERSION_COMMIT)
	set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}${VERSION_SUFFIX}-${VERSION_COMMIT}")
else()
	set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}${VERSION_SUFFIX}")
endif()

# Define Project
project(
	StreamFX
	VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}
)
set(PROJECT_FULL_NAME "StreamFX for OBS Studio")
set(PROJECT_DESCRIPTION "Better Production Quality, for free.")
set(PROJECT_AUTHORS "Michael Fabian 'Xaymar' Dirks <info@xaymar.com>")
set(PROJECT_COPYRIGHT_YEARS "2018 - 2020")

# Internal Configuration
set(OBS_DOWNLOAD_VERSION "26.0.2-ci")
if(WIN32)
	set(OBS_DOWNLOAD_HASH_32 "SHA512=C8CABFAA59BDF5E4CD1C69CBC349F3E62FD6FE37A1A1A8BE4AC1B37BF087F597A313B2B004E019827C43A5951B50957B60578B7F2249383C117E634FD8714844")
	set(OBS_DOWNLOAD_HASH_64 "SHA512=75E83548AD8FD994D45BE2395E97499BED8444C245857C811BA44D35BF3C49186B1187D3EF250F2618295D7AFA7D8ED5A66582BD140A01A46A77F6BC19BDDBE2")
	set(OBS_DEPENDENCIES_VERSION "26.0.0")
	set(OBS_DEPENDENCIES_HASH "SHA512=7545696B5B684E6BF57F11158FBDF7A0477C4C2CBB872070105A400E56ACD16A54934928BB917E8C952631667DB63953B56F8BACB9C52D36285EA3DD83B9F473")
	set(OBS_QT_VERSION "5.10.1")
	set(OBS_QT_HASH "SHA512=848B9AC00B06FCA1F1A85BD4EFEA4138D278E8EC96823C5C36CC988DDE5D27E2F91300B02F2F0E71F075CCB0D791D3C888CDA6A5048DDFE6F946A8697DFEF1E9")
elseif(UNIX AND NOT APPLE)
	set(OBS_DOWNLOAD_HASH_32 "")
	set(OBS_DOWNLOAD_HASH_64 "SHA512=EAB0450F4B3D9C6252C347CD138A4AB49725CF930A742FF6D1EAEADDEDE31627D1C0A45831FE26A19C0B278386A476736CAA170B351EF833E8A5E8DAD6BB57D2")
	# Qt & Dependencies are system packages.
endif()

################################################################################
# Setup / Bootstrap
################################################################################

# Detect Project Type (solo, combined)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
	set(PREFIX "")
else()
	set(PREFIX "${PROJECT_NAME}_")
endif()

# Search Path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# CMake Modules
include("util")
## Clang Integration
if(${PREFIX}ENABLE_CLANG AND (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang/Clang.cmake"))
	include("Clang")
	set(HAVE_CLANG ON)
endif()
## Download Project
include("DownloadProject")

# Detect Platform & Architecture
math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")
if(BITS EQUAL 32)
	set(D_PLATFORM_BITS 32)
	set(ARCH "x86")
elseif(BITS EQUAL 64)
	set(D_PLATFORM_BITS 64)
	set(ARCH "x64")
else()
	message(FATAL_ERROR "Unknown architecture, please file a bug with your exact setup if you believe this to be an error.")
	return()
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(D_PLATFORM_WINDOWS 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(D_PLATFORM_LINUX 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(D_PLATFORM_MAC 1)
else()
	message(FATAL_ERROR "Unknown platform, please file a bug with your exact setup if you believe this to be an error.")
	return()
endif()

################################################################################
# C/C++ Compiler Adjustments
################################################################################
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
	message(STATUS "Applying custom flags for MSVC style build.")

	# MSVC/ClangCL
	# - Dynamically link Microsoft C/C++ Redistributable.
	# - Enable /W3 and disable useless warnings.
	# - Enable C++ exceptions with SEH exceptions.
	# - Enable multi-processor compiling.

	# Build with dynamic MSVC linkage.
    add_compile_options(
        $<$<CONFIG:>:/MD>
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
        $<$<CONFIG:RelWithDebInfo>:/MD>
        $<$<CONFIG:MinSizeRel>:/MD>
    )

	# Enable most useful warnings.
	set(DISABLED_WARNINGS 
		"/wd4061" "/wd4100" "/wd4180" "/wd4201" "/wd4464" "/wd4505" "/wd4514"
		"/wd4571" "/wd4623" "/wd4625" "/wd4626" "/wd4668" "/wd4710" "/wd4774"
		"/wd4820" "/wd5026" "/wd5027" "/wd5039" "/wd5045" "/wd26812"
	)
	add_compile_options("/W3")
	foreach(WARN ${DISABLED_WARNINGS})
		add_compile_options("${WARN}")
	endforeach()

	# C++ Exceptions & SEH
	add_compile_options("/EHa")

	# Multiprocessor compiling
	add_compile_options("/MP")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	message(STATUS "Applying custom flags for GNU style build.")
	
	# Clang/AppleClang/GNU
	# - Don't export by default.
	# - Enable all and extra warnings.
	
	add_compile_options("-Wall")
	add_compile_options("-Wextra")
	add_compile_options("-fvisibility=hidden")
endif()

# C++ Standard and Extensions
## Use C++17 and no non-standard extensions.
set(_CXX_STANDARD 17)
set(_CXX_EXTENSIONS OFF)

################################################################################
# Resolve Plugin Build Type
################################################################################
if(TARGET libobs)
	# Plugin is built together with libOBS.
	message(STATUS "${PROJECT_NAME}: Using native libOBS.")
	set(${PREFIX}OBS_NATIVE TRUE)
	if (TARGET obs-frontend-api)
		set(HAVE_OBS_FRONTEND TRUE)
	endif()
else()
	# Plugin is built by itself.
	message(STATUS "${PROJECT_NAME}: Using downloaded libOBS.")
	if(NOT WIN32)
		message(STATUS "${PROJECT_NAME}: Linux builds require preinstalled development packages. Refer to the manual for more information.")
	endif()
	set(${PREFIX}OBS_NATIVE FALSE)
endif()

################################################################################
# Options
################################################################################

# Configuration by build type
if(${PREFIX}OBS_NATIVE)
else()
	# Packaging
	set(CMAKE_PACKAGE_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Path for generated archives.")
	set(CMAKE_PACKAGE_NAME "${PROJECT_NAME}" CACHE STRING "Name for the generated archives.")
	set(CMAKE_PACKAGE_SUFFIX_OVERRIDE "" CACHE STRING "Override for the suffix.")

	# Unix builds
	if(UNIX)
		set(UNIX_LOCAL_STRUCTURE TRUE CACHE BOOL "Package for a local linux install.")
	endif()
endif()

# Features
## Encoders
set(${PREFIX}ENABLE_ENCODER_FFMPEG TRUE CACHE BOOL "Enable FFmpeg Encoder")

## Filters
set(${PREFIX}ENABLE_FILTER_BLUR TRUE CACHE BOOL "Enable Blur Filter")
set(${PREFIX}ENABLE_FILTER_COLOR_GRADE TRUE CACHE BOOL "Enable Color Grade Filter")
set(${PREFIX}ENABLE_FILTER_DISPLACEMENT TRUE CACHE BOOL "Enable Displacement Filter")
set(${PREFIX}ENABLE_FILTER_DYNAMIC_MASK TRUE CACHE BOOL "Enable Dynamic Mask Filter")
set(${PREFIX}ENABLE_FILTER_NVIDIA_FACE_TRACKING TRUE CACHE BOOL "Enable NVidia Face Tracking Filter")
set(${PREFIX}ENABLE_FILTER_SDF_EFFECTS TRUE CACHE BOOL "Enable SDF Effects Filter")
set(${PREFIX}ENABLE_FILTER_SHADER TRUE CACHE BOOL "Enable Shader Filter")
set(${PREFIX}ENABLE_FILTER_TRANSFORM TRUE CACHE BOOL "Enable Transform Filter")

## Sources
set(${PREFIX}ENABLE_SOURCE_MIRROR TRUE CACHE BOOL "Enable Mirror Source")
set(${PREFIX}ENABLE_SOURCE_SHADER TRUE CACHE BOOL "Enable Shader Source")

## Transitions
set(${PREFIX}ENABLE_TRANSITION_SHADER TRUE CACHE BOOL "Enable Shader Transition")

## FrontEnd / UI
set(${PREFIX}ENABLE_FRONTEND TRUE CACHE BOOL "Enable Frontend code.")

## Code Related
set(${PREFIX}ENABLE_CLANG TRUE CACHE BOOL "Enable Clang integration for supported compilers.")
set(${PREFIX}ENABLE_PROFILING FALSE CACHE BOOL "Enable CPU and GPU performance tracking, which has a non-zero overhead at all times. Do not enable this for release builds.")
set(${PREFIX}ENABLE_UPDATER TRUE CACHE BOOL "Enable automatic update checks.")

# Code Signing
set(${PREFIX}SIGN_ENABLED FALSE CACHE BOOL "Enable signing builds.")
set(${PREFIX}SIGN_KEY "" CACHE FILEPATH "Path to the private key with which to sign.")
set(${PREFIX}SIGN_PASSWORD "" CACHE STRING "Password for the private key.")

################################################################################
# CMake / Compiler Dependencies
################################################################################

# Configure Files
configure_file(
	"templates/config.hpp.in"
	"${PROJECT_BINARY_DIR}/generated/config.hpp"
)
configure_file(
	"templates/version.hpp.in"
	"${PROJECT_BINARY_DIR}/generated/version.hpp"
)
configure_file(
	"templates/module.cpp.in"
	"${PROJECT_BINARY_DIR}/generated/module.cpp"
)
if(WIN32) # Windows Resource Definition
	set(PROJECT_PRODUCT_NAME "${PROJECT_FULL_NAME}")
	set(PROJECT_COMPANY_NAME "${PROJECT_AUTHORS}")
	set(PROJECT_COPYRIGHT "${PROJECT_AUTHORS} © ${PROJECT_COPYRIGHT_YEARS}")
	set(PROJECT_LEGAL_TRADEMARKS_1 "")
	set(PROJECT_LEGAL_TRADEMARKS_2 "")

	configure_file(
		"templates/version.rc.in"
		"${PROJECT_BINARY_DIR}/generated/version.rc"
		@ONLY
	)
endif()

################################################################################
# Code & Dependencies
################################################################################

function(refresh_components)
	# CURL
	if((${PREFIX}ENABLE_UPDATER AND NOT ${PREFIX}DISABLE_UPDATER))
		set(REQUIRE_CURL TRUE PARENT_SCOPE)
	else()
		set(REQUIRE_CURL FALSE PARENT_SCOPE)
	endif()

	# FFmpeg
	if(${PREFIX}ENABLE_ENCODER_FFMPEG AND NOT ${PREFIX}DISABLE_ENCODER_FFMPEG)
		set(REQUIRE_FFMPEG TRUE PARENT_SCOPE)
	else()
		set(REQUIRE_FFMPEG FALSE PARENT_SCOPE)
	endif()

	# JSON
	if((${PREFIX}ENABLE_UPDATER AND NOT ${PREFIX}DISABLE_UPDATER))
		set(REQUIRE_JSON TRUE PARENT_SCOPE)
	else()
		set(REQUIRE_JSON FALSE PARENT_SCOPE)
	endif()

	# OBS Frontend API
	if((${PREFIX}ENABLE_FRONTEND AND NOT ${PREFIX}DISABLE_FRONTEND))
		set(REQUIRE_OBSFE TRUE PARENT_SCOPE)
	else()
		set(REQUIRE_OBSFE FALSE PARENT_SCOPE)
	endif()

	# Qt
	if((${PREFIX}ENABLE_FRONTEND AND NOT ${PREFIX}DISABLE_FRONTEND))
		set(REQUIRE_QT TRUE PARENT_SCOPE)
	else()
		set(REQUIRE_QT FALSE PARENT_SCOPE)
	endif()
	
	# NVIDIA Broadcast/Augmented Reality
	if((${PREFIX}ENABLE_FILTER_NVIDIA_FACE_TRACKING AND NOT ${PREFIX}DISABLE_FILTER_NVIDIA_FACE_TRACKING))
		set(REQUIRE_NVAR TRUE PARENT_SCOPE)
	else()
		set(REQUIRE_NVAR FALSE PARENT_SCOPE)
	endif()

	# NVIDIA CUDA
	if((${PREFIX}ENABLE_FILTER_NVIDIA_FACE_TRACKING AND NOT ${PREFIX}DISABLE_FILTER_NVIDIA_FACE_TRACKING))
		set(REQUIRE_NVCUDA TRUE PARENT_SCOPE)
	else()
		set(REQUIRE_NVCUDA FALSE PARENT_SCOPE)
	endif()
endfunction()

function(resolve_components)
	# Resolve features by dependencies
	if(${PREFIX}ENABLE_ENCODER_FFMPEG AND HAVE_FFMPEG)
		set(${PREFIX}DISABLE_ENCODER_FFMPEG FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_ENCODER_FFMPEG TRUE PARENT_SCOPE)
		if(${PREFIX}ENABLE_ENCODER_FFMPEG)
			message(WARNING "Encoder 'FFmpeg' requires FFmpeg.")
		endif()
	endif()

	if(${PREFIX}ENABLE_FILTER_BLUR)
		set(${PREFIX}DISABLE_FILTER_BLUR FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_FILTER_BLUR TRUE PARENT_SCOPE)
	endif()
	if(${PREFIX}ENABLE_FILTER_COLOR_GRADE)
		set(${PREFIX}DISABLE_FILTER_COLOR_GRADE FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_FILTER_COLOR_GRADE TRUE PARENT_SCOPE)
	endif()
	if(${PREFIX}ENABLE_FILTER_DISPLACEMENT)
		set(${PREFIX}DISABLE_FILTER_DISPLACEMENT FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_FILTER_DISPLACEMENT TRUE PARENT_SCOPE)
	endif()
	if(${PREFIX}ENABLE_FILTER_DYNAMIC_MASK)
		set(${PREFIX}DISABLE_FILTER_DYNAMIC_MASK FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_FILTER_DYNAMIC_MASK TRUE PARENT_SCOPE)
	endif()
	if(${PREFIX}ENABLE_FILTER_NVIDIA_FACE_TRACKING AND WIN32 AND HAVE_NVCUDA AND HAVE_NVAR)
		set(${PREFIX}DISABLE_FILTER_NVIDIA_FACE_TRACKING FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_FILTER_NVIDIA_FACE_TRACKING TRUE PARENT_SCOPE)
		if(${PREFIX}ENABLE_FILTER_NVIDIA_FACE_TRACKING)
			message(WARNING "Filter 'NVIDIA Face Tracking' requires Windows, NVIDIA CUDA and NVIDIA AR SDK.")
		endif()
	endif()
	if(${PREFIX}ENABLE_FILTER_SDF_EFFECTS)
		set(${PREFIX}DISABLE_FILTER_SDF_EFFECTS FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_FILTER_SDF_EFFECTS TRUE PARENT_SCOPE)
	endif()
	if(${PREFIX}ENABLE_FILTER_SHADER)
		set(${PREFIX}DISABLE_FILTER_SHADER FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_FILTER_SHADER TRUE PARENT_SCOPE)
	endif()

	if(${PREFIX}ENABLE_SOURCE_MIRROR)
		set(${PREFIX}DISABLE_SOURCE_MIRROR FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_SOURCE_MIRROR TRUE PARENT_SCOPE)
	endif()
	if(${PREFIX}ENABLE_SOURCE_SHADER)
		set(${PREFIX}DISABLE_SOURCE_SHADER FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_SOURCE_SHADER TRUE PARENT_SCOPE)
	endif()

	if(${PREFIX}ENABLE_TRANSITION_SHADER)
		set(${PREFIX}DISABLE_TRANSITION_SHADER FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_TRANSITION_SHADER TRUE PARENT_SCOPE)
	endif()

	if(${PREFIX}ENABLE_FRONTEND AND HAVE_QT AND HAVE_OBSFE)
		set(${PREFIX}DISABLE_FRONTEND FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_FRONTEND TRUE PARENT_SCOPE)
		if(${PREFIX}ENABLE_FRONTEND)
			message(WARNING "Front-End requires Qt5 and OBS Frontend API.")
		endif()
	endif()
	if(${PREFIX}ENABLE_PROFILING)
		set(${PREFIX}DISABLE_PROFILING FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_PROFILING TRUE PARENT_SCOPE)
	endif()
	if(${PREFIX}ENABLE_UPDATER AND HAVE_CURL AND HAVE_JSON)
		set(${PREFIX}DISABLE_UPDATER FALSE PARENT_SCOPE)
	else()
		set(${PREFIX}DISABLE_UPDATER TRUE PARENT_SCOPE)
		if(${PREFIX}ENABLE_UPDATER)
			message(WARNING "Updater requires CURL and nlohmann/json")
		endif()
	endif()
endfunction()

# Update Required Parts
refresh_components()

# OBS (Always first)
if(NOT ${PREFIX}OBS_NATIVE)
	if(D_PLATFORM_WINDOWS)
		set(DLSUFFIX "windows${BITS}")
	elseif(D_PLATFORM_LINUX)
		set(DLSUFFIX "ubuntu${BITS}")
	elseif(D_PLATFORM_MAC)
		set(DLSUFFIX "macos${BITS}")
	else()
		message(FATAL_ERROR "Standalone builds are not supported for this platform. Please consider submitting the necessary changes to make them work.")
	endif()

	# Library
	download_project(
		PROJ libobs
		URL https://github.com/Xaymar/obs-studio/releases/download/${OBS_DOWNLOAD_VERSION}/obs-studio-${ARCH}-0.0.0.0-${DLSUFFIX}.7z
		URL_HASH "${OBS_DOWNLOAD_HASH_${ARCH}}"
		DOWNLOAD_NAME "libobs.7z"
		DOWNLOAD_NO_PROGRESS OFF
		UPDATE_DISCONNECTED OFF
	)

	# Dependencies
	if(D_PLATFORM_WINDOWS)
		download_project(
			PROJ obsdeps
			URL https://github.com/Xaymar/obs-studio/releases/download/${OBS_DEPENDENCIES_VERSION}/dependencies.7z
			URL_HASH "${OBS_DEPENDENCIES_HASH}"
			DOWNLOAD_NAME "obsdeps.7z"
			DOWNLOAD_NO_PROGRESS OFF
			UPDATE_DISCONNECTED OFF
		)
	else() # Unix systems use system packages instead.
		message(STATUS "StreamFX: Builds on non-Windows platforms require that you install the necessary packages.")
	endif()

	# Include config file.
	include("${libobs_SOURCE_DIR}/cmake/LibObs/LibObsConfig.cmake")
endif()

# CURL
if(REQUIRE_CURL)
	if(WIN32) # CURL built by OBS Project is not compatible with find modules.
		if(${PREFIX}OBS_NATIVE) # Already defined by OBS
			set(CURL_LIBRARIES "${CURL_LIB}")
			set(CURL_INCLUDE_DIRS "${CURL_INCLUDE_DIR}")
		else()
			set(CURL_LIBRARIES "${obsdeps_SOURCE_DIR}/win${BITS}/bin/libcurl.lib")
			set(CURL_INCLUDE_DIRS "${obsdeps_SOURCE_DIR}/win${BITS}/include")
		endif()
		set(CURL_LIBRARY_DEBUG ${CURL_LIBRARIES})
		set(CURL_LIBRARY_RELEASE ${CURL_LIBRARIES})
		set(CURL_INCLUDE_DIR ${CURL_INCLUDE_DIRS})
		set(CURL_FOUND ON)
	else()
		find_package(CURL)
	endif()

	set(HAVE_CURL ${CURL_FOUND})
else()
	set(HAVE_CURL FALSE)
endif()
if(NOT HAVE_CURL AND REQUIRE_CURL)
	message(WARNING "CURL could not be found.")
endif()

# FFmpeg
if(REQUIRE_FFMPEG)
	if(WIN32 AND NOT ${PREFIX}OBS_NATIVE)
		find_path(
			FFmpegPath "libavcodec/avcodec.h"
			HINTS
				${OBS_DEPENDENCIES_DIR}
				${obsdeps_SOURCE_DIR}
				${DepsPath}
				${DepsPath32}
				${DepsPath64}
			
			PATHS
				/usr/include
				/usr/local/include
				/opt/local/include
				/sw/include
			PATH_SUFFIXES
				win${BITS}
				win${BITS}/bin
				win${BITS}/include
				win${ARCH}
				win${ARCH}/bin
				win${ARCH}/include
				bin
				include
		)
	endif()
	find_package(FFmpeg COMPONENTS avutil avcodec swscale)
	
	set(HAVE_FFMPEG ${FFmpeg_FOUND})
else()
	set(HAVE_FFMPEG FALSE)
endif()
if(NOT HAVE_FFMPEG AND REQUIRE_FFMPEG)
	message(WARNING "FFmpeg could not be found.")
endif()

# JSON
if(REQUIRE_JSON AND EXISTS "${PROJECT_SOURCE_DIR}/third-party/nlohmann-json/single_include")
	set(JSON_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/third-party/nlohmann-json/single_include")
	set(HAVE_JSON TRUE)
else()
	set(HAVE_JSON FALSE)
endif()
if(NOT HAVE_JSON AND REQUIRE_JSON)
	message(WARNING "nlohmann/json was not found.")
endif()

# Qt
if(REQUIRE_QT)
	if(WIN32 AND NOT ${PREFIX}OBS_NATIVE)
		download_project(
			PROJ qt
			URL https://github.com/Xaymar/obs-studio/releases/download/${OBS_DEPENDENCIES_VERSION}/qt_${OBS_QT_VERSION}.7z
			URL_HASH "${OBS_QT_HASH}"
			DOWNLOAD_NAME "qt.7z"
			DOWNLOAD_NO_PROGRESS ON
			UPDATE_DISCONNECTED ON
			QUIET
		)

		set(Qt5_DIR "${qt_SOURCE_DIR}" CACHE STRING "Path to Qt5")
		if("${BITS}" STREQUAL "32")
			CacheSet(Qt5_DIR "${qt_SOURCE_DIR}/msvc2017/lib/cmake/Qt5/")
		else()
			CacheSet(Qt5_DIR "${qt_SOURCE_DIR}/msvc2017_64/lib/cmake/Qt5/")
		endif()
	endif()

	find_package(Qt5 COMPONENTS Core Widgets)

	set(HAVE_QT ${Qt5_FOUND})
else()
	set(HAVE_QT FALSE)
endif()
if(NOT HAVE_QT AND REQUIRE_QT)
	message(WARNING "Qt5 was not found.")
endif()

# OBS Frontend API
if(REQUIRE_OBSFE)
	if(NOT ${PREFIX}OBS_NATIVE)
		if (EXISTS "${libobs_SOURCE_DIR}/cmake/obs-frontend-api/obs-frontend-apiConfig.cmake")
			include("${libobs_SOURCE_DIR}/cmake/obs-frontend-api/obs-frontend-apiConfig.cmake")
			set(HAVE_OBSFE TRUE)
		else()
			set(HAVE_OBSFE FALSE)
		endif()
	else()
		if(TARGET obs-frontend-api)
			set(HAVE_OBSFE TRUE)
		else()
			set(HAVE_OBSFE FALSE)
		endif()
	endif()
endif()
if(NOT HAVE_OBSFE AND REQUIRE_OBSFE)
	message(WARNING "obs-frontend-api was as not found.")
endif()

# NVIDIA CUDA
if(REQUIRE_NVCUDA)
	# CUDA is provided by the NVIDIA Driver, we just load it dynamically.
	set(HAVE_NVCUDA TRUE)
else()
	set(HAVE_NVCUDA FALSE)
endif()
if(NOT HAVE_NVCUDA AND REQUIRE_NVCUDA)
	message(WARNING "NVIDIA CUDA was as not found.")
endif()

# NVIDIA Broadcast / Augmented Reality
if(REQUIRE_NVAR AND EXISTS "${PROJECT_SOURCE_DIR}/third-party/nvidia-arsdk/version.h")
	set(NVAR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third-party/nvidia-arsdk")
	find_package(NVAR)

	set(HAVE_NVAR ${NVAR_FOUND})
else()
	set(HAVE_NVAR FALSE)
endif()
if(NOT HAVE_NVAR AND REQUIRE_NVAR)
	message(WARNING "NVIDIA AR was as not found.")
endif()

# Update selected Components
resolve_components()
refresh_components()

################################################################################
# Code
################################################################################
set(PROJECT_DATA_LOCALE )
set(PROJECT_DATA_EFFECTS )
set(PROJECT_DATA_SHADERS )
set(PROJECT_LIBRARIES )
set(PROJECT_LIBRARIES_DELAYED )
set(PROJECT_INCLUDE_DIRS )
set(PROJECT_TEMPLATES )
set(PROJECT_PRIVATE_GENERATED )
set(PROJECT_PRIVATE_SOURCE )
set(PROJECT_UI )
set(PROJECT_UI_SOURCE )
set(PROJECT_DEFINITIONS )

# Dependencies
list(APPEND PROJECT_LIBRARIES libobs)
if(REQUIRE_QT AND HAVE_QT)
	list(APPEND PROJECT_LIBRARIES Qt5::Core Qt5::Widgets)
endif()
if(REQUIRE_OBSFE AND HAVE_OBSFE)
	list(APPEND PROJECT_LIBRARIES obs-frontend-api)
endif()
if(REQUIRE_JSON AND HAVE_JSON)
	list(APPEND PROJECT_INCLUDE_DIRS ${JSON_INCLUDE_DIR})
endif()
if(REQUIRE_CURL AND HAVE_CURL)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/util/util-curl.hpp"
		"source/util/util-curl.cpp"
	)
	list(APPEND PROJECT_LIBRARIES ${CURL_LIBRARY_RELEASE})
	list(APPEND PROJECT_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
endif()
if(REQUIRE_FFMPEG AND HAVE_FFMPEG)
	list(APPEND PROJECT_LIBRARIES
		${FFMPEG_LIBRARIES}
	)
	list(APPEND PROJECT_INCLUDE_DIRS
		${FFMPEG_INCLUDE_DIRS}
	)	
endif()
if(REQUIRE_NVCUDA AND HAVE_NVCUDA)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/nvidia/cuda/nvidia-cuda.hpp"
		"source/nvidia/cuda/nvidia-cuda.cpp"
		"source/nvidia/cuda/nvidia-cuda-context.hpp"
		"source/nvidia/cuda/nvidia-cuda-context.cpp"
		"source/nvidia/cuda/nvidia-cuda-context-stack.hpp"
		"source/nvidia/cuda/nvidia-cuda-context-stack.cpp"
		"source/nvidia/cuda/nvidia-cuda-gs-texture.hpp"
		"source/nvidia/cuda/nvidia-cuda-gs-texture.cpp"
		"source/nvidia/cuda/nvidia-cuda-memory.hpp"
		"source/nvidia/cuda/nvidia-cuda-memory.cpp"
		"source/nvidia/cuda/nvidia-cuda-stream.hpp"
		"source/nvidia/cuda/nvidia-cuda-stream.cpp"
	)
endif()
if(REQUIRE_NVAR AND HAVE_NVAR)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/nvidia/ar/nvidia-ar.hpp"
		"source/nvidia/ar/nvidia-ar.cpp"
		"source/nvidia/ar/nvidia-ar-feature.hpp"
		"source/nvidia/ar/nvidia-ar-feature.cpp"
	)
	list(APPEND PROJECT_LIBRARIES
		nvARProxy
	)
endif()

# Core
list(APPEND PROJECT_PRIVATE_SOURCE
	"source/configuration.hpp"
	"source/configuration.cpp"
	"source/common.hpp"
	"source/strings.hpp"
	"source/plugin.hpp"
	"source/plugin.cpp"
	"source/util/utility.hpp"
	"source/util/utility.cpp"
	"source/util/util-bitmask.hpp"
	"source/util/util-event.hpp"
	"source/util/util-library.cpp"
	"source/util/util-library.hpp"
	"source/util/util-threadpool.cpp"
	"source/util/util-threadpool.hpp"
	"source/gfx/gfx-source-texture.hpp"
	"source/gfx/gfx-source-texture.cpp"
	"source/obs/gs/gs-helper.hpp"
	"source/obs/gs/gs-helper.cpp"
	"source/obs/gs/gs-effect.hpp"
	"source/obs/gs/gs-effect.cpp"
	"source/obs/gs/gs-effect-parameter.hpp"
	"source/obs/gs/gs-effect-parameter.cpp"
	"source/obs/gs/gs-effect-pass.hpp"
	"source/obs/gs/gs-effect-pass.cpp"
	"source/obs/gs/gs-effect-technique.hpp"
	"source/obs/gs/gs-effect-technique.cpp"
	"source/obs/gs/gs-indexbuffer.hpp"
	"source/obs/gs/gs-indexbuffer.cpp"
	"source/obs/gs/gs-limits.hpp"
	"source/obs/gs/gs-mipmapper.hpp"
	"source/obs/gs/gs-mipmapper.cpp"
	"source/obs/gs/gs-rendertarget.hpp"
	"source/obs/gs/gs-rendertarget.cpp"
	"source/obs/gs/gs-sampler.hpp"
	"source/obs/gs/gs-sampler.cpp"
	"source/obs/gs/gs-texture.hpp"
	"source/obs/gs/gs-texture.cpp"
	"source/obs/gs/gs-vertex.hpp"
	"source/obs/gs/gs-vertex.cpp"
	"source/obs/gs/gs-vertexbuffer.hpp"
	"source/obs/gs/gs-vertexbuffer.cpp"
	"source/obs/obs-encoder-factory.hpp"
	"source/obs/obs-encoder-factory.cpp"
	"source/obs/obs-signal-handler.hpp"
	"source/obs/obs-signal-handler.cpp"
	"source/obs/obs-source.hpp"
	"source/obs/obs-source.cpp"
	"source/obs/obs-source-factory.hpp"
	"source/obs/obs-source-factory.cpp"
	"source/obs/obs-source-tracker.hpp"
	"source/obs/obs-source-tracker.cpp"
	"source/obs/obs-tools.hpp"
	"source/obs/obs-tools.cpp"
)
list(APPEND PROJECT_PRIVATE_GENERATED
	"${PROJECT_BINARY_DIR}/generated/module.cpp"
	"${PROJECT_BINARY_DIR}/generated/version.hpp"
	"${PROJECT_BINARY_DIR}/generated/config.hpp"
)
list(APPEND PROJECT_TEMPLATES
	"templates/version.hpp.in"
	"templates/module.cpp.in"
	"templates/config.hpp.in"
)
list(APPEND PROJECT_DATA
	"data/locale/en-US.ini"
	"data/effects/color-conversion.effect"
	"data/effects/mipgen.effect"
	"data/effects/pack-unpack.effect"
)
if(WIN32)
	list(APPEND PROJECT_TEMPLATES
		"templates/version.rc.in"
	)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/windll.cpp"
	)
	list(APPEND PROJECT_PRIVATE_GENERATED
		"${PROJECT_BINARY_DIR}/generated/version.rc"
	)
	if(NOT ${PREFIX}OBS_NATIVE)
		list(APPEND PROJECT_TEMPLATES
			"templates/installer.iss.in"
		)
	endif()
endif()

# Old Compiler Support
if((CMAKE_C_COMPILER_ID STREQUAL "GNU")
	OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	OR (CMAKE_C_COMPILER_ID STREQUAL "Clang")
	OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
		list(APPEND PROJECT_LIBRARIES
			"stdc++fs"
		)
	endif()
endif()

################################################################################
# Components
################################################################################

# Component: Profiling
if(NOT ${PREFIX}DISABLE_PROFILING)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/util/util-profiler.cpp"
		"source/util/util-profiler.hpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_PROFILING
	)
endif()

# Component: Updater
if(NOT ${PREFIX}DISABLE_UPDATER)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/updater.hpp"
		"source/updater.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_UPDATER
	)
endif()

# Component: Encoder/FFmpeg
if(NOT ${PREFIX}DISABLE_ENCODER_FFMPEG)
	list(APPEND PROJECT_PRIVATE_SOURCE
		# FFmpeg
		"source/ffmpeg/avframe-queue.cpp"
		"source/ffmpeg/avframe-queue.hpp"
		"source/ffmpeg/swscale.hpp"
		"source/ffmpeg/swscale.cpp"
		"source/ffmpeg/tools.hpp"
		"source/ffmpeg/tools.cpp"
		"source/ffmpeg/hwapi/base.hpp"
		"source/ffmpeg/hwapi/base.cpp"
		"source/ffmpeg/hwapi/d3d11.hpp"
		"source/ffmpeg/hwapi/d3d11.cpp"
	
		# Encoders
		"source/encoders/encoder-ffmpeg.hpp"
		"source/encoders/encoder-ffmpeg.cpp"

		# Encoders/Codecs
		"source/encoders/codecs/hevc.hpp"
		"source/encoders/codecs/hevc.cpp"
		"source/encoders/codecs/h264.hpp"
		"source/encoders/codecs/h264.cpp"
		"source/encoders/codecs/prores.hpp"
		"source/encoders/codecs/prores.cpp"

		# Encoders/Handlers
		"source/encoders/handlers/handler.hpp"
		"source/encoders/handlers/handler.cpp"
		"source/encoders/handlers/debug_handler.hpp"
		"source/encoders/handlers/debug_handler.cpp"
		"source/encoders/handlers/prores_aw_handler.hpp"
		"source/encoders/handlers/prores_aw_handler.cpp"
		"source/encoders/handlers/nvenc_shared.hpp"
		"source/encoders/handlers/nvenc_shared.cpp"
		"source/encoders/handlers/nvenc_h264_handler.hpp"
		"source/encoders/handlers/nvenc_h264_handler.cpp"
		"source/encoders/handlers/nvenc_hevc_handler.hpp"
		"source/encoders/handlers/nvenc_hevc_handler.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_ENCODER_FFMPEG
	)
endif()

# Component: Filter/Blur
if(NOT ${PREFIX}DISABLE_FILTER_BLUR)
	list(APPEND PROJECT_DATA
		"data/effects/mask.effect"
		"data/effects/blur/box.effect"
		"data/effects/blur/box-linear.effect"
		"data/effects/blur/dual-filtering.effect"
		"data/effects/blur/gaussian.effect"
		"data/effects/blur/gaussian-linear.effect"
	)
	list (APPEND PROJECT_PRIVATE_SOURCE
		"source/gfx/blur/gfx-blur-base.hpp"
		"source/gfx/blur/gfx-blur-base.cpp"
		"source/gfx/blur/gfx-blur-box.hpp"
		"source/gfx/blur/gfx-blur-box.cpp"
		"source/gfx/blur/gfx-blur-box-linear.hpp"
		"source/gfx/blur/gfx-blur-box-linear.cpp"
		"source/gfx/blur/gfx-blur-dual-filtering.hpp"
		"source/gfx/blur/gfx-blur-dual-filtering.cpp"
		"source/gfx/blur/gfx-blur-gaussian.hpp"
		"source/gfx/blur/gfx-blur-gaussian.cpp"
		"source/gfx/blur/gfx-blur-gaussian-linear.hpp"
		"source/gfx/blur/gfx-blur-gaussian-linear.cpp"
		"source/filters/filter-blur.hpp"
		"source/filters/filter-blur.cpp"		
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_BLUR
	)
endif()

# Component: Filter/Color Grade
if(NOT ${PREFIX}DISABLE_FILTER_COLOR_GRADE)
	list(APPEND PROJECT_DATA
		"data/effects/color-grade.effect"
	)
	list (APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-color-grade.hpp"
		"source/filters/filter-color-grade.cpp"	
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_COLOR_GRADE
	)
endif()

# Component: Filter/Displacement
if(NOT ${PREFIX}DISABLE_FILTER_DISPLACEMENT)
	list(APPEND PROJECT_DATA
		"data/effects/displace.effect"
	)
	list (APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-displacement.hpp"
		"source/filters/filter-displacement.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_DISPLACEMENT
	)
endif()

# Component: Filter/Dynamic Mask
if(NOT ${PREFIX}DISABLE_FILTER_DYNAMIC_MASK)
	list(APPEND PROJECT_DATA
		"data/effects/channel-mask.effect"
	)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-dynamic-mask.hpp"
		"source/filters/filter-dynamic-mask.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_DYNAMIC_MASK
	)
endif()

# Component: Filter/NVIDIA Face Tracking
if(NOT ${PREFIX}DISABLE_FILTER_NVIDIA_FACE_TRACKING)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-nv-face-tracking.hpp"
		"source/filters/filter-nv-face-tracking.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_NVIDIA_FACE_TRACKING
	)
endif()

# Component: Filter/SDF Effects
if(NOT ${PREFIX}DISABLE_FILTER_SDF_EFFECTS)
	list(APPEND PROJECT_DATA
		"data/effects/sdf/sdf-producer.effect"
		"data/effects/sdf/sdf-consumer.effect"
	)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-sdf-effects.hpp"
		"source/filters/filter-sdf-effects.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_SDF_EFFECTS
	)
endif()

# Component: Filter/Shader
if(NOT ${PREFIX}DISABLE_FILTER_SHADER)
	set(REQUIRE_SHADER_CODE ON)
	list(APPEND PROJECT_DATA
		"data/examples/shaders/filter/crt-curvature.effect"
		"data/examples/shaders/filter/crt-scanlines.effect"
		"data/examples/shaders/filter/drunk.effect"
		"data/examples/shaders/filter/hexagonize.effect"
		"data/examples/shaders/filter/semiline.effect"
	)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-shader.hpp"
		"source/filters/filter-shader.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_SHADER
	)
endif()

# Component: Filter/Transform
if(NOT ${PREFIX}DISABLE_FILTER_TRANSFORM)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-transform.hpp"
		"source/filters/filter-transform.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_TRANSFORM
	)
endif()

# Component: Source/Mirror
if(NOT ${PREFIX}DISABLE_SOURCE_MIRROR)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/sources/source-mirror.hpp"
		"source/sources/source-mirror.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_SOURCE_MIRROR
	)
endif()

# Component: Source/Shader
if(NOT ${PREFIX}DISABLE_SOURCE_SHADER)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/sources/source-shader.hpp"
		"source/sources/source-shader.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_SOURCE_SHADER
	)
endif()

# Component: Transition/Shader
if(NOT ${PREFIX}DISABLE_TRANSITION_SHADER)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/transitions/transition-shader.hpp"
		"source/transitions/transition-shader.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_TRANSITION_SHADER
	)
endif()

# Component: Frontend
if(NOT ${PREFIX}DISABLE_FRONTEND)
	list(APPEND PROJECT_UI
		"ui/streamfx.qrc"
		"ui/about.ui"
		"ui/about-entry.ui"
	)
	list(APPEND PROJECT_UI_SOURCE
		"source/ui/ui-common.hpp"
		"source/ui/ui.hpp"
		"source/ui/ui.cpp"
		"source/ui/ui-about.hpp"
		"source/ui/ui-about.cpp"
		"source/ui/ui-about-entry.hpp"
		"source/ui/ui-about-entry.cpp"
	)
	list(APPEND PROJECT_INCLUDE_DIRS
		"source/ui"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FRONTEND
	)
	
	if(NOT ${PREFIX}DISABLE_UPDATER)
		list(APPEND PROJECT_UI_SOURCE
			"source/ui/ui-updater.hpp"
			"source/ui/ui-updater.cpp"
		)
		list(APPEND PROJECT_UI
			"ui/updater.ui"
		)
	endif()
endif()

# Extra Parts
if(NOT ${PREFIX}DISABLE_FILTER_SHADER OR NOT ${PREFIX}DISABLE_SOURCE_SHADER OR NOT ${PREFIX}DISABLE_TRANSITION_SHADER)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/gfx/shader/gfx-shader.hpp"
		"source/gfx/shader/gfx-shader.cpp"
		"source/gfx/shader/gfx-shader-param.hpp"
		"source/gfx/shader/gfx-shader-param.cpp"
		"source/gfx/shader/gfx-shader-param-audio.hpp"
		"source/gfx/shader/gfx-shader-param-audio.cpp"
		"source/gfx/shader/gfx-shader-param-basic.hpp"
		"source/gfx/shader/gfx-shader-param-basic.cpp"
		"source/gfx/shader/gfx-shader-param-matrix.hpp"
		"source/gfx/shader/gfx-shader-param-matrix.cpp"
		"source/gfx/shader/gfx-shader-param-texture.hpp"
		"source/gfx/shader/gfx-shader-param-texture.cpp"
	)
	list(APPEND PROJECT_DATA
		"data/examples/shaders/feature-test.effect"
	)
endif()

# Combine it all
set(PROJECT_FILES
	# Always exists
	${PROJECT_TEMPLATES}
	${PROJECT_PRIVATE_GENERATED}
	${PROJECT_PRIVATE_SOURCE}
	${PROJECT_DATA}
	# UI-only (empty if not enabled)
	${PROJECT_UI}
	${PROJECT_UI_SOURCE}
)

source_group(TREE "${PROJECT_SOURCE_DIR}/data" PREFIX "Data" FILES ${PROJECT_DATA})
source_group(TREE "${PROJECT_SOURCE_DIR}/source" PREFIX "Source" FILES ${PROJECT_PRIVATE_SOURCE} ${PROJECT_UI_SOURCE})
source_group(TREE "${PROJECT_BINARY_DIR}/generated" PREFIX "Source" FILES ${PROJECT_PRIVATE_GENERATED})
source_group(TREE "${PROJECT_SOURCE_DIR}/templates" PREFIX "Templates" FILES ${PROJECT_TEMPLATES})
source_group(TREE "${PROJECT_SOURCE_DIR}/ui" PREFIX "User Interface" FILES ${PROJECT_UI})

if(NOT ${PREFIX}DISABLE_FRONTEND)
	set_source_files_properties(${PROJECT_DATA} ${PROJECT_TEMPLATES} ${PROJECT_PRIVATE_GENERATED} ${PROJECT_PRIVATE_SOURCE} PROPERTIES
		SKIP_AUTOUIC TRUE
		SKIP_AUTOGEN TRUE
	)
endif()
set_source_files_properties(${PROJECT_DATA} ${PROJECT_TEMPLATES} PROPERTIES
	HEADER_FILE_ONLY TRUE
)

################################################################################
# Target
################################################################################

# Set up
add_library(${PROJECT_NAME} MODULE
	${PROJECT_FILES}
)
target_include_directories(${PROJECT_NAME} PRIVATE
	"${PROJECT_BINARY_DIR}/generated"
	"${PROJECT_SOURCE_DIR}/source"
	${PROJECT_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES} )
target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_DEFINITIONS})

# Extra Changes
set_target_properties(${PROJECT_NAME}
	PROPERTIES
		PREFIX ""
		IMPORT_PREFIX ""
)
if(WIN32)
	# /DELAYLOAD MSVC
	target_link_libraries(${PROJECT_NAME}
		Delayimp.lib
	)
	foreach(DELAYLOAD ${PROJECT_LIBRARIES_DELAYED})
		get_target_property(_lf ${PROJECT_NAME} LINK_FLAGS)
		if (NOT _lf)
			set(_lf "")
		endif()
		set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${_lf} /DELAYLOAD:${DELAYLOAD}")
		add_link_options("/DELAYLOAD:${DELAYLOAD}")
	endforeach()

	# Windows.h shrinking
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		_CRT_SECURE_NO_WARNINGS
		_ENABLE_EXTENDED_ALIGNED_STORAGE
		# windows.h
		WIN32_LEAN_AND_MEAN
		NOGPICAPMASKS
		NOVIRTUALKEYCODES
		#NOWINMESSAGES
		NOWINSTYLES
		NOSYSMETRICS
		NOMENUS
		NOICONS
		NOKEYSTATES
		NOSYSCOMMANDS
		NORASTEROPS
		NOSHOWWINDOW
		NOATOM
		NOCLIPBOARD
		NOCOLOR
		#NOCTLMGR
		NODRAWTEXT
		#NOGDI
		NOKERNEL
		#NOUSER
		#NONLS
		NOMB
		NOMEMMGR
		NOMETAFILE
		NOMINMAX
		#NOMSG
		NOOPENFILE
		NOSCROLL
		NOSERVICE
		NOSOUND
		#NOTEXTMETRIC
		NOWH
		NOWINOFFSETS
		NOCOMM
		NOKANJI
		#NOHELP
		NOPROFILER
		NODEFERWINDOWPOS
		NOMCX
		NOIME
		NOMDI
		NOINOUT
	)
endif()

# C++ Standard and Extensions
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD ${_CXX_STANDARD}
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS ${_CXX_EXTENSIONS}
)

# File Version
if(NOT APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
		SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
	)
endif()

# UI
if(NOT ${PREFIX}DISABLE_FRONTEND)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		AUTOUIC TRUE
		AUTOUIC_SEARCH_PATHS "${PROJECT_SOURCE_DIR};${PROJECT_SOURCE_DIR}/ui"
		AUTOMOC TRUE
		AUTORCC TRUE
		AUTOGEN_BUILD_DIR "${PROJECT_BINARY_DIR}/generated"
		AUTOGEN_SOURCE_GROUP "Qt/GEN"
		AUTOMOC_SOURCE_GROUP "Qt/MOC"
		AUTORCC_SOURCE_GROUP "Qt/RCC"
	)
endif()

# Clang Tools
if(${PREFIX}ENABLE_CLANG AND HAVE_CLANG)
	generate_compile_commands_json(
		TARGETS ${PROJECT_NAME}
	)
	clang_tidy(
		TARGETS ${PROJECT_NAME}
		VERSION 9.0.0
	)
	clang_format(
		TARGETS ${PROJECT_NAME}
		DEPENDENCY
		VERSION 9.0.0
	)
endif()

# Signing
if(${PREFIX}SIGN_ENABLED)
	# Investigate: https://github.com/Monetra/mstdlib/blob/master/CMakeModules/CodeSign.cmake
	if(MSVC) 
		find_program(${PREFIX}SIGN_TOOL
			NAMES "signtool"
			DOC "Path to the signing tool."
			REQUIRED
		)
		
		if(${PREFIX}SIGN_TOOL)
			message(STATUS "${PROJECT_NAME}: Signing enabled")
			add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
				COMMAND ${${PREFIX}SIGN_TOOL}
				ARGS sign /p "${${PREFIX}SIGN_PASSWORD}" /f "${${PREFIX}SIGN_KEY}" $<TARGET_FILE:${PROJECT_NAME}>
			)
		endif()
	endif()
endif()

################################################################################
# Installation
################################################################################

if(${PREFIX}OBS_NATIVE)
	install_obs_plugin_with_data(${PROJECT_NAME} data)
else()
	# Packaging
	if("${CMAKE_PACKAGE_SUFFIX_OVERRIDE}" STREQUAL "")
		set(_PACKAGE_SUFFIX_OVERRIDE "${VERSION_STRING}")
	else()
		set(_PACKAGE_SUFFIX_OVERRIDE "${CMAKE_PACKAGE_SUFFIX_OVERRIDE}")
	endif()
	set(_PACKAGE_FULL_NAME "${CMAKE_PACKAGE_PREFIX}/${CMAKE_PACKAGE_NAME}-${_PACKAGE_SUFFIX_OVERRIDE}")

	if(UNIX_LOCAL_STRUCTURE)
		install(
			TARGETS ${PROJECT_NAME}
			RUNTIME DESTINATION "./plugins/${PROJECT_NAME}/bin/${BITS}bit/" COMPONENT Runtime
			LIBRARY DESTINATION "./plugins/${PROJECT_NAME}/bin/${BITS}bit/" COMPONENT Runtime		
		)

		install(
			DIRECTORY "data/"
			DESTINATION "./plugins/${PROJECT_NAME}/data/"
		)
		
		add_custom_target(
			PACKAGE_7Z
			${CMAKE_COMMAND} -E tar cfv "${_PACKAGE_FULL_NAME}.7z" --format=7zip --
				"${CMAKE_INSTALL_PREFIX}/plugins/${PROJECT_NAME}"
			WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
		)
		add_custom_target(
			PACKAGE_ZIP
			${CMAKE_COMMAND} -E tar cfv "${_PACKAGE_FULL_NAME}.zip" --format=zip --
				"${CMAKE_INSTALL_PREFIX}/plugins/${PROJECT_NAME}"
			WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
		)
	else()
		install(
			TARGETS ${PROJECT_NAME}
			RUNTIME DESTINATION "./obs-plugins/${BITS}bit/" COMPONENT Runtime
			LIBRARY DESTINATION "./obs-plugins/${BITS}bit/" COMPONENT Runtime
		)
		if(MSVC)
			install(
				FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
				DESTINATION "./obs-plugins/${BITS}bit/"
				OPTIONAL
			)
		endif()
	
		install(
			DIRECTORY "data/"
			DESTINATION "./data/obs-plugins/${PROJECT_NAME}/"
		)
		
		add_custom_target(
			PACKAGE_7Z
			${CMAKE_COMMAND} -E tar cfv "${_PACKAGE_FULL_NAME}.7z" --format=7zip --
				"${CMAKE_INSTALL_PREFIX}/obs-plugins"
				"${CMAKE_INSTALL_PREFIX}/data"
			WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
		)
		add_custom_target(
			PACKAGE_ZIP
			${CMAKE_COMMAND} -E tar cfv "${_PACKAGE_FULL_NAME}.zip" --format=zip --
				"${CMAKE_INSTALL_PREFIX}/obs-plugins"
				"${CMAKE_INSTALL_PREFIX}/data"
			WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
		)
		
		# Windows
		if(WIN32)
			## Installer (InnoSetup)
			get_filename_component(ISS_FILES_DIR "${CMAKE_INSTALL_PREFIX}" ABSOLUTE)
			file(TO_NATIVE_PATH "${ISS_FILES_DIR}" ISS_FILES_DIR)
			get_filename_component(ISS_PACKAGE_DIR "${CMAKE_PACKAGE_PREFIX}" ABSOLUTE)
			file(TO_NATIVE_PATH "${ISS_PACKAGE_DIR}" ISS_PACKAGE_DIR)
			get_filename_component(ISS_SOURCE_DIR "${PROJECT_SOURCE_DIR}" ABSOLUTE)
			file(TO_NATIVE_PATH "${ISS_SOURCE_DIR}" ISS_SOURCE_DIR)
			get_filename_component(ISS_MSVCHELPER_PATH "${msvc-redist-helper_BUILD_DIR}" ABSOLUTE)
			file(TO_NATIVE_PATH "${ISS_MSVCHELPER_PATH}" ISS_MSVCHELPER_PATH)
			configure_file(
				"templates/installer.iss.in"
				"${PROJECT_BINARY_DIR}/installer.iss"
			)
		endif()
	endif()	
endif()
