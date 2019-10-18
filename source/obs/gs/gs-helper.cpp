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

#include "gs-helper.hpp"

gs::context::context()
{
	obs_enter_graphics();
}

gs::context::~context()
{
	obs_leave_graphics();
}

/*gs::debug_marker::debug_marker(const float color[4], std::string name) : _name(name)
{
	gs_debug_marker_begin(color, _name.c_str());
}*/

gs::debug_marker::debug_marker(const float color[4], std::string format, ...)
{
	size_t            size;
	std::vector<char> buffer(64);

	va_list vargs;
	va_start(vargs, format);
	size = vsnprintf(buffer.data(), buffer.size(), format.c_str(), vargs);
	va_end(vargs);

	_name = std::string(buffer.data(), buffer.data() + size);
	gs_debug_marker_begin(color, _name.c_str());
}

gs::debug_marker::~debug_marker()
{
	gs_debug_marker_end();
}
