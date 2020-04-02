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
cmake_minimum_required(VERSION 3.8.0)

# Automatic Versioning
set(VERSION_MAJOR 0)
set(VERSION_MINOR 8)
set(VERSION_PATCH 0)
set(VERSION_TWEAK 0)
set(VERSION_SUFFIX "a3")
set(PROJECT_COMMIT "N/A")
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
	set(GIT_RESULT "")
	set(GIT_OUTPUT "")
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
	execute_process(
		COMMAND git rev-parse --short=8 HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		RESULT_VARIABLE GIT_RESULT
		OUTPUT_VARIABLE GIT_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	if(GIT_RESULT EQUAL 0)
		set(PROJECT_COMMIT ${GIT_OUTPUT})
	endif()
endif()

# Policies
cmake_policy(PUSH)
cmake_policy(SET CMP0074 NEW)

# Define Project
project(
	StreamFX
	VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}
)
set(PROJECT_FULL_NAME "StreamFX for OBS Studio")
set(PROJECT_DESCRIPTION "Better Production Quality, for free.")
set(PROJECT_AUTHORS "Michael Fabian 'Xaymar' Dirks <info@xaymar.com>")
set(PROJECT_COPYRIGHT_YEARS "2018 - 2020")

################################################################################
# Setup / Bootstrap
################################################################################

# Search Path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# CMake Modules
include("util")
if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/Clang/Clang.cmake")
	include("Clang")
	set(HAVE_CLANG ON)
endif()

# Detect Build Type
if("${CMAKE_SOURCE_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
	set(PropertyPrefix "")
else()
	set(PropertyPrefix "${PROJECT_NAME}_")
endif()

# Detect Architecture
math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")
if("${BITS}" STREQUAL "32")
	set(ARCH "x86")
	message(STATUS "StreamFX no longer supports 32-bit builds, you are on your own if you continue.")
else()
	set(ARCH "x64")
endif()

# All Warnings, Extra Warnings, Pedantic
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	# using Clang
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-missing-braces -Wmissing-field-initializers -Wno-c++98-compat-pedantic -Wold-style-cast -Wno-documentation -Wno-documentation-unknown-command -Wno-covered-switch-default -Wno-switch-enum")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-missing-braces -Wmissing-field-initializers -Wno-c++98-compat-pedantic -Wold-style-cast -Wno-documentation -Wno-documentation-unknown-command -Wno-covered-switch-default -Wno-switch-enum")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# GCC: -fpermissive is required as GCC does not allow the same template to be in different namespaces.
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wpedantic -fpermissive -Wno-long-long -Wno-missing-braces -Wmissing-field-initializers")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -fpermissive -Wno-long-long -Wno-missing-braces -Wmissing-field-initializers")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	# using Intel C++
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(MSVC_EXTRA_FLAGS "/wd4061 /wd4100 /wd4180 /wd4201 /wd4464 /wd4505 /wd4514 /wd4571 /wd4623 /wd4625 /wd4626 /wd4668 /wd4710 /wd4774 /wd4820 /wd5026 /wd5027 /wd5039 /wd5045 /wd26812")
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/Wall" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_EXTRA_FLAGS}")
	
	if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/Wall" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	else()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Wall")
	endif()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MSVC_EXTRA_FLAGS}")

	if("${PropertyPrefix}" STREQUAL "")
		# Speed Optimized Configuration
		set(_SPEED_COMPILER_FLAGS "/O2 /Oi /Ot /Oy /GF /GS- /Qpar /arch:AVX /GR /GL")
		set(_SPEED_LINKER_FLAGS "/LTCG:incremental /OPT:ICF=4 /INCREMENTAL /OPT:REF")
		set(CMAKE_CXX_FLAGS_SPEED "${CMAKE_CXX_FLAGS_RELWTIHDEBINFO} ${_SPEED_COMPILER_FLAGS}")
		set(CMAKE_C_FLAGS_SPEED "${CMAKE_C_FLAGS_RELWTIHDEBINFO} ${_SPEED_COMPILER_FLAGS}")
		set(CMAKE_EXE_LINKER_FLAGS_SPEED "${CMAKE_EXE_LINKER_FLAGS_RELWTIHDEBINFO} ${_SPEED_LINKER_FLAGS}")
		set(CMAKE_SHARED_LINKER_FLAGS_SPEED "${CMAKE_EXE_LINKER_FLAGS_RELWTIHDEBINFO} ${_SPEED_LINKER_FLAGS}")
		set(CMAKE_MODULE_LINKER_FLAGS_SPEED "${CMAKE_EXE_LINKER_FLAGS_RELWTIHDEBINFO} ${_SPEED_LINKER_FLAGS}")
		mark_as_advanced(
			CMAKE_CXX_FLAGS_SPEED
			CMAKE_C_FLAGS_SPEED
			CMAKE_EXE_LINKER_FLAGS_SPEED
			CMAKE_SHARED_LINKER_FLAGS_SPEED
			CMAKE_MODULE_LINKER_FLAGS_SPEED
		)

		if(CMAKE_CONFIGURATION_TYPES)
			list(APPEND CMAKE_CONFIGURATION_TYPES Speed)
			list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
			CacheSet(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}")
		endif()
	endif()
