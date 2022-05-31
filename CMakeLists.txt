# Copyright (C) 2017 Michael Fabian Dirks
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
cmake_minimum_required(VERSION 3.13...4.0)

if(${CMAKE_VERSION} VERSION_LESS 3.19)
	set(LOGPREFIX "[StreamFX]")
else()
	list(APPEND CMAKE_MESSAGE_INDENT "[StreamFX]")
	set(LOGPREFIX "")
endif()

################################################################################
# Detect if we are building with OBS Studio (different from Grouped builds)
################################################################################

set(STANDALONE ON)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_LIST_DIR}")
	set(GROUPED OFF)
	set(PREFIX "")
else()
	set(GROUPED ON)
	set(PREFIX "StreamFX_")	
	if(GROUPED AND (TARGET libobs))
		set(STANDALONE OFF)
	endif()
endif()

if(STANDALONE)
	message(STATUS "${LOGPREFIX} This is a standalone build, please make sure you've followed the instructions.")
else()
	message(STATUS "${LOGPREFIX} This is a combined build.")
endif()

################################################################################
# Versioning
################################################################################

set(VERSION_MAJOR 0)
set(VERSION_MINOR 12)
set(VERSION_PATCH 0)
set(VERSION_TWEAK 0)
set(VERSION_SUFFIX "a1")
set(VERSION_COMMIT "00000000")

# Check if we are in a git repository.
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
	# Try and figure out where git is.
	find_program(GIT git
		PATHS
			/bin
			/sbin
			/usr/bin
			/usr/local/bin
	)

	if(GIT)
		set(GIT_RESULT)
		set(GIT_OUTPUT)
		set(GIT_ERROR)

		execute_process(
			COMMAND "${GIT}" describe --tags --long --match "[0-9]*.[0-9]*.[0-9]*" --abbrev=8 HEAD
			WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
			RESULT_VARIABLE GIT_RESULT
			OUTPUT_VARIABLE GIT_OUTPUT
			ERROR_VARIABLE GIT_ERROR
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_STRIP_TRAILING_WHITESPACE
			ERROR_QUIET
		)

		if(GIT_RESULT EQUAL 0)
			string(REPLACE "-" "." GIT_OUTPUT "${GIT_OUTPUT}")
			string(REPLACE "." ";" GIT_OUTPUT "${GIT_OUTPUT}")

			# Parse Version
			list(GET GIT_OUTPUT 0 VERSION_MAJOR)
			list(GET GIT_OUTPUT 1 VERSION_MINOR)
			list(GET GIT_OUTPUT 2 VERSION_PATCH)
			list(GET GIT_OUTPUT 3 VERSION_TWEAK)
			list(GET GIT_OUTPUT 4 VERSION_COMMIT)

			# Patch needs additional parsing.
			# This may be a [0-9]*[a-z]*[0-9]+ string.
			string(REGEX MATCHALL "^([0-9]+)([a-z]+[0-9]+)?" T_MATCHES "${VERSION_PATCH}")
			set(VERSION_PATCH "${CMAKE_MATCH_1}")
			if(CMAKE_MATCH_2)
				set(VERSION_SUFFIX "${CMAKE_MATCH_2}")
			else()
				set(VERSION_SUFFIX "")
			endif()
		else()
			message(WARNING "${LOGPREFIX} Failed to detect version, using default instead.")
		endif()
	endif()
else()
	message(STATUS "${LOGPREFIX} Not a git repository, automatic version detection disabled.")
endif()

# Allow manual overrides of the detected version.
set(${PREFIX}VERSION "" CACHE STRING "Override StreamFX version with this string. Format: Major.Minor.Patch[Suffix][-Tweak[-Commit8c]]")
if(NOT (${PREFIX}VERSION STREQUAL ""))
	string(REPLACE "-" "." T_VERSION "${${PREFIX}VERSION}")
	string(REPLACE "." ";" T_VERSION "${${PREFIX}VERSION}")

	list(LENGTH T_VERSION T_VERSIONLEN)
	list(GET T_VERSION 0 VERSION_MAJOR)
	list(GET T_VERSION 1 VERSION_MINOR)
	list(GET T_VERSION 2 VERSION_PATCH)
	if (T_VERSIONLEN GREATER_EQUAL 3)
		list(GET T_VERSION 3 VERSION_TWEAK)
	else()
		set(VERSION_BUILD 0)
	endif()
	if (T_VERSIONLEN GREATER_EQUAL 4)
		list(GET T_VERSION 4 VERSION_COMMIT)
	else()
		set(VERSION_COMMIT "")
	endif()

	# Patch needs additional parsing.
	# This may be a [0-9]*[a-z]*[0-9]+ string.
	string(REGEX MATCHALL "^([0-9]+)([a-z]+[0-9]+)?" T_MATCHES "${VERSION_PATCH}")
	set(VERSION_PATCH "${CMAKE_MATCH_1}")
	if(CMAKE_MATCH_2)
		set(VERSION_SUFFIX "${CMAKE_MATCH_2}")
	else()
		set(VERSION_SUFFIX "")
	endif()
endif()

# Generate Version String
if(NOT (VERSION_COMMIT STREQUAL ""))
	set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}${VERSION_SUFFIX}-${VERSION_COMMIT}")
else()
	set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}${VERSION_SUFFIX}")
endif()

# Log the detected version.
message(STATUS "${LOGPREFIX} Version ${VERSION_STRING}")

################################################################################
# Project
################################################################################
project(
	StreamFX
	VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}
	DESCRIPTION "Additional sources, filters, transitions and encoders for OBS Studio."
	HOMEPAGE_URL "https://streamfx.xaymar.com/"
)

# Full Project Name
set(PROJECT_FULL_NAME "StreamFX (for OBS Studio)")

# Description
set(PROJECT_DESCRIPTION "Better Production Quality, for free.")

# Authors (TODO: Generate this from AUTHORS)
set(PROJECT_AUTHORS "Michael Fabian 'Xaymar' Dirks <info@xaymar.com>")

# Copyright Years (TODO: Update every year)
set(PROJECT_COPYRIGHT_YEARS "2018 - 2020")

# Versioning
set(PROJECT_VERSION_STRING ${VERSION_STRING})

################################################################################
# Modules
################################################################################

