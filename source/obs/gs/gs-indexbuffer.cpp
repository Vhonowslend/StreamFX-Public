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

#include "gs-indexbuffer.hpp"
#include <stdexcept>
#include "gs-limits.hpp"
#include "obs/gs/gs-helper.hpp"

gs::index_buffer::index_buffer(std::uint32_t maximumVertices)
{
	this->reserve(maximumVertices);
	auto gctx     = gs::context();
	_index_buffer = gs_indexbuffer_create(gs_index_type::GS_UNSIGNED_LONG, this->data(), maximumVertices, GS_DYNAMIC);
}

gs::index_buffer::index_buffer() : index_buffer(MAXIMUM_VERTICES) {}

gs::index_buffer::index_buffer(index_buffer& other) : index_buffer((uint32_t)other.size())
{
	std::copy(other.begin(), other.end(), this->end());
}

gs::index_buffer::index_buffer(std::vector<uint32_t>& other) : index_buffer((uint32_t)other.size())
{
	std::copy(other.begin(), other.end(), this->end());
}

gs::index_buffer::~index_buffer()
{
	auto gctx = gs::context();
	gs_indexbuffer_destroy(_index_buffer);
}

gs_indexbuffer_t* gs::index_buffer::get()
{
	return get(true);
}

gs_indexbuffer_t* gs::index_buffer::get(bool refreshGPU)
{
	if (refreshGPU) {
		auto gctx = gs::context();
		gs_indexbuffer_flush(_index_buffer);
	}
	return _index_buffer;
}
