// Modern effects for a modern Streamer
// Copyright (C) 2017 Michael Fabian Dirks
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

#include "gfx-source-texture.h"

gfx::source_texture::~source_texture() {
	if (m_source) {
		obs_source_release(m_source);
		m_source = nullptr;
	}
	m_rt = nullptr;
}

gfx::source_texture::source_texture() {
	m_rt = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
}

obs_source_t* gfx::source_texture::get_object() {
	return m_source;
}

gfx::source_texture::source_texture(const char* name) : source_texture() {
	m_source = obs_get_source_by_name(name);
	if (!m_source) {
		throw std::invalid_argument("No such source.");
	}
}

gfx::source_texture::source_texture(std::string name) : source_texture(name.c_str()) {}

gfx::source_texture::source_texture(obs_source_t* src) : source_texture() {
	m_source = src;
	if (!m_source) {
		throw std::invalid_argument("No such source.");
	}
}

std::shared_ptr<gs::texture> gfx::source_texture::render(size_t width, size_t height) {
	if (!m_source) {
		throw std::invalid_argument("Missing source to render.");
	}
	if ((width == 0) || (width >= 16384)) {
		throw std::runtime_error("Width too large or too small.");
	}
	if ((height == 0) || (height >= 16384)) {
		throw std::runtime_error("Height too large or too small.");
	}

	{
		auto op = m_rt->render((uint32_t)width, (uint32_t)height);
		vec4 black; vec4_zero(&black);
		gs_ortho(0, (float_t)width, 0, (float_t)height, 0, 1);
		gs_clear(GS_CLEAR_COLOR, &black, 0, 0);
		obs_source_video_render(m_source);
	}

	std::shared_ptr<gs::texture> tex;
	m_rt->get_texture(tex);
	return tex;
}