endif()

# C++ Standard and Extensions
## Use C++17 and no non-standard extensions.
set(_CXX_STANDARD 17)
set(_CXX_EXTENSIONS OFF)

################################################################################
# Options
################################################################################
set(${PropertyPrefix}OBS_NATIVE FALSE CACHE BOOL "Use native obs-studio build" FORCE)
set(${PropertyPrefix}OBS_REFERENCE FALSE CACHE BOOL "Use referenced obs-studio build" FORCE)
set(${PropertyPrefix}OBS_PACKAGE FALSE CACHE BOOL "Use packaged obs-studio build" FORCE)
set(${PropertyPrefix}OBS_DOWNLOAD FALSE CACHE BOOL "Use downloaded obs-studio build")
mark_as_advanced(FORCE OBS_NATIVE OBS_PACKAGE OBS_REFERENCE OBS_DOWNLOAD)

# Solve OBS_NATIVE, OBS_REFERENCE, OBS_PACKAGE, OBS_DOWNLOAD
if(TARGET libobs)
	message(STATUS "${PROJECT_NAME}: Using native obs-studio.")
	CacheSet(${PropertyPrefix}OBS_NATIVE TRUE)
else()
	message(STATUS "${PROJECT_NAME}: Using packaged or remote obs-studio.")
	CacheSet(${PropertyPrefix}OBS_NATIVE FALSE)

	set(CMAKE_PACKAGE_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Path for generated archives.")
	set(CMAKE_PACKAGE_NAME "${PROJECT_NAME}" CACHE STRING "Name for the generated archives.")
	set(CMAKE_PACKAGE_SUFFIX_OVERRIDE "" CACHE STRING "Override for the suffix.")

	if(${PropertyPrefix}OBS_DOWNLOAD)
		set(${PropertyPrefix}OBS_DOWNLOAD_VERSION "25.0.1" CACHE STRING "OBS Studio Version to download")
		set(${PropertyPrefix}OBS_DEPENDENCIES_VERSION "25.0.0" CACHE STRING "OBS Studio Version to download")
		set(${PropertyPrefix}OBS_DOWNLOAD_URL "https://github.com/Xaymar/obs-studio/releases/download/${OBS_DOWNLOAD_VERSION}-ci/obs-studio-${ARCH}-0.0.0.0-vs2019.7z")
		set(${PropertyPrefix}OBS_DEPENDENCIES_URL "https://cdn.xaymar.com/obs/dependencies_${OBS_DEPENDENCIES_VERSION}.zip")
	else()
		set(${PropertyPrefix}OBS_STUDIO_DIR "" CACHE PATH "OBS Studio Source/Package Directory")
		set(${PropertyPrefix}OBS_DEPENDENCIES_DIR "" CACHE PATH "OBS Studio Dependencies Directory")
		if(EXISTS "${OBS_STUDIO_DIR}/cmake/LibObs/LibObsConfig.cmake")
			message(STATUS "${PROJECT_NAME}: Using packaged obs-studio.")
			CacheSet(${PropertyPrefix}OBS_PACKAGE TRUE)
		elseif(EXISTS "${OBS_STUDIO_DIR}/libobs/obs-module.h")
			message(STATUS "${PROJECT_NAME}: Using referenced obs-studio.")
			CacheSet(${PropertyPrefix}OBS_REFERENCE TRUE)
		else()
			message(FATAL_ERROR "${PROJECT_NAME}: No OBS Studio detected. If you wish to continue, either check ${PropertyPrefix}OBS_DOWNLOAD or fix your configuration")
			return()
		endif()
	endif()
endif()

# Features
set(${PropertyPrefix}ENABLE_ENCODER_FFMPEG TRUE CACHE BOOL "Enable FFmpeg Encoder")
set(${PropertyPrefix}ENABLE_FILTER_BLUR TRUE CACHE BOOL "Enable Blur Filter")
set(${PropertyPrefix}ENABLE_FILTER_COLOR_GRADE TRUE CACHE BOOL "Enable Color Grade Filter")
set(${PropertyPrefix}ENABLE_FILTER_DISPLACEMENT TRUE CACHE BOOL "Enable Displacement Filter")
set(${PropertyPrefix}ENABLE_FILTER_DYNAMIC_MASK TRUE CACHE BOOL "Enable Dynamic Mask Filter")
set(${PropertyPrefix}ENABLE_FILTER_NVIDIA_FACE_TRACKING TRUE CACHE BOOL "Enable NVidia Face Tracking Filter")
set(${PropertyPrefix}ENABLE_FILTER_SDF_EFFECTS TRUE CACHE BOOL "Enable SDF Effects Filter")
set(${PropertyPrefix}ENABLE_FILTER_SHADER TRUE CACHE BOOL "Enable Shader Filter")
set(${PropertyPrefix}ENABLE_FILTER_TRANSFORM TRUE CACHE BOOL "Enable Transform Filter")
set(${PropertyPrefix}ENABLE_SOURCE_MIRROR TRUE CACHE BOOL "Enable Mirror Source")
set(${PropertyPrefix}ENABLE_SOURCE_SHADER TRUE CACHE BOOL "Enable Shader Source")
set(${PropertyPrefix}ENABLE_TRANSITION_SHADER TRUE CACHE BOOL "Enable Shader Transition")

################################################################################
# CMake / Compiler Dependencies
################################################################################

# Configure Version Header
configure_file(
	"cmake/version.hpp.in"
	"${PROJECT_BINARY_DIR}/source/version.hpp"
)
configure_file(
	"cmake/module.cpp.in"
	"${PROJECT_BINARY_DIR}/source/module.cpp"
)

# Packaging
if("${CMAKE_PACKAGE_SUFFIX_OVERRIDE}" STREQUAL "")
	set(_PACKAGE_SUFFIX_OVERRIDE "${PROJECT_VERSION}${VERSION_SUFFIX}-${PROJECT_COMMIT}")
else()
	set(_PACKAGE_SUFFIX_OVERRIDE "${CMAKE_PACKAGE_SUFFIX_OVERRIDE}")
endif()
set(_PACKAGE_FULL_NAME "${CMAKE_PACKAGE_PREFIX}/${CMAKE_PACKAGE_NAME}-${_PACKAGE_SUFFIX_OVERRIDE}")

# Windows
if(WIN32)
	## Installer (InnoSetup)
	get_filename_component(ISS_FILES_DIR "${CMAKE_INSTALL_PREFIX}" ABSOLUTE)
	file(TO_NATIVE_PATH "${ISS_FILES_DIR}" ISS_FILES_DIR)
	get_filename_component(ISS_PACKAGE_DIR "${CMAKE_PACKAGE_PREFIX}" ABSOLUTE)
	file(TO_NATIVE_PATH "${ISS_PACKAGE_DIR}" ISS_PACKAGE_DIR)
	get_filename_component(ISS_SOURCE_DIR "${PROJECT_SOURCE_DIR}" ABSOLUTE)
	file(TO_NATIVE_PATH "${ISS_SOURCE_DIR}" ISS_SOURCE_DIR)
	configure_file(
		"cmake/installer.iss.in"
		"${PROJECT_BINARY_DIR}/installer.iss"
	)

	# Windows Specific Resource Definition
	set(PROJECT_PRODUCT_NAME "${PROJECT_FULL_NAME}")
	set(PROJECT_COMPANY_NAME "${PROJECT_AUTHORS}")
	set(PROJECT_COPYRIGHT "${PROJECT_AUTHORS} © ${PROJECT_COPYRIGHT_YEARS}")
	set(PROJECT_LEGAL_TRADEMARKS_1 "")
	set(PROJECT_LEGAL_TRADEMARKS_2 "")
	
	configure_file(
		"cmake/version.rc.in"
		"${PROJECT_BINARY_DIR}/cmake/version.rc"
		@ONLY
	)	
endif()

# Download OBS Studio and OBS Dependencies
if(${PropertyPrefix}OBS_DOWNLOAD)
	include("DownloadProject")
	
	download_project(
		PROJ libobs
		URL ${OBS_DOWNLOAD_URL}
		UPDATE_DISCONNECTED 1
	)
	
	download_project(
		PROJ obsdeps
		URL ${OBS_DEPENDENCIES_URL}
		UPDATE_DISCONNECTED 1
	)
endif()

# Load OBS Studio & Dependencies
if(${PropertyPrefix}OBS_PACKAGE)
	include("${OBS_STUDIO_DIR}/cmake/LibObs/LibObsConfig.cmake")
elseif(${PropertyPrefix}OBS_REFERENCE)
	set(obsPath "${OBS_STUDIO_DIR}")
	include("${OBS_STUDIO_DIR}/cmake/external/FindLibobs.cmake")
elseif(${PropertyPrefix}OBS_DOWNLOAD)
	include("${libobs_SOURCE_DIR}/cmake/LibObs/LibObsConfig.cmake")
endif()

################################################################################
# Code
################################################################################
set(PROJECT_DATA_LOCALE )
set(PROJECT_DATA_EFFECTS )
set(PROJECT_DATA_SHADERS )
set(PROJECT_LIBRARIES )
set(PROJECT_LIBRARIES_DELAYED )
set(PROJECT_TEMPLATES )
set(PROJECT_PUBLIC )
set(PROJECT_PRIVATE_GENERATED )
set(PROJECT_PRIVATE_SOURCE )
set(PROJECT_DEFINES )

## OBS Studio
if(${PropertyPrefix}OBS_NATIVE)
	list(APPEND PROJECT_LIBRARIES
		libobs
	)
elseif(${PropertyPrefix}OBS_REFERENCE)
	list(APPEND PROJECT_INCLUDE_DIRS
		"${OBS_STUDIO_DIR}/libobs"
	)
	list(APPEND PROJECT_LIBRARIES
		"${LIBOBS_LIB}"
	)
elseif(${PropertyPrefix}OBS_PACKAGE)
	list(APPEND PROJECT_INCLUDE_DIRS
		"${OBS_STUDIO_DIR}/include"
	)
	list(APPEND PROJECT_LIBRARIES
		libobs
	)
elseif(${PropertyPrefix}OBS_DOWNLOAD)
	list(APPEND PROJECT_LIBRARIES
		libobs
	)
endif()

## Data & Source
list(APPEND PROJECT_DATA
	"data/locale/en-US.ini"
	"data/effects/mipgen.effect"
	"data/effects/pack-unpack.effect"
)
list(APPEND PROJECT_TEMPLATES
	"cmake/version.hpp.in"
	"cmake/module.cpp.in"
)
if(WIN32)
	list(APPEND PROJECT_TEMPLATES
		"cmake/installer.iss.in"
		"cmake/version.rc.in"
	)
endif()
list(APPEND PROJECT_PRIVATE_SOURCE
	# Plugin
	"source/common.hpp"
	"source/plugin.hpp"
	"source/plugin.cpp"
	"source/strings.hpp"
	"source/utility.hpp"
	"source/utility.cpp"
	"source/util-event.hpp"
	"source/util-profiler.cpp"
	"source/util-profiler.hpp"
	"source/util-threadpool.cpp"
	"source/util-threadpool.hpp"

	# Graphics
	"source/gfx/gfx-source-texture.hpp"
	"source/gfx/gfx-source-texture.cpp"

	# OBS
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
	"${PROJECT_BINARY_DIR}/source/module.cpp"
	"${PROJECT_BINARY_DIR}/source/version.hpp"
)

## Feature - FFmpeg Encoder
if(${PropertyPrefix}ENABLE_ENCODER_FFMPEG)
	if(NOT ${PropertyPrefix}OBS_NATIVE)
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
	find_package(FFmpeg REQUIRED COMPONENTS avutil avcodec swscale)

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
		"source/encoders/ffmpeg-encoder.hpp"
		"source/encoders/ffmpeg-encoder.cpp"

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
	list(APPEND PROJECT_LIBRARIES
		${FFMPEG_LIBRARIES}
	)
	list(APPEND PROJECT_INCLUDE_DIRS
		${FFMPEG_INCLUDE_DIRS}
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_ENCODER_FFMPEG
	)
endif()

## Feature - Blur Filter
if(${PropertyPrefix}ENABLE_FILTER_BLUR)
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

## Features - Color Grade Filter
if(${PropertyPrefix}ENABLE_FILTER_COLOR_GRADE)
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

## Features - Displacement Filter
if(${PropertyPrefix}ENABLE_FILTER_DISPLACEMENT)
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

## Features - Dynamic Mask Filter
if(${PropertyPrefix}ENABLE_FILTER_DYNAMIC_MASK)
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

## Features - Nvidia Face Tracking Filter
if(${PropertyPrefix}ENABLE_FILTER_NVIDIA_FACE_TRACKING AND WIN32)
	set(NVAR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third-party/nvidia-arsdk")
	find_package(NVAR REQUIRED)

	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-nv-face-tracking.hpp"
		"source/filters/filter-nv-face-tracking.cpp"
		"source/nvidia/ar/nvidia-ar.hpp"
		"source/nvidia/ar/nvidia-ar.cpp"
		"source/nvidia/ar/nvidia-ar-feature.hpp"
		"source/nvidia/ar/nvidia-ar-feature.cpp"
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
	list(APPEND PROJECT_LIBRARIES
		nvARProxy
	)
	list(APPEND PROJECT_INCLUDE_DIRS
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_NVIDIA_FACE_TRACKING
	)
endif()

## Features - SDF Effects Filter
if(${PropertyPrefix}ENABLE_FILTER_SDF_EFFECTS)
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

## Features - Shader Filter
if(${PropertyPrefix}ENABLE_FILTER_SHADER)
	set(REQUIRE_SHADER_CODE ON)
	list(APPEND PROJECT_DATA
		"data/examples/shaders/filter/drunk.effect"
		"data/examples/shaders/filter/example.effect"
		"data/examples/shaders/filter/example_displace_by_texture.effect"
	)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-shader.hpp"
		"source/filters/filter-shader.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_SHADER
	)
endif()

## Features - Transform Filter
if(${PropertyPrefix}ENABLE_FILTER_TRANSFORM)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-transform.hpp"
		"source/filters/filter-transform.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_TRANSFORM
	)
