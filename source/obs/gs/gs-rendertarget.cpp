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
#include "obs/gs/gs-helper.hpp"

#include "warning-disable.hpp"
#include <stdexcept>
#include "warning-enable.hpp"

streamfx::obs::gs::rendertarget::~rendertarget()
{
	auto gctx = streamfx::obs::gs::context();
	gs_texrender_destroy(_render_target);
}

streamfx::obs::gs::rendertarget::rendertarget(gs_color_format colorFormat, gs_zstencil_format zsFormat)
	: _color_format(colorFormat), _zstencil_format(zsFormat)
{
	_is_being_rendered = false;
	auto gctx          = streamfx::obs::gs::context();
	_render_target     = gs_texrender_create(colorFormat, zsFormat);
	if (!_render_target) {
		throw std::runtime_error("Failed to create render target.");
	}
}

streamfx::obs::gs::rendertarget_op streamfx::obs::gs::rendertarget::render(uint32_t width, uint32_t height)
{
	return {this, width, height};
}

gs_texture_t* streamfx::obs::gs::rendertarget::get_object()
{
	auto          gctx = streamfx::obs::gs::context();
	gs_texture_t* tex  = gs_texrender_get_texture(_render_target);
	return tex;
}

std::shared_ptr<streamfx::obs::gs::texture> streamfx::obs::gs::rendertarget::get_texture()
{
	return std::make_shared<streamfx::obs::gs::texture>(get_object(), false);
}

void streamfx::obs::gs::rendertarget::get_texture(streamfx::obs::gs::texture& tex)
{
	tex = streamfx::obs::gs::texture(get_object(), false);
}

void streamfx::obs::gs::rendertarget::get_texture(std::shared_ptr<streamfx::obs::gs::texture>& tex)
{
	tex = std::make_shared<streamfx::obs::gs::texture>(get_object(), false);
}

void streamfx::obs::gs::rendertarget::get_texture(std::unique_ptr<streamfx::obs::gs::texture>& tex)
{
	tex = std::make_unique<streamfx::obs::gs::texture>(get_object(), false);
}

gs_color_format streamfx::obs::gs::rendertarget::get_color_format()
{
	return _color_format;
}

gs_zstencil_format streamfx::obs::gs::rendertarget::get_zstencil_format()
{
	return _zstencil_format;
}

streamfx::obs::gs::rendertarget_op::rendertarget_op(streamfx::obs::gs::rendertarget* rt, uint32_t width,
													uint32_t height)
	: parent(rt)
{
	if (parent == nullptr)
		throw std::invalid_argument("rt");
	if (parent->_is_being_rendered)
		throw std::logic_error("Can't start rendering to the same render target twice.");

	auto gctx = streamfx::obs::gs::context();
	gs_texrender_reset(parent->_render_target);
	if (!gs_texrender_begin(parent->_render_target, width, height)) {
		throw std::runtime_error("Failed to begin rendering to render target.");
	}
	parent->_is_being_rendered = true;
}

streamfx::obs::gs::rendertarget_op::rendertarget_op(streamfx::obs::gs::rendertarget_op&& r) noexcept
{
	this->parent = r.parent;
	r.parent     = nullptr;
}

streamfx::obs::gs::rendertarget_op::~rendertarget_op()
{
	if (parent == nullptr)
		return;

	auto gctx = streamfx::obs::gs::context();
	gs_texrender_end(parent->_render_target);
	parent->_is_being_rendered = false;
}
