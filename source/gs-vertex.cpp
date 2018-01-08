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

#include "gs-vertexbuffer.h"
#include "util-memory.h"
#include <malloc.h>

GS::Vertex& GS::Vertex::operator=(const Vertex& r) {
	vec3_copy(&this->position, &r.position);
	vec3_copy(&this->normal, &r.normal);
	vec3_copy(&this->tangent, &r.tangent);
	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		vec4_copy(&this->uv[n], &r.uv[n]);
	}
	return *this;
}

GS::Vertex* GS::Vertex::operator=(const Vertex* r) {
	vec3_copy(&this->position, &r->position);
	vec3_copy(&this->normal, &r->normal);
	vec3_copy(&this->tangent, &r->tangent);
	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		vec4_copy(&this->uv[n], &r->uv[n]);
	}
	return this;
}

void* GS::Vertex::operator new(size_t count) {
	return _aligned_malloc(count, 16);
}

void* GS::Vertex::operator new(size_t count, void* d){
	return d;
}

void* GS::Vertex::operator new[](size_t count) {
	return _aligned_malloc(count, 16);
}

void* GS::Vertex::operator new[](size_t count, void* d) {
	return d;
}

void GS::Vertex::operator delete(void* p) {
	return _aligned_free(p);
}

void GS::Vertex::operator delete[](void* p) {
	return _aligned_free(p);
}
