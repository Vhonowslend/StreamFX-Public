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
#include <cinttypes>
#include <cstdint>

#define P_LOG(...) streamfx::util::logging::log(__VA_ARGS__);
#define P_LOG_ERROR(...) P_LOG(streamfx::util::logging::level::LEVEL_ERROR, __VA_ARGS__)
#define P_LOG_WARN(...) P_LOG(streamfx::util::logging::level::LEVEL_WARN, __VA_ARGS__)
#define P_LOG_INFO(...) P_LOG(streamfx::util::logging::level::LEVEL_INFO, __VA_ARGS__)
#ifdef _DEBUG
#define P_LOG_DEBUG(...) P_LOG(streamfx::util::logging::level::LEVEL_DEBUG, __VA_ARGS__)
#else
#define P_LOG_DEBUG(...)
#endif

// Function/Class/Struct Name
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

namespace streamfx::util::logging {
	enum class level {
		LEVEL_DEBUG, // Debug information, which is not necessary to know at runtime.
		LEVEL_INFO,  // Runtime information, which may or may not be needed for support.
		LEVEL_WARN,  // Warnings, which should be respected and fixed.
		LEVEL_ERROR, // Errors that must be fixed.
	};

	void log(level lvl, const char* format, ...);
} // namespace streamfx::util::logging