# Search Paths
set(CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

# Include
include("util")							# CacheClear, CacheSet
include("DownloadProject")				# DownloadProject
include("CheckIPOSupported")			# check_ipo_supported

################################################################################
# Platform Setup
################################################################################

# Operating System
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(D_PLATFORM_OS "windows")
	set(D_PLATFORM_WINDOWS 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(D_PLATFORM_OS "linux")
	set(D_PLATFORM_LINUX 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(D_PLATFORM_OS "macos")
	set(D_PLATFORM_MAC 1)
else()
	set(D_PLATFORM_OS "unknown")
	set(D_PLATFORM_UNKNOWN 1)
	message(WARNING "${LOGPREFIX} The operating system '${CMAKE_SYSTEM_NAME}' is unknown to to this script, continue at your own risk.")
endif()

# Architecture
set(ARCH_INSTR_32 "i386;i686;x86;arm;ARM")
set(ARCH_INSTR_64 "x86_64;AMD64;IA64;arm64;ARM64")
set(ARCH_INSTR_X86 "i386;i686;x86;x86_64;AMD64")
set(ARCH_INSTR_ARM "arm;ARM;arm64;ARM64")
set(ARCH_INSTR_ITANIUM "IA64")
set(ARCH_BITS 0)
set(ARCH_BITS_POINTER 0)
set(ARCH_INST "")

# - Bitness
list(FIND ARCH_INSTR_32 "${CMAKE_SYSTEM_PROCESSOR}" FOUND)
if(FOUND GREATER -1)
	set(ARCH_BITS 32)
endif()

list(FIND ARCH_INSTR_64 "${CMAKE_SYSTEM_PROCESSOR}" FOUND)
if(FOUND GREATER -1)
	set(ARCH_BITS 64)
endif()
set(D_PLATFORM_BITS ${ARCH_BITS})

# - Pointer Size (bits)
math(EXPR ARCH_BITS_POINTER "8*${CMAKE_SIZEOF_VOID_P}")
set(D_PLATFORM_BITS_PTR ${ARCH_BITS_POINTER})

# - Basic Instruction Set
list(FIND ARCH_INSTR_X86 "${CMAKE_SYSTEM_PROCESSOR}" FOUND)
if(FOUND GREATER -1)
	list(APPEND ARCH_INST "x86")
	set(D_PLATFORM_INSTR_X86 ON)
	set(D_PLATFORM_ARCH_X86 ON)
endif()

list(FIND ARCH_INSTR_ARM "${CMAKE_SYSTEM_PROCESSOR}" FOUND)
if(FOUND GREATER -1)
	list(APPEND ARCH_INST "ARM")
	set(D_PLATFORM_INSTR_ARM ON)
	set(D_PLATFORM_ARCH_ARM ON)
endif()

list(FIND ARCH_INSTR_ITANIUM "${CMAKE_SYSTEM_PROCESSOR}" FOUND)
if(FOUND GREATER -1)
	list(APPEND ARCH_INST "Itanium")
	set(D_PLATFORM_INSTR_ITANIUM ON)
	set(D_PLATFORM_ARCH_ITANIUM ON)
endif()

check_ipo_supported(RESULT D_HAS_IPO)

set(D_PLATFORM_INSTR ${ARCH_INST})
set(D_PLATFORM_ARCH ${ARCH_INST})
message(STATUS "${LOGPREFIX} Taget is ${D_PLATFORM_BITS}bit ${ARCH_INST} with a pointer size of ${D_PLATFORM_BITS_PTR}bit.")

################################################################################
# Options
################################################################################

# Features
## Encoders
set(${PREFIX}ENABLE_ENCODER_FFMPEG ON CACHE BOOL "Enable FFmpeg Encoder integration.")
set(${PREFIX}ENABLE_ENCODER_FFMPEG_AMF ON CACHE BOOL "Enable AMF Encoder in FFmpeg.")
set(${PREFIX}ENABLE_ENCODER_FFMPEG_NVENC ON CACHE BOOL "Enable NVENC Encoder in FFmpeg.")
set(${PREFIX}ENABLE_ENCODER_FFMPEG_PRORES ON CACHE BOOL "Enable ProRes Encoder in FFmpeg.")
set(${PREFIX}ENABLE_ENCODER_FFMPEG_DNXHR ON CACHE BOOL "Enable DNXHR Encoder in FFmpeg.")
set(${PREFIX}ENABLE_ENCODER_AOM_AV1 ON CACHE BOOL "Enable AOM AV1 Encoder.")

## Filters
set(${PREFIX}ENABLE_FILTER_AUTOFRAMING ON CACHE BOOL "Enable Auto-Framing Filter")
set(${PREFIX}ENABLE_FILTER_AUTOFRAMING_NVIDIA ON CACHE BOOL "Enable NVIDIA provider(s) Auto-Framing Filter")
set(${PREFIX}ENABLE_FILTER_BLUR ON CACHE BOOL "Enable Blur Filter")
set(${PREFIX}ENABLE_FILTER_COLOR_GRADE ON CACHE BOOL "Enable Color Grade Filter")
set(${PREFIX}ENABLE_FILTER_DENOISING ON CACHE BOOL "Enable Denoising filter")
set(${PREFIX}ENABLE_FILTER_DENOISING_NVIDIA ON CACHE BOOL "Enable NVIDIA provider(s) for Denoising Filter")
set(${PREFIX}ENABLE_FILTER_DISPLACEMENT ON CACHE BOOL "Enable Displacement Filter")
set(${PREFIX}ENABLE_FILTER_DYNAMIC_MASK ON CACHE BOOL "Enable Dynamic Mask Filter")
set(${PREFIX}ENABLE_FILTER_SDF_EFFECTS ON CACHE BOOL "Enable SDF Effects Filter")
set(${PREFIX}ENABLE_FILTER_SHADER ON CACHE BOOL "Enable Shader Filter")
set(${PREFIX}ENABLE_FILTER_TRANSFORM ON CACHE BOOL "Enable Transform Filter")
set(${PREFIX}ENABLE_FILTER_UPSCALING ON CACHE BOOL "Enable Upscaling Filter")
set(${PREFIX}ENABLE_FILTER_UPSCALING_NVIDIA ON CACHE BOOL "Enable NVIDIA provider(s) for Upscaling Filter")
set(${PREFIX}ENABLE_FILTER_VIRTUAL_GREENSCREEN ON CACHE BOOL "Enable Virtual Greenscreen Filter")
set(${PREFIX}ENABLE_FILTER_VIRTUAL_GREENSCREEN_NVIDIA ON CACHE BOOL "Enable NVIDIA provider(s) for Virtual Greenscreen Filter")

## Sources
set(${PREFIX}ENABLE_SOURCE_MIRROR ON CACHE BOOL "Enable Mirror Source")
set(${PREFIX}ENABLE_SOURCE_SHADER ON CACHE BOOL "Enable Shader Source")

## Transitions
set(${PREFIX}ENABLE_TRANSITION_SHADER ON CACHE BOOL "Enable Shader Transition")

## FrontEnd & UI
set(${PREFIX}ENABLE_FRONTEND ON CACHE BOOL "Enable Frontend code.")
set(${PREFIX}ENABLE_UPDATER ON CACHE BOOL "Enable automatic update checks.")

## Code Related
set(${PREFIX}ENABLE_CLANG ON CACHE BOOL "Enable Clang integration for supported compilers.")
set(${PREFIX}ENABLE_CODESIGN OFF CACHE BOOL "Enable Code Signing integration for supported environments.")
set(${PREFIX}ENABLE_PROFILING OFF CACHE BOOL "Enable CPU and GPU performance tracking, which has a non-zero overhead at all times. Do not enable this for release builds.")

## Compile/Link Related
set(${PREFIX}ENABLE_LTO ${D_HAS_IPO} CACHE BOOL "Enable Link Time Optimization for faster and smaller binaries.")

# Installation / Packaging
if(STANDALONE)
	set(STRUCTURE_UNIFIED CACHE BOOL "Install for use in a Plugin Manager")
	if(D_PLATFORM_LINUX)
		set(STRUCTURE_PACKAGEMANAGER CACHE BOOL "Install for use in a Package Manager (system-wide installation)")
	endif()

	set(PACKAGE_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Where to place the packages?")
	set(PACKAGE_NAME "${PROJECT_NAME}" CACHE STRING "What should the package be called?")
	set(PACKAGE_SUFFIX "" CACHE STRING "Any suffix for the package name? (Defaults to the current version string)")
endif()

################################################################################
# Clang
################################################################################

if(${PREFIX}ENABLE_CLANG AND (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang/Clang.cmake"))
	include("cmake/clang/Clang.cmake")
	set(HAVE_CLANG ON)
endif()

################################################################################
# Codesign
################################################################################

if(${PREFIX}ENABLE_CODESIGN AND (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/codesign/CodeSign.cmake"))
	include("cmake/codesign/CodeSign.cmake")
	set(HAVE_CODESIGN ON)

	set(${PREFIX}CODESIGN_FILE "" CACHE FILEPATH "Path to Code-Signing certificate.")
	if(WIN32)
		set(${PREFIX}CODESIGN_NAME "" CACHE STRING "Name of Code-Signing certificate in Windows's certificate storage.")
	endif()
	set(${PREFIX}CODESIGN_PASS "" CACHE STRING "Password for Code-Signing certificate.")
	set(${PREFIX}CODESIGN_TIMESTAMPS ON CACHE BOOL "Should the resulting binary be timestamped?")

	set(_CODESIGN_FILE "$ENV{${PREFIX}CODESIGN_FILE}")
	if(${PREFIX}CODESIGN_FILE)
		set(_CODESIGN_FILE "${${PREFIX}CODESIGN_FILE}")
	endif()
	set(_CODESIGN_NAME "$ENV{${PREFIX}CODESIGN_NAME}")
	if(${PREFIX}CODESIGN_NAME)
		set(_CODESIGN_NAME "${${PREFIX}CODESIGN_NAME}")
	endif()
	set(_CODESIGN_PASS "$ENV{${PREFIX}CODESIGN_PASS}")
	if(${PREFIX}CODESIGN_PASS)
		set(_CODESIGN_PASS "${${PREFIX}CODESIGN_PASS}")
	endif()
endif()

################################################################################
# Auto-Dependency: libOBS + obs-frontend-api
################################################################################

if(STANDALONE)
	# Options
	set(${PREFIX}OBS_DOWNLOAD TRUE CACHE BOOL "Automatically download libOBS and obs-frontend-api?")
	set(${PREFIX}OBS_PATH "" CACHE PATH "Path to libOBS, if ${PREFIX}OBS_DOWNLOAD is not set.")
	set(${PREFIX}OBS_DOWNLOAD_URL "" CACHE STRING "The URL from which to download libOBS and obs-frontend-api, if autodetection fails. (Optional)")
	set(${PREFIX}OBS_DOWNLOAD_HASH "" CACHE STRING "The hash string for the given URL. Must be a SHA-256 hash if provided. (Optional)")
	mark_as_advanced(
		${PREFIX}OBS_PATH
		${PREFIX}OBS_DOWNLOAD_URL
		${PREFIX}OBS_DOWNLOAD_HASH
	)

	if(${PREFIX}OBS_DOWNLOAD)
		if(${PREFIX}OBS_DOWNLOAD_URL STREQUAL "")
			# Figure out download URLs and hashes.
			if (D_PLATFORM_WINDOWS)
				if (D_PLATFORM_ARCH_X86)
					if (D_PLATFORM_BITS EQUAL 64)
						set(OBS_DOWNLOAD_URL "https://github.com/Xaymar/obs-studio/releases/download/27.2.4-ci/windows-x86_64.tar.gz")
						set(OBS_DOWNLOAD_HASH "DD931BBB2E0720F1D7573C65D8CC9D638F1EE2AC99F7173BF8935CD3A3BCE3F4")
					endif()
				endif()
			elseif(D_PLATFORM_LINUX)
				if (D_PLATFORM_ARCH_X86)
					if (D_PLATFORM_BITS EQUAL 64)
						set(OBS_DOWNLOAD_URL "https://github.com/Xaymar/obs-studio/releases/download/27.2.4-ci/linux-x86_64.tar.gz")
						set(OBS_DOWNLOAD_HASH "CA19E8260E1A556E6231D75064837C61C6D480BD90C97A0B930005AE527BF625")
					endif()
				endif()
			elseif(D_PLATFORM_MAC)
				if (D_PLATFORM_ARCH_X86)
					if (D_PLATFORM_BITS EQUAL 64)
						set(OBS_DOWNLOAD_URL "https://github.com/Xaymar/obs-studio/releases/download/27.2.4-ci/macos-x86_64.tar.gz")
						set(OBS_DOWNLOAD_HASH "0DD5A57DD537B97CAA8470212F8F6B637F47D4742A5D1F17787F4FE9DBC70B33")
					endif()
				endif()
			endif()

			# Verify that the platform, architecture and bitness is supported.
			if(OBS_DOWNLOAD_URL STREQUAL "")
				message(FATAL_ERROR "${LOGPREFIX} Download for libOBS failed, as Platform '${D_PLATFORM_OS}' with architecture '${D_PLATFORM_ARCH}' and bitness '${D_PLATFORM_BITS}' is not supported.")
				return()
			endif()
		else()
			set(OBS_DOWNLOAD_URL "${${PREFIX}OBS_DOWNLOAD_URL}")
			set(OBS_DOWNLOAD_HASH "${${PREFIX}OBS_DOWNLOAD_HASH}")
		endif()

		if(OBS_DOWNLOAD_HASH STREQUAL "")
			download_project(
				PROJ libobs
				PREFIX "autodeps"
				URL "${OBS_DOWNLOAD_URL}"
				DOWNLOAD_NO_PROGRESS OFF
				UPDATE_DISCONNECTED OFF
			)
		else()
			download_project(
				PROJ libobs
				PREFIX "autodeps"
				URL "${OBS_DOWNLOAD_URL}"
				URL_HASH "SHA256=${OBS_DOWNLOAD_HASH}"
				DOWNLOAD_NO_PROGRESS OFF
				UPDATE_DISCONNECTED OFF
			)
		endif()
		CacheSet(${PREFIX}OBS_PATH "${libobs_SOURCE_DIR}")
	endif()
	
	if(NOT EXISTS "${${PREFIX}OBS_PATH}/cmake/LibObs/LibObsConfig.cmake")
		message(FATAL_ERROR "${LOGPREFIX} The provided path for libOBS is invalid as it did not contain '/cmake/LibObs/LibObsConfig.cmake'.")
		return()
	else()
		include("${${PREFIX}OBS_PATH}/cmake/LibObs/LibObsConfig.cmake")
	endif()

	if (NOT TARGET obs-frontend-api)
		if(EXISTS "${${PREFIX}OBS_PATH}/cmake/obs-frontend-api/obs-frontend-apiConfig.cmake")
			include("${${PREFIX}OBS_PATH}/cmake/obs-frontend-api/obs-frontend-apiConfig.cmake")
		elseif((EXISTS "${${PREFIX}OBS_PATH}/bin/${D_PLATFORM_BITS}bit/obs-frontend-api.lib") AND (EXISTS "${${PREFIX}OBS_PATH}/include/obs-frontend-api.h"))
			add_library(obs-frontend-api SHARED IMPORTED)
			set_target_properties(obs-frontend-api PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${${PREFIX}OBS_PATH}/include"
				INTERFACE_LINK_LIBRARIES "libobs"
				IMPORTED_CONFIGURATIONS RELWITHDEBINFO
				IMPORTED_IMPLIB_RELWITHDEBINFO "${${PREFIX}OBS_PATH}/bin/${D_PLATFORM_BITS}bit/obs-frontend-api.lib"
				IMPORTED_LOCATION_RELWITHDEBINFO "${${PREFIX}OBS_PATH}/bin/${D_PLATFORM_BITS}bit/obs-frontend-api.dll"
			)
		else()
			message(WARNING "${LOGPREFIX} The provided path for libOBS did not contain obs-frontend-api.")
		endif()
	endif()
endif()

################################################################################
# Auto-Dependency: OBS Studio Dependencies
################################################################################

if(STANDALONE AND NOT D_PLATFORM_LINUX)
	# Options
	set(${PREFIX}OBSDEPS_DOWNLOAD TRUE CACHE BOOL "Automatically download pre-built libOBS dependencies?")
	set(${PREFIX}OBSDEPS_PATH "" CACHE PATH "Path to pre-build libOBS dependencies, if ${PREFIX}OBSDEPS_DOWNLOAD is not set.")
	set(${PREFIX}OBSDEPS_DOWNLOAD_URL "" CACHE STRING "The URL from which to download pre-built libOBS dependencies, if autodetection fails. (Optional)")
	set(${PREFIX}OBSDEPS_DOWNLOAD_HASH "" CACHE STRING "The hash string for the given URL. Must be a SHA-256 hash if provided. (Optional)")
	mark_as_advanced(
		${PREFIX}OBSDEPS_PATH
		${PREFIX}OBSDEPS_DOWNLOAD_URL
		${PREFIX}OBSDEPS_DOWNLOAD_HASH
	)

	if(${PREFIX}OBSDEPS_DOWNLOAD)
		if(${PREFIX}OBSDEPS_DOWNLOAD_URL STREQUAL "")
			# Figure out download URLs and hashes.
			if (D_PLATFORM_WINDOWS)
				set(DEPS_VERSION "2022-01-31")
				if (D_PLATFORM_ARCH_X86)
					if (D_PLATFORM_BITS EQUAL 64)
						set(OBSDEPS_DOWNLOAD_URL "https://github.com/obsproject/obs-deps/releases/download/win-${DEPS_VERSION}/windows-deps-${DEPS_VERSION}.zip")
						set(OBSDEPS_DOWNLOAD_HASH "66E55FE35A507C902C036EB11E691D0257FECA545A8EE57324B69427717026DD")
					endif()
				endif()
			elseif(D_PLATFORM_MAC)
				set(DEPS_VERSION "2022-02-13")
				if (D_PLATFORM_ARCH_X86 AND D_PLATFORM_ARCH_ARM)
					if (D_PLATFORM_BITS EQUAL 64)
						set(OBSDEPS_DOWNLOAD_URL "https://github.com/obsproject/obs-deps/releases/download/${DEPS_VERSION}/macos-deps-${DEPS_VERSION}-universal.tar.xz")
						set(OBSDEPS_DOWNLOAD_HASH "77471B1D345A768E8EFEC3F6AD9DC79F3C7CD34840B66F721B80025D29713F5D")
					endif()
				elseif (D_PLATFORM_ARCH_X86)
					if (D_PLATFORM_BITS EQUAL 64)
						set(OBSDEPS_DOWNLOAD_URL "https://github.com/obsproject/obs-deps/releases/download/${DEPS_VERSION}/macos-deps-${DEPS_VERSION}-x86_64.tar.xz")
						set(OBSDEPS_DOWNLOAD_HASH "1A8715D66E664B857942DEADED0DC46C4F6CD22E88F01ED1188F3BD3FCF632C4")
					endif()
				elseif (D_PLATFORM_ARCH_ARM)
					if (D_PLATFORM_BITS EQUAL 64)
						set(OBSDEPS_DOWNLOAD_URL "https://github.com/obsproject/obs-deps/releases/download/${DEPS_VERSION}/macos-deps-${DEPS_VERSION}-arm64.tar.xz")
						set(OBSDEPS_DOWNLOAD_HASH "2CFCAF05765400C696908F242AEA87B6E1848E1A48CD3EDC2EB7F8CB249C9D48")
					endif()
				endif()
			endif()

			# Verify that the platform, architecture and bitness is supported.
			if(OBSDEPS_DOWNLOAD_URL STREQUAL "")
				message(FATAL_ERROR "${LOGPREFIX} Download for pre-built OBS dependencies failed, as Platform '${D_PLATFORM_OS}' with architecture '${D_PLATFORM_ARCH}' and bitness '${D_PLATFORM_BITS}' is not supported.")
				return()
			endif()
		else()
			set(OBSDEPS_DOWNLOAD_URL "${${PREFIX}OBSDEPS_DOWNLOAD_URL}")
			set(OBSDEPS_DOWNLOAD_HASH "${${PREFIX}OBSDEPS_DOWNLOAD_HASH}")
		endif()

		if(OBSDEPS_DOWNLOAD_HASH STREQUAL "")
			download_project(
				PROJ obsdeps
				PREFIX "autodeps"
				URL "${OBSDEPS_DOWNLOAD_URL}"
				DOWNLOAD_NO_PROGRESS OFF
				UPDATE_DISCONNECTED OFF
			)
		else()
			download_project(
				PROJ obsdeps
				PREFIX "autodeps"
				URL "${OBSDEPS_DOWNLOAD_URL}"
				URL_HASH "SHA256=${OBSDEPS_DOWNLOAD_HASH}"
				DOWNLOAD_NO_PROGRESS OFF
				UPDATE_DISCONNECTED OFF
			)
		endif()
		CacheSet(${PREFIX}OBSDEPS_PATH "${obsdeps_SOURCE_DIR}")
	endif()
	
	if (D_PLATFORM_WINDOWS)
		set(_OBSDEPS_PATH "${${PREFIX}OBSDEPS_PATH}/win${D_PLATFORM_BITS}")
		set(FFmpegPath "${${PREFIX}OBSDEPS_PATH}/win${D_PLATFORM_BITS}")
	elseif(D_PLATFORM_MAC)
		set(_OBSDEPS_PATH "${${PREFIX}OBSDEPS_PATH}/obsdeps")
		set(FFmpegPath "${${PREFIX}OBSDEPS_PATH}/obsdeps")
	endif()
endif()

################################################################################
# Auto-Dependency: Qt v5.x
################################################################################

if(STANDALONE AND NOT D_PLATFORM_LINUX)
	# Options
	set(${PREFIX}QT_DOWNLOAD TRUE CACHE BOOL "Automatically download Qt?")
	set(${PREFIX}QT_PATH "" CACHE PATH "Path to Qt, if ${PREFIX}QT_DOWNLOAD is not set.")
	set(${PREFIX}QT_DOWNLOAD_URL "" CACHE STRING "The URL from which to download Qt, if autodetection fails. (Optional)")
	set(${PREFIX}QT_DOWNLOAD_HASH "" CACHE STRING "The hash string for the given URL. Must be a SHA-256 hash if provided. (Optional)")
	mark_as_advanced(
		${PREFIX}QT_PATH
		${PREFIX}QT_DOWNLOAD_URL
		${PREFIX}QT_DOWNLOAD_HASH
	)

	if(${PREFIX}QT_DOWNLOAD)
		if(${PREFIX}QT_DOWNLOAD_URL STREQUAL "")
			# Figure out download URLs and hashes.
			if (D_PLATFORM_WINDOWS)
				set(DEPS_VERSION "5.15.2")
				if (D_PLATFORM_ARCH_X86)
					if (D_PLATFORM_BITS EQUAL 64)
						set(QT_DOWNLOAD_URL "https://cdn-fastly.obsproject.com/downloads/Qt_${DEPS_VERSION}.7z")
						set(QT_DOWNLOAD_HASH "9EF1DFEEBE6AB7FFC55FD285667CC5D9CB2D298646C107C2295D13F14708E64E")
					endif()
				endif()
			elseif(D_PLATFORM_MAC)
				set(DEPS_VERSION "2022-02-13")
				if (D_PLATFORM_ARCH_X86 AND D_PLATFORM_ARCH_ARM)
					if (D_PLATFORM_BITS EQUAL 64)
						set(QT_DOWNLOAD_URL "https://github.com/obsproject/obs-deps/releases/download/${DEPS_VERSION}/macos-deps-qt-${DEPS_VERSION}-universal.tar.xz")
						set(QT_DOWNLOAD_HASH "13FBCC45FD9D08B30E702D481FE4D58B23F93AA06848CEDE4EBE0956C79A5E8A")
					endif()
				elseif (D_PLATFORM_ARCH_X86)
					if (D_PLATFORM_BITS EQUAL 64)
						set(QT_DOWNLOAD_URL "https://github.com/obsproject/obs-deps/releases/download/${DEPS_VERSION}/macos-deps-qt-${DEPS_VERSION}-x86_64.tar.xz")
						set(QT_DOWNLOAD_HASH "35A58FEE8DFD70D3D2DCC0AE0B77132C04A451C6F041A02DC41B207B375FC74B")
					endif()
				elseif (D_PLATFORM_ARCH_ARM)
					if (D_PLATFORM_BITS EQUAL 64)
						set(QT_DOWNLOAD_URL "https://github.com/obsproject/obs-deps/releases/download/${DEPS_VERSION}/macos-deps-qt-${DEPS_VERSION}-arm64.tar.xz")
						set(QT_DOWNLOAD_HASH "E99146B9C7775C245A2D22F2EF24FC111FCCD71BAD0F03B64DB707124FFB8707")
					endif()
				endif()
			endif()

			# Verify that the platform, architecture and bitness is supported.
			if(QT_DOWNLOAD_URL STREQUAL "")
				message(FATAL_ERROR "${LOGPREFIX} Download for Qt failed, as Platform '${D_PLATFORM_OS}' with architecture '${D_PLATFORM_ARCH}' and bitness '${D_PLATFORM_BITS}' is not supported.")
				return()
			endif()
		else()
			set(QT_DOWNLOAD_URL "${${PREFIX}QT_DOWNLOAD_URL}")
			set(QT_DOWNLOAD_HASH "${${PREFIX}QT_DOWNLOAD_HASH}")
		endif()

		if(QT_DOWNLOAD_HASH STREQUAL "")
			download_project(
				PROJ qt
				PREFIX "autodeps"
				URL "${QT_DOWNLOAD_URL}"
				DOWNLOAD_NO_PROGRESS OFF
				UPDATE_DISCONNECTED OFF
			)
		else()
			download_project(
				PROJ qt
				PREFIX "autodeps"
				URL "${QT_DOWNLOAD_URL}"
				URL_HASH "SHA256=${QT_DOWNLOAD_HASH}"
				DOWNLOAD_NO_PROGRESS OFF
				UPDATE_DISCONNECTED OFF
			)
		endif()
		execute_process(COMMAND "xattr" "-r" "-d" "com.apple.quarantine" "${qt_SOURCE_DIR}")

		if(D_PLATFORM_WINDOWS)
			CacheSet(${PREFIX}QT_PATH "${qt_SOURCE_DIR}/msvc2019_64")
		else()
			CacheSet(${PREFIX}QT_PATH "${qt_SOURCE_DIR}")
		endif()
	endif()
	
	set(Qt5_DIR "${${PREFIX}QT_PATH}/lib/cmake/Qt5" CACHE STRING "Path to Qt")
	CacheSet(Qt5_DIR "${${PREFIX}QT_PATH}/lib/cmake/Qt5")
endif()

################################################################################
# Auto-Dependency: libAOM
################################################################################

if(D_PLATFORM_WINDOWS)
	# Options
	set(${PREFIX}AOM_DOWNLOAD TRUE CACHE BOOL "Automatically download AOM?")
	set(${PREFIX}AOM_PATH "" CACHE PATH "Path to AOM, if ${PREFIX}AOM_DOWNLOAD is not set.")
	set(${PREFIX}AOM_DOWNLOAD_URL "" CACHE STRING "The URL from which to download AOM, if autodetection fails. (Optional)")
	set(${PREFIX}AOM_DOWNLOAD_HASH "" CACHE STRING "The hash string for the given URL. Must be a SHA-256 hash if provided. (Optional)")
	mark_as_advanced(
		${PREFIX}AOM_PATH
		${PREFIX}AOM_DOWNLOAD_URL
		${PREFIX}AOM_DOWNLOAD_HASH
	)

	if(${PREFIX}AOM_DOWNLOAD)
		if(${PREFIX}AOM_DOWNLOAD_URL STREQUAL "")
			set(AOM_VERSION "v3.2.0.1")
			# Figure out download URLs and hashes.
			if (D_PLATFORM_WINDOWS)
				if (D_PLATFORM_ARCH_X86)
					set(AOM_DOWNLOAD_URL "https://github.com/Xaymar/aom/releases/download/${AOM_VERSION}/aom-windows-${D_PLATFORM_BITS}-shared.7z")
					set(AOM_DOWNLOAD_HASH "2DE0C215C5B00D6761AD2D1FEAFB545C04249B2CBD542F9F7D423E1A26BA59BD")
				endif()
			endif()

			# Verify that the platform, architecture and bitness is supported.
			if(AOM_DOWNLOAD_URL STREQUAL "")
				message(FATAL_ERROR "${LOGPREFIX} Download for AOM failed, as Platform '${D_PLATFORM_OS}' with architecture '${D_PLATFORM_ARCH}' and bitness '${D_PLATFORM_BITS}' is not supported.")
				return()
			endif()
		else()
			set(AOM_DOWNLOAD_URL "${${PREFIX}AOM_DOWNLOAD_URL}")
			set(AOM_DOWNLOAD_HASH "${${PREFIX}AOM_DOWNLOAD_HASH}")
		endif()

		if(AOM_DOWNLOAD_HASH STREQUAL "")
			download_project(
				PROJ AOM
				PREFIX "autodeps"
				URL "${AOM_DOWNLOAD_URL}"
				DOWNLOAD_NO_PROGRESS OFF
				UPDATE_DISCONNECTED OFF
			)
		else()
			download_project(
				PROJ AOM
				PREFIX "autodeps"
				URL "${AOM_DOWNLOAD_URL}"
				URL_HASH "SHA256=${AOM_DOWNLOAD_HASH}"
				DOWNLOAD_NO_PROGRESS OFF
				UPDATE_DISCONNECTED OFF
			)
		endif()

		if(D_PLATFORM_WINDOWS)
			CacheSet(${PREFIX}AOM_PATH "${AOM_SOURCE_DIR}/msvc2019_64")
		else()
			CacheSet(${PREFIX}AOM_PATH "${AOM_SOURCE_DIR}")
		endif()
	endif()
	
	set(AOM_PATH "${${PREFIX}AOM_PATH}" CACHE STRING "Path to AOM")
endif()

################################################################################
# Components
################################################################################

# Component resolving:
# 1. Check which features are enabled. For each feature, set what they require to ON.
# 2. Try and find required items.
# 3. Again check which features are enabled, if their requirements are missing, warn about it and disable them.
# TODO: Consider making this an error instead.

macro(is_feature_enabled FEATURE OUTPUT)
	set(T_ENABLED ${${PREFIX}ENABLE_${FEATURE}})
	set(T_DISABLED ${${PREFIX}DISABLE_${FEATURE}})
	if(T_ENABLED AND NOT T_DISABLED)
		set(${OUTPUT} ON)
	else()
		# set(${PREFIX}DISABLE_${FEATURE} ON CACHE INTERNAL "" FORCE)
		set(${OUTPUT} OFF)
	endif()
endmacro()

macro(set_feature_disabled FEATURE DISABLED)
	set(${PREFIX}DISABLE_${FEATURE} ${DISABLED} CACHE INTERNAL "" FORCE)
endmacro()

# Features
function(feature_encoder_ffmpeg RESOLVE)
	is_feature_enabled(ENCODER_FFMPEG T_CHECK)
	if(RESOLVE AND T_CHECK)
		if(NOT HAVE_FFMPEG)
			message(WARNING "${LOGPREFIX} FFmpeg Encoder requires FFmpeg. Disabling...")
			set_feature_disabled(ENCODER_FFMPEG ON)
		else()
			# AMF
			is_feature_enabled(ENCODER_FFMPEG_AMF T_CHECK)
			if(T_CHECK AND D_PLATFORM_MAC)
				message(WARNING "${LOGPREFIX} FFmpeg Encoder 'AMF' requires Windows or Linux. Disabling...")
				set_feature_disabled(ENCODER_FFMPEG_AMF ON)
			endif()

			# NVENC
			is_feature_enabled(ENCODER_FFMPEG_NVENC T_CHECK)
			if(T_CHECK AND D_PLATFORM_MAC)
				message(WARNING "${LOGPREFIX} FFmpeg Encoder 'NVENC' requires Windows or Linux. Disabling...")
				set_feature_disabled(ENCODER_FFMPEG_NVENC ON)
			endif()

			# ProRes
			is_feature_enabled(ENCODER_FFMPEG_PRORES T_CHECK)

			# DNxHR
			is_feature_enabled(ENCODER_FFMPEG_DNXHR T_CHECK)
		endif()
	elseif(T_CHECK)
		set(REQUIRE_FFMPEG ON PARENT_SCOPE)
	endif()
endfunction()

function(feature_encoder_aom_av1 RESOLVE)
	is_feature_enabled(ENCODER_AOM_AV1 T_CHECK)
	if(RESOLVE AND T_CHECK)
		if(NOT HAVE_AOM)
			message(WARNING "${LOGPREFIX} AOM AV1 encoder missing AOM library. Disabling...")
			set_feature_disabled(ENCODER_AOM_AV1 ON)
		endif()
	elseif(T_CHECK)
		set(REQUIRE_AOM ON PARENT_SCOPE)
	endif()
endfunction()

function(feature_filter_autoframing RESOLVE)
	is_feature_enabled(FILTER_AUTOFRAMING T_CHECK)
	if(RESOLVE AND T_CHECK)
		# Verify that the requirements for the providers are available
		if(NOT HAVE_NVIDIA_AR_SDK)
			message(WARNING "${LOGPREFIX} 'NVIDIA Augmented Reality SDK' is missing. Disabling NVIDIA provider...")
			set_feature_disabled(FILTER_AUTOFRAMING_NVIDIA ON)
		endif()

		# Verify that we have at least one provider for Auto-Framing.
		is_feature_enabled(FILTER_AUTOFRAMING_NVIDIA T_CHECK_NVIDIA)
		if (NOT T_CHECK_NVIDIA)
			message(WARNING "${LOGPREFIX} Auto-Framing has no available providers. Disabling...")
			set_feature_disabled(FILTER_AUTOFRAMING ON)
		endif()
	elseif(T_CHECK)
		set(REQUIRE_NVIDIA_AR_SDK ON PARENT_SCOPE)
		set(REQUIRE_NVIDIA_CUDA ON PARENT_SCOPE)
	endif()
endfunction()

function(feature_filter_blur RESOLVE)
	is_feature_enabled(FILTER_BLUR T_CHECK)
endfunction()

function(feature_filter_color_grade RESOLVE)
	is_feature_enabled(FILTER_COLOR_GRADE T_CHECK)
endfunction()

function(feature_filter_denoising RESOLVE)
	is_feature_enabled(FILTER_DENOISING T_CHECK)
	if(RESOLVE AND T_CHECK)
		# Verify that the requirements for the providers are available
		if(NOT HAVE_NVIDIA_VFX_SDK)
			message(WARNING "${LOGPREFIX} 'NVIDIA Video Effects SDK' is missing. Disabling NVIDIA provider...")
			set_feature_disabled(FILTER_DENOISING_NVIDIA ON)
		endif()

		# Verify that we have at least one provider for Video Denoising.
		is_feature_enabled(FILTER_DENOISING_NVIDIA T_CHECK_NVIDIA)
		if (NOT T_CHECK_NVIDIA)
			message(WARNING "${LOGPREFIX} Denoising has no available providers. Disabling...")
			set_feature_disabled(FILTER_DENOISING ON)
		endif()
	elseif(T_CHECK)
		set(REQUIRE_NVIDIA_VFX_SDK ON PARENT_SCOPE)
	endif()
endfunction()

function(feature_filter_displacement RESOLVE)
	is_feature_enabled(FILTER_DISPLACEMENT T_CHECK)
endfunction()

function(feature_filter_dynamic_mask RESOLVE)
	is_feature_enabled(FILTER_DYNAMIC_MASK T_CHECK)
endfunction()

function(feature_filter_sdf_effects RESOLVE)
	is_feature_enabled(FILTER_SDF_EFFECTS T_CHECK)
endfunction()

function(feature_filter_shader RESOLVE)
	is_feature_enabled(FILTER_SHADER T_CHECK)
endfunction()

function(feature_filter_transform RESOLVE)
	is_feature_enabled(FILTER_TRANSFORM T_CHECK)
endfunction()

function(feature_filter_upscaling RESOLVE)
	is_feature_enabled(FILTER_UPSCALING T_CHECK)
	if(RESOLVE AND T_CHECK)
		# Verify that the requirements for the providers are available
		if(NOT HAVE_NVIDIA_VFX_SDK)
			message(WARNING "${LOGPREFIX} 'NVIDIA Video Effects SDK' is missing. Disabling NVIDIA provider(s)...")
			set_feature_disabled(FILTER_UPSCALING_NVIDIA ON)
		endif()

		# Verify that we have at least one provider for Video Super-Resolution.
		is_feature_enabled(FILTER_UPSCALING_NVIDIA T_CHECK_NVIDIA)
		if (NOT T_CHECK_NVIDIA)
			message(WARNING "${LOGPREFIX} Upscaling has no available providers. Disabling...")
			set_feature_disabled(FILTER_UPSCALING ON)
		endif()
	elseif(T_CHECK)
		set(REQUIRE_NVIDIA_VFX_SDK ON PARENT_SCOPE)
	endif()
endfunction()

function(feature_filter_virtual_greenscreen RESOLVE)
	is_feature_enabled(FILTER_VIRTUAL_GREENSCREEN T_CHECK)
	if(RESOLVE AND T_CHECK)
		# Verify that the requirements for the providers are available
		if(NOT HAVE_NVIDIA_VFX_SDK)
			message(WARNING "${LOGPREFIX} 'NVIDIA Video Effects SDK' is missing. Disabling NVIDIA provider(s)...")
			set_feature_disabled(FILTER_VIRTUAL_GREENSCREEN_NVIDIA ON)
		endif()

		# Verify that we have at least one provider for Video Super-Resolution.
		is_feature_enabled(FILTER_VIRTUAL_GREENSCREEN_NVIDIA T_CHECK_NVIDIA)
		if (NOT T_CHECK_NVIDIA)
			message(WARNING "${LOGPREFIX} Virtual Greenscreen has no available providers. Disabling...")
			set_feature_disabled(FILTER_VIRTUAL_GREENSCREEN ON)
		endif()
	elseif(T_CHECK)
		set(REQUIRE_NVIDIA_VFX_SDK ON PARENT_SCOPE)
	endif()
endfunction()

function(feature_source_mirror RESOLVE)
	is_feature_enabled(SOURCE_MIRROR T_CHECK)
endfunction()

function(feature_source_shader RESOLVE)
	is_feature_enabled(SOURCE_SHADER T_CHECK)
endfunction()

function(feature_transition_shader RESOLVE)
	is_feature_enabled(TRANSITION_SHADER T_CHECK)
endfunction()

function(feature_frontend RESOLVE)
	is_feature_enabled(FRONTEND T_CHECK)
	if(RESOLVE AND T_CHECK)
		if(NOT HAVE_QT)
			message(WARNING "${LOGPREFIX} Front-End requires Qt. Disabling...")
			set_feature_disabled(FRONTEND ON)
		elseif(NOT HAVE_OBSFE)
			message(WARNING "${LOGPREFIX} Front-End requires OBS FrontEnd API. Disabling...")
			set_feature_disabled(FRONTEND ON)
		endif()
	elseif(T_CHECK)
		set(REQUIRE_QT ON PARENT_SCOPE)
		set(REQUIRE_OBSFE ON PARENT_SCOPE)
	endif()
endfunction()

function(feature_updater RESOLVE)
	is_feature_enabled(UPDATER T_CHECK)
	if(RESOLVE AND T_CHECK)
		if(NOT HAVE_CURL)
			message(WARNING "${LOGPREFIX} Updater requires CURL. Disabling...")
			set_feature_disabled(UPDATER ON)
		elseif(NOT HAVE_JSON)
			message(WARNING "${LOGPREFIX} Updater requires nlohmann::json. Disabling...")
			set_feature_disabled(UPDATER ON)
		endif()
	elseif(T_CHECK)
		set(REQUIRE_CURL ON PARENT_SCOPE)
		set(REQUIRE_JSON ON PARENT_SCOPE)
	endif()
endfunction()

# Set Requirements
feature_encoder_ffmpeg(OFF)
feature_encoder_aom_av1(OFF)
feature_filter_autoframing(OFF)
feature_filter_blur(OFF)
feature_filter_color_grade(OFF)
feature_filter_denoising(OFF)
feature_filter_displacement(OFF)
feature_filter_dynamic_mask(OFF)
feature_filter_sdf_effects(OFF)
feature_filter_shader(OFF)
feature_filter_transform(OFF)
feature_filter_upscaling(OFF)
feature_filter_virtual_greenscreen(OFF)
feature_source_mirror(OFF)
feature_source_shader(OFF)
feature_transition_shader(OFF)
feature_frontend(OFF)
feature_updater(OFF)

# Fulfill Requirements
#- CURL
set(HAVE_CURL OFF)
if(REQUIRE_CURL)
	if(D_PLATFORM_WINDOWS)
		if(NOT STANDALONE) # Already defined by OBS
			set(CURL_LIBRARIES "${CURL_LIB}")
			set(CURL_INCLUDE_DIRS "${CURL_INCLUDE_DIR}")
		else()
			set(CURL_LIBRARIES "${_OBSDEPS_PATH}/bin/libcurl.lib")
			set(CURL_INCLUDE_DIRS "${_OBSDEPS_PATH}/include")
		endif()
		set(CURL_LIBRARY_DEBUG ${CURL_LIBRARIES})
		set(CURL_LIBRARY_RELEASE ${CURL_LIBRARIES})
		set(CURL_INCLUDE_DIR ${CURL_INCLUDE_DIRS})
		set(CURL_FOUND ON)
	else()
		find_package(CURL)
	endif()

	set(HAVE_CURL ${CURL_FOUND})
endif()

#- FFmpeg
set(HAVE_FFMPEG OFF)
if(REQUIRE_FFMPEG)
	find_package(FFmpeg COMPONENTS avutil avcodec swscale)
	set(HAVE_FFMPEG ${FFmpeg_FOUND})
endif()

#- AOM
set(HAVE_AOM OFF)
if(REQUIRE_AOM)
	if(NOT D_PLATFORM_MAC)
		find_package(AOM)
		set(HAVE_AOM ${AOM_FOUND})
	endif()
endif()

#- JSON
set(HAVE_JSON OFF)
if(REQUIRE_JSON)
	if(NOT EXISTS "${PROJECT_SOURCE_DIR}/third-party/nlohmann-json/single_include")
		message(FATAL_ERROR "${LOGPREFIX} Please make sure to update git submodules to their latest supported version.")
		return()
	else()
		set(JSON_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/third-party/nlohmann-json/single_include")
		set(HAVE_JSON ON)
	endif()
endif()

#- NVIDIA Augmented Reality SDK
set(HAVE_NVIDIA_AR_SDK OFF)
if(REQUIRE_NVIDIA_AR_SDK AND D_PLATFORM_WINDOWS)
	if(EXISTS "${PROJECT_SOURCE_DIR}/third-party/nvidia-maxine-ar-sdk/version.h")
		set(HAVE_NVIDIA_AR_SDK ON)
	endif()

	if(NOT TARGET NVIDIA::AR)
		add_library(NVIDIA::AR IMPORTED INTERFACE)
		target_include_directories(NVIDIA::AR
			INTERFACE
				"${PROJECT_SOURCE_DIR}/third-party/nvidia-maxine-ar-sdk/nvar/include/"
				"${PROJECT_SOURCE_DIR}/third-party/nvidia-maxine-ar-sdk/nvar/src/"
		)
	endif()
endif()

#- NVIDIA Video Effects SDK
set(HAVE_NVIDIA_VFX_SDK OFF)
if(REQUIRE_NVIDIA_VFX_SDK AND D_PLATFORM_WINDOWS)
	if(EXISTS "${PROJECT_SOURCE_DIR}/third-party/nvidia-maxine-vfx-sdk/version.h")
		set(HAVE_NVIDIA_VFX_SDK ON)
	endif()

	if(NOT TARGET NVIDIA::VFX)
		add_library(NVIDIA::VFX IMPORTED INTERFACE)
		target_include_directories(NVIDIA::VFX
			INTERFACE
				"${PROJECT_SOURCE_DIR}/third-party/nvidia-maxine-vfx-sdk/nvvfx/include/"
				"${PROJECT_SOURCE_DIR}/third-party/nvidia-maxine-vfx-sdk/nvvfx/src/"
		)
	endif()

	set(REQUIRE_NVIDIA_CUDA ON)
endif()

#- NVIDIA CUDA (Windows)
set(HAVE_NVIDIA_CUDA OFF)
if(REQUIRE_NVIDIA_CUDA AND D_PLATFORM_WINDOWS)
	set(HAVE_NVIDIA_CUDA ON)
endif()

#- OBS Front-End API
set(HAVE_OBSFE OFF)
if(REQUIRE_OBSFE)
	if(TARGET obs-frontend-api)
		set(HAVE_OBSFE ON)
	endif()
endif()

#- Qt5
set(HAVE_QT OFF)
if(REQUIRE_QT)
	find_package(Qt5
		COMPONENTS Widgets Core REQUIRED
	)
	set(HAVE_QT ${Qt5_FOUND})
endif()

# Verify Requirements
feature_encoder_ffmpeg(ON)
feature_encoder_aom_av1(ON)
feature_filter_autoframing(ON)
feature_filter_blur(ON)
feature_filter_color_grade(ON)
feature_filter_denoising(ON)
feature_filter_displacement(ON)
feature_filter_dynamic_mask(ON)
feature_filter_sdf_effects(ON)
feature_filter_shader(ON)
feature_filter_transform(ON)
feature_filter_upscaling(ON)
feature_filter_virtual_greenscreen(ON)
feature_source_mirror(ON)
feature_source_shader(ON)
feature_transition_shader(ON)
feature_frontend(ON)
feature_updater(ON)

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

# Configure Files
configure_file(
	"templates/config.hpp.in"
	"generated/config.hpp"
)
LIST(APPEND PROJECT_TEMPLATES "templates/config.hpp.in")
LIST(APPEND PROJECT_PRIVATE_GENERATED "${PROJECT_BINARY_DIR}/generated/config.hpp")

configure_file(
	"templates/version.hpp.in"
	"generated/version.hpp"
)
LIST(APPEND PROJECT_TEMPLATES "templates/version.hpp.in")
LIST(APPEND PROJECT_PRIVATE_GENERATED "${PROJECT_BINARY_DIR}/generated/version.hpp")

configure_file(
	"templates/module.cpp.in"
	"generated/module.cpp"
)
LIST(APPEND PROJECT_TEMPLATES "templates/module.cpp.in")
LIST(APPEND PROJECT_PRIVATE_GENERATED "${PROJECT_BINARY_DIR}/generated/module.cpp")

if(D_PLATFORM_WINDOWS) # Windows Support
	set(PROJECT_PRODUCT_NAME "${PROJECT_FULL_NAME}")
	set(PROJECT_COMPANY_NAME "${PROJECT_AUTHORS}")
	set(PROJECT_COPYRIGHT "${PROJECT_AUTHORS} © ${PROJECT_COPYRIGHT_YEARS}")
	set(PROJECT_LEGAL_TRADEMARKS_1 "")
	set(PROJECT_LEGAL_TRADEMARKS_2 "")

	configure_file(
		"templates/version.rc.in"
		"generated/version.rc"
		@ONLY
	)
	LIST(APPEND PROJECT_TEMPLATES "templates/version.rc.in")
	LIST(APPEND PROJECT_PRIVATE_GENERATED "${PROJECT_BINARY_DIR}/generated/version.rc")
endif()

# Minimum Dependencies
list(APPEND PROJECT_LIBRARIES libobs)

# Components
if(HAVE_CURL)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/util/util-curl.hpp"
		"source/util/util-curl.cpp"
	)
	list(APPEND PROJECT_LIBRARIES ${CURL_LIBRARY_RELEASE})
	list(APPEND PROJECT_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
endif()

if(HAVE_FFMPEG)
	list(APPEND PROJECT_LIBRARIES
		${FFMPEG_LIBRARIES}
	)
	list(APPEND PROJECT_INCLUDE_DIRS
		${FFMPEG_INCLUDE_DIRS}
	)
endif()

if(HAVE_JSON)
	list(APPEND PROJECT_INCLUDE_DIRS ${JSON_INCLUDE_DIR})
endif()

if(TRUE) # OpenGL
	if(NOT TARGET khronos_glad)
		add_library(khronos_glad STATIC
			"${PROJECT_SOURCE_DIR}/third-party/khronos/glad/src/gl.c"
		)
		target_include_directories(khronos_glad
			PRIVATE
				"${PROJECT_SOURCE_DIR}/third-party/khronos/glad/include"
			INTERFACE
				"${PROJECT_SOURCE_DIR}/third-party/khronos/glad/include"
			PUBLIC
				"${PROJECT_SOURCE_DIR}/third-party/khronos/glad/include"
		)
		if(D_PLATFORM_WINDOWS)
			target_sources(khronos_glad
				PRIVATE
				"${PROJECT_SOURCE_DIR}/third-party/khronos/glad/src/wgl.c"
			)
		elseif(D_PLATFORM_LINUX)
			target_sources(khronos_glad
				PRIVATE
				"${PROJECT_SOURCE_DIR}/third-party/khronos/glad/src/glx.c"
			)
		endif()

		# Requires for shared/static mixing.
		set_property(TARGET khronos_glad PROPERTY POSITION_INDEPENDENT_CODE ON)
	endif()

	list(APPEND PROJECT_LIBRARIES khronos_glad)
endif()

if(HAVE_NVIDIA_VFX_SDK OR HAVE_NVIDIA_AR_SDK)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/nvidia/cv/nvidia-cv.hpp"
		"source/nvidia/cv/nvidia-cv.cpp"
		"source/nvidia/cv/nvidia-cv-image.hpp"
		"source/nvidia/cv/nvidia-cv-image.cpp"
		"source/nvidia/cv/nvidia-cv-texture.hpp"
		"source/nvidia/cv/nvidia-cv-texture.cpp"
	)
endif()

if(HAVE_NVIDIA_AR_SDK)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/nvidia/ar/nvidia-ar.hpp"
		"source/nvidia/ar/nvidia-ar.cpp"
		"source/nvidia/ar/nvidia-ar-feature.hpp"
		"source/nvidia/ar/nvidia-ar-feature.cpp"
		"source/nvidia/ar/nvidia-ar-facedetection.hpp"
		"source/nvidia/ar/nvidia-ar-facedetection.cpp"
	)
	list(APPEND PROJECT_LIBRARIES
		NVIDIA::AR
	)
endif()

if(HAVE_NVIDIA_VFX_SDK)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/nvidia/vfx/nvidia-vfx.hpp"
		"source/nvidia/vfx/nvidia-vfx.cpp"
		"source/nvidia/vfx/nvidia-vfx-denoising.hpp"
		"source/nvidia/vfx/nvidia-vfx-denoising.cpp"
		"source/nvidia/vfx/nvidia-vfx-effect.hpp"
		"source/nvidia/vfx/nvidia-vfx-effect.cpp"
		"source/nvidia/vfx/nvidia-vfx-greenscreen.hpp"
		"source/nvidia/vfx/nvidia-vfx-greenscreen.cpp"
		"source/nvidia/vfx/nvidia-vfx-superresolution.hpp"
		"source/nvidia/vfx/nvidia-vfx-superresolution.cpp"
	)
	list(APPEND PROJECT_LIBRARIES
		NVIDIA::VFX
	)
endif()

if(HAVE_NVIDIA_CUDA)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/nvidia/cuda/nvidia-cuda.hpp"
		"source/nvidia/cuda/nvidia-cuda.cpp"
		"source/nvidia/cuda/nvidia-cuda-obs.hpp"
		"source/nvidia/cuda/nvidia-cuda-obs.cpp"
		"source/nvidia/cuda/nvidia-cuda-context.hpp"
		"source/nvidia/cuda/nvidia-cuda-context.cpp"
		"source/nvidia/cuda/nvidia-cuda-gs-texture.hpp"
		"source/nvidia/cuda/nvidia-cuda-gs-texture.cpp"
		"source/nvidia/cuda/nvidia-cuda-memory.hpp"
		"source/nvidia/cuda/nvidia-cuda-memory.cpp"
		"source/nvidia/cuda/nvidia-cuda-stream.hpp"
		"source/nvidia/cuda/nvidia-cuda-stream.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_NVIDIA_CUDA
	)
endif()

if(REQUIRE_OBSFE AND HAVE_OBSFE)
	list(APPEND PROJECT_LIBRARIES obs-frontend-api)
endif()

if(REQUIRE_QT AND HAVE_QT)
	list(APPEND PROJECT_LIBRARIES Qt5::Core Qt5::Widgets)
endif()

################################################################################
# Features
################################################################################

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
	"source/util/util-logging.cpp"
	"source/util/util-logging.hpp"
	"source/util/util-platform.hpp"
	"source/util/util-platform.cpp"
	"source/util/util-threadpool.cpp"
	"source/util/util-threadpool.hpp"
	"source/gfx/gfx-debug.hpp"
	"source/gfx/gfx-debug.cpp"
	"source/gfx/gfx-opengl.hpp"
	"source/gfx/gfx-opengl.cpp"
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
	"source/obs/obs-signal-handler.hpp"
	"source/obs/obs-signal-handler.cpp"
	"source/obs/obs-source-tracker.hpp"
	"source/obs/obs-source-tracker.cpp"
	"source/obs/obs-tools.hpp"
	"source/obs/obs-tools.cpp"

	# obs_encoder_info_t, obs_encoder_t, obs_weak_encoder_t
	"source/obs/obs-encoder-factory.hpp"
	"source/obs/obs-encoder-factory.cpp"

	# obs_source_info_t, obs_source_t, obs_weak_source_t
	"source/obs/obs-source-factory.hpp"
	"source/obs/obs-source-factory.cpp"
	"source/obs/obs-source-info.hpp"
	"source/obs/obs-source-info.cpp"
	"source/obs/obs-source.hpp"
	"source/obs/obs-source.cpp"
	"source/obs/obs-source-active-child.hpp"
	"source/obs/obs-source-active-child.cpp"
	"source/obs/obs-source-active-reference.hpp"
	"source/obs/obs-source-active-reference.cpp"
	"source/obs/obs-source-showing-reference.hpp"
	"source/obs/obs-source-showing-reference.cpp"
	"source/obs/obs-weak-source.hpp"
	"source/obs/obs-weak-source.cpp"
)
list(APPEND PROJECT_DATA
	"data/effects/color_conversion_rgb_hsl.effect"
	"data/effects/color_conversion_rgb_hsv.effect"
	"data/effects/color_conversion_rgb_yuv.effect"
	"data/effects/mipgen.effect"
	"data/effects/pack-unpack.effect"
	"data/effects/standard.effect"
	"data/effects/shared.effect"
	"data/locale/en-US.ini"
)
list(APPEND PROJECT_INCLUDE_DIRS
	"${PROJECT_BINARY_DIR}/generated"
	"${PROJECT_SOURCE_DIR}/source"
)
file(GLOB_RECURSE PROJECT_EXAMPLES "data/examples/*")
list(APPEND PROJECT_DATA ${PROJECT_EXAMPLES})

# Encoder/FFmpeg
is_feature_enabled(ENCODER_FFMPEG T_CHECK)
if(T_CHECK)
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
		"source/encoders/codecs/dnxhr.hpp"
		"source/encoders/codecs/dnxhr.cpp"

		# Encoders/Handlers
		"source/encoders/handlers/handler.hpp"
		"source/encoders/handlers/handler.cpp"
		"source/encoders/handlers/debug_handler.hpp"
		"source/encoders/handlers/debug_handler.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_ENCODER_FFMPEG
	)

	# AMF
	is_feature_enabled(ENCODER_FFMPEG_AMF T_CHECK)
	if(T_CHECK)
		list(APPEND PROJECT_PRIVATE_SOURCE
			"source/encoders/handlers/amf_shared.hpp"
			"source/encoders/handlers/amf_shared.cpp"
			"source/encoders/handlers/amf_h264_handler.hpp"
			"source/encoders/handlers/amf_h264_handler.cpp"
			"source/encoders/handlers/amf_hevc_handler.hpp"
			"source/encoders/handlers/amf_hevc_handler.cpp"
		)
		list(APPEND PROJECT_DEFINITIONS
			ENABLE_ENCODER_FFMPEG_AMF
		)
	endif()

	# NVENC
	is_feature_enabled(ENCODER_FFMPEG_NVENC T_CHECK)
	if(T_CHECK)
		list(APPEND PROJECT_PRIVATE_SOURCE
			"source/encoders/handlers/nvenc_shared.hpp"
			"source/encoders/handlers/nvenc_shared.cpp"
			"source/encoders/handlers/nvenc_h264_handler.hpp"
			"source/encoders/handlers/nvenc_h264_handler.cpp"
			"source/encoders/handlers/nvenc_hevc_handler.hpp"
			"source/encoders/handlers/nvenc_hevc_handler.cpp"
		)
		list(APPEND PROJECT_DEFINITIONS
			ENABLE_ENCODER_FFMPEG_NVENC
		)
	endif()

	# ProRES
	is_feature_enabled(ENCODER_FFMPEG_PRORES T_CHECK)
	if(T_CHECK)
		list(APPEND PROJECT_PRIVATE_SOURCE
			"source/encoders/handlers/prores_aw_handler.hpp"
			"source/encoders/handlers/prores_aw_handler.cpp"
		)
		list(APPEND PROJECT_DEFINITIONS
			ENABLE_ENCODER_FFMPEG_PRORES
		)
		endif()

	# DNxHR
	is_feature_enabled(ENCODER_FFMPEG_DNXHR T_CHECK)
	if(T_CHECK)
		list(APPEND PROJECT_PRIVATE_SOURCE
				"source/encoders/handlers/dnxhd_handler.hpp"
				"source/encoders/handlers/dnxhd_handler.cpp"
		)
		list(APPEND PROJECT_DEFINITIONS
				ENABLE_ENCODER_FFMPEG_DNXHR
		)
	endif()
endif()

# Encoder/AOM-AV1
is_feature_enabled(ENCODER_AOM_AV1 T_CHECK)
if(T_CHECK)
	list (APPEND PROJECT_PRIVATE_SOURCE
		"source/encoders/codecs/av1.hpp"
		"source/encoders/codecs/av1.cpp"
		"source/encoders/encoder-aom-av1.hpp"
		"source/encoders/encoder-aom-av1.cpp"
	)
	list (APPEND PROJECT_INCLUDE_DIRS
		${AOM_INCLUDE_DIR}
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_ENCODER_AOM_AV1
	)
endif()

# Filter/Auto-Framing
is_feature_enabled(FILTER_AUTOFRAMING T_CHECK)
if(T_CHECK)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-autoframing.hpp"
		"source/filters/filter-autoframing.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_AUTOFRAMING
	)
	is_feature_enabled(FILTER_AUTOFRAMING_NVIDIA T_CHECK)
	if (T_CHECK)
		list(APPEND PROJECT_DEFINITIONS
			ENABLE_FILTER_AUTOFRAMING_NVIDIA
		)
	endif()
endif()

# Filter/Blur
is_feature_enabled(FILTER_BLUR T_CHECK)
if(T_CHECK)
	list(APPEND PROJECT_DATA
		"data/effects/mask.effect"
		"data/effects/blur/common.effect"
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

# Filter/Color Grade
is_feature_enabled(FILTER_COLOR_GRADE T_CHECK)
if(T_CHECK)
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
	set(REQUIRE_LUT ON)
endif()

# Filter/Displacement
is_feature_enabled(FILTER_DISPLACEMENT T_CHECK)
if(T_CHECK)
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

# Filter/Dynamic Mask
is_feature_enabled(FILTER_DYNAMIC_MASK T_CHECK)
if(T_CHECK)
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

# Filter/SDF Effects
is_feature_enabled(FILTER_SDF_EFFECTS T_CHECK)
if(T_CHECK)
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

# Filter/Shader
is_feature_enabled(FILTER_SHADER T_CHECK)
if(T_CHECK)
	set(REQUIRE_PART_SHADER ON)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-shader.hpp"
		"source/filters/filter-shader.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_SHADER
	)
endif()

# Filter/Transform
is_feature_enabled(FILTER_TRANSFORM T_CHECK)
if(T_CHECK)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-transform.hpp"
		"source/filters/filter-transform.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_TRANSFORM
	)
endif()

# Filter/Denoising
is_feature_enabled(FILTER_DENOISING T_CHECK)
if(T_CHECK)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-denoising.hpp"
		"source/filters/filter-denoising.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_DENOISING
	)
	is_feature_enabled(FILTER_DENOISING_NVIDIA T_CHECK)
	if (T_CHECK)
		list(APPEND PROJECT_DEFINITIONS
			ENABLE_FILTER_DENOISING_NVIDIA
		)
	endif()
