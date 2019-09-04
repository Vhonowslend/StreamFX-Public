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
CMake_Minimum_Required(VERSION 3.8.0)
Include("cmake/util.cmake")

# Automatic Versioning
Set(VERSION_MAJOR 0)
Set(VERSION_MINOR 7)
Set(VERSION_PATCH 1)
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
set(PROJECT_DESCRIPTION "New Sources, Filters and Transitions for OBS Studio")
set(PROJECT_AUTHORS "Michael Fabian 'Xaymar' Dirks <info@xaymar.com>")
set(PROJECT_COPYRIGHT_YEARS "2018 - 2019")

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

# All Warnings, Extra Warnings, Pedantic
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	# using Clang
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-missing-braces -Wmissing-field-initializers -Wno-c++98-compat-pedantic -Wold-style-cast -Wno-documentation -Wno-documentation-unknown-command -Wno-covered-switch-default -Wno-switch-enum")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-missing-braces -Wmissing-field-initializers -Wno-c++98-compat-pedantic -Wold-style-cast -Wno-documentation -Wno-documentation-unknown-command -Wno-covered-switch-default -Wno-switch-enum")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# GCC: -fpermissive is required as GCC does not allow the same template to be in different namespaces.
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wpedantic -fpermissive -Wno-long-long -Wno-missing-braces -Wmissing-field-initializers")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -fpermissive -Wno-long-long -Wno-missing-braces -Wmissing-field-initializers")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	# using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(MSVC_EXTRA_FLAGS "/diagnostics:column /wd4201 /wd4514 /wd4623 /wd4625 /wd4626 /wd4820 /wd5026 /wd5027")
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
endif()

# C++ Standard and Extensions
## Use C++17 and no non-standard extensions.
SET(_CXX_STANDARD 17)
SET(_CXX_EXTENSIONS OFF)

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
	set(${PropertyPrefix}OBS_DOWNLOAD_VERSION "24.0.0-rc2-ci" CACHE STRING "OBS Studio Version to download")
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
		set(${PropertyPrefix}OBS_DOWNLOAD_URL "https://github.com/Xaymar/obs-studio/releases/download/${OBS_DOWNLOAD_VERSION}/obs-studio-${ARCH}-0.0.0.0-vs2017.7z")
	endif()
endif()

# CMake Modules
if(${PropertyPrefix}OBS_DOWNLOAD)
	include("cmake/DownloadProject.cmake")
endif()

# Load OBS Studio
if(${PropertyPrefix}OBS_NATIVE)
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
	"${PROJECT_SOURCE_DIR}/data/effects/channel-mask.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/color-conversion.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/color-grade.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/displace.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/mask.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/mipgen.effect"

	# Blur
	"${PROJECT_SOURCE_DIR}/data/effects/blur/box.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/blur/box-linear.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/blur/dual-filtering.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/blur/gaussian.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/blur/gaussian-linear.effect"

	# Signed Distance Field
	"${PROJECT_SOURCE_DIR}/data/effects/sdf/sdf-producer.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/sdf/sdf-consumer.effect"
)
SET(PROJECT_DATA_SHADERS
#	"${PROJECT_SOURCE_DIR}/data/shaders/name.effect"
	"${PROJECT_SOURCE_DIR}/data/shaders/filter/example.effect"
)
SET(PROJECT_DATA
	${PROJECT_DATA_LOCALE}
	${PROJECT_DATA_EFFECTS}
	${PROJECT_DATA_SHADERS}
)

SET(PROJECT_LIBRARIES
)

SET(PROJECT_TEMPLATES
	"${PROJECT_SOURCE_DIR}/cmake/version.hpp.in"
	"${PROJECT_SOURCE_DIR}/cmake/module.cpp.in"
)
if(WIN32)
	list(APPEND PROJECT_TEMPLATES
		"${PROJECT_SOURCE_DIR}/cmake/installer.iss.in"
		"${PROJECT_SOURCE_DIR}/cmake/version.rc.in"
	)
endif()

SET(PROJECT_PUBLIC
)
SET(PROJECT_PRIVATE_GENERATED
	"${PROJECT_BINARY_DIR}/source/module.cpp"
	"${PROJECT_BINARY_DIR}/source/version.hpp"
)
SET(PROJECT_PRIVATE_SOURCE

	# Plugin
	"${PROJECT_SOURCE_DIR}/source/plugin.hpp"
	"${PROJECT_SOURCE_DIR}/source/plugin.cpp"
	"${PROJECT_SOURCE_DIR}/source/strings.hpp"
	"${PROJECT_SOURCE_DIR}/source/utility.hpp"
	"${PROJECT_SOURCE_DIR}/source/utility.cpp"
	"${PROJECT_SOURCE_DIR}/source/util-event.hpp"
	"${PROJECT_SOURCE_DIR}/source/util-event.cpp"
	"${PROJECT_SOURCE_DIR}/source/util-math.hpp"
	"${PROJECT_SOURCE_DIR}/source/util-math.cpp"
	"${PROJECT_SOURCE_DIR}/source/util-memory.hpp"
	"${PROJECT_SOURCE_DIR}/source/util-memory.cpp"
	
	# Graphics
	"${PROJECT_SOURCE_DIR}/source/gfx/gfx-effect-source.hpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/gfx-effect-source.cpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/gfx-source-texture.hpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/gfx-source-texture.cpp"
	# Graphics/Blur
	"${PROJECT_SOURCE_DIR}/source/gfx/blur/gfx-blur-base.hpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/blur/gfx-blur-base.cpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/blur/gfx-blur-box.hpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/blur/gfx-blur-box.cpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/blur/gfx-blur-box-linear.hpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/blur/gfx-blur-box-linear.cpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/blur/gfx-blur-dual-filtering.hpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/blur/gfx-blur-dual-filtering.cpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/blur/gfx-blur-gaussian.hpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/blur/gfx-blur-gaussian.cpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/blur/gfx-blur-gaussian-linear.hpp"
	"${PROJECT_SOURCE_DIR}/source/gfx/blur/gfx-blur-gaussian-linear.cpp"

	# OBS
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-helper.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-helper.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-effect.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-effect.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-indexbuffer.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-indexbuffer.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-limits.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-mipmapper.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-mipmapper.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-rendertarget.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-rendertarget.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-sampler.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-sampler.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-texture.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-texture.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-vertex.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-vertex.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-vertexbuffer.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/gs/gs-vertexbuffer.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs/obs-tools.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/obs-tools.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs/obs-source.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/obs-source.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs/obs-source-tracker.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs/obs-source-tracker.cpp"
	
	# Sources
	"${PROJECT_SOURCE_DIR}/source/sources/source-mirror.hpp"
	"${PROJECT_SOURCE_DIR}/source/sources/source-mirror.cpp"
	"${PROJECT_SOURCE_DIR}/source/sources/source-shader.hpp"
	"${PROJECT_SOURCE_DIR}/source/sources/source-shader.cpp"

	# Filters
	"${PROJECT_SOURCE_DIR}/source/filters/filter-blur.hpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-blur.cpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-color-grade.hpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-color-grade.cpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-displacement.hpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-displacement.cpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-dynamic-mask.hpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-dynamic-mask.cpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-sdf-effects.hpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-sdf-effects.cpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-shader.hpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-shader.cpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-transform.hpp"
	"${PROJECT_SOURCE_DIR}/source/filters/filter-transform.cpp"

	# Transitions
)
set(PROJECT_PRIVATE
	${PROJECT_DATA}
	${PROJECT_PRIVATE_GENERATED}
	${PROJECT_PRIVATE_SOURCE}
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
