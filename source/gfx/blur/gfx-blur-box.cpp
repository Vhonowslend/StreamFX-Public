// Modern effects for a modern Streamer
// Copyright (C) 2019 Michael Fabian Dirks
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

#include "gfx-blur-box.hpp"
#include <cmath>
#include <memory>
#include "plugin.hpp"
#include "util-math.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs-module.h>
#include <obs.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define MAX_BLUR_SIZE 128 // Also change this in box.effect if modified.

gfx::blur::box_data::box_data()
{
	try {
		char* file = obs_module_file("effects/blur/box.effect");
		m_effect   = std::make_shared<::gs::effect>(file);
		bfree(file);
	} catch (...) {
		P_LOG_ERROR("<gfx::blur::box> Failed to load effect.");
	}
}

gfx::blur::box_data::~box_data()
{
	m_effect.reset();
}

std::shared_ptr<::gs::effect> gfx::blur::box_data::get_effect()
{
	return m_effect;
}

gfx::blur::box_factory::box_factory() {}

gfx::blur::box_factory::~box_factory() {}

bool gfx::blur::box_factory::is_type_supported(::gfx::blur::type type)
{
	switch (type) {
	case ::gfx::blur::type::Area:
		return true;
	case ::gfx::blur::type::Directional:
		return true;
	case ::gfx::blur::type::Rotational:
		return true;
	case ::gfx::blur::type::Zoom:
		return true;
	default:
		return false;
	}
}

std::shared_ptr<::gfx::blur::ibase> gfx::blur::box_factory::create(::gfx::blur::type type)
{
	switch (type) {
	case ::gfx::blur::type::Area:
		return std::make_shared<::gfx::blur::box>();
	case ::gfx::blur::type::Directional:
		return std::static_pointer_cast<::gfx::blur::box>(std::make_shared<::gfx::blur::box_directional>());
	case ::gfx::blur::type::Rotational:
		return std::make_shared<::gfx::blur::box_rotational>();
	case ::gfx::blur::type::Zoom:
		return std::make_shared<::gfx::blur::box_zoom>();
	default:
		throw std::runtime_error("Invalid type.");
	}
}

double_t gfx::blur::box_factory::get_min_size(::gfx::blur::type)
{
	return double_t(1.0);
}

double_t gfx::blur::box_factory::get_step_size(::gfx::blur::type)
{
	return double_t(1.0);
}

double_t gfx::blur::box_factory::get_max_size(::gfx::blur::type)
{
	return double_t(MAX_BLUR_SIZE);
}

double_t gfx::blur::box_factory::get_min_angle(::gfx::blur::type v)
{
	switch (v) {
	case ::gfx::blur::type::Directional:
	case ::gfx::blur::type::Rotational:
		return -180.0;
	default:
		return 0;
	}
}

double_t gfx::blur::box_factory::get_step_angle(::gfx::blur::type)
{
	return double_t(0.01);
}

double_t gfx::blur::box_factory::get_max_angle(::gfx::blur::type v)
{
	switch (v) {
	case ::gfx::blur::type::Directional:
	case ::gfx::blur::type::Rotational:
		return 180.0;
	default:
		return 0;
	}
}

bool gfx::blur::box_factory::is_step_scale_supported(::gfx::blur::type v)
{
	switch (v) {
	case ::gfx::blur::type::Area:
	case ::gfx::blur::type::Zoom:
	case ::gfx::blur::type::Directional:
		return true;
	default:
		return false;
	}
}

double_t gfx::blur::box_factory::get_min_step_scale_x(::gfx::blur::type)
{
	return double_t(0.01);
}

double_t gfx::blur::box_factory::get_step_step_scale_x(::gfx::blur::type)
{
	return double_t(0.01);
}

double_t gfx::blur::box_factory::get_max_step_scale_x(::gfx::blur::type)
{
	return double_t(1000.0);
}

double_t gfx::blur::box_factory::get_min_step_scale_y(::gfx::blur::type)
{
	return double_t(0.01);
}

double_t gfx::blur::box_factory::get_step_step_scale_y(::gfx::blur::type)
{
	return double_t(0.01);
}

double_t gfx::blur::box_factory::get_max_step_scale_y(::gfx::blur::type)
{
	return double_t(1000.0);
}

std::shared_ptr<::gfx::blur::box_data> gfx::blur::box_factory::data()
{
	std::unique_lock<std::mutex>           ulock(m_data_lock);
	std::shared_ptr<::gfx::blur::box_data> data = m_data.lock();
	if (!data) {
		data   = std::make_shared<::gfx::blur::box_data>();
		m_data = data;
	}
	return data;
}

::gfx::blur::box_factory& gfx::blur::box_factory::get()
{
	static ::gfx::blur::box_factory instance;
	return instance;
}

gfx::blur::box::box() : m_size(1.), m_step_scale({1., 1.}), m_data(::gfx::blur::box_factory::get().data())
{
	m_rendertarget  = std::make_shared<::gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	m_rendertarget2 = std::make_shared<::gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
}

