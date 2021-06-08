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

gfx::source_texture::~source_texture()
{
	if (_child && _parent) {
		obs_source_remove_active_child(_parent->get(), _child->get());
	}

	_parent.reset();
	_child.reset();
}

gfx::source_texture::source_texture(obs_source_t* parent)
{
	if (!parent) {
		throw std::invalid_argument("_parent must not be null");
	}
	_parent = std::make_shared<streamfx::obs::deprecated_source>(parent, false, false);
	_rt     = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
}

gfx::source_texture::source_texture(obs_source_t* _source, obs_source_t* _parent) : source_texture(_parent)
{
	if (!_source) {
		throw std::invalid_argument("source must not be null");
	}
	if (!obs_source_add_active_child(_parent, _source)) {
		throw std::runtime_error("_parent is contained in _child");
	}
	_child = std::make_shared<streamfx::obs::deprecated_source>(_source, true, true);
}

gfx::source_texture::source_texture(const char* _name, obs_source_t* _parent) : source_texture(_parent)
{
	if (!_name) {
		throw std::invalid_argument("name must not be null");
	}
	_child = std::make_shared<streamfx::obs::deprecated_source>(_name, true, true);
	if (!obs_source_add_active_child(_parent, _child->get())) {
		throw std::runtime_error("_parent is contained in _child");
	}
}

gfx::source_texture::source_texture(std::string _name, obs_source_t* _parent) : source_texture(_name.c_str(), _parent)
{}

gfx::source_texture::source_texture(std::shared_ptr<streamfx::obs::deprecated_source> pchild,
									std::shared_ptr<streamfx::obs::deprecated_source> pparent)
{
	if (!pchild) {
		throw std::invalid_argument("_child must not be null");
	}
	if (!pparent) {
		throw std::invalid_argument("_parent must not be null");
	}
	if (!obs_source_add_active_child(pparent->get(), pchild->get())) {
		throw std::runtime_error("_parent is contained in _child");
	}
	this->_child  = pchild;
	this->_parent = pparent;
	this->_rt     = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
}

gfx::source_texture::source_texture(std::shared_ptr<streamfx::obs::deprecated_source> _child, obs_source_t* _parent)
	: source_texture(_child, std::make_shared<streamfx::obs::deprecated_source>(_parent, false, false))
{}

obs_source_t* gfx::source_texture::get_object()
{
	if (_child) {
		return _child->get();
	}
	return nullptr;
}

obs_source_t* gfx::source_texture::get_parent()
{
	return _parent->get();
}

void gfx::source_texture::clear()
{
	if (_child && _parent) {
		obs_source_remove_active_child(_parent->get(), _child->get());
	}
	_child->clear();
	_child.reset();
}

std::shared_ptr<gs::texture> gfx::source_texture::render(std::size_t width, std::size_t height)
{
	if ((width == 0) || (width >= 16384)) {
		throw std::runtime_error("Width too large or too small.");
	}
	if ((height == 0) || (height >= 16384)) {
		throw std::runtime_error("Height too large or too small.");
	}
	if (_child->destroyed() || _parent->destroyed()) {
		return nullptr;
	}

	if (_child) {
#ifdef ENABLE_PROFILING
		auto cctr =
			gs::debug_marker(gs::debug_color_capture, "gfx::source_texture '%s'", obs_source_get_name(_child->get()));
#endif
		auto op = _rt->render(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		vec4 black;
		vec4_zero(&black);
		gs_ortho(0, static_cast<float>(width), 0, static_cast<float_t>(height), 0, 1);
		gs_clear(GS_CLEAR_COLOR, &black, 0, 0);
		obs_source_video_render(_child->get());
	}

	std::shared_ptr<gs::texture> tex;
	_rt->get_texture(tex);
	return tex;
}
