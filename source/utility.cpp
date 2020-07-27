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

const char* obs_module_recursive_text(const char* to_translate, std::size_t depth)
{
	static std::unordered_map<std::string, std::string> translate_map;

	if (depth == 0) {
		return obs_module_text(to_translate);
	}

	std::string key   = to_translate;
	auto        value = translate_map.find(std::string(key));
	if (value != translate_map.end()) {
		return value->second.c_str();
	} else {
		std::string       orig = obs_module_text(to_translate);
		std::stringstream out;

		{
			std::size_t seq_start = 0, seq_end = 0;
			bool        seq_got = false;

			for (std::size_t pos = 0; pos <= orig.length(); pos++) {
				std::string chr = orig.substr(pos, 2);
				if (chr == "\\@") {
					if (seq_got) {
						out << obs_module_recursive_text(orig.substr(seq_start, pos - seq_start).c_str(), (depth - 1));
						seq_end = pos + 2;
					} else {
						out << orig.substr(seq_end, pos - seq_end);
						seq_start = pos + 2;
					}
					seq_got = !seq_got;
					pos += 1;
				}
			}
			if (seq_end != orig.length()) {
				out << orig.substr(seq_end, orig.length() - seq_end);
			}

			translate_map.insert({key, out.str()});
		}

		auto value = translate_map.find(key);
		if (value != translate_map.end()) {
			return value->second.c_str();
		} else {
			throw std::runtime_error("Insert into map failed.");
		}
	}
}

obs_property_t* util::obs_properties_add_tristate(obs_properties_t* props, const char* name, const char* desc)
{
	obs_property_t* p = obs_properties_add_list(props, name, desc, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DEFAULT), -1);
	obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DISABLED), 0);
	obs_property_list_add_int(p, D_TRANSLATE(S_STATE_ENABLED), 1);
	return p;
}

void* util::vec2a::operator new(std::size_t count)
{
	return util::malloc_aligned(16, count);
}

void* util::vec2a::operator new[](std::size_t count)
{
	return util::malloc_aligned(16, count);
}

void util::vec2a::operator delete(void* p)
{
	util::free_aligned(p);
}

void util::vec2a::operator delete[](void* p)
{
	util::free_aligned(p);
}

void* util::vec3a::operator new(std::size_t count)
{
	return util::malloc_aligned(16, count);
}

void* util::vec3a::operator new[](std::size_t count)
{
	return util::malloc_aligned(16, count);
}

void util::vec3a::operator delete(void* p)
{
	util::free_aligned(p);
}

void util::vec3a::operator delete[](void* p)
{
	util::free_aligned(p);
}

void* util::vec4a::operator new(std::size_t count)
{
	return util::malloc_aligned(16, count);
}

void* util::vec4a::operator new[](std::size_t count)
{
	return util::malloc_aligned(16, count);
}

void util::vec4a::operator delete(void* p)
{
	util::free_aligned(p);
}

void util::vec4a::operator delete[](void* p)
{
	util::free_aligned(p);
}

std::pair<int64_t, int64_t> util::size_from_string(std::string text, bool allowSquare)
{
	int64_t width, height;

	const char* begin = text.c_str();
	const char* end   = text.c_str() + text.size() + 1;
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

void* util::malloc_aligned(std::size_t align, std::size_t size)
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

void util::free_aligned(void* mem)
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
