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

#include "util-math.hpp"
#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include "util-memory.hpp"

void* util::vec2a::operator new(size_t count)
{
	return util::malloc_aligned(16, count);
}

void* util::vec2a::operator new[](size_t count)
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

void* util::vec3a::operator new(size_t count)
{
	return util::malloc_aligned(16, count);
}

void* util::vec3a::operator new[](size_t count)
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

void* util::vec4a::operator new(size_t count)
{
	return util::malloc_aligned(16, count);
}

void* util::vec4a::operator new[](size_t count)
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
