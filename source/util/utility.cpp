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

#include "utility.hpp"
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include "plugin.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#pragma warning(disable : 5039)
#endif
#include <obs.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

obs_property_t* streamfx::util::obs_properties_add_tristate(obs_properties_t* props, const char* name, const char* desc)
{
	obs_property_t* p = obs_properties_add_list(props, name, desc, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DEFAULT), -1);
	obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DISABLED), 0);
	obs_property_list_add_int(p, D_TRANSLATE(S_STATE_ENABLED), 1);
	return p;
}

void* streamfx::util::vec2a::operator new(std::size_t count)
{
	return streamfx::util::malloc_aligned(16, count);
}

void* streamfx::util::vec2a::operator new[](std::size_t count)
{
	return streamfx::util::malloc_aligned(16, count);
}

void streamfx::util::vec2a::operator delete(void* p)
{
	streamfx::util::free_aligned(p);
}

void streamfx::util::vec2a::operator delete[](void* p)
{
	streamfx::util::free_aligned(p);
}

void* streamfx::util::vec3a::operator new(std::size_t count)
{
	return streamfx::util::malloc_aligned(16, count);
}

void* streamfx::util::vec3a::operator new[](std::size_t count)
{
	return streamfx::util::malloc_aligned(16, count);
}

void streamfx::util::vec3a::operator delete(void* p)
{
	streamfx::util::free_aligned(p);
}

void streamfx::util::vec3a::operator delete[](void* p)
{
	streamfx::util::free_aligned(p);
}

void* streamfx::util::vec4a::operator new(std::size_t count)
{
	return streamfx::util::malloc_aligned(16, count);
}

void* streamfx::util::vec4a::operator new[](std::size_t count)
{
	return streamfx::util::malloc_aligned(16, count);
}

void streamfx::util::vec4a::operator delete(void* p)
{
	streamfx::util::free_aligned(p);
}

void streamfx::util::vec4a::operator delete[](void* p)
{
	streamfx::util::free_aligned(p);
}

std::pair<int64_t, int64_t> streamfx::util::size_from_string(std::string_view text, bool allowSquare)
{
	int64_t width, height;

	const auto* begin = text.data();
	const auto* end   = text.data() + text.size() + 1;
	char*       here  = const_cast<char*>(end);

	long long res = strtoll(begin, &here, 0);
	if (errno == ERANGE) {
		return {0, 0};
	}
	width = res;

	while (here != end) {
		if (isdigit(*here) || (*here == '-') || (*here == '+')) {
			break;
		}
		here++;
	}
	if (here == end) {
		// Are we allowed to return a square?
		if (allowSquare) {
			// Yes: Return width,width.
			return {width, width};
		} else {
			// No: Return width,0.
			return {width, 0};
		}
	}

	res = strtoll(here, nullptr, 0);
	if (errno == ERANGE) {
		return {width, 0};
	}
	height = res;

	return {width, height};
}

void* streamfx::util::malloc_aligned(std::size_t align, std::size_t size)
{
#ifdef USE_MSC_ALLOC
	return _aligned_malloc(size, align);
#elif defined(USE_STD_ALLOC)
	return aligned_alloc(size, align);
#else
	// Ensure that we have space for the pointer and the data.
	std::size_t asize = aligned_offset(align, size + (sizeof(void*) * 2));

	// Allocate memory and store integer representation of pointer.
	void* ptr = malloc(asize);

	// Calculate actual aligned position
	intptr_t ptr_off = static_cast<intptr_t>(aligned_offset(align, reinterpret_cast<size_t>(ptr) + sizeof(void*)));

	// Store actual pointer at ptr_off - sizeof(void*).
	*reinterpret_cast<intptr_t*>(ptr_off - sizeof(void*)) = reinterpret_cast<intptr_t>(ptr);

	// Return aligned pointer
	return reinterpret_cast<void*>(ptr_off);
#endif
}

void streamfx::util::free_aligned(void* mem)
{
#ifdef USE_MSC_ALLOC
	_aligned_free(mem);
#elif defined(USE_STD_ALLOC_FREE)
	free(mem);
#else
	if (mem == nullptr)
		return;
	void* ptr = reinterpret_cast<void*>(*reinterpret_cast<intptr_t*>(static_cast<char*>(mem) - sizeof(void*)));
	free(ptr);
#endif
}
