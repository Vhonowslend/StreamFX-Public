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

gfx::source_texture::~source_texture()
{
	if (child && parent) {
		obs_source_remove_active_child(parent->get(), child->get());
	}

	parent.reset();
	child.reset();
}

gfx::source_texture::source_texture(obs_source_t* _parent)
{
	if (!_parent) {
		throw std::invalid_argument("parent must not be null");
	}
	parent        = std::make_shared<obs::source>(_parent, false, false);
	render_target = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
}

gfx::source_texture::source_texture(obs_source_t* _source, obs_source_t* _parent) : source_texture(_parent)
{
	if (!_source) {
		throw std::invalid_argument("source must not be null");
	}
	if (!obs_source_add_active_child(_parent, _source)) {
		throw std::runtime_error("parent is contained in child");
	}
	child = std::make_shared<obs::source>(_source, true, true);
}

gfx::source_texture::source_texture(const char* _name, obs_source_t* _parent) : source_texture(_parent)
{
	if (!_name) {
		throw std::invalid_argument("name must not be null");
	}
	child = std::make_shared<obs::source>(_name, true, true);
	if (!obs_source_add_active_child(_parent, child->get())) {
		throw std::runtime_error("parent is contained in child");
	}
}

gfx::source_texture::source_texture(std::string _name, obs_source_t* _parent) : source_texture(_name.c_str(), _parent)
{}

gfx::source_texture::source_texture(std::shared_ptr<obs::source> child, std::shared_ptr<obs::source> parent)
{
	if (!child) {
		throw std::invalid_argument("child must not be null");
	}
	if (!parent) {
		throw std::invalid_argument("parent must not be null");
	}
	if (!obs_source_add_active_child(parent->get(), child->get())) {
		throw std::runtime_error("parent is contained in child");
	}
	this->child         = child;
	this->parent        = parent;
	this->render_target = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
}

gfx::source_texture::source_texture(std::shared_ptr<obs::source> child, obs_source_t* _parent)
	: source_texture(child, std::make_shared<obs::source>(_parent, false, false))
{}

obs_source_t* gfx::source_texture::get_object()
{
	if (child) {
		return child->get();
	}
	return nullptr;
}

obs_source_t* gfx::source_texture::get_parent()
{
	return parent->get();
}

void gfx::source_texture::clear()
{
	if (child && parent) {
		obs_source_remove_active_child(parent->get(), child->get());
	}
	child->clear();
	child.reset();
}

std::shared_ptr<gs::texture> gfx::source_texture::render(size_t width, size_t height)
{
	if ((width == 0) || (width >= 16384)) {
		throw std::runtime_error("Width too large or too small.");
	}
	if ((height == 0) || (height >= 16384)) {
		throw std::runtime_error("Height too large or too small.");
	}
	if (child->destroyed() || parent->destroyed()) {
		return nullptr;
	}

	{
		auto op = render_target->render((uint32_t)width, (uint32_t)height);
		vec4 black;
		vec4_zero(&black);
		gs_ortho(0, (float_t)width, 0, (float_t)height, 0, 1);
		gs_clear(GS_CLEAR_COLOR, &black, 0, 0);
		if (child) {
			obs_source_video_render(child->get());
		}
	}

	std::shared_ptr<gs::texture> tex;
	render_target->get_texture(tex);
	return tex;
}
