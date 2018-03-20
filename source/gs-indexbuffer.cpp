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

#include "gs-indexbuffer.h"
#include "gs-limits.h"
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/obs.h>
	#pragma warning( pop )
}

gs::index_buffer::index_buffer(uint32_t maximumVertices) {
	this->reserve(maximumVertices);

	obs_enter_graphics();
	m_indexBuffer = gs_indexbuffer_create(gs_index_type::GS_UNSIGNED_LONG, this->data(), maximumVertices, GS_DYNAMIC);
	obs_leave_graphics();
}

gs::index_buffer::index_buffer() : index_buffer(MAXIMUM_VERTICES) {}

gs::index_buffer::index_buffer(index_buffer& other) : index_buffer((uint32_t)other.size()) {
	std::copy(other.begin(), other.end(), this->end());
}

gs::index_buffer::index_buffer(std::vector<uint32_t>& other) : index_buffer((uint32_t)other.size()) {
	std::copy(other.begin(), other.end(), this->end());
}

gs::index_buffer::~index_buffer() {
	obs_enter_graphics();
	gs_indexbuffer_destroy(m_indexBuffer);
	obs_leave_graphics();
}

gs_indexbuffer_t* gs::index_buffer::get() {
	return get(true);
}

gs_indexbuffer_t* gs::index_buffer::get(bool refreshGPU) {
	if (refreshGPU) {
		obs_enter_graphics();
		gs_indexbuffer_flush(m_indexBuffer);
		obs_leave_graphics();
	}
	return m_indexBuffer;
}
