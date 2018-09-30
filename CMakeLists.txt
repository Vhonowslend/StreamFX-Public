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
Include("cmake/DownloadProject.cmake")
Include("cmake/cppcheck.cmake")

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
	DESCRIPTION ""
	HOMEPAGE_URL "https://www.xaymar.com/portfolio/obs-stream-effects/"
)

################################################################################
# CMake / Compiler
################################################################################

# Detect Build Type
If("${CMAKE_SOURCE_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
	Set(PropertyPrefix "")
Else()
	Set(PropertyPrefix "${PROJECT_NAME}_")
EndIf()

# Detect Architecture
math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")
IF("${BITS}" STREQUAL "32")
	SET(ARCH "x86")
Else()
	SET(ARCH "x64")
ENDIF()

# Configure Installer script
Configure_File(
	"${PROJECT_SOURCE_DIR}/ci/installer.iss.in"
	"${PROJECT_BINARY_DIR}/ci/installer.iss"
)

# Configure Version Header
Configure_File(
	"${PROJECT_SOURCE_DIR}/source/version.h.in"
	"${PROJECT_BINARY_DIR}/source/version.h"
)

# Windows Specific Resource Definition
If(WIN32)
	Set(PROJECT_PRODUCT_NAME "OBS Studio Stream Effects")
	Set(PROJECT_COMPANY_NAME "Xaymar")
	Set(PROJECT_COPYRIGHT "Xaymar © 2017 - 2018")
	Set(PROJECT_LEGAL_TRADEMARKS_1 "")
	Set(PROJECT_LEGAL_TRADEMARKS_2 "")
	Set(PROJECT_DESCRIPTION "Stream Effects for OBS Studio")
	
	Configure_File(
		"${PROJECT_SOURCE_DIR}/cmake/version.rc.in"
		"${PROJECT_BINARY_DIR}/cmake/version.rc"
		@ONLY
	)	
EndIf()

# All Warnings, Extra Warnings, Pedantic
If(MSVC)
	# Force to always compile with W4
	If(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		String(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	Else()
		Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	EndIf()
ElseIf(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	# Update If necessary
	Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
EndIf()

math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

################################################################################
# Options
################################################################################
Set(${PropertyPrefix}OBS_NATIVE FALSE CACHE BOOL "Use native obs-studio build" FORCE)
Set(${PropertyPrefix}OBS_REFERENCE FALSE CACHE BOOL "Use referenced obs-studio build" FORCE)
Set(${PropertyPrefix}OBS_PACKAGE FALSE CACHE BOOL "Use packaged obs-studio build" FORCE)
Set(${PropertyPrefix}OBS_DOWNLOAD FALSE CACHE BOOL "Use downloaded obs-studio build" FORCE)
MARK_AS_ADVANCED(FORCE OBS_NATIVE OBS_PACKAGE OBS_REFERENCE OBS_DOWNLOAD)

If(NOT TARGET libobs)
	Set(${PropertyPrefix}OBS_STUDIO_DIR "" CACHE PATH "OBS Studio Source/Package Directory")
	Set(${PropertyPrefix}OBS_DOWNLOAD_VERSION "22.0.2" CACHE STRING "OBS Studio Version to download")
EndIf()

If(NOT ${PropertyPrefix}OBS_NATIVE)
	Set(CMAKE_PACKAGE_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Path for generated archives.")
	Set(CMAKE_PACKAGE_NAME "${PROJECT_NAME}" CACHE STRING "Name for the generated archives.")
	Set(CMAKE_PACKAGE_SUFFIX_OVERRIDE "" CACHE STRING "Override for the suffix.")
EndIf()

################################################################################
# Dependencies
################################################################################

# Detect OBS Studio Type
If(TARGET libobs)
	Message(STATUS "${PROJECT_NAME}: Using native obs-studio.")
	CacheSet(${PropertyPrefix}OBS_NATIVE TRUE)	
Else()
	CacheSet(${PropertyPrefix}OBS_NATIVE FALSE)
	IF(EXISTS "${OBS_STUDIO_DIR}/cmake/LibObs/LibObsConfig.cmake")
		Message(STATUS "${PROJECT_NAME}: Using packaged obs-studio.")
		CacheSet(${PropertyPrefix}OBS_PACKAGE TRUE)
	ElseIf(EXISTS "${OBS_STUDIO_DIR}/libobs/obs-module.h")
		Message(STATUS "${PROJECT_NAME}: Using referenced obs-studio.")
		CacheSet(${PropertyPrefix}OBS_REFERENCE TRUE)
	Else()
		Message(STATUS "${PROJECT_NAME}: No OBS Studio detected, using downloadable prebuilt binaries.")
		CacheSet(${PropertyPrefix}OBS_DOWNLOAD TRUE)
		Set(${PropertyPrefix}OBS_DOWNLOAD_URL "https://github.com/Xaymar/obs-studio/releases/download/${OBS_DOWNLOAD_VERSION}/obs-studio-${ARCH}-vs2017.7z")
	EndIf()
EndIf()

If(${PropertyPrefix}OBS_NATIVE)
	Option(BUILD_AMD_ENCODER "Build AMD Encoder module" ON)	
	If (NOT BUILD_AMD_ENCODER)
		Message(STATUS "Not building AMD Encoder")
		Return()
	EndIf()
ElseIf(${PropertyPrefix}OBS_PACKAGE)
	Include("${OBS_STUDIO_DIR}/cmake/LibObs/LibObsConfig.cmake")
ElseIf(${PropertyPrefix}OBS_REFERENCE)
	Set(obsPath "${OBS_STUDIO_DIR}")
	Include("${OBS_STUDIO_DIR}/cmake/external/Findlibobs.cmake")
ElseIf(${PropertyPrefix}OBS_DOWNLOAD)
	download_project(
		PROJ libobs
		URL ${OBS_DOWNLOAD_URL}
		UPDATE_DISCONNECTED 1
	)
	INCLUDE("${libobs_SOURCE_DIR}/cmake/LibObs/LibObsConfig.cmake")
Else()
	Message(CRITICAL "Impossible case reached, very system stability.")
	Return()
EndIf()

################################################################################
# Code
################################################################################
SET(PROJECT_DATA_LOCALE
	"${PROJECT_SOURCE_DIR}/data/locale/en-US.ini"
)
SET(PROJECT_DATA_EFFECTS
	"${PROJECT_SOURCE_DIR}/data/effects/bilateral-blur.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/box-blur.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/gaussian-blur.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/displace.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/color-conversion.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/mask.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/mip-mapper.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/mipgen.effect"
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
	"${PROJECT_SOURCE_DIR}/source/plugin.h"
	"${PROJECT_SOURCE_DIR}/source/plugin.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-displacement.h"
	"${PROJECT_SOURCE_DIR}/source/filter-displacement.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-blur.h"
	"${PROJECT_SOURCE_DIR}/source/filter-blur.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-shape.h"
	"${PROJECT_SOURCE_DIR}/source/filter-shape.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-transform.h"
	"${PROJECT_SOURCE_DIR}/source/filter-transform.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-custom-shader.h"
	"${PROJECT_SOURCE_DIR}/source/filter-custom-shader.cpp"
	"${PROJECT_SOURCE_DIR}/source/source-mirror.h"
	"${PROJECT_SOURCE_DIR}/source/source-mirror.cpp"
	"${PROJECT_SOURCE_DIR}/source/gfx-effect-source.h"
	"${PROJECT_SOURCE_DIR}/source/gfx-effect-source.cpp"
	"${PROJECT_SOURCE_DIR}/source/gfx-source-texture.h"
	"${PROJECT_SOURCE_DIR}/source/gfx-source-texture.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-helper.h"
	"${PROJECT_SOURCE_DIR}/source/gs-helper.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-effect.h"
	"${PROJECT_SOURCE_DIR}/source/gs-effect.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-indexbuffer.h"
	"${PROJECT_SOURCE_DIR}/source/gs-indexbuffer.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-limits.h"
	"${PROJECT_SOURCE_DIR}/source/gs-mipmapper.h"
	"${PROJECT_SOURCE_DIR}/source/gs-mipmapper.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-rendertarget.h"
	"${PROJECT_SOURCE_DIR}/source/gs-rendertarget.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-sampler.h"
	"${PROJECT_SOURCE_DIR}/source/gs-sampler.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-texture.h"
	"${PROJECT_SOURCE_DIR}/source/gs-texture.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-vertex.h"
	"${PROJECT_SOURCE_DIR}/source/gs-vertex.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-vertexbuffer.h"
	"${PROJECT_SOURCE_DIR}/source/gs-vertexbuffer.cpp"
	"${PROJECT_SOURCE_DIR}/source/obs-audio-capture.h"
	"${PROJECT_SOURCE_DIR}/source/obs-audio-capture.cpp"
	"${PROJECT_BINARY_DIR}/source/version.h"
	"${PROJECT_SOURCE_DIR}/source/strings.h"
	"${PROJECT_SOURCE_DIR}/source/utility.h"
	"${PROJECT_SOURCE_DIR}/source/utility.cpp"
	"${PROJECT_SOURCE_DIR}/source/util-math.h"
	"${PROJECT_SOURCE_DIR}/source/util-math.cpp"
	"${PROJECT_SOURCE_DIR}/source/util-memory.h"
	"${PROJECT_SOURCE_DIR}/source/util-memory.cpp"
)

Source_Group("Data Files\\Locale" FILES ${PROJECT_DATA_LOCALE})
Source_Group("Data Files\\Effects" FILES ${PROJECT_DATA_EFFECTS})
Source_Group("Data Files\\Shaders" FILES ${PROJECT_DATA_SHADERS})
Source_Group("Data Files\\Shaders\\Filter" FILES ${PROJECT_DATA_SHADERS_FILTER})

################################################################################
# Target
################################################################################
Add_Library(${PROJECT_NAME} MODULE
	${PROJECT_PUBLIC}
	${PROJECT_PRIVATE}
)

# Include Directories
Target_Include_Directories(${PROJECT_NAME}
	PUBLIC
		"${PROJECT_BINARY_DIR}/source"
		"${PROJECT_SOURCE_DIR}/source"
	PRIVATE
		"${PROJECT_BINARY_DIR}/source"
		"${PROJECT_SOURCE_DIR}/source"
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_SOURCE_DIR}"
		"${CMAKE_SOURCE_DIR}"
)

# OBS Studio
If(${PropertyPrefix}OBS_NATIVE)
	Target_Link_Libraries(${PROJECT_NAME}
		libobs
	)
ElseIf(${PropertyPrefix}OBS_REFERENCE)
	Target_Include_Directories(${PROJECT_NAME}
		PRIVATE
			"${OBS_STUDIO_DIR}/libobs"
	)
	Target_Link_Libraries(${PROJECT_NAME}
		"${LIBOBS_LIB}"
	)
ElseIf(${PropertyPrefix}OBS_PACKAGE)
	Target_Include_Directories(${PROJECT_NAME}
		PRIVATE
			"${OBS_STUDIO_DIR}/include"
	)
	Target_Link_Libraries(${PROJECT_NAME}
		libobs
	)
ElseIf(${PropertyPrefix}OBS_DOWNLOAD)
	Target_Link_Libraries(${PROJECT_NAME}
		libobs
	)
EndIf()


# Link Libraries
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	"${PROJECT_LIBRARIES}"
)

# Definitions
If (WIN32)
	Target_Compile_Definitions(${PROJECT_NAME}
		PRIVATE
			_CRT_SECURE_NO_WARNINGS
			# windows.h
			WIN32_LEAN_AND_MEAN
			NOGPICAPMASKS
			NOVIRTUALKEYCODES
			NOWINMESSAGES
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
			NOGDI
			NOKERNEL
			#NOUSER
			NONLS
			NOMB
			NOMEMMGR
			NOMETAFILE
			NOMINMAX
			#NOMSG
			NOOPENFILE
			NOSCROLL
			NOSERVICE
			NOSOUND
			NOTEXTMETRIC
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
EndIf()

# File Version
If(WIN32)
	Set_Target_Properties(
		${PROJECT_NAME}
		PROPERTIES
		VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
		SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
	)
Else()
	Set_Target_Properties(
		${PROJECT_NAME}
		PROPERTIES
		VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
		SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
	)
EndIf()

# CPPCheck
If(NOT ${PropertyPrefix}OBS_NATIVE)
	SET(excludes )
	If(${PropertyPrefix}OBS_REFERENCE)
		LIST(APPEND excludes "${OBS_STUDIO_DIR}/libobs")
	ElseIf(${PropertyPrefix}OBS_PACKAGE)
		LIST(APPEND excludes "${OBS_STUDIO_DIR}/libobs")
	ElseIf(${PropertyPrefix}OBS_DOWNLOAD)
		LIST(APPEND excludes "${libobs_SOURCE_DIR}")
	EndIf()

	CppCheck(
		EXCLUDE ${excludes}
	)
	CppCheck_Add_Project(${PROJECT_NAME})
EndIf()

################################################################################
# Installation
################################################################################

If(${PropertyPrefix}OBS_NATIVE)
	install_obs_plugin_with_data(${PROJECT_NAME} data)
Else()
	Install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION "./obs-plugins/${BITS}bit/" COMPONENT Runtime
		LIBRARY DESTINATION "./obs-plugins/${BITS}bit/" COMPONENT Runtime
	)
	If(MSVC)
		Install(
			FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
			DESTINATION "./obs-plugins/${BITS}bit/"
			OPTIONAL
		)
	EndIf()
	
	Install(
		DIRECTORY "${PROJECT_SOURCE_DIR}/data/"
		DESTINATION "./data/obs-plugins/${PROJECT_NAME}/"
	)
	
	# Packaging
	If("${CMAKE_PACKAGE_SUFFIX_OVERRIDE}" STREQUAL "")
		Set(PackageFullName "${CMAKE_PACKAGE_PREFIX}/${CMAKE_PACKAGE_NAME}-${PROJECT_VERSION}")
	Else()
		Set(PackageFullName "${CMAKE_PACKAGE_PREFIX}/${CMAKE_PACKAGE_NAME}-${CMAKE_PACKAGE_SUFFIX_OVERRIDE}")
	EndIf()
	
	Add_Custom_Target(
		PACKAGE_7Z
		${CMAKE_COMMAND} -E tar cfv "${PackageFullName}.7z" --format=7zip --
			"${CMAKE_INSTALL_PREFIX}/obs-plugins"
			"${CMAKE_INSTALL_PREFIX}/data"
		WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
	)
	Add_Custom_Target(
		PACKAGE_ZIP
		${CMAKE_COMMAND} -E tar cfv "${PackageFullName}.zip" --format=zip --
			"${CMAKE_INSTALL_PREFIX}/obs-plugins"
			"${CMAKE_INSTALL_PREFIX}/data"
		WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
	)
	
	# CPack
	Set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
	Set(CPACK_PACKAGE_VENDOR "Xaymar")
	Set(CPACK_PACKAGE_HOMEPAGE_URL "https://xaymar.com/")
	
	Set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
	Set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
	Set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
	Set(CPACK_PACKAGE_VERSION_TWEAK ${PROJECT_VERSION_TWEAK})
	Set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}.${CPACK_PACKAGE_VERSION_TWEAK}")
	
	Set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
	Set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}-source")
	Set(CPACK_PACKAGE_CHECKSUM SHA512)
	Set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
	
	include(CPack)
EndIf()
