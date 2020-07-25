/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2020 Michael Fabian Dirks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#pragma once

// Common C includes
#include <cfloat>
#include <cinttypes>
#include <climits>
#include <clocale>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ctime>

// Common C++ includes
#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// Common Plugin includes
#include "strings.hpp"
#include "version.hpp"
#include "util-profiler.hpp"
#include "util-threadpool.hpp"
#include "utility.hpp"

// Common OBS includes
extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs.h>

#include <obs-config.h>
#include <obs-data.h>
#include <obs-encoder.h>
#include <obs-module.h>
#include <obs-properties.h>
#include <obs-source.h>

#include <graphics/graphics.h>

#include <graphics/effect.h>
#include <graphics/matrix4.h>
#include <graphics/vec2.h>
#include <graphics/vec3.h>
#include <graphics/vec4.h>

#include <util/platform.h>

// Fix libOBS's global defines
#undef strtoll

#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

// Common Global defines
/// Logging
#define DLOG_(level, ...) blog(level, "[" PLUGIN_NAME "] " __VA_ARGS__)
#define DLOG_ERROR(...) DLOG_(LOG_ERROR, __VA_ARGS__)
#define DLOG_WARNING(...) DLOG_(LOG_WARNING, __VA_ARGS__)
#define DLOG_INFO(...) DLOG_(LOG_INFO, __VA_ARGS__)
#define DLOG_DEBUG(...) DLOG_(LOG_DEBUG, __VA_ARGS__)
/// Currrent function name (as const char*)
#ifdef _MSC_VER
// Microsoft Visual Studio
#define __FUNCTION_SIG__ __FUNCSIG__
#define __FUNCTION_NAME__ __func__
#elif defined(__GNUC__) || defined(__MINGW32__)
// GCC and MinGW
#define __FUNCTION_SIG__ __PRETTY_FUNCTION__
#define __FUNCTION_NAME__ __func__
#else
// Any other compiler
#define __FUNCTION_SIG__ __func__
#define __FUNCTION_NAME__ __func__
#endif