endif()

## Features - Mirror Source
if(${PropertyPrefix}ENABLE_SOURCE_MIRROR)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/sources/source-mirror.hpp"
		"source/sources/source-mirror.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_SOURCE_MIRROR
	)
endif()

## Features - Shader Source
if(${PropertyPrefix}ENABLE_SOURCE_SHADER)
	set(REQUIRE_SHADER_CODE ON)
	list(APPEND PROJECT_DATA
		"data/examples/shaders/source/example.effect"
	)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/sources/source-shader.hpp"
		"source/sources/source-shader.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_SOURCE_SHADER
	)
endif()

## Features - Shader Transition
if(${PropertyPrefix}ENABLE_TRANSITION_SHADER)
	set(REQUIRE_SHADER_CODE ON)
	list(APPEND PROJECT_DATA
	)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/transitions/transition-shader.hpp"
		"source/transitions/transition-shader.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_TRANSITION_SHADER
	)
endif()

## Parts
if(REQUIRE_SHADER_CODE)
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
endif()

set(PROJECT_PRIVATE
	${PROJECT_DATA}
	${PROJECT_PRIVATE_GENERATED}
	${PROJECT_PRIVATE_SOURCE}
	${PROJECT_TEMPLATES}
)

source_group(TREE "${PROJECT_SOURCE_DIR}/data" PREFIX "Data Files" FILES ${PROJECT_DATA})
source_group(TREE "${PROJECT_BINARY_DIR}/source" PREFIX "Generated Files" FILES ${PROJECT_PRIVATE_GENERATED})
source_group(TREE "${PROJECT_SOURCE_DIR}/cmake" PREFIX "Template Files" FILES ${PROJECT_TEMPLATES})

