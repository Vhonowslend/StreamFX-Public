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
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/graphics/vec3.h>
	#pragma warning( pop )
}

namespace GS {
	const uint32_t MAXIMUM_UVW_LAYERS = 8u;
	// ToDo: Optimize for use with GS::VertexBuffer so that it doesn't require in-memory copy.
	struct Vertex {
		vec3 position;
		vec3 normal;
		vec3 tangent;
		vec4 uv[MAXIMUM_UVW_LAYERS];
		uint32_t color;
	};
}
