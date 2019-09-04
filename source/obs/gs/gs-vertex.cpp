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
#include <stdexcept>
#include "util-memory.hpp"

gs::vertex::vertex()
	: position(nullptr), normal(nullptr), tangent(nullptr), color(nullptr), _has_store(true), _store(nullptr)
{
	_store = util::malloc_aligned(16, sizeof(vec3) * 3 + sizeof(uint32_t) + sizeof(vec4) * MAXIMUM_UVW_LAYERS);

	size_t offset = 0;

	position = reinterpret_cast<vec3*>(reinterpret_cast<char*>(_store) + offset);
	offset += sizeof(vec3);

	normal = reinterpret_cast<vec3*>(reinterpret_cast<char*>(_store) + offset);
	offset += sizeof(vec3);

	tangent = reinterpret_cast<vec3*>(reinterpret_cast<char*>(_store) + offset);
	offset += sizeof(vec3);

	color = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(_store) + offset);
	offset += sizeof(uint32_t);

	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		uv[n] = reinterpret_cast<vec4*>(reinterpret_cast<char*>(_store) + offset);
		offset += sizeof(vec4);
	}
}

gs::vertex::~vertex()
{
	if (_has_store) {
		util::free_aligned(_store);
	}
}

gs::vertex::vertex(vec3* p, vec3* n, vec3* t, uint32_t* col, vec4* uvs[MAXIMUM_UVW_LAYERS])
	: position(p), normal(n), tangent(t), color(col), _has_store(false)
{
	if (uvs != nullptr) {
		for (size_t idx = 0; idx < MAXIMUM_UVW_LAYERS; idx++) {
			this->uv[idx] = uvs[idx];
		}
	}
}