gfx::blur::box::~box() {}

void gfx::blur::box::set_input(std::shared_ptr<::gs::texture> texture)
{
	m_input_texture = texture;
}

::gfx::blur::type gfx::blur::box::get_type()
{
	return ::gfx::blur::type::Area;
}

double_t gfx::blur::box::get_size()
{
	return m_size;
}

void gfx::blur::box::set_size(double_t width)
{
	m_size = width;
	if (m_size < 1.0) {
		m_size = 1.0;
	}
	if (m_size > MAX_BLUR_SIZE) {
		m_size = MAX_BLUR_SIZE;
	}
}

void gfx::blur::box::set_step_scale(double_t x, double_t y)
{
	m_step_scale = {x, y};
}

void gfx::blur::box::get_step_scale(double_t& x, double_t& y)
{
	x = m_step_scale.first;
	y = m_step_scale.second;
}

double_t gfx::blur::box::get_step_scale_x()
{
	return m_step_scale.first;
}

double_t gfx::blur::box::get_step_scale_y()
{
	return m_step_scale.second;
}

std::shared_ptr<::gs::texture> gfx::blur::box::render()
{
	float_t width, height;
	width  = float_t(m_input_texture->get_width());
	height = float_t(m_input_texture->get_height());

	obs_enter_graphics();
	gs_set_cull_mode(GS_NEITHER);
	gs_enable_color(true, true, true, true);
	gs_enable_depth_test(false);
	gs_depth_function(GS_ALWAYS);
	gs_blend_state_push();
	gs_reset_blend_state();
	gs_enable_blending(false);
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
	gs_enable_stencil_test(false);
	gs_enable_stencil_write(false);
	gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
	gs_stencil_op(GS_STENCIL_BOTH, GS_ZERO, GS_ZERO, GS_ZERO);

	// Two Pass Blur
	std::shared_ptr<::gs::effect> effect = m_data->get_effect();
	if (effect) {
		// Pass 1
		effect->get_parameter("pImage").set_texture(m_input_texture);
		effect->get_parameter("pImageTexel").set_float2(float_t(1. / width), 0.);
		effect->get_parameter("pStepScale").set_float2(float_t(m_step_scale.first), float_t(m_step_scale.second));
		effect->get_parameter("pSize").set_float(float_t(m_size));
		effect->get_parameter("pSizeInverseMul").set_float(float_t(1.0 / (float_t(m_size) * 2.0 + 1.0)));

		{
			auto op = m_rendertarget2->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect->get_object(), "Draw")) {
				gs_draw_sprite(0, 0, 1, 1);
			}
		}

		// Pass 2
		effect->get_parameter("pImage").set_texture(m_rendertarget2->get_texture());
		effect->get_parameter("pImageTexel").set_float2(0., float_t(1. / height));

		{
			auto op = m_rendertarget->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect->get_object(), "Draw")) {
				gs_draw_sprite(0, 0, 1, 1);
			}
		}
	}

	gs_blend_state_pop();
	obs_leave_graphics();

	return m_rendertarget->get_texture();
}

std::shared_ptr<::gs::texture> gfx::blur::box::get()
{
	return m_rendertarget->get_texture();
}

gfx::blur::box_directional::box_directional() : m_angle(0) {}

::gfx::blur::type gfx::blur::box_directional::get_type()
{
	return ::gfx::blur::type::Directional;
}

double_t gfx::blur::box_directional::get_angle()
{
	return RAD_TO_DEG(m_angle);
}

void gfx::blur::box_directional::set_angle(double_t angle)
{
	m_angle = DEG_TO_RAD(angle);
}

std::shared_ptr<::gs::texture> gfx::blur::box_directional::render()
{
	float_t width, height;
	width  = float_t(m_input_texture->get_width());
	height = float_t(m_input_texture->get_height());

	obs_enter_graphics();
	gs_blend_state_push();
	gs_reset_blend_state();
	gs_enable_color(true, true, true, true);
	gs_enable_blending(false);
	gs_enable_depth_test(false);
	gs_enable_stencil_test(false);
	gs_enable_stencil_write(false);
	gs_set_cull_mode(GS_NEITHER);
	gs_depth_function(GS_ALWAYS);
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
	gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
	gs_stencil_op(GS_STENCIL_BOTH, GS_ZERO, GS_ZERO, GS_ZERO);

	// One Pass Blur
	std::shared_ptr<::gs::effect> effect = m_data->get_effect();
	if (effect) {
		effect->get_parameter("pImage").set_texture(m_input_texture);
		effect->get_parameter("pImageTexel")
			.set_float2(float_t(1. / width * cos(m_angle)), float_t(1. / height * sin(m_angle)));
		effect->get_parameter("pStepScale").set_float2(float_t(m_step_scale.first), float_t(m_step_scale.second));
		effect->get_parameter("pSize").set_float(float_t(m_size));
		effect->get_parameter("pSizeInverseMul").set_float(float_t(1.0 / (float_t(m_size) * 2.0 + 1.0)));

		{
			auto op = m_rendertarget->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect->get_object(), "Draw")) {
				gs_draw_sprite(0, 0, 1, 1);
			}
		}
	}

	gs_blend_state_pop();
	obs_leave_graphics();

	return m_rendertarget->get_texture();
}