endif()

# Filter/Upscaling
is_feature_enabled(FILTER_UPSCALING T_CHECK)
if(T_CHECK)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-upscaling.hpp"
		"source/filters/filter-upscaling.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_UPSCALING
	)
	is_feature_enabled(FILTER_UPSCALING_NVIDIA T_CHECK)
	if (T_CHECK)
		list(APPEND PROJECT_DEFINITIONS
			ENABLE_FILTER_UPSCALING_NVIDIA
		)
	endif()
endif()

# Filter/Virtual Greenscreen
is_feature_enabled(FILTER_VIRTUAL_GREENSCREEN T_CHECK)
if(T_CHECK)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/filters/filter-virtual-greenscreen.hpp"
		"source/filters/filter-virtual-greenscreen.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_FILTER_VIRTUAL_GREENSCREEN
	)
	is_feature_enabled(FILTER_VIRTUAL_GREENSCREEN_NVIDIA T_CHECK)
	if (T_CHECK)
		list(APPEND PROJECT_DEFINITIONS
			ENABLE_FILTER_VIRTUAL_GREENSCREEN_NVIDIA
		)
	endif()
endif()

# Source/Mirror
is_feature_enabled(SOURCE_MIRROR T_CHECK)
if(T_CHECK)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/sources/source-mirror.hpp"
		"source/sources/source-mirror.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_SOURCE_MIRROR
	)
