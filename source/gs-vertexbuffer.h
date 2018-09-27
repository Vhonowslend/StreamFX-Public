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
#include <cinttypes>
#include "gs-limits.h"
#include "gs-vertex.h"
#include "util-math.h"
#include "util-memory.h"

extern "C" {
#pragma warning(push)
#pragma warning(disable : 4201)
#include <graphics/graphics.h>
#pragma warning(pop)
}

namespace gs {
	class vertex_buffer {
		public:
#pragma region Constructor& Destructor
		virtual ~vertex_buffer();

		/*!
		* \brief Create a Vertex Buffer with the default number of Vertices.
		*/
		vertex_buffer();

		/*!
		* \brief Create a Vertex Buffer with a specific number of Vertices.
		*
		* \param vertices Number of vertices to store.
		*/
		vertex_buffer(uint32_t vertices);

		/*!
		* \brief Create a Vertex Buffer with a specific number of Vertices and uv layers.
		*
		* \param vertices Number of vertices to store.
		* \param layers Number of uv layers to store.
		*/
		vertex_buffer(uint32_t vertices, uint8_t layers);

		/*!
		* \brief Create a copy of a Vertex Buffer
		* Full Description below
		*
		* \param other The Vertex Buffer to copy
		*/
		vertex_buffer(gs_vertbuffer_t* other);

#pragma endregion Constructor& Destructor

#pragma region Copy / Move Constructors
		// Copy Constructor & Assignments

		/*!
		* \brief Copy Constructor
		* 
		*
		* \param other 
		*/
		vertex_buffer(vertex_buffer const& other);

		/*!
		* \brief Copy Assignment
		* Unsafe operation and as such marked as deleted.
		*
		* \param other
		*/
		void operator=(vertex_buffer const& other) = delete;

		// Move Constructor & Assignments

		/*!
		* \brief Move Constructor
		*
		*
		* \param other
		*/
		vertex_buffer(vertex_buffer const&& other);

		/*!
		* \brief Move Assignment
		*
		*
		* \param other
		*/
		void operator=(vertex_buffer const&& other);
#pragma endregion Copy / Move Constructors

		void resize(uint32_t new_size);

		uint32_t size();

		bool empty();

		const gs::vertex at(uint32_t idx);

		const gs::vertex operator[](uint32_t const pos);

		void set_uv_layers(uint32_t layers);

		uint32_t get_uv_layers();

		/*!
		* \brief Directly access the positions buffer
		* Returns the internal memory that is assigned to hold all vertex positions.
		*
		* \return A <vec3*> that points at the first vertex's position.
		*/
		vec3* get_positions();

		/*!
		* \brief Directly access the normals buffer
		* Returns the internal memory that is assigned to hold all vertex normals.
		*
		* \return A <vec3*> that points at the first vertex's normal.
		*/
		vec3* get_normals();

		/*!
		* \brief Directly access the tangents buffer
		* Returns the internal memory that is assigned to hold all vertex tangents.
		*
		* \return A <vec3*> that points at the first vertex's tangent.
		*/
		vec3* get_tangents();

		/*!
		* \brief Directly access the colors buffer
		* Returns the internal memory that is assigned to hold all vertex colors.
		*
		* \return A <uint32_t*> that points at the first vertex's color.
		*/
		uint32_t* get_colors();

		/*!
		* \brief Directly access the uv buffer
		* Returns the internal memory that is assigned to hold all vertex uvs.
		*
		* \return A <vec4*> that points at the first vertex's uv.
		*/
		vec4* get_uv_layer(size_t idx);

#pragma region Update / Grab GS object
		gs_vertbuffer_t* update();

		gs_vertbuffer_t* update(bool refreshGPU);
#pragma endregion Update / Grab GS object

		private:
		uint32_t m_size;
		uint32_t m_capacity;
		uint32_t m_layers;

		// Memory Storage
		vec3*     m_positions;
		vec3*     m_normals;
		vec3*     m_tangents;
		uint32_t* m_colors;
		vec4*     m_uvs[MAXIMUM_UVW_LAYERS];

		// OBS GS Data
		gs_vb_data*      m_vertexbufferdata;
		gs_vertbuffer_t* m_vertexbuffer;
		gs_tvertarray*   m_layerdata;
	};
} // namespace gs
