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
#include "gs-vertex.h"
#include "util-math.h"
#include "util-memory.h"
#include <inttypes.h>
extern "C" {
#pragma warning( push )
#pragma warning( disable: 4201 )
#include <libobs/graphics/graphics.h>
#pragma warning( pop )
}

namespace GS {
	class VertexBuffer {
		public:
	#pragma region Constructor & Destructor
		virtual ~VertexBuffer();

		/*!
		* \brief Create a Vertex Buffer with a specific number of Vertices.
		*
		* \param maximumVertices Maximum amount of vertices to store.
		*/
		VertexBuffer(uint32_t maximumVertices);

		/*!
		* \brief Create a Vertex Buffer with the maximum number of Vertices.
		*
		* \param maximumVertices Maximum amount of vertices to store.
		*/
		VertexBuffer() : VertexBuffer(MAXIMUM_VERTICES) {};

		/*!
		* \brief Create a copy of a Vertex Buffer
		* Full Description below
		*
		* \param other The Vertex Buffer to copy
		*/
		VertexBuffer(gs_vertbuffer_t* other);

	#pragma endregion Constructor & Destructor

	#pragma region Copy/Move Constructors
		// Copy Constructor & Assignments

		/*!
		* \brief Copy Constructor
		* 
		*
		* \param other 
		*/
		VertexBuffer(VertexBuffer const& other);

		/*!
		* \brief Copy Assignment
		* Unsafe operation and as such marked as deleted.
		*
		* \param other
		*/
		void operator=(VertexBuffer const& other) = delete;

		// Move Constructor & Assignments

		/*!
		* \brief Move Constructor
		*
		*
		* \param other
		*/
		VertexBuffer(VertexBuffer const&& other);

		/*!
		* \brief Move Assignment
		*
		*
		* \param other
		*/
		void operator=(VertexBuffer const&& other);
	#pragma endregion Copy/Move Constructors
		


		void Resize(uint32_t new_size);

		uint32_t Size();

		bool Empty();

		const GS::Vertex At(uint32_t idx);

		const GS::Vertex operator[](uint32_t const pos);

		void SetUVLayers(uint32_t layers);

		uint32_t GetUVLayers();

	#pragma region Update / Grab GS object
		gs_vertbuffer_t* Update();

		gs_vertbuffer_t* Update(bool refreshGPU);
	#pragma endregion Update / Grab GS object

		private:
		uint32_t m_size;
		uint32_t m_capacity;
		uint32_t m_layers;

		// Memory Storage
		vec3 *m_positions;
		vec3 *m_normals;
		vec3 *m_tangents;
		uint32_t *m_colors;
		vec4 *m_uvs[MAXIMUM_UVW_LAYERS];

		// OBS GS Data
		gs_vb_data* m_vertexbufferdata;
		gs_vertbuffer_t* m_vertexbuffer;
		gs_tvertarray* m_layerdata;
	};
}
