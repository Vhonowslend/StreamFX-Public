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

#include "gfx-source-texture.hpp"
#include <stdexcept>
#include "obs/gs/gs-helper.hpp"
#include "obs/obs-tools.hpp"

streamfx::gfx::source_texture::~source_texture()
{
	if (_child && _parent) {
		obs_source_remove_active_child(_parent.get(), _child.get());
	}
}

streamfx::gfx::source_texture::source_texture(streamfx::obs::weak_source child, streamfx::obs::weak_source parent)
	: _child(child.lock()), _parent(parent.lock())
{
	// Verify that 'child' and 'parent' exist.
	if (!_child || !_parent) {
		throw std::invalid_argument("Child or Parent does not exist.");
	}

	// Verify that 'child' does not contain 'parent'.
	if (::streamfx::obs::tools::source_find_source(_child, _parent)) {
		throw std::runtime_error("Child contains Parent");
	} else if (!obs_source_add_active_child(_parent.get(), _child.get())) {
		throw std::runtime_error("Child contains Parent");
	}

	_rt = std::make_shared<streamfx::obs::gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
}

obs_source_t* streamfx::gfx::source_texture::get_object()
{
	if (_child) {
		return _child.get();
	}
	return nullptr;
}

obs_source_t* streamfx::gfx::source_texture::get_parent()
{
	return _parent.get();
}

void streamfx::gfx::source_texture::clear()
{
	if (_child && _parent) {
		obs_source_remove_active_child(_parent.get(), _child.get());
	}
	_child = {};
}

std::shared_ptr<streamfx::obs::gs::texture> streamfx::gfx::source_texture::render(std::size_t width, std::size_t height)
{
	if ((width == 0) || (width >= 16384)) {
		throw std::runtime_error("Width too large or too small.");
	}
	if ((height == 0) || (height >= 16384)) {
		throw std::runtime_error("Height too large or too small.");
	}
	if (!_child || !_parent) {
		return nullptr;
	}

	if (_child) {
#ifdef ENABLE_PROFILING
		auto cctr = streamfx::obs::gs::debug_marker(streamfx::obs::gs::debug_color_capture, "gfx::source_texture '%s'",
													obs_source_get_name(_child.get()));
#endif
		auto op = _rt->render(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		vec4 black;
		vec4_zero(&black);
		gs_ortho(0, static_cast<float>(width), 0, static_cast<float_t>(height), 0, 1);
		gs_clear(GS_CLEAR_COLOR, &black, 0, 0);
		obs_source_video_render(_child.get());
	}

	std::shared_ptr<streamfx::obs::gs::texture> tex;
	_rt->get_texture(tex);
	return tex;
}
