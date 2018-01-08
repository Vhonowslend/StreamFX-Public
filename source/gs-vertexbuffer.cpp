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
#include <stdexcept>
extern "C" {
#pragma warning( push )
#pragma warning( disable: 4201 )
#include <libobs/obs.h>
#pragma warning( pop )
}

const uint32_t defaultMaximumVertices = 65535u;

GS::VertexBuffer::VertexBuffer(uint32_t maximumVertices) {
	m_maximumVertices = maximumVertices;
	m_uvwLayers = MAXIMUM_UVW_LAYERS;

	// Reserve Space
	m_vertexbufferdata = gs_vbdata_create();
	m_vertexbufferdata->num = m_maximumVertices;
	m_data.positions.resize(m_maximumVertices);
	m_vertexbufferdata->points = m_data.positions.data();
	m_data.normals.resize(m_maximumVertices);
	m_vertexbufferdata->normals = m_data.normals.data();
	m_data.tangents.resize(m_maximumVertices);
	m_vertexbufferdata->tangents = m_data.tangents.data();
	m_data.colors.resize(m_maximumVertices);
	m_vertexbufferdata->colors = m_data.colors.data();
	m_vertexbufferdata->num_tex = m_uvwLayers;
	m_data.uvws.resize(m_uvwLayers);
	m_data.uvwdata.resize(m_uvwLayers);
	for (uint32_t n = 0; n < m_uvwLayers; n++) {
		m_data.uvws[n].resize(m_maximumVertices);
		m_data.uvwdata[n].width = 4;
		m_data.uvwdata[n].array = m_data.uvws[n].data();
	}
	m_vertexbufferdata->tvarray = m_data.uvwdata.data();

	// Allocate GPU
	obs_enter_graphics();
	m_vertexbuffer = gs_vertexbuffer_create(m_vertexbufferdata, GS_DYNAMIC);
	std::memset(m_vertexbufferdata, 0, sizeof(gs_vb_data));
	obs_leave_graphics();
	if (!m_vertexbuffer) {
		throw std::runtime_error("Failed to create vertex buffer.");
	}
}

GS::VertexBuffer::VertexBuffer(gs_vertbuffer_t* vb) {
	m_vertexbuffer = vb;
}

GS::VertexBuffer::VertexBuffer() : VertexBuffer(defaultMaximumVertices) {}

GS::VertexBuffer::VertexBuffer(std::vector<Vertex*>& other) : VertexBuffer((uint32_t)other.capacity()) {
	std::copy(other.begin(), other.end(), this->end());
}

GS::VertexBuffer::VertexBuffer(VertexBuffer& other) : VertexBuffer(other.m_maximumVertices) {
	std::copy(other.begin(), other.end(), this->end());
}

GS::VertexBuffer::~VertexBuffer() {
	if (m_vertexbuffer) {
		std::memset(m_vertexbufferdata, 0, sizeof(gs_vb_data));

		obs_enter_graphics();
		gs_vertexbuffer_destroy(m_vertexbuffer);
		obs_leave_graphics();
	}
	m_vertexbuffer = nullptr;
}

void GS::VertexBuffer::set_uv_layers(uint32_t layers) {
	m_uvwLayers = layers;
}

uint32_t GS::VertexBuffer::uv_layers() {
	return m_uvwLayers;
}

gs_vertbuffer_t* GS::VertexBuffer::get(bool refreshGPU) {
	if (refreshGPU) {
		if (size() > m_maximumVertices)
			throw std::runtime_error("Too many vertices in Vertex Buffer.");

		// Update data pointer from Graphics Subsystem.
		m_vertexbufferdata = gs_vertexbuffer_get_data(m_vertexbuffer);
		std::memset(m_vertexbufferdata, 0, sizeof(gs_vb_data));

		// Resize buffers.
		m_data.positions.resize(m_maximumVertices);
		m_data.normals.resize(m_maximumVertices);
		m_data.tangents.resize(m_maximumVertices);
		m_data.colors.resize(m_maximumVertices);
		m_data.uvws.resize(m_uvwLayers);
		m_data.uvwdata.resize(m_uvwLayers);

		// Assign new data.
		m_vertexbufferdata->num = m_maximumVertices;
		m_vertexbufferdata->points = m_data.positions.data();
		m_vertexbufferdata->normals = m_data.normals.data();
		m_vertexbufferdata->tangents = m_data.tangents.data();
		m_vertexbufferdata->colors = m_data.colors.data();
		m_vertexbufferdata->num_tex = m_uvwLayers;
		for (uint32_t n = 0; n < m_uvwLayers; n++) {
			m_data.uvws[n].resize(m_maximumVertices);
			m_data.uvwdata[n].width = 4;
			m_data.uvwdata[n].array = m_data.uvws[n].data();
		}
		m_vertexbufferdata->tvarray = m_data.uvwdata.data();

		// Copy Data
		for (size_t vertexIdx = 0; vertexIdx < size(); vertexIdx++) {
			GS::Vertex& v = this->at(vertexIdx);
			vec3_copy(&m_data.positions[vertexIdx], &(v.position));
			vec3_copy(&m_data.normals[vertexIdx], &(v.normal));
			vec3_copy(&m_data.tangents[vertexIdx], &(v.tangent));
			for (size_t texcoordIdx = 0; texcoordIdx < m_uvwLayers; texcoordIdx++) {
				vec4_copy(&m_data.uvws[texcoordIdx][vertexIdx], &(v.uv[texcoordIdx]));
			}
			m_data.colors[vertexIdx] = v.color;
		}

		// Update GPU
		obs_enter_graphics();
		gs_vertexbuffer_flush(m_vertexbuffer);
		obs_leave_graphics();

		// WORKAROUND: OBS Studio 20.x and below incorrectly deletes data that it doesn't own.
		std::memset(m_vertexbufferdata, 0, sizeof(gs_vb_data));
		m_vertexbufferdata->num = m_maximumVertices;
		m_vertexbufferdata->num_tex = m_uvwLayers;
		for (uint32_t n = 0; n < m_uvwLayers; n++) {
			m_data.uvwdata[n].width = 4;
		}

	}
	return m_vertexbuffer;
}

gs_vertbuffer_t* GS::VertexBuffer::get() {
	return get(true);
}
