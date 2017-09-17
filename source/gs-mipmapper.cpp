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

#include "gs-mipmapper.h"
extern "C" {
#pragma warning (push)
#pragma warning (disable: 4201)
#include "libobs/obs.h"
#include "libobs/graphics/graphics.h"
#pragma warning (pop)
}

#define MAX_LAYERS 16

GS::MipMapper::MipMapper() : m_rt(GS_RGBA, GS_ZS_NONE), m_vb(65535) {
	// Sub layers start here now.
	double_t size = 0.5, offset = 0.0;
	for (size_t layer = 0; layer < MAX_LAYERS; layer++) {
		// Large Block
		if (layer == 0) {
			{
				GS::Vertex& v = m_vb.at(layer * 12);
				vec3_set(&v.position, offset, offset, 0);
				vec4_set(&v.uv[0], 0, 0, 0, 0);
			}
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 1);
				vec3_set(&v.position, offset + size, offset, 0);
				vec4_set(&v.uv[0], 1, 0, 0, 0);
			}
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 2);
				vec3_set(&v.position, offset + size, offset + size, 0);
				vec4_set(&v.uv[0], 1, 1, 0, 0);
			}
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 3);
				vec3_set(&v.position, offset, offset + size, 0);
				vec4_set(&v.uv[0], 0, 1, 0, 0);
			}
		} else {
			{
				GS::Vertex& v = m_vb.at(layer * 12);
				vec3_set(&v.position, offset, offset, 0);
				vec4_set(&v.uv[0], offset - size * 2, offset - size * 2, 0, 0);
			}
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 1);
				vec3_set(&v.position, offset + size, offset, 0);
				vec4_set(&v.uv[0], offset, offset - size * 2, 0, 0);
			}
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 2);
				vec3_set(&v.position, offset + size, offset + size, 0);
				vec4_set(&v.uv[0], offset, offset, 0, 0);
			}
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 3);
				vec3_set(&v.position, offset, offset + size, 0);
				vec4_set(&v.uv[0], offset - size * 2, offset, 0, 0);
			}
		}

		if (layer != 0) {
			// Horizontal Slice
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 4);
				vec3_set(&v.position, 0, offset, 0);
				vec4_set(&v.uv[0], 0, offset - size, 0, 0);
			}
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 5);
				vec3_set(&v.position, size, offset, 0);
				vec4_set(&v.uv[0], .5, offset - size, 0, 0);
			}
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 6);
				vec3_set(&v.position, size, offset + size, 0);
				vec4_set(&v.uv[0], .5, offset, 0, 0);
			}
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 7);
				vec3_set(&v.position, 0, offset + size, 0);
				vec4_set(&v.uv[0], 0, offset, 0, 0);
			}

			// Vertical Slice
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 8);
				vec3_set(&v.position, offset, 0, 0);
				vec4_set(&v.uv[0], offset - size, 0, 0, 0);
			}
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 9);
				vec3_set(&v.position, offset + size, 0, 0);
				vec4_set(&v.uv[0], offset, 0, 0, 0);
			}
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 10);
				vec3_set(&v.position, offset + size, size, 0);
				vec4_set(&v.uv[0], offset, 0.5, 0, 0);
			}
			{
				GS::Vertex& v = m_vb.at(layer * 12 + 11);
				vec3_set(&v.position, offset, size, 0);
				vec4_set(&v.uv[0], offset - size, 0.5, 0, 0);
			}
		}

		offset += size;
		size /= 2.0;
	}
}

GS::MipMapper::~MipMapper() {}

gs_texture_t* GS::MipMapper::Render(gs_texture_t* intex) {
	// Get Texture Size
	uint32_t width = gs_texture_get_width(intex),
		height = gs_texture_get_height(intex);

	// Calculate total mipmap layers.
	uint32_t layers = 1, size = height;
	if (width > height) size = width;
	layers = static_cast<uint32_t>(ceil(log(double_t(size)) / log(2.0)));
	size = pow(2, layers);

	// Render
	m_buf = GS::Texture(size, size, GS_RGBA, 1, nullptr, GS_DYNAMIC);
	for (uint32_t layer = 0; layer < layers; layer++) {
		try {
			GS::RenderTargetOp op = m_rt.Render(size * 2, size * 2);
			gs_load_vertexbuffer(m_vb.get());
			gs_load_indexbuffer(nullptr);
			gs_draw(gs_draw_mode::GS_TRIS, layer * 12, 12);
		} catch (...) {
			return intex;
		}
		gs_copy_texture(m_buf.GetObject(), m_rt.GetTextureObject());
	}

	return m_buf.GetObject();
}
