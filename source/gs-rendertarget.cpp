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

#include "gs-rendertarget.hpp"
#include <stdexcept>

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <graphics/graphics.h>
#include <obs.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

gs::rendertarget::~rendertarget()
{
	obs_enter_graphics();
	gs_texrender_destroy(render_target);
	obs_leave_graphics();
}

gs::rendertarget::rendertarget(gs_color_format colorFormat, gs_zstencil_format zsFormat)
	: color_format(colorFormat), zstencil_format(zsFormat)
{
	is_being_rendered = false;
	obs_enter_graphics();
	render_target = gs_texrender_create(colorFormat, zsFormat);
	obs_leave_graphics();
	if (!render_target) {
		throw std::runtime_error("Failed to create render target.");
	}
}

gs::rendertarget_op gs::rendertarget::render(uint32_t width, uint32_t height)
{
	return {this, width, height};
}

gs_texture_t* gs::rendertarget::get_object()
{
	obs_enter_graphics();
	gs_texture_t* tex = gs_texrender_get_texture(render_target);
	obs_leave_graphics();
	return tex;
}

std::shared_ptr<gs::texture> gs::rendertarget::get_texture()
{
	return std::make_shared<gs::texture>(get_object(), false);
}

void gs::rendertarget::get_texture(gs::texture& tex)
{
	tex = gs::texture(get_object(), false);
}

void gs::rendertarget::get_texture(std::shared_ptr<gs::texture>& tex)
{
	tex = std::make_shared<gs::texture>(get_object(), false);
}

void gs::rendertarget::get_texture(std::unique_ptr<gs::texture>& tex)
{
	tex = std::make_unique<gs::texture>(get_object(), false);
}

gs_color_format gs::rendertarget::get_color_format()
{
	return color_format;
}

gs_zstencil_format gs::rendertarget::get_zstencil_format()
{
	return zstencil_format;
}

gs::rendertarget_op::rendertarget_op(gs::rendertarget* rt, uint32_t width, uint32_t height) : parent(rt)
{
	if (parent == nullptr)
		throw std::invalid_argument("rt");
	if (parent->is_being_rendered)
		throw std::logic_error("Can't start rendering to the same render target twice.");
	obs_enter_graphics();
	gs_texrender_reset(parent->render_target);
	if (!gs_texrender_begin(parent->render_target, width, height)) {
		obs_leave_graphics();
		throw std::runtime_error("Failed to begin rendering to render target.");
	}
	obs_leave_graphics();
	parent->is_being_rendered = true;
}

gs::rendertarget_op::rendertarget_op(gs::rendertarget_op&& r)
{
	this->parent = r.parent;
	r.parent     = nullptr;
}

gs::rendertarget_op::~rendertarget_op()
{
	if (parent == nullptr)
		return;
	obs_enter_graphics();
	gs_texrender_end(parent->render_target);
	obs_leave_graphics();
	parent->is_being_rendered = false;
}
