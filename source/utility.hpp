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
#include <cinttypes>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <graphics/vec2.h>
#include <graphics/vec3.h>
#include <graphics/vec4.h>
#include <obs-config.h>
#include <obs.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

// Constants
#define S_PI 3.1415926535897932384626433832795        // PI = pi
#define S_PI2 6.283185307179586476925286766559        // 2PI = 2 * pi
#define S_PI2_SQROOT 2.506628274631000502415765284811 // sqrt(2 * pi)
#define S_RAD 57.295779513082320876798154814105       // 180/pi
#define S_DEG 0.01745329251994329576923690768489      // pi/180
#define D_DEG_TO_RAD(x) (x * S_DEG)
#define D_RAD_TO_DEG(x) (x * S_RAD)

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

template<typename Enum>
typename std::enable_if<enable_bitmask_operators<Enum>::enable, bool>::type any(Enum lhs)
{
	using underlying = typename std::underlying_type<Enum>::type;
	return static_cast<underlying>(lhs) != static_cast<underlying>(0);
}

template<typename Enum>
typename std::enable_if<enable_bitmask_operators<Enum>::enable, bool>::type exact(Enum lhs, Enum rhs)
{
	using underlying = typename std::underlying_type<Enum>::type;
	return static_cast<underlying>(lhs) == static_cast<underlying>(rhs);
}

#define P_ENABLE_BITMASK_OPERATORS(x)    \
	template<>                           \
	struct enable_bitmask_operators<x> { \
		static const bool enable = true; \
	};

#define D_STR(s) #s
#define D_VSTR(s) D_STR(s)

namespace util {
	bool inline are_property_groups_broken()
	{
		return obs_get_version() < MAKE_SEMANTIC_VERSION(24, 0, 0);
	}

	struct obs_graphics {
		obs_graphics()
		{
			obs_enter_graphics();
		}
		~obs_graphics()
		{
			obs_leave_graphics();
		}
	};

	obs_property_t* obs_properties_add_tristate(obs_properties_t* props, const char* name, const char* desc);

	inline bool is_tristate_enabled(int64_t tristate)
	{
		return tristate == 1;
	}

	inline bool is_tristate_disabled(int64_t tristate)
	{
		return tristate == 0;
	}

	inline bool is_tristate_default(int64_t tristate)
	{
		return tristate == -1;
	}

	typedef union {
		uint32_t color;
		struct {
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};
	} rgba32;
	typedef union {
		uint32_t color;
		struct {
			uint8_t a;
			uint8_t r;
			uint8_t g;
			uint8_t b;
		};
	} argb32;

	struct vec2a : public vec2 {
		// 16-byte Aligned version of vec2
		static void* operator new(size_t count);
		static void* operator new[](size_t count);
		static void  operator delete(void* p);
		static void  operator delete[](void* p);
	};

#ifdef _MSC_VER
	__declspec(align(16))
#endif
		struct vec3a : public vec3 {
		// 16-byte Aligned version of vec3
		static void* operator new(size_t count);
		static void* operator new[](size_t count);
		static void  operator delete(void* p);
		static void  operator delete[](void* p);
	};

#ifdef _MSC_VER
	__declspec(align(16))
#endif
		struct vec4a : public vec4 {
		// 16-byte Aligned version of vec4
		static void* operator new(size_t count);
		static void* operator new[](size_t count);
		static void  operator delete(void* p);
		static void  operator delete[](void* p);
	};

	inline size_t GetNearestPowerOfTwoAbove(size_t v)
	{
		return 1ull << size_t(ceil(log10(double(v)) / log10(2.0)));
	}

	inline size_t GetNearestPowerOfTwoBelow(size_t v)
	{
		return 1ull << size_t(floor(log10(double(v)) / log10(2.0)));
	}

	std::pair<int64_t, int64_t> size_from_string(std::string text, bool allowSquare = true);

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
				bool cur = (v & (static_cast<T>(1ull) << index)) != 0;
				if (cur) {
					if (have_bit)
						return false;
					have_bit = true;
				}
			}
			return true;
		}

