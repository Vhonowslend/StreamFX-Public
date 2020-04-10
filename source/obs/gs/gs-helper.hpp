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

	static const float_t debug_color_white[4]           = {1.f, 1.f, 1.f, 1.f};
	static const float_t debug_color_gray[4]            = {.5f, .5f, .5f, 1.f};
	static const float_t debug_color_black[4]           = {0.f, 0.f, 0.f, 1.f};
	static const float_t debug_color_red[4]             = {1.f, 0.f, 0.f, 1.f};
	static const float_t debug_color_flush_orange[4]    = {1.f, .5f, 0.f, 1.f};
	static const float_t debug_color_yellow[4]          = {1.f, 1.f, 0.f, 1.f};
	static const float_t debug_color_chartreuse[4]      = {.5f, 1.f, 0.f, 1.f};
	static const float_t debug_color_green[4]           = {0.f, 1.f, 0.f, 1.f};
	static const float_t debug_color_spring_green[4]    = {0.f, 1.f, .5f, 1.f};
	static const float_t debug_color_teal[4]            = {0.f, 1.f, 1.f, 1.f};
	static const float_t debug_color_azure_radiance[4]  = {0.f, .5f, 1.f, 1.f};
	static const float_t debug_color_blue[4]            = {0.f, 0.f, 1.f, 1.f};
	static const float_t debug_color_electric_violet[4] = {.5f, 0.f, 1.f, 1.f};
	static const float_t debug_color_magenta[4]         = {1.f, 0.f, 1.f, 1.f};
	static const float_t debug_color_rose[4]            = {1.f, 0.f, .5f, 1.f};

	static const float_t* debug_color_source       = debug_color_white;
	static const float_t* debug_color_capture      = debug_color_flush_orange;
	static const float_t* debug_color_cache        = debug_color_capture;
	static const float_t* debug_color_convert      = debug_color_electric_violet;
	static const float_t* debug_color_cache_render = debug_color_convert;
	static const float_t* debug_color_copy         = debug_color_azure_radiance;
	static const float_t* debug_color_allocate     = debug_color_red;
	static const float_t* debug_color_render       = debug_color_teal;

	class debug_marker {
		std::string _name;

		public:
		//debug_marker(const float color[4], std::string name);
		debug_marker(const float_t color[4], const char* format, ...);
		~debug_marker();
	};
} // namespace gs