# Filter Sources
set(_TMP_SOURCE ${PROJECT_PRIVATE_SOURCE})
list(FILTER _TMP_SOURCE INCLUDE REGEX "\.(c|cpp)$")
source_group(TREE "${PROJECT_SOURCE_DIR}/source" PREFIX "Source Files" FILES ${_TMP_SOURCE})

# Filter Headers
set(_TMP_HEADER ${PROJECT_PRIVATE_SOURCE})
list(FILTER _TMP_HEADER INCLUDE REGEX "\.(h|hpp)$")
source_group(TREE "${PROJECT_SOURCE_DIR}/source" PREFIX "Header Files" FILES ${_TMP_HEADER})

################################################################################
# Target
################################################################################

add_library(${PROJECT_NAME} MODULE
	${PROJECT_PUBLIC}
	${PROJECT_PRIVATE}
)

# Clang-Format
if(HAVE_CLANG)
	clang_format(
		TARGETS ${PROJECT_NAME}
		DEPENDENCY
		VERSION 9.0.0
	)
endif()

# Include Directories
target_include_directories(${PROJECT_NAME}
	PUBLIC
	PRIVATE
		"${PROJECT_BINARY_DIR}/source"
		"${PROJECT_SOURCE_DIR}/source"
		${PROJECT_INCLUDE_DIRS}
)

