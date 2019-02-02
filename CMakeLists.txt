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
CMake_Minimum_Required(VERSION 3.1.0)
Include("cmake/util.cmake")

# Automatic Versioning
Set(VERSION_MAJOR 0)
Set(VERSION_MINOR 4)
Set(VERSION_PATCH 2)
Set(VERSION_TWEAK 0)
Set(PROJECT_COMMIT "N/A")
If(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
	Set(GIT_RESULT "")
	Set(GIT_OUTPUT "")
	EXECUTE_PROCESS(
		COMMAND git rev-list --count --topo-order ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}..HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		RESULT_VARIABLE GIT_RESULT
		OUTPUT_VARIABLE GIT_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	If(GIT_RESULT EQUAL 0)
		Set(VERSION_TWEAK ${GIT_OUTPUT})
	EndIf()
	EXECUTE_PROCESS(
		COMMAND git rev-parse HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		RESULT_VARIABLE GIT_RESULT
		OUTPUT_VARIABLE GIT_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	If(GIT_RESULT EQUAL 0)
		Set(PROJECT_COMMIT ${GIT_OUTPUT})
	EndIf()
EndIf()

# Define Project
PROJECT(
	obs-stream-effects
	VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}
)
set(PROJECT_FULL_NAME "Stream Effects for OBS Studio")
set(PROJECT_DESCRIPTION "New Sources, Filters and Transitions for OB Studio")
set(PROJECT_AUTHORS "Michael Fabian 'Xaymar' Dirks <info@xaymar.com>")
set(PROJECT_COPYRIGHT_YEARS "2018")

################################################################################
# Setup / Bootstrap
################################################################################

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
else()
	set(ARCH "x64")
endif()

# Search Path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

################################################################################
# CMake / Compiler
################################################################################

# Configure Version Header
configure_file(
	"${PROJECT_SOURCE_DIR}/cmake/version.hpp.in"
	"${PROJECT_BINARY_DIR}/source/version.hpp"
)
configure_file(
	"${PROJECT_SOURCE_DIR}/cmake/module.cpp.in"
	"${PROJECT_BINARY_DIR}/source/module.cpp"
)

# Windows
if(WIN32)
	## Installer (InnoSetup)
	Configure_File(
		"${PROJECT_SOURCE_DIR}/cmake/installer.iss.in"
		"${PROJECT_BINARY_DIR}/installer.iss"
	)

	# Windows Specific Resource Definition
	set(PROJECT_PRODUCT_NAME "${PROJECT_FULL_NAME}")
	set(PROJECT_COMPANY_NAME "${PROJECT_AUTHORS}")
	set(PROJECT_COPYRIGHT "${PROJECT_AUTHORS} © ${PROJECT_COPYRIGHT_YEARS}")
	set(PROJECT_LEGAL_TRADEMARKS_1 "")
	set(PROJECT_LEGAL_TRADEMARKS_2 "")
	
	configure_file(
		"${PROJECT_SOURCE_DIR}/cmake/version.rc.in"
		"${PROJECT_BINARY_DIR}/cmake/version.rc"
		@ONLY
	)	
endif()

# All Warnings, Extra Warnings, Pedantic
if(MSVC)
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	# Update if necessary
	# GCC: -fpermissive is required as GCC does not allow the same template to be in different namespaces.
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -Wpedantic -fpermissive")
endif()

# C++ Standard and Extensions
IF (CMAKE_VERSION VERSION_LESS "3.8")
    SET(CMAKE_CXX_STANDARD 14)
ELSEIF (CMAKE_VERSION VERSION_LESS "3.11")
    SET(CMAKE_CXX_STANDARD 17)
ELSE() # CMake 3.11 or higher:
    SET(CMAKE_CXX_STANDARD 20)
ENDIF()
## Disable nonstandard extensions
SET(CMAKE_CXX_EXTENSIONS OFF)

################################################################################
# Options
################################################################################
set(${PropertyPrefix}OBS_NATIVE FALSE CACHE BOOL "Use native obs-studio build" FORCE)
set(${PropertyPrefix}OBS_REFERENCE FALSE CACHE BOOL "Use referenced obs-studio build" FORCE)
set(${PropertyPrefix}OBS_PACKAGE FALSE CACHE BOOL "Use packaged obs-studio build" FORCE)
set(${PropertyPrefix}OBS_DOWNLOAD FALSE CACHE BOOL "Use downloaded obs-studio build" FORCE)
mark_as_advanced(FORCE OBS_NATIVE OBS_PACKAGE OBS_REFERENCE OBS_DOWNLOAD)

if(NOT TARGET libobs)
	set(${PropertyPrefix}OBS_STUDIO_DIR "" CACHE PATH "OBS Studio Source/Package Directory")
	set(${PropertyPrefix}OBS_DOWNLOAD_VERSION "22.0.2" CACHE STRING "OBS Studio Version to download")
endif()

if(NOT ${PropertyPrefix}OBS_NATIVE)
	set(${PropertyPrefix}OBS_DEPENDENCIES_DIR "" CACHE PATH "Path to OBS Dependencies")
	set(CMAKE_PACKAGE_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Path for generated archives.")
	set(CMAKE_PACKAGE_NAME "${PROJECT_NAME}" CACHE STRING "Name for the generated archives.")
	set(CMAKE_PACKAGE_SUFFIX_OVERRIDE "" CACHE STRING "Override for the suffix.")
endif()

################################################################################
# Dependencies
################################################################################

# Detect OBS Studio Type
if(TARGET libobs)
	message(STATUS "${PROJECT_NAME}: Using native obs-studio.")
	CacheSet(${PropertyPrefix}OBS_NATIVE TRUE)	
else()
	CacheSet(${PropertyPrefix}OBS_NATIVE FALSE)
	if(EXISTS "${OBS_STUDIO_DIR}/cmake/LibObs/LibObsConfig.cmake")
		message(STATUS "${PROJECT_NAME}: Using packaged obs-studio.")
		CacheSet(${PropertyPrefix}OBS_PACKAGE TRUE)
	elseif(EXISTS "${OBS_STUDIO_DIR}/libobs/obs-module.h")
		message(STATUS "${PROJECT_NAME}: Using referenced obs-studio.")
		CacheSet(${PropertyPrefix}OBS_REFERENCE TRUE)
	else()
		message(STATUS "${PROJECT_NAME}: No OBS Studio detected, using downloadable prebuilt binaries.")
		CacheSet(${PropertyPrefix}OBS_DOWNLOAD TRUE)
		set(${PropertyPrefix}OBS_DOWNLOAD_URL "https://github.com/Xaymar/obs-studio/releases/download/${OBS_DOWNLOAD_VERSION}/obs-studio-${ARCH}-vs2017.7z")
	endif()
endif()

# CMake Modules
if(${PropertyPrefix}OBS_DOWNLOAD)
	include("cmake/DownloadProject.cmake")
endif()
if(NOT ${PropertyPrefix}OBS_NATIVE)
	include("cmake/cppcheck.cmake")
endif()

# Load OBS Studio
if(${PropertyPrefix}OBS_NATIVE)
	option(BUILD_FFMPEG_ENCODER "Build AMD Encoder module" ON)	
	if (NOT BUILD_FFMPEG_ENCODER)
		message(STATUS "Not building AMD Encoder")
		return()
	endif()
elseif(${PropertyPrefix}OBS_PACKAGE)
	include("${OBS_STUDIO_DIR}/cmake/LibObs/LibObsConfig.cmake")
elseif(${PropertyPrefix}OBS_REFERENCE)
	set(obsPath "${OBS_STUDIO_DIR}")
	include("${OBS_STUDIO_DIR}/cmake/external/FindLibobs.cmake")
elseif(${PropertyPrefix}OBS_DOWNLOAD)
	download_project(
		PROJ libobs
		URL ${OBS_DOWNLOAD_URL}
		UPDATE_DISCONNECTED 1
	)
	include("${libobs_SOURCE_DIR}/cmake/LibObs/LibObsConfig.cmake")
else()
	message(CRITICAL "Impossible case reached, verify system stability.")
	return()
endif()

################################################################################
# Code
################################################################################
SET(PROJECT_DATA_LOCALE
	"${PROJECT_SOURCE_DIR}/data/locale/en-US.ini"
)
SET(PROJECT_DATA_EFFECTS
	"${PROJECT_SOURCE_DIR}/data/effects/blur.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/color-conversion.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/displace.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/mask.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/mipgen.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/sdf-generator.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/sdf-shadow.effect"
)
SET(PROJECT_DATA_SHADERS
#	"${PROJECT_SOURCE_DIR}/data/shaders/name.effect"
)
SET(PROJECT_DATA_SHADERS_FILTER
	"${PROJECT_SOURCE_DIR}/data/shaders/filter/example.effect"
)
SET(PROJECT_LIBRARIES
)
SET(PROJECT_PUBLIC
)
SET(PROJECT_PRIVATE
	${PROJECT_DATA_LOCALE}
	${PROJECT_DATA_EFFECTS}
	${PROJECT_DATA_SHADERS}
	${PROJECT_DATA_SHADERS_FILTER}
	"${PROJECT_BINARY_DIR}/source/module.cpp"
	"${PROJECT_BINARY_DIR}/source/version.hpp"
	"${PROJECT_SOURCE_DIR}/source/plugin.hpp"
	"${PROJECT_SOURCE_DIR}/source/plugin.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-displacement.hpp"
	"${PROJECT_SOURCE_DIR}/source/filter-displacement.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-blur.hpp"
	"${PROJECT_SOURCE_DIR}/source/filter-blur.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-sdf-effects.hpp"
	"${PROJECT_SOURCE_DIR}/source/filter-sdf-effects.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-shape.hpp"
	"${PROJECT_SOURCE_DIR}/source/filter-shape.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-transform.hpp"
	"${PROJECT_SOURCE_DIR}/source/filter-transform.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-custom-shader.hpp"
	"${PROJECT_SOURCE_DIR}/source/filter-custom-shader.cpp"
	"${PROJECT_SOURCE_DIR}/source/source-mirror.hpp"
	"${PROJECT_SOURCE_DIR}/source/source-mirror.cpp"
	"${PROJECT_SOURCE_DIR}/source/gfx-effect-source.hpp"
	"${PROJECT_SOURCE_DIR}/source/gfx-effect-source.cpp"
	"${PROJECT_SOURCE_DIR}/source/gfx-source-texture.hpp"
	"${PROJECT_SOURCE_DIR}/source/gfx-source-texture.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-helper.hpp"
	"${PROJECT_SOURCE_DIR}/source/gs-helper.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-effect.hpp"
	"${PROJECT_SOURCE_DIR}/source/gs-effect.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-indexbuffer.hpp"
	"${PROJECT_SOURCE_DIR}/source/gs-indexbuffer.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-limits.hpp"
	"${PROJECT_SOURCE_DIR}/source/gs-mipmapper.hpp"
	"${PROJECT_SOURCE_DIR}/source/gs-mipmapper.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-rendertarget.hpp"
	"${PROJECT_SOURCE_DIR}/source/gs-rendertarget.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-sampler.hpp"
	"${PROJECT_SOURCE_DIR}/source/gs-sampler.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-texture.hpp"
	"${PROJECT_SOURCE_DIR}/source/gs-texture.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-vertex.hpp"
	"${PROJECT_SOURCE_DIR}/source/gs-vertex.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-vertexbuffer.hpp"
	"${PROJECT_SOURCE_DIR}/source/gs-vertexbuffer.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs-audio-capture.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs-audio-capture.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs-tools.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs-tools.cpp"
	"${PROJECT_SOURCE_DIR}/source/strings.hpp"
	"${PROJECT_SOURCE_DIR}/source/utility.hpp"
	"${PROJECT_SOURCE_DIR}/source/utility.cpp"
	"${PROJECT_SOURCE_DIR}/source/util-event.hpp"
	"${PROJECT_SOURCE_DIR}/source/util-event.cpp"
	"${PROJECT_SOURCE_DIR}/source/util-math.hpp"
	"${PROJECT_SOURCE_DIR}/source/util-math.cpp"
	"${PROJECT_SOURCE_DIR}/source/util-memory.hpp"
	"${PROJECT_SOURCE_DIR}/source/util-memory.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs-source.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs-source.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs-source-tracker.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs-source-tracker.cpp"
)

source_group("Data Files\\Locale" FILES ${PROJECT_DATA_LOCALE})
source_group("Data Files\\Effects" FILES ${PROJECT_DATA_EFFECTS})
source_group("Data Files\\Shaders" FILES ${PROJECT_DATA_SHADERS})
source_group("Data Files\\Shaders\\Filter" FILES ${PROJECT_DATA_SHADERS_FILTER})

################################################################################
# Target
################################################################################

add_library(${PROJECT_NAME} MODULE
	${PROJECT_PUBLIC}
	${PROJECT_PRIVATE}
)

# Include Directories
target_include_directories(${PROJECT_NAME}
	PUBLIC
	PRIVATE
		"${PROJECT_BINARY_DIR}/source"
		"${PROJECT_SOURCE_DIR}/source"
		${FFMPEG_INCLUDE_DIRS}
)

# OBS Studio
if(${PropertyPrefix}OBS_NATIVE)
	target_link_libraries(${PROJECT_NAME}
		libobs
	)
elseif(${PropertyPrefix}OBS_REFERENCE)
	target_include_directories(${PROJECT_NAME}
		PRIVATE
			"${OBS_STUDIO_DIR}/libobs"
	)
	target_link_libraries(${PROJECT_NAME}
		"${LIBOBS_LIB}"
	)
elseif(${PropertyPrefix}OBS_PACKAGE)
	target_include_directories(${PROJECT_NAME}
		PRIVATE
			"${OBS_STUDIO_DIR}/include"
	)
	target_link_libraries(${PROJECT_NAME}
		libobs
	)
elseif(${PropertyPrefix}OBS_DOWNLOAD)
	target_link_libraries(${PROJECT_NAME}
		libobs
	)
endif()

# Link Libraries
target_link_libraries(${PROJECT_NAME}
	"${PROJECT_LIBRARIES}"
)

# Definitions
if (WIN32)
	target_compile_definitions(${PROJECT_NAME}
		PRIVATE
			_CRT_SECURE_NO_WARNINGS
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
			NOCTLMGR
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
			NOHELP
			NOPROFILER
			NODEFERWINDOWPOS
			NOMCX
			NOIME
			NOMDI
			NOINOUT
	)
endif()

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

# CPPCheck
if(NOT ${PropertyPrefix}OBS_NATIVE)
	set(excludes )
	list(APPEND excludes "${OBS_DEPENDENCIES_DIR}")
	if(${PropertyPrefix}OBS_REFERENCE)
		list(APPEND excludes "${OBS_STUDIO_DIR}/libobs")
	elseif(${PropertyPrefix}OBS_PACKAGE)
		list(APPEND excludes "${OBS_STUDIO_DIR}/libobs")
	elseif(${PropertyPrefix}OBS_DOWNLOAD)
		list(APPEND excludes "${libobs_SOURCE_DIR}")
	endif()

	cppcheck(
		EXCLUDE ${excludes}
	)
	cppcheck_add_project(${PROJECT_NAME})
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
		DIRECTORY "${PROJECT_SOURCE_DIR}/data/"
		DESTINATION "./data/obs-plugins/${PROJECT_NAME}/"
	)
	
	if("${CMAKE_PACKAGE_SUFFIX_OVERRIDE}" STREQUAL "")
		set(PackageFullName "${CMAKE_PACKAGE_PREFIX}/${CMAKE_PACKAGE_NAME}-${PROJECT_VERSION}")
	else()
		set(PackageFullName "${CMAKE_PACKAGE_PREFIX}/${CMAKE_PACKAGE_NAME}-${CMAKE_PACKAGE_SUFFIX_OVERRIDE}")
	endif()
	
	add_custom_target(
		PACKAGE_7Z
		${CMAKE_COMMAND} -E tar cfv "${PackageFullName}.7z" --format=7zip --
			"${CMAKE_INSTALL_PREFIX}/obs-plugins"
			"${CMAKE_INSTALL_PREFIX}/data"
		WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
	)
	add_custom_target(
		PACKAGE_ZIP
		${CMAKE_COMMAND} -E tar cfv "${PackageFullName}.zip" --format=zip --
			"${CMAKE_INSTALL_PREFIX}/obs-plugins"
			"${CMAKE_INSTALL_PREFIX}/data"
		WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
	)
endif()
