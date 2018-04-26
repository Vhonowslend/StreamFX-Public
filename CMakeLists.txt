cmake_minimum_required(VERSION 3.2)
PROJECT(obs-stream-effects VERSION 0.4.2.0)

################################################################################
# CMake Setup
################################################################################

math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

# Modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(ExternalProject)
include(DownloadProject)

################################################################################
# Dependencies
################################################################################

# OBS Studio / libOBS
IF(NOT TARGET libobs)
	SET(LIBOBS_EXISTS FALSE)
	SET(LIBOBS_CUSTOM FALSE CACHE BOOL "Use custom libOBS")
	IF(NOT LIBOBS_CUSTOM)
		IF(WIN32)
			IF (${BITS} STREQUAL "64")
				SET(LIBOBS_URL "https://ci.appveyor.com/api/buildjobs/jp18y3sc5icoq8qs/artifacts/build%2Fobs-studio-x64-vs2017-x64.7z" CACHE STRING "URL to libobs Cpack")
			ELSE()
				SET(LIBOBS_URL "https://ci.appveyor.com/api/buildjobs/nj276esfv3ho0j49/artifacts/build%2Fobs-studio-x86-vs2017-x86.7z" CACHE STRING "URL to libobs Cpack")
			ENDIF()
		ELSEIF(UNIX AND NOT APPLE)
			IF (${BITS} STREQUAL "64")
				SET(LIBOBS_URL "" CACHE STRING "URL to libobs Cpack")
			ELSE()
				SET(LIBOBS_URL "" CACHE STRING "URL to libobs Cpack")
			ENDIF()
		ELSEIF(APPLE)
			IF (${BITS} STREQUAL "64")
				SET(LIBOBS_URL "" CACHE STRING "URL to libobs Cpack")
			ELSE()
				SET(LIBOBS_URL "" CACHE STRING "URL to libobs Cpack")
			ENDIF()
		ENDIF()
		
		# OBS Studio (CPack Release)
		download_project(
			PROJ libobs
			URL "${LIBOBS_URL}"
			UPDATE_DISCONNECTED 1
		)
		INCLUDE("${libobs_SOURCE_DIR}/cmake/LibObs/LibObsConfig.cmake")
	ELSE()
		SET(LIBOBS_PATH "" CACHE STRING "Path to libOBS")
	ENDIF()
ELSE()
	SET(LIBOBS_EXISTS TRUE)
ENDIF()

################################################################################
# Version
################################################################################

# Retrieve Tweak version from git.
if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
	SET(GIT_RESULT "")
	SET(GIT_OUTPUT "")
	EXECUTE_PROCESS(
		COMMAND git rev-list --count --topo-order ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}..HEAD
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		RESULT_VARIABLE GIT_RESULT
		OUTPUT_VARIABLE GIT_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	IF(GIT_RESULT EQUAL 0)
		SET(PROJECT_VERSION_TWEAK ${GIT_OUTPUT})
	ENDIF()
endif()

SET(VERSION_MAJOR PROJECT_VERSION_MAJOR)
SET(VERSION_MINOR PROJECT_VERSION_MINOR)
SET(VERSION_PATCH PROJECT_VERSION_PATCH)
SET(VERSION_TWEAK PROJECT_VERSION_TWEAK)
SET(VERSION_STR "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}")

configure_file(
	"${PROJECT_SOURCE_DIR}/source/version.h.in"
	"${PROJECT_BINARY_DIR}/source/version.h"
)