#pragma push_macro("P_IS_POWER_OF_TWO_AS_LOOP")
#define P_IS_POWER_OF_TWO_AS_LOOP(x)    \
	template<>                          \
	inline bool is_power_of_two(x v)    \
	{                                   \
		return is_power_of_two_loop(v); \
	}
		P_IS_POWER_OF_TWO_AS_LOOP(int8_t);
		P_IS_POWER_OF_TWO_AS_LOOP(uint8_t);
		P_IS_POWER_OF_TWO_AS_LOOP(int16_t);
		P_IS_POWER_OF_TWO_AS_LOOP(uint16_t);
		P_IS_POWER_OF_TWO_AS_LOOP(int32_t);
		P_IS_POWER_OF_TWO_AS_LOOP(uint32_t);
		P_IS_POWER_OF_TWO_AS_LOOP(int64_t);
		P_IS_POWER_OF_TWO_AS_LOOP(uint64_t);
#undef P_IS_POWER_OF_TWO_AS_LOOP
#pragma pop_macro("P_IS_POWER_OF_TWO_AS_LOOP")

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

		template<typename T, typename C>
		inline bool is_equal(T target, C value)
		{
			return (target > (value - std::numeric_limits<T>::epsilon()))
				   && (target < (value + std::numeric_limits<T>::epsilon()));
		}

		template<typename T>
		inline T gaussian(T x, T o /*, T u = 0*/)
		{
			// u/µ can be simulated by subtracting that value from x.
			static const double_t pi            = 3.1415926535897932384626433832795;
			static const double_t two_pi        = pi * 2.;
			static const double_t two_pi_sqroot = 2.506628274631000502415765284811; //sqrt(two_pi);

			if (is_equal<double_t>(0, o)) {
				return T(std::numeric_limits<double_t>::infinity());
			}

			// g(x) = (1 / o√(2Π)) * e(-(1/2) * ((x-u)/o)²)
			double_t left_e      = 1. / (o * two_pi_sqroot);
			double_t mid_right_e = ((x /* - u*/) / o);
			double_t right_e     = -0.5 * mid_right_e * mid_right_e;
			double_t final       = left_e * exp(right_e);

			return T(final);
		}
	} // namespace math

	inline size_t aligned_offset(size_t align, size_t pos)
	{
		return ((pos / align) + 1) * align;
	}
	void* malloc_aligned(size_t align, size_t size);
	void  free_aligned(void* mem);

	template<typename T, size_t N = 16>
	class AlignmentAllocator {
		public:
		typedef T      value_type;
		typedef size_t size_type;
#ifdef __clang__
		typedef ptrdiff_t difference_type;
#else
		typedef std::ptrdiff_t difference_type;
#endif

		typedef T*       pointer;
		typedef const T* const_pointer;

		typedef T&       reference;
		typedef const T& const_reference;

		public:
		inline AlignmentAllocator() {}

		template<typename T2>
		inline AlignmentAllocator(const AlignmentAllocator<T2, N>&)
		{}

		inline ~AlignmentAllocator() {}

		inline pointer adress(reference r)
		{
			return &r;
		}

		inline const_pointer adress(const_reference r) const
		{
			return &r;
		}

		inline pointer allocate(size_type n)
		{
			return (pointer)malloc_aligned(n * sizeof(value_type), N);
		}

		inline void deallocate(pointer p, size_type)
		{
			free_aligned(p);
		}

		inline void construct(pointer p, const value_type& wert)
		{
			new (p) value_type(wert);
		}

		inline void destroy(pointer p)
		{
			p->~value_type();
			p;
		}

		inline size_type max_size() const
		{
			return size_type(-1) / sizeof(value_type);
		}

		template<typename T2>
		struct rebind {
			typedef AlignmentAllocator<T2, N> other;
		};

		bool operator!=(const AlignmentAllocator<T, N>& other) const
		{
			return !(*this == other);
		}

		// Returns true if and only if storage allocated from *this
		// can be deallocated from other, and vice versa.
		// Always returns true for stateless allocators.
		bool operator==(const AlignmentAllocator<T, N>&) const
		{
			return true;
		}
	};
} // namespace util