endif()

# Source/Shader
is_feature_enabled(SOURCE_SHADER T_CHECK)
if(T_CHECK)
	set(REQUIRE_PART_SHADER ON)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/sources/source-shader.hpp"
		"source/sources/source-shader.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_SOURCE_SHADER
	)
endif()

# Transition/Shader
is_feature_enabled(TRANSITION_SHADER T_CHECK)
if(T_CHECK)
	set(REQUIRE_PART_SHADER ON)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/transitions/transition-shader.hpp"
		"source/transitions/transition-shader.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_TRANSITION_SHADER
	)
endif()

# Profiling
is_feature_enabled(PROFILING T_CHECK)
if(T_CHECK)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/util/util-profiler.cpp"
		"source/util/util-profiler.hpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_PROFILING
	)
endif()

# Updater
is_feature_enabled(UPDATER T_CHECK)
if(T_CHECK)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/updater.hpp"
		"source/updater.cpp"
	)
	list(APPEND PROJECT_DEFINITIONS
		ENABLE_UPDATER
	)
endif()

# Frontend
is_feature_enabled(FRONTEND T_CHECK)
if(T_CHECK)
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

	is_feature_enabled(UPDATER T_CHECK)
	if(T_CHECK)
		list(APPEND PROJECT_UI_SOURCE
			"source/ui/ui-updater.hpp"
			"source/ui/ui-updater.cpp"
		)
		list(APPEND PROJECT_UI
			"ui/updater.ui"
		)
	endif()