################################################################################
# Code
################################################################################
SET(obs-stream-effects_HEADERS
	"${PROJECT_SOURCE_DIR}/source/plugin.h"
	"${PROJECT_SOURCE_DIR}/source/filter-displacement.h"
	"${PROJECT_SOURCE_DIR}/source/filter-blur.h"
	"${PROJECT_SOURCE_DIR}/source/filter-shape.h"
	"${PROJECT_SOURCE_DIR}/source/filter-transform.h"
	"${PROJECT_SOURCE_DIR}/source/filter-custom-shader.h"
	"${PROJECT_SOURCE_DIR}/source/source-mirror.h"
	"${PROJECT_SOURCE_DIR}/source/gfx-effect-source.h"
	"${PROJECT_SOURCE_DIR}/source/gfx-source-texture.h"
	"${PROJECT_SOURCE_DIR}/source/gs-helper.h"
	"${PROJECT_SOURCE_DIR}/source/gs-effect.h"
	"${PROJECT_SOURCE_DIR}/source/gs-indexbuffer.h"
	"${PROJECT_SOURCE_DIR}/source/gs-limits.h"
#	"${PROJECT_SOURCE_DIR}/source/gs-mipmapper.h"
	"${PROJECT_SOURCE_DIR}/source/gs-rendertarget.h"
	"${PROJECT_SOURCE_DIR}/source/gs-sampler.h"
	"${PROJECT_SOURCE_DIR}/source/gs-texture.h"
	"${PROJECT_SOURCE_DIR}/source/gs-vertex.h"
	"${PROJECT_SOURCE_DIR}/source/gs-vertexbuffer.h"
	"${PROJECT_BINARY_DIR}/source/version.h"
	"${PROJECT_SOURCE_DIR}/source/strings.h"
	"${PROJECT_SOURCE_DIR}/source/util-math.h"
	"${PROJECT_SOURCE_DIR}/source/util-memory.h"
)
SET(obs-stream-effects_SOURCES
	"${PROJECT_SOURCE_DIR}/source/plugin.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-displacement.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-blur.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-shape.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-transform.cpp"
	"${PROJECT_SOURCE_DIR}/source/filter-custom-shader.cpp"
	"${PROJECT_SOURCE_DIR}/source/source-mirror.cpp"
	"${PROJECT_SOURCE_DIR}/source/gfx-effect-source.cpp"
	"${PROJECT_SOURCE_DIR}/source/gfx-source-texture.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-helper.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-effect.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-indexbuffer.cpp"
#	"${PROJECT_SOURCE_DIR}/source/gs-mipmapper.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-rendertarget.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-sampler.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-texture.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-vertex.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-vertexbuffer.cpp"
	"${PROJECT_SOURCE_DIR}/source/util-math.cpp"
	"${PROJECT_SOURCE_DIR}/source/util-memory.cpp"
)
SET(obs-stream-effects_LOCALE
	"${PROJECT_SOURCE_DIR}/data/locale/en-US.ini"
)
SET(obs-stream-effects_EFFECTS
	"${PROJECT_SOURCE_DIR}/data/effects/bilateral-blur.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/box-blur.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/gaussian-blur.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/displace.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/color-conversion.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/mip-mapper.effect"
)
SET(obs-stream-effects_SHADERS
#	"${PROJECT_SOURCE_DIR}/data/shaders/name.effect"
)
SET(obs-stream-effects_SHADERS_FILTER
	"${PROJECT_SOURCE_DIR}/data/shaders/filter/example.effect"
)
SET(obs-stream-effects_LIBRARIES
)
#set_property(GLOBAL PROPERTY USE_FOLDERS ON)
source_group("Data Files\\Locale" FILES ${obs-stream-effects_LOCALE})
source_group("Data Files\\Effects" FILES ${obs-stream-effects_EFFECTS})
source_group("Data Files\\Shaders" FILES ${obs-stream-effects_SHADERS})
source_group("Data Files\\Shaders\\Filter" FILES ${obs-stream-effects_SHADERS_FILTER})

################################################################################
# Standalone and OBS Studio Build Data
################################################################################
if(LIBOBS_EXISTS)
	# OBS Studio Specific
	INCLUDE_DIRECTORIES(
		"${CMAKE_SOURCE_DIR}"
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_BINARY_DIR}/source"
		"${PROJECT_SOURCE_DIR}"
		"${PROJECT_SOURCE_DIR}/source"
	)
	SET(LIBOBS_LIBRARIES libobs)
else()
	# Compiling
	INCLUDE_DIRECTORIES(
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_BINARY_DIR}/source"
		"${PROJECT_SOURCE_DIR}"
		"${PROJECT_SOURCE_DIR}/source"
		${LIBOBS_INCLUDE_DIRS}
	)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
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
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

################################################################################
# Build
################################################################################
ADD_LIBRARY(obs-stream-effects MODULE
	${obs-stream-effects_HEADERS}
	${obs-stream-effects_SOURCES}
	${obs-stream-effects_LOCALE}
	${obs-stream-effects_EFFECTS}
	${obs-stream-effects_SHADERS}
	${obs-stream-effects_SHADERS_FILTER}
)
TARGET_LINK_LIBRARIES(obs-stream-effects
	${LIBOBS_LIBRARIES}
	${obs-stream-effects_LIBRARIES}
)

if(LIBOBS_EXISTS)
	install_obs_plugin_with_data(obs-stream-effects data)
else()
	# Install
	INSTALL(
		TARGETS obs-stream-effects
		RUNTIME DESTINATION "obs-plugins/${BITS}bit" COMPONENT Runtime
		LIBRARY DESTINATION "obs-plugins/${BITS}bit" COMPONENT Runtime
	)
	IF(MSVC)
		INSTALL(FILES $<TARGET_PDB_FILE:obs-stream-effects> DESTINATION "obs-plugins/${BITS}bit" OPTIONAL)
	ENDIF()
	INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/data/" DESTINATION "data/obs-plugins/obs-stream-effects" OPTIONAL)
	
	# CPack
	SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
	SET(CPACK_PACKAGE_VENDOR "Xaymar")
	
	SET(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
	SET(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
	SET(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
	SET(CPACK_PACKAGE_VERSION_TWEAK ${PROJECT_VERSION_TWEAK})
	SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}.${CPACK_PACKAGE_VERSION_TWEAK}")
	
	SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
	SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}-source")
	SET(CPACK_PACKAGE_CHECKSUM SHA512)
	include(CPack)
endif()
