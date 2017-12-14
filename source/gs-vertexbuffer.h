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
#include "gs-vertex.h"
#include <inttypes.h>
#include <vector>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/graphics/graphics.h>
	#pragma warning( pop )
}

namespace GS {
	class VertexBuffer : public std::vector<Vertex> {
		public:
		/*!
		* \brief Create a Vertex Buffer with specific size
		*
		* \param maximumVertices Maximum amount of vertices to store.
		*/
		VertexBuffer(uint32_t maximumVertices);

		/*!
		* \brief Create a Vertex Buffer with default size
		* This will create a new vertex buffer with the default maximum size.
		*
		*/
		VertexBuffer();

		/*!
		 * \brief Create a copy of a Vertex Buffer
		 * Full Description below
		 *
		 * \param other The Vertex Buffer to copy
		 */
		VertexBuffer(VertexBuffer& other);

		/*!
		* \brief Create a Vertex Buffer from a Vertex array
		* Full Description below
		*
		* \param other The Vertex array to use
		*/
		VertexBuffer(std::vector<Vertex>& other);


		VertexBuffer(gs_vertbuffer_t* vb);

		virtual ~VertexBuffer();

		void set_uv_layers(uint32_t layers);

		uint32_t uv_layers();

		gs_vertbuffer_t* get();

		gs_vertbuffer_t* get(bool refreshGPU);

		protected:
		uint32_t m_maximumVertices;
		uint32_t m_uvwLayers;
		gs_vb_data* m_vertexbufferdata;
		gs_vertbuffer_t* m_vertexbuffer;

		// Data Storage
		struct {
			std::vector<vec3> positions;
			std::vector<vec3> normals;
			std::vector<vec3> tangents;
			std::vector<uint32_t> colors;
			std::vector<std::vector<vec4>> uvws;
			std::vector<gs_tvertarray> uvwdata;
		} m_data;
	};
}