endif()

################################################################################
# Parts
################################################################################

# Shaders
if(REQUIRE_PART_SHADER)
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

# LUT
if(REQUIRE_LUT)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/gfx/lut/gfx-lut.hpp"
		"source/gfx/lut/gfx-lut.cpp"
		"source/gfx/lut/gfx-lut-consumer.hpp"
		"source/gfx/lut/gfx-lut-consumer.cpp"
		"source/gfx/lut/gfx-lut-producer.hpp"
		"source/gfx/lut/gfx-lut-producer.cpp"
	)
	list(APPEND PROJECT_DATA
		"data/effects/lut.effect"
		"data/effects/lut-consumer.effect"
		"data/effects/lut-producer.effect"
	)
endif()

# Windows
if(D_PLATFORM_WINDOWS)
	list(APPEND PROJECT_PRIVATE_SOURCE
		"source/windll.cpp"
	)
	list(APPEND PROJECT_LIBRARIES
		Delayimp.lib
	)
	# Disable/Enable a ton of things.
	list(APPEND PROJECT_DEFINITIONS
		# Microsoft Visual C++
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

# GCC before 9.0, Clang before 9.0
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
# Register Library
################################################################################

# Combine all variables that matter.
set(PROJECT_FILES
	# Always exist
	${PROJECT_DATA}
	${PROJECT_PRIVATE_GENERATED}
	${PROJECT_PRIVATE_SOURCE}
	${PROJECT_TEMPLATES}
	# UI-only (empty if not enabled)
	${PROJECT_UI}
	${PROJECT_UI_SOURCE}
)

# Set source groups for IDE generators.
source_group(TREE "${PROJECT_SOURCE_DIR}/data" PREFIX "Data" FILES ${PROJECT_DATA})
source_group(TREE "${PROJECT_SOURCE_DIR}/source" PREFIX "Source" FILES ${PROJECT_PRIVATE_SOURCE} ${PROJECT_UI_SOURCE})
source_group(TREE "${PROJECT_BINARY_DIR}/generated" PREFIX "Source" FILES ${PROJECT_PRIVATE_GENERATED})
source_group(TREE "${PROJECT_SOURCE_DIR}/templates" PREFIX "Templates" FILES ${PROJECT_TEMPLATES})
source_group(TREE "${PROJECT_SOURCE_DIR}/ui" PREFIX "User Interface" FILES ${PROJECT_UI})

# Prevent unwanted files from being built as source.
set_source_files_properties(${PROJECT_DATA} ${PROJECT_TEMPLATES} ${PROJECT_UI} PROPERTIES
	HEADER_FILE_ONLY ON
)

# Prevent non-UI files from being Qt'd
if(HAVE_QT)
	set_source_files_properties(${PROJECT_DATA} ${PROJECT_TEMPLATES} ${PROJECT_PRIVATE_GENERATED} ${PROJECT_PRIVATE_SOURCE} PROPERTIES
		SKIP_AUTOGEN ON
		SKIP_AUTOMOC ON
		SKIP_AUTORCC ON
		SKIP_AUTOUIC ON
	)
endif()

# Register the library
add_library(${PROJECT_NAME} MODULE ${PROJECT_FILES}) # We are a module for libOBS.
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_DEFINITIONS})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES})

