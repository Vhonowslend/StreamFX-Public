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
#include <stdexcept>
extern "C" {
#pragma warning( push )
#pragma warning( disable: 4201 )
#include <libobs/obs.h>
#pragma warning( pop )
}

#pragma region Constructor & Destructor
GS::VertexBuffer::VertexBuffer(uint32_t maximumVertices) {
	if (maximumVertices > MAXIMUM_VERTICES) {
		throw std::out_of_range("maximumVertices out of range");
	}

	// Assign limits.
	m_capacity = maximumVertices;
	m_layers = MAXIMUM_UVW_LAYERS;

	// Allocate memory for data.
	m_vertexbufferdata = gs_vbdata_create();
	m_vertexbufferdata->num = m_capacity;
	m_vertexbufferdata->points = m_positions = (vec3*)util::malloc_aligned(16, sizeof(vec3) * m_capacity);
	m_vertexbufferdata->normals = m_normals = (vec3*)util::malloc_aligned(16, sizeof(vec3) * m_capacity);
	m_vertexbufferdata->tangents = m_tangents = (vec3*)util::malloc_aligned(16, sizeof(vec3) * m_capacity);
	m_vertexbufferdata->colors = m_colors = (uint32_t*)util::malloc_aligned(16, sizeof(uint32_t) * m_capacity);
	m_vertexbufferdata->num_tex = m_layers;
	m_vertexbufferdata->tvarray = m_layerdata = (gs_tvertarray*)util::malloc_aligned(16, sizeof(gs_tvertarray)* m_layers);
	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		m_layerdata[n].array = m_uvs[n] = (vec4*)util::malloc_aligned(16, sizeof(vec4) * m_capacity);
		m_layerdata[n].width = 4;
	}

	// Allocate GPU
	obs_enter_graphics();
	m_vertexbuffer = gs_vertexbuffer_create(m_vertexbufferdata, GS_DYNAMIC);
	std::memset(m_vertexbufferdata, 0, sizeof(gs_vb_data));
	m_vertexbufferdata->num = m_capacity;
	m_vertexbufferdata->num_tex = m_layers;
	obs_leave_graphics();
	if (!m_vertexbuffer) {
		throw std::runtime_error("Failed to create vertex buffer.");
	}
}

GS::VertexBuffer::VertexBuffer() : VertexBuffer(MAXIMUM_VERTICES) {}

GS::VertexBuffer::~VertexBuffer() {
	if (m_positions) {
		util::free_aligned(m_positions);
		m_positions = nullptr;
	}
	if (m_normals) {
		util::free_aligned(m_normals);
		m_normals = nullptr;
	}
	if (m_tangents) {
		util::free_aligned(m_tangents);
		m_tangents = nullptr;
	}
	if (m_colors) {
		util::free_aligned(m_colors);
		m_colors = nullptr;
	}
	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		if (m_uvs[n]) {
			util::free_aligned(m_uvs[n]);
			m_uvs[n] = nullptr;
		}
	}
	if (m_layerdata) {
		util::free_aligned(m_layerdata);
		m_layerdata = nullptr;
	}
	if (m_vertexbufferdata) {
		std::memset(m_vertexbufferdata, 0, sizeof(gs_vb_data));
		if (!m_vertexbuffer) {
			gs_vbdata_destroy(m_vertexbufferdata);
			m_vertexbufferdata = nullptr;
		}
	}
	if (m_vertexbuffer) {
		obs_enter_graphics();
		gs_vertexbuffer_destroy(m_vertexbuffer);
		obs_leave_graphics();
		m_vertexbuffer = nullptr;
	}
}
#pragma endregion Constructor & Destructor

#pragma region Copy & Move Constructor
GS::VertexBuffer::VertexBuffer(VertexBuffer& other) : VertexBuffer(other.m_capacity) {

}

GS::VertexBuffer::VertexBuffer(gs_vertbuffer_t* vb) {
	m_vertexbuffer = vb;
}
#pragma endregion Copy & Move Constructor

void GS::VertexBuffer::resize(size_t new_size) {
	if (new_size > m_capacity) {
		throw std::out_of_range("new_size out of range");
	}
	m_size = new_size;
}

size_t GS::VertexBuffer::size() {
	return m_size;
}

bool GS::VertexBuffer::empty() {
	return m_size == 0;
}

const GS::Vertex GS::VertexBuffer::at(size_t idx) {
	if ((idx < 0) || (idx >= m_size)) {
		throw std::out_of_range("idx out of range");
	}

	GS::Vertex vtx;
	vtx.position = &m_positions[idx];
	vtx.normal = &m_normals[idx];
	vtx.tangent = &m_tangents[idx];
	vtx.color = &m_colors[idx];
	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		vtx.uv[n] = &m_uvs[n][idx];
	}
	return vtx;
}

const GS::Vertex GS::VertexBuffer::operator[](const size_t pos) {
	return at(pos);
}

void GS::VertexBuffer::set_uv_layers(uint32_t layers) {
	m_layers = layers;
}

uint32_t GS::VertexBuffer::uv_layers() {
	return m_layers;
}

gs_vertbuffer_t* GS::VertexBuffer::get(bool refreshGPU) {
	if (!refreshGPU)
		return m_vertexbuffer;

	if (m_size > m_capacity)
		throw std::out_of_range("size is larger than capacity");

	// Update VertexBuffer data.
	m_vertexbufferdata = gs_vertexbuffer_get_data(m_vertexbuffer);
	std::memset(m_vertexbufferdata, 0, sizeof(gs_vb_data));
	m_vertexbufferdata->num = m_capacity;
	m_vertexbufferdata->points = m_positions;
	m_vertexbufferdata->normals = m_normals;
	m_vertexbufferdata->tangents = m_tangents;
	m_vertexbufferdata->colors = m_colors;
	m_vertexbufferdata->num_tex = m_layers;
	m_vertexbufferdata->tvarray = m_layerdata;
	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		m_layerdata[n].array = m_uvs[n];
		m_layerdata[n].width = 4;
	}

	// Update GPU
	obs_enter_graphics();
	gs_vertexbuffer_flush(m_vertexbuffer);
	obs_leave_graphics();

	// WORKAROUND: OBS Studio 20.x and below incorrectly deletes data that it doesn't own.
	std::memset(m_vertexbufferdata, 0, sizeof(gs_vb_data));
	m_vertexbufferdata->num = m_capacity;
	m_vertexbufferdata->num_tex = m_layers;
	for (uint32_t n = 0; n < m_layers; n++) {
		m_layerdata[n].width = 4;
	}

	return m_vertexbuffer;
}

gs_vertbuffer_t* GS::VertexBuffer::get() {
	return get(true);
}