::gfx::blur::type gfx::blur::box_rotational::get_type()
{
	return ::gfx::blur::type::Rotational;
}

void gfx::blur::box_rotational::set_center(double_t x, double_t y)
{
	m_center.first  = x;
	m_center.second = y;
}

void gfx::blur::box_rotational::get_center(double_t& x, double_t& y)
{
	x = m_center.first;
	y = m_center.second;
}

double_t gfx::blur::box_rotational::get_angle()
{
	return RAD_TO_DEG(m_angle);
}

void gfx::blur::box_rotational::set_angle(double_t angle)
{
	m_angle = DEG_TO_RAD(angle);
}

std::shared_ptr<::gs::texture> gfx::blur::box_rotational::render()
{
	float_t width, height;
	width  = float_t(m_input_texture->get_width());
	height = float_t(m_input_texture->get_height());

	obs_enter_graphics();
	gs_blend_state_push();
	gs_reset_blend_state();
	gs_enable_color(true, true, true, true);
	gs_enable_blending(false);
	gs_enable_depth_test(false);
	gs_enable_stencil_test(false);
	gs_enable_stencil_write(false);
	gs_set_cull_mode(GS_NEITHER);
	gs_depth_function(GS_ALWAYS);
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
	gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
	gs_stencil_op(GS_STENCIL_BOTH, GS_ZERO, GS_ZERO, GS_ZERO);

	// One Pass Blur
	std::shared_ptr<::gs::effect> effect = m_data->get_effect();
	if (effect) {
		effect->get_parameter("pImage").set_texture(m_input_texture);
		effect->get_parameter("pImageTexel").set_float2(float_t(1. / width), float_t(1. / height));
		effect->get_parameter("pStepScale").set_float2(float_t(m_step_scale.first), float_t(m_step_scale.second));
		effect->get_parameter("pSize").set_float(float_t(m_size));
		effect->get_parameter("pSizeInverseMul").set_float(float_t(1.0 / (float_t(m_size) * 2.0 + 1.0)));
		effect->get_parameter("pAngle").set_float(float_t(m_angle / m_size));
		effect->get_parameter("pCenter").set_float2(float_t(m_center.first), float_t(m_center.second));

		{
			auto op = m_rendertarget->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect->get_object(), "Rotate")) {
				gs_draw_sprite(0, 0, 1, 1);
			}
		}
	}

	gs_blend_state_pop();
	obs_leave_graphics();

	return m_rendertarget->get_texture();
}

::gfx::blur::type gfx::blur::box_zoom::get_type()
{
	return ::gfx::blur::type::Zoom;
}

void gfx::blur::box_zoom::set_center(double_t x, double_t y)
{
	m_center.first  = x;
	m_center.second = y;
}

void gfx::blur::box_zoom::get_center(double_t& x, double_t& y)
{
	x = m_center.first;
	y = m_center.second;
}

std::shared_ptr<::gs::texture> gfx::blur::box_zoom::render()
{
	float_t width, height;
	width  = float_t(m_input_texture->get_width());
	height = float_t(m_input_texture->get_height());

	obs_enter_graphics();
	gs_blend_state_push();
	gs_reset_blend_state();
	gs_enable_color(true, true, true, true);
	gs_enable_blending(false);
	gs_enable_depth_test(false);
	gs_enable_stencil_test(false);
	gs_enable_stencil_write(false);
	gs_set_cull_mode(GS_NEITHER);
	gs_depth_function(GS_ALWAYS);
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
	gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
	gs_stencil_op(GS_STENCIL_BOTH, GS_ZERO, GS_ZERO, GS_ZERO);

	// One Pass Blur
	std::shared_ptr<::gs::effect> effect = m_data->get_effect();
	if (effect) {
		effect->get_parameter("pImage").set_texture(m_input_texture);
		effect->get_parameter("pImageTexel").set_float2(float_t(1. / width), float_t(1. / height));
		effect->get_parameter("pStepScale").set_float2(float_t(m_step_scale.first), float_t(m_step_scale.second));
		effect->get_parameter("pSize").set_float(float_t(m_size));
		effect->get_parameter("pSizeInverseMul").set_float(float_t(1.0 / (float_t(m_size) * 2.0 + 1.0)));
		effect->get_parameter("pCenter").set_float2(float_t(m_center.first), float_t(m_center.second));

		{
			auto op = m_rendertarget->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect->get_object(), "Zoom")) {
				gs_draw_sprite(0, 0, 1, 1);
			}
		}
	}

	gs_blend_state_pop();
	obs_leave_graphics();

	return m_rendertarget->get_texture();
}
