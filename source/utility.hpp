/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2018 Michael Fabian Dirks
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
#include <type_traits>
#include <cinttypes>
#include <limits>

extern "C" {
#include <obs-config.h>
#include <obs.h>
}

const char* obs_module_recursive_text(const char* to_translate, size_t depth = std::numeric_limits<size_t>::max());

template<typename Enum>
struct enable_bitmask_operators {
	static const bool enable = false;
};

template<typename Enum>
typename std::enable_if<enable_bitmask_operators<Enum>::enable, Enum>::type operator|(Enum lhs, Enum rhs)
{
	using underlying = typename std::underlying_type<Enum>::type;
	return static_cast<Enum>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
}

template<typename Enum>
typename std::enable_if<enable_bitmask_operators<Enum>::enable, Enum>::type operator&(Enum lhs, Enum rhs)
{
	using underlying = typename std::underlying_type<Enum>::type;
	return static_cast<Enum>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
}

#define P_ENABLE_BITMASK_OPERATORS(x)      \
	template<>                           \
	struct enable_bitmask_operators<x> { \
		static const bool enable = true; \
	};

#define D_STR(s) #s
#define D_VSTR(s) D_STR(s)

#ifdef __cplusplus
#define P_INITIALIZER(f)   \
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
#define P_INITIALIZER(f) INITIALIZER2_(f, "")
#else
#define P_INITIALIZER(f) INITIALIZER2_(f, "_")
#endif
#else
#define P_INITIALIZER(f)                                \
	static void f(void) __attribute__((constructor)); \
	static void f(void)
#endif

namespace util {
	bool inline are_property_groups_broken()
	{
		return obs_get_version() < MAKE_SEMANTIC_VERSION(24, 0, 0);
	}
}
