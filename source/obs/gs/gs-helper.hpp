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
#include "common.hpp"
#include <vector>
#include "plugin.hpp"

namespace gs {
	class context {
		public:
		context();
		~context();
	};

	static const float_t debug_color_source[4]       = {0.f, .5f, 5.f, 1.f};
	static const float_t debug_color_cache[4]        = {1.f, .75f, 0.f, 1.f};
	static const float_t debug_color_cache_render[4] = {.2f, .15f, 0.f, 1.f};
	static const float_t debug_color_convert[4]      = {.5f, .5f, 0.5f, 1.f};
	static const float_t debug_color_render[4]       = {0.f, 1.f, 0.0f, 1.f};

	class debug_marker {
		std::string _name;

		public:
		//debug_marker(const float color[4], std::string name);
		debug_marker(const float_t color[4], const char* format, ...);
		~debug_marker();
	};
} // namespace gs
