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

#include "gs-vertex.h"
#include "util-memory.h"

gs::vertex::vertex() {
	this->hasStore = true;
	this->store = util::malloc_aligned(16, sizeof(vec3) * 3 + sizeof(uint32_t) + sizeof(vec4)*MAXIMUM_UVW_LAYERS);
	this->position = reinterpret_cast<vec3*>(store);
	this->normal = reinterpret_cast<vec3*>(reinterpret_cast<char*>(store) + (16 * 1));
	this->tangent = reinterpret_cast<vec3*>(reinterpret_cast<char*>(store) + (16 * 2));
	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		this->uv[n] = reinterpret_cast<vec4*>(reinterpret_cast<char*>(store) + (16 * (2 + n)));
	}
	this->color = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(store) + (16 * (3 + MAXIMUM_UVW_LAYERS)));
}

gs::vertex::~vertex() {
	if (hasStore)
		util::free_aligned(store);
}

gs::vertex::vertex(vec3* p, vec3* n, vec3* t, uint32_t* col, vec4* uvs[MAXIMUM_UVW_LAYERS])
	: position(p), normal(n), tangent(t), color(col) {
	if (uvs != nullptr) {
		for (size_t idx = 0; idx < MAXIMUM_UVW_LAYERS; idx++) {
			this->uv[idx] = uvs[idx];
		}
	}
	this->hasStore = false;
}
