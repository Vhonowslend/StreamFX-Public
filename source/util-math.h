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
#include <inttypes.h>
#include <math.h>
#include <string>
#include <utility>

// OBS
#include <graphics/vec2.h>
#include <graphics/vec3.h>
#include <graphics/vec4.h>

// Constants
#define PI 3.1415926535897932384626433832795
#define PI2 6.283185307179586476925286766559
#define PI2_SQROOT 2.506628274631000502415765284811

inline double_t Gaussian1D(double_t x, double_t o)
{
	double_t c = (x / o);
	double_t b = exp(-0.5 * c * c);
	double_t a = (1.0 / (o * PI2_SQROOT));
	return a * b;
}

inline double_t Bilateral1D(double_t x, double_t o)
{
	double_t c = (x / 0);
	double_t d = c * c;
	double_t b = exp(-0.5 * d) / o;
	return 0.39894 * b; // Seems to be (1.0 / (1 * PI2_SQROOT)) * b, otherwise no difference from Gaussian Blur
}

inline size_t GetNearestPowerOfTwoAbove(size_t v)
{
	return 1ull << size_t(ceil(log10(double(v)) / log10(2.0)));
}

inline size_t GetNearestPowerOfTwoBelow(size_t v)
{
	return 1ull << size_t(floor(log10(double(v)) / log10(2.0)));
}

namespace util {
	__declspec(align(16)) struct vec3a : public vec3 {
		static void* vec3a::operator new(size_t count);
		static void* vec3a::operator new[](size_t count);
		static void vec3a::operator delete(void* p);
		static void vec3a::operator delete[](void* p);
	};

	__declspec(align(16)) struct vec4a : public vec4 {
		static void* vec4a::operator new(size_t count);
		static void* vec4a::operator new[](size_t count);
		static void vec4a::operator delete(void* p);
		static void vec4a::operator delete[](void* p);
	};

	std::pair<int64_t, int64_t> SizeFromString(std::string text, bool allowSquare = true);

	namespace math {
		// Proven by tests to be the fastest implementation on Intel and AMD CPUs.
		// Ranking: log10, loop < bitscan < pow
		// loop and log10 trade blows, usually almost identical.
		// loop is used for integers, log10 for anything else.
		template<typename T>
		inline bool is_power_of_two(T v)
		{
			return T(1ull << uint64_t(floor(log10(T(v)) / log10(2.0)))) == v;
		};

		template<typename T>
		inline bool is_power_of_two_loop(T v)
		{
			bool have_bit = false;
			for (size_t index = 0; index < (sizeof(T) * 8); index++) {
				bool cur = (v & (1ull << index)) != 0;
				if (cur) {
					if (have_bit)
						return false;
					have_bit = true;
				}
			}
			return true;
		}

#pragma push_macro("is_power_of_two_as_loop")
#define is_power_of_two_as_loop(x)      \
	template<>                          \
	inline bool is_power_of_two(x v)    \
	{                                   \
		return is_power_of_two_loop(v); \
	};
		is_power_of_two_as_loop(int8_t);
		is_power_of_two_as_loop(uint8_t);
		is_power_of_two_as_loop(int16_t);
		is_power_of_two_as_loop(uint16_t);
		is_power_of_two_as_loop(int32_t);
		is_power_of_two_as_loop(uint32_t);
		is_power_of_two_as_loop(int64_t);
		is_power_of_two_as_loop(uint64_t);
#undef is_power_of_two_as_loop
#pragma pop_macro("is_power_of_two_as_loop")

		template<typename T>
		inline uint64_t get_power_of_two_exponent_floor(T v)
		{
			return uint64_t(floor(log10(T(v)) / log10(2.0)));
		}

		template<typename T>
		inline uint64_t get_power_of_two_exponent_ceil(T v)
		{
			return uint64_t(ceil(log10(T(v)) / log10(2.0)));
		}
	} // namespace math
} // namespace util