# Always generate position independent code.
set_target_properties(${PROJECT_NAME} PROPERTIES
	POSITION_INDEPENDENT_CODE ON
)

# Set C++ Standard and Extensions
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

# Link-Time/Interprocedural Optimization
if(${PREFIX}ENABLE_LTO)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		INTERPROCEDURAL_OPTIMIZATION ON
	)
endif()

# C/C++ Compiler Adjustments
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
	# MSVC/ClangCL
	# - Dynamically link Microsoft C/C++ Redistributable.
	# - Enable /W3 and disable useless warnings.
	# - Enable C++ exceptions with SEH exceptions.
	# - Enable multi-processor compiling.
	# - Enable updated __cplusplus macro
	message(STATUS "${LOGPREFIX} Applying custom flags for MSVC style build.")

	# Build with dynamic MSVC linkage.
	target_compile_options(${PROJECT_NAME} PUBLIC 
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
	target_compile_options(${PROJECT_NAME} PUBLIC "/W3")
	foreach(WARN ${DISABLED_WARNINGS})
		target_compile_options(${PROJECT_NAME} PUBLIC "${WARN}")
	endforeach()

	# C++ Exceptions & SEH
	target_compile_options(${PROJECT_NAME} PUBLIC "/EHa")

	# Multiprocessor compiling
	target_compile_options(${PROJECT_NAME} PUBLIC "/MP")

	# Updated __cplusplus
	target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# Clang/AppleClang/GNU
	# - Don't export by default. (Temporarily disabled)
	# - Enable all and extra warnings.
	message(STATUS "${LOGPREFIX} Applying custom flags for GNU style build.")

	target_compile_options(${PROJECT_NAME} PUBLIC "-Wall")
	target_compile_options(${PROJECT_NAME} PUBLIC "-Wextra")
	# add_compile_options("-fvisibility=hidden")
endif()

# Remove prefix on other platforms.
set_target_properties(${PROJECT_NAME} PROPERTIES
	PREFIX ""
	IMPORT_PREFIX ""
)

# Set file version (on anything but MacOS)
if(NOT D_PLATFORM_MAC)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
		SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
	)
