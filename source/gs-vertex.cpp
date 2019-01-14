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

#include "gs-vertex.hpp"
#include "util-memory.hpp"

gs::vertex::vertex()
	: hasStore(true), store(nullptr), position(nullptr), normal(nullptr), tangent(nullptr), color(nullptr)
{
	store = util::malloc_aligned(16, sizeof(vec3) * 3 + sizeof(uint32_t) + sizeof(vec4) * MAXIMUM_UVW_LAYERS);

	size_t offset = 0;

	position = reinterpret_cast<vec3*>(reinterpret_cast<char*>(store) + offset);
	offset += sizeof(vec3);

	normal = reinterpret_cast<vec3*>(reinterpret_cast<char*>(store) + offset);
	offset += sizeof(vec3);

	tangent = reinterpret_cast<vec3*>(reinterpret_cast<char*>(store) + offset);
	offset += sizeof(vec3);

	color = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(store) + offset);
	offset += sizeof(uint32_t);

	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		uv[n] = reinterpret_cast<vec4*>(reinterpret_cast<char*>(store) + offset);
		offset += sizeof(vec4);
	}
}

gs::vertex::~vertex()
{
	if (hasStore) {
		util::free_aligned(store);
	}
}

gs::vertex::vertex(vec3* p, vec3* n, vec3* t, uint32_t* col, vec4* uvs[MAXIMUM_UVW_LAYERS])
	: hasStore(false), position(p), normal(n), tangent(t), color(col)
{
	if (uvs != nullptr) {
		for (size_t idx = 0; idx < MAXIMUM_UVW_LAYERS; idx++) {
			this->uv[idx] = uvs[idx];
		}
	}
}
