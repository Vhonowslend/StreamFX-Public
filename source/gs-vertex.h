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
#include "gs-limits.h"
#include <inttypes.h>
#include <xmmintrin.h>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/graphics/vec3.h>
	#pragma warning( pop )
}

namespace gs {
	struct vertex {
		vec3* position;
		vec3* normal;
		vec3* tangent;
		uint32_t* color;
		vec4* uv[MAXIMUM_UVW_LAYERS];

		vertex();
		vertex(vec3* p, vec3* n, vec3* t, uint32_t* col, vec4* uv[MAXIMUM_UVW_LAYERS]);
		~vertex();

		private:
		bool hasStore;
		void* store;
	};
}