endif()

# Delay-Loading on Microsoft Visual C++
if(D_PLATFORM_WINDOWS)
	foreach(DELAYLOAD ${PROJECT_LIBRARIES_DELAYED})
		get_target_property(_lf ${PROJECT_NAME} LINK_FLAGS)
		if (NOT _lf)
			set(_lf "")
		endif()
		set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${_lf} /DELAYLOAD:${DELAYLOAD}")
		add_link_options("/DELAYLOAD:${DELAYLOAD}")
	endforeach()
endif()

# Enable Qt if needed
if(HAVE_QT)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		AUTOUIC ON
		AUTOUIC_SEARCH_PATHS "${PROJECT_SOURCE_DIR};${PROJECT_SOURCE_DIR}/ui"
		AUTOMOC ON
		AUTORCC ON
		AUTOGEN_BUILD_DIR "${PROJECT_BINARY_DIR}/generated"
	)
endif()

# MacOS: Disable automatic Code Signing in Xcode
if(D_PLATFORM_MAC)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
		XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
	)
endif()

################################################################################
# Extra Tools
################################################################################

# Clang
is_feature_enabled(CLANG T_CHECK)
if(T_CHECK AND HAVE_CLANG)
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

# Apple otool
if(D_PLATFORM_MAC)
	# OBS
	mac_get_linker_id(TARGET libobs OUTPUT T_OBS_LINK)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND install_name_tool ARGS -change "${T_OBS_LINK}" "@executable_path/../Frameworks/libobs.0.dylib" $<TARGET_FILE:${PROJECT_NAME}>
	)
	message(STATUS "${LOGPREFIX} Added post-build step for adjusting libobs linking path.")

	# OBS Front-End API
	if(REQUIRE_OBSFE AND HAVE_OBSFE)
		mac_get_linker_id(TARGET obs-frontend-api OUTPUT T_OBSFE_LINK)
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND install_name_tool ARGS -change "${T_OBSFE_LINK}" "@executable_path/../Frameworks/libobs-frontend-api.dylib" $<TARGET_FILE:${PROJECT_NAME}>
		)
		message(STATUS "${LOGPREFIX} Added post-build step for adjusting libobs-frontend-api linking path.")
	endif()

	# Qt5
	if(REQUIRE_QT AND HAVE_QT)
		# Figure out the linker location for Qt5::Core
		mac_get_linker_id(TARGET Qt5::Core OUTPUT T_QT5CORE_LINK)

		# Figure out the linker location for Qt5::Gui
		mac_get_linker_id(TARGET Qt5::Gui OUTPUT T_QT5GUI_LINK)

		# Figure out the linker location for Qt5::Widsgets
		mac_get_linker_id(TARGET Qt5::Widgets OUTPUT T_QT5WIDGETS_LINK)

		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		# - QtCore
			COMMAND install_name_tool ARGS -change "${T_QT5CORE_LINK}" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore" $<TARGET_FILE:${PROJECT_NAME}>
		# - QtGui
			COMMAND install_name_tool ARGS -change "${T_QT5GUI_LINK}" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui" $<TARGET_FILE:${PROJECT_NAME}>
		# - QtWidgets
			COMMAND install_name_tool ARGS -change "${T_QT5WIDGETS_LINK}" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets" $<TARGET_FILE:${PROJECT_NAME}>
		)
		message(STATUS "${LOGPREFIX} Added post-build step for adjusting Qt5::Core linking path (Found: ${Qt5_DIR} resolved to ${T_QT5CORE_LINK}).")
		message(STATUS "${LOGPREFIX} Added post-build step for adjusting Qt5::Gui linking path (Found: ${Qt5_DIR} resolved to ${T_QT5GUI_LINK}).")
		message(STATUS "${LOGPREFIX} Added post-build step for adjusting Qt5::Widgets linking path (Found: ${Qt5_DIR} resolved to ${T_QT5WIDGETS_LINK}).")
	endif()