# Link Libraries
target_link_libraries(${PROJECT_NAME}
	${PROJECT_LIBRARIES}
)
if(WIN32)
	target_link_libraries(${PROJECT_NAME}
		Delayimp.lib
	)
	foreach(DELAYLOAD ${PROJECT_LIBRARIES_DELAYED})
		get_target_property(_lf ${PROJECT_NAME} LINK_FLAGS)
		if (NOT _lf)
			set(_lf "")
		endif()
		set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${_lf} /DELAYLOAD:${DELAYLOAD}")
	endforeach()
endif()

# Definitions
target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		${PROJECT_DEFINITIONS}
)
if(WIN32)
	target_compile_definitions(${PROJECT_NAME}
		PRIVATE
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
set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD ${_CXX_STANDARD}
		CXX_EXTENSIONS ${_CXX_EXTENSIONS}
)

# File Version
if(WIN32)
	set_target_properties(
		${PROJECT_NAME}
		PROPERTIES
		VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
		SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
	)
else()
	set_target_properties(
		${PROJECT_NAME}
		PROPERTIES
		VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
		SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
	)
endif()

################################################################################
# Installation
################################################################################

if(${PropertyPrefix}OBS_NATIVE)
	install_obs_plugin_with_data(${PROJECT_NAME} data)
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
endif()

cmake_policy(POP)
