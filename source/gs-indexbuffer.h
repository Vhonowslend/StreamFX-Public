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
#include <vector>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/graphics/graphics.h>
	#pragma warning( pop )
}

namespace gs {
	class index_buffer : public std::vector<uint32_t> {
		public:
		index_buffer(uint32_t maximumVertices);
		index_buffer();
		index_buffer(index_buffer& other);
		index_buffer(std::vector<uint32_t>& other);
		virtual ~index_buffer();

		gs_indexbuffer_t* get();

		gs_indexbuffer_t* get(bool refreshGPU);

		protected:
		gs_indexbuffer_t* m_indexBuffer;
	};
}