endif()

# Code Sign
if(HAVE_CODESIGN)
	set(_CODESIGN_TIMESTAMP "")
	if(${PREFIX}CODESIGN_TIMESTAMPS)
		set(_CODESIGN_TIMESTAMP "TIMESTAMPS")
	endif()
	codesign(
		TARGETS ${PROJECT_NAME}
		CERTIFICATE_FILE "${_CODESIGN_FILE}"
		CERTIFICATE_NAME "${_CODESIGN_NAME}"
		CERTIFICATE_PASS "${_CODESIGN_PASS}"
		${_CODESIGN_TIMESTAMP}
	)
endif()

################################################################################
# Installation
################################################################################

if(NOT STANDALONE)
	# Grouped builds don't offer standalone services.
	install_obs_plugin_with_data(${PROJECT_NAME} data)

	# Dependency: AOM
	if(HAVE_AOM AND AOM_BINARY AND D_PLATFORM_WINDOWS)
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND "${CMAKE_COMMAND}" -E copy
				"${AOM_BINARY}"
				"${CMAKE_BINARY_DIR}/rundir/$<CONFIGURATION>/data/obs-plugins/${PROJECT_NAME}"
			VERBATIM)
	endif()
else()
	if(STRUCTURE_UNIFIED)
		install(
			DIRECTORY "data/"
			DESTINATION "data/"
			FILE_PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
			DIRECTORY_PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
		)

		if(D_PLATFORM_WINDOWS)
			install(
				TARGETS ${PROJECT_NAME}
				RUNTIME DESTINATION "bin/windows-${D_PLATFORM_INSTR}-${D_PLATFORM_BITS}/" COMPONENT StreamFX
				LIBRARY DESTINATION "bin/windows-${D_PLATFORM_INSTR}-${D_PLATFORM_BITS}/" COMPONENT StreamFX
				PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
			)
			if(MSVC)
				install(
					FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
					DESTINATION "bin/windows-${D_PLATFORM_INSTR}-${D_PLATFORM_BITS}/"
					COMPONENT StreamFX
					OPTIONAL
				)
			endif()

			# Dependency: AOM
			if(HAVE_AOM AND AOM_BINARY AND D_PLATFORM_WINDOWS)
				install(
					FILES ${AOM_BINARY}
					DESTINATION "data/" COMPONENT StreamFX
				)
			endif()
		elseif(D_PLATFORM_LINUX)
			install(
				TARGETS ${PROJECT_NAME}
				RUNTIME DESTINATION "bin/linux-${D_PLATFORM_INSTR}-${D_PLATFORM_BITS}/" COMPONENT StreamFX
				LIBRARY DESTINATION "bin/linux-${D_PLATFORM_INSTR}-${D_PLATFORM_BITS}/" COMPONENT StreamFX
				PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
			)
		elseif(D_PLATFORM_MAC)
			install(
				TARGETS ${PROJECT_NAME}
				RUNTIME DESTINATION "bin/mac-${D_PLATFORM_INSTR}-${D_PLATFORM_BITS}/" COMPONENT StreamFX
				LIBRARY DESTINATION "bin/mac-${D_PLATFORM_INSTR}-${D_PLATFORM_BITS}/" COMPONENT StreamFX
				PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
			)
		endif()

		install(
			FILES LICENSE
			DESTINATION "LICENSE"
			COMPONENT StreamFX
		)
		install(
			FILES icon.png
			DESTINATION "icon.png"
			COMPONENT StreamFX
		)
	elseif(D_PLATFORM_WINDOWS)
		install(
			TARGETS ${PROJECT_NAME}
			RUNTIME DESTINATION "obs-plugins/${D_PLATFORM_BITS}bit/" COMPONENT StreamFX
			LIBRARY DESTINATION "obs-plugins/${D_PLATFORM_BITS}bit/" COMPONENT StreamFX
		)
		install(
			DIRECTORY "data/"
			DESTINATION "data/obs-plugins/${PROJECT_NAME}/"
		)
		if(MSVC)
			install(
				FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
				DESTINATION "obs-plugins/${D_PLATFORM_BITS}bit/"
				OPTIONAL
			)
		endif()

		# Dependency: AOM
		if(HAVE_AOM AND AOM_BINARY AND D_PLATFORM_WINDOWS)
			install(
				FILES ${AOM_BINARY}
				DESTINATION "data/obs-plugins/${PROJECT_NAME}/" COMPONENT StreamFX
			)
		endif()
	elseif(D_PLATFORM_LINUX)
		if(STRUCTURE_PACKAGEMANAGER)
			install(
				TARGETS ${PROJECT_NAME}
				RUNTIME DESTINATION "lib/obs-plugins/" COMPONENT StreamFX
				LIBRARY DESTINATION "lib/obs-plugins/" COMPONENT StreamFX
				PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
			)
			install(
				DIRECTORY "data/"
				DESTINATION "share/obs/obs-plugins/${PROJECT_NAME}"
				COMPONENT StreamFX
				FILE_PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
				DIRECTORY_PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
			)
		else()
			install(
				TARGETS ${PROJECT_NAME}
				RUNTIME DESTINATION "plugins/${PROJECT_NAME}/bin/${D_PLATFORM_BITS}bit/" COMPONENT StreamFX
				LIBRARY DESTINATION "plugins/${PROJECT_NAME}/bin/${D_PLATFORM_BITS}bit/" COMPONENT StreamFX
				PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
			)
			install(
				DIRECTORY "data/"
				DESTINATION "plugins/${PROJECT_NAME}/data/"
				COMPONENT StreamFX
				FILE_PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
				DIRECTORY_PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
			)
		endif()
	elseif(D_PLATFORM_MAC)
		install(
			TARGETS ${PROJECT_NAME}
			RUNTIME DESTINATION "${PROJECT_NAME}/bin/" COMPONENT StreamFX
			LIBRARY DESTINATION "${PROJECT_NAME}/bin/" COMPONENT StreamFX
			PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
		)
		install(
			DIRECTORY "data/"
			DESTINATION "${PROJECT_NAME}/data/"
			COMPONENT StreamFX
			FILE_PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
			DIRECTORY_PERMISSIONS WORLD_EXECUTE;WORLD_READ;OWNER_EXECUTE;OWNER_READ;OWNER_WRITE;GROUP_EXECUTE;GROUP_READ;GROUP_WRITE
		)
	endif()
endif()

################################################################################
# Packaging
################################################################################

if(STANDALONE)
	# Packaging
	if(NOT PACKAGE_SUFFIX)
		set(_PACKAGE_SUFFIX_OVERRIDE "${VERSION_STRING}")
	else()
		set(_PACKAGE_SUFFIX_OVERRIDE "${PACKAGE_SUFFIX}")
	endif()
	set(_PACKAGE_FULL_NAME "${PACKAGE_PREFIX}/${PACKAGE_NAME}-${_PACKAGE_SUFFIX_OVERRIDE}")

	if(STRUCTURE_UNIFIED)
		add_custom_target(
			PACKAGE_ZIP
			${CMAKE_COMMAND} -E tar cfv "${_PACKAGE_FULL_NAME}.obs" --format=zip --
				"."
			WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
		)
	else()
		add_custom_target(
			PACKAGE_7Z
			${CMAKE_COMMAND} -E tar cfv "${_PACKAGE_FULL_NAME}.7z" --format=7zip --
				"."
			WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
		)
		add_custom_target(
			PACKAGE_ZIP
			${CMAKE_COMMAND} -E tar cfv "${_PACKAGE_FULL_NAME}.zip" --format=zip --
				"."
			WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
		)

		# Windows
		if(D_PLATFORM_WINDOWS)
			## Installer (InnoSetup)
			get_filename_component(ISS_FILES_DIR "${CMAKE_INSTALL_PREFIX}" ABSOLUTE)
			file(TO_NATIVE_PATH "${ISS_FILES_DIR}" ISS_FILES_DIR)

			get_filename_component(ISS_PACKAGE_DIR "${PACKAGE_PREFIX}" ABSOLUTE)
			file(TO_NATIVE_PATH "${ISS_PACKAGE_DIR}" ISS_PACKAGE_DIR)

			get_filename_component(ISS_SOURCE_DIR "${PROJECT_SOURCE_DIR}" ABSOLUTE)
			file(TO_NATIVE_PATH "${ISS_SOURCE_DIR}" ISS_SOURCE_DIR)

			get_filename_component(ISS_MSVCHELPER_PATH "${msvc-redist-helper_BUILD_DIR}" ABSOLUTE)
			file(TO_NATIVE_PATH "${ISS_MSVCHELPER_PATH}" ISS_MSVCHELPER_PATH)

			if(HAVE_CODESIGN)
				codesign_command_win32(
					SHA1
					RETURN_BIN ISS_CODESIGN_BIN_SHA1
					RETURN_ARGS ISS_CODESIGN_CMD_SHA1
					CERTIFICATE_FILE "${_CODESIGN_FILE}"
					CERTIFICATE_NAME "${_CODESIGN_NAME}"
					CERTIFICATE_PASS "${_CODESIGN_PASS}"
					${_CODESIGN_TIMESTAMP}
				)
				codesign_command_win32(
					SHA2 APPEND
					RETURN_BIN ISS_CODESIGN_BIN_SHA2
					RETURN_ARGS ISS_CODESIGN_CMD_SHA2
					CERTIFICATE_FILE "${_CODESIGN_FILE}"
					CERTIFICATE_NAME "${_CODESIGN_NAME}"
					CERTIFICATE_PASS "${_CODESIGN_PASS}"
					${_CODESIGN_TIMESTAMP}
				)
				list(JOIN ISS_CODESIGN_CMD_SHA1 " " ISS_CODESIGN_CMD_SHA1)
				list(JOIN ISS_CODESIGN_CMD_SHA2 " " ISS_CODESIGN_CMD_SHA2)

				configure_file(
					"templates/installer-signed.iss.in"
					"installer.iss"
				)
			else()
				configure_file(
					"templates/installer.iss.in"
					"installer.iss"
				)
			endif()
		endif()

		# Apple MacOS
		if(D_PLATFORM_MAC)
			# .pkg Installer
			configure_file(
				"templates/installer.pkgproj.in"
				"installer.pkgproj"
				@ONLY
			)
		endif()
	endif()
endif()
