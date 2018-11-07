/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017 Michael Fabian Dirks
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
#include <functional>
#include <inttypes.h>
#include <list>

extern "C" {
#pragma warning(push)
#pragma warning(disable : 4201)
#include "obs-module.h"
#include "util/platform.h"
#pragma warning(pop)
}

// Plugin
#define PLUGIN_NAME "Stream Effects"
#include "version.h"

#define P_LOG(level, ...) blog(level, "[" PLUGIN_NAME "] " __VA_ARGS__);
#define P_LOG_ERROR(...) P_LOG(LOG_ERROR, __VA_ARGS__)
#define P_LOG_WARNING(...) P_LOG(LOG_WARNING, __VA_ARGS__)
#define P_LOG_INFO(...) P_LOG(LOG_INFO, __VA_ARGS__)
#define P_LOG_DEBUG(...) P_LOG(LOG_DEBUG, __VA_ARGS__)

// Utility
#define vstr(s) dstr(s)
#define dstr(s) #s

#define clamp(val, low, high) (val > high ? high : (val < low ? low : val))
#ifdef max
#undef max
#endif
#define max(val, high) (val > high ? val : high)
#ifdef min
#undef min
#endif
#define min(val, low) (val < low ? val : low)

#ifndef __FUNCTION_NAME__
#if defined(_WIN32) || defined(_WIN64) //WINDOWS
#define __FUNCTION_NAME__ __FUNCTION__
#else //*NIX
#define __FUNCTION_NAME__ __func__
#endif
#endif

#ifdef __cplusplus
#define INITIALIZER(f)   \
	static void f(void); \
	struct f##_t_ {      \
		f##_t_(void)     \
		{                \
			f();         \
		}                \
	};                   \
	static f##_t_ f##_;  \
	static void   f(void)
#elif defined(_MSC_VER)
#pragma section(".CRT$XCU", read)
#define INITIALIZER2_(f, p)                                  \
	static void f(void);                                     \
	__declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
	__pragma(comment(linker, "/include:" p #f "_")) static void f(void)
#ifdef _WIN64
#define INITIALIZER(f) INITIALIZER2_(f, "")
#else
#define INITIALIZER(f) INITIALIZER2_(f, "_")
#endif
#else
#define INITIALIZER(f)                                \
	static void f(void) __attribute__((constructor)); \
	static void f(void)
#endif

// Initializer & Finalizer
extern std::list<std::function<void()>> initializerFunctions;
extern std::list<std::function<void()>> finalizerFunctions;
