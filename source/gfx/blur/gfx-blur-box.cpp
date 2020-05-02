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
#include <stdexcept>
#include "obs/gs/gs-helper.hpp"
#include "plugin.hpp"
#include "util-math.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs.h>
#include <obs-module.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define MAX_BLUR_SIZE 128 // Also change this in box.effect if modified.

gfx::blur::box_data::box_data()
{
	auto gctx = gs::context();
	try {
		char* file = obs_module_file("effects/blur/box.effect");
		_effect    = gs::effect::create(file);
		bfree(file);
	} catch (...) {
		LOG_ERROR("<gfx::blur::box> Failed to load _effect.");
	}
}

gfx::blur::box_data::~box_data()
{
	auto gctx = gs::context();
	_effect.reset();
}

gs::effect gfx::blur::box_data::get_effect()
{
	return _effect;
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

std::shared_ptr<::gfx::blur::base> gfx::blur::box_factory::create(::gfx::blur::type type)
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
	std::unique_lock<std::mutex>           ulock(_data_lock);
	std::shared_ptr<::gfx::blur::box_data> data = _data.lock();
	if (!data) {
		data  = std::make_shared<::gfx::blur::box_data>();
		_data = data;
	}
	return data;
}

::gfx::blur::box_factory& gfx::blur::box_factory::get()
{
	static ::gfx::blur::box_factory instance;
	return instance;
}

gfx::blur::box::box() : _data(::gfx::blur::box_factory::get().data()), _size(1.), _step_scale({1., 1.})
{
	auto gctx      = gs::context();
	_rendertarget  = std::make_shared<::gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	_rendertarget2 = std::make_shared<::gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
}

gfx::blur::box::~box() {}

void gfx::blur::box::set_input(std::shared_ptr<::gs::texture> texture)
{
	_input_texture = texture;
}

::gfx::blur::type gfx::blur::box::get_type()
{
	return ::gfx::blur::type::Area;
}

double_t gfx::blur::box::get_size()
{
	return _size;
}

void gfx::blur::box::set_size(double_t width)
{
	_size = width;
	if (_size < 1.0) {
		_size = 1.0;
	}
	if (_size > MAX_BLUR_SIZE) {
		_size = MAX_BLUR_SIZE;
	}
}

void gfx::blur::box::set_step_scale(double_t x, double_t y)
{
	_step_scale = {x, y};
}

void gfx::blur::box::get_step_scale(double_t& x, double_t& y)
{
	x = _step_scale.first;
	y = _step_scale.second;
}

double_t gfx::blur::box::get_step_scale_x()
{
	return _step_scale.first;
}

double_t gfx::blur::box::get_step_scale_y()
{
	return _step_scale.second;
}

std::shared_ptr<::gs::texture> gfx::blur::box::render()
{
	auto gctx = gs::context();

#ifdef ENABLE_PROFILING
	auto gdmp = gs::debug_marker(gs::debug_color_azure_radiance, "Box Blur");
#endif

	float_t width  = float_t(_input_texture->get_width());
	float_t height = float_t(_input_texture->get_height());

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
	gs::effect effect = _data->get_effect();
	if (effect) {
		// Pass 1
		effect.get_parameter("pImage").set_texture(_input_texture);
		effect.get_parameter("pImageTexel").set_float2(float_t(1.f / width), 0.f);
		effect.get_parameter("pStepScale").set_float2(float_t(_step_scale.first), float_t(_step_scale.second));
		effect.get_parameter("pSize").set_float(float_t(_size));
		effect.get_parameter("pSizeInverseMul").set_float(float_t(1.0f / (float_t(_size) * 2.0f + 1.0f)));

		{
#ifdef ENABLE_PROFILING
			auto gdm = gs::debug_marker(gs::debug_color_azure_radiance, "Horizontal");
#endif

			auto op = _rendertarget2->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect.get_object(), "Draw")) {
				streamfx::gs_draw_fullscreen_tri();
			}
		}

		// Pass 2
		effect.get_parameter("pImage").set_texture(_rendertarget2->get_texture());
		effect.get_parameter("pImageTexel").set_float2(0.f, float_t(1.f / height));

		{
#ifdef ENABLE_PROFILING
			auto gdm = gs::debug_marker(gs::debug_color_azure_radiance, "Vertical");
#endif

			auto op = _rendertarget->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect.get_object(), "Draw")) {
				streamfx::gs_draw_fullscreen_tri();
			}
		}
	}

	gs_blend_state_pop();

	return _rendertarget->get_texture();
}

std::shared_ptr<::gs::texture> gfx::blur::box::get()
{
	return _rendertarget->get_texture();
}

gfx::blur::box_directional::box_directional() : _angle(0) {}

::gfx::blur::type gfx::blur::box_directional::get_type()
{
	return ::gfx::blur::type::Directional;
}

double_t gfx::blur::box_directional::get_angle()
{
	return D_RAD_TO_DEG(_angle);
}

void gfx::blur::box_directional::set_angle(double_t angle)
{
	_angle = D_DEG_TO_RAD(angle);
}

std::shared_ptr<::gs::texture> gfx::blur::box_directional::render()
{
	auto gctx = gs::context();

#ifdef ENABLE_PROFILING
	auto gdmp = gs::debug_marker(gs::debug_color_azure_radiance, "Box Directional Blur");
#endif

	float_t width  = float_t(_input_texture->get_width());
	float_t height = float_t(_input_texture->get_height());

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
	gs::effect effect = _data->get_effect();
	if (effect) {
		effect.get_parameter("pImage").set_texture(_input_texture);
		effect.get_parameter("pImageTexel")
			.set_float2(float_t(1. / width * cos(_angle)), float_t(1.f / height * sin(_angle)));
		effect.get_parameter("pStepScale").set_float2(float_t(_step_scale.first), float_t(_step_scale.second));
		effect.get_parameter("pSize").set_float(float_t(_size));
		effect.get_parameter("pSizeInverseMul").set_float(float_t(1.0f / (float_t(_size) * 2.0f + 1.0f)));

		{
			auto op = _rendertarget->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect.get_object(), "Draw")) {
				streamfx::gs_draw_fullscreen_tri();
			}
		}
	}

	gs_blend_state_pop();

	return _rendertarget->get_texture();
}

::gfx::blur::type gfx::blur::box_rotational::get_type()
{
	return ::gfx::blur::type::Rotational;
}

void gfx::blur::box_rotational::set_center(double_t x, double_t y)
{
	_center.first  = x;
	_center.second = y;
}

void gfx::blur::box_rotational::get_center(double_t& x, double_t& y)
{
	x = _center.first;
	y = _center.second;
}

double_t gfx::blur::box_rotational::get_angle()
{
	return D_RAD_TO_DEG(_angle);
}

void gfx::blur::box_rotational::set_angle(double_t angle)
{
	_angle = D_DEG_TO_RAD(angle);
}

std::shared_ptr<::gs::texture> gfx::blur::box_rotational::render()
{
	auto gctx = gs::context();

#ifdef ENABLE_PROFILING
	auto gdmp = gs::debug_marker(gs::debug_color_azure_radiance, "Box Rotational Blur");
#endif

	float_t width  = float_t(_input_texture->get_width());
	float_t height = float_t(_input_texture->get_height());

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
	gs::effect effect = _data->get_effect();
	if (effect) {
		effect.get_parameter("pImage").set_texture(_input_texture);
		effect.get_parameter("pImageTexel").set_float2(float_t(1.f / width), float_t(1.f / height));
		effect.get_parameter("pStepScale").set_float2(float_t(_step_scale.first), float_t(_step_scale.second));
		effect.get_parameter("pSize").set_float(float_t(_size));
		effect.get_parameter("pSizeInverseMul").set_float(float_t(1.0f / (float_t(_size) * 2.0f + 1.0f)));
		effect.get_parameter("pAngle").set_float(float_t(_angle / _size));
		effect.get_parameter("pCenter").set_float2(float_t(_center.first), float_t(_center.second));

		{
			auto op = _rendertarget->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect.get_object(), "Rotate")) {
				streamfx::gs_draw_fullscreen_tri();
			}
		}
	}

	gs_blend_state_pop();

	return _rendertarget->get_texture();
}

::gfx::blur::type gfx::blur::box_zoom::get_type()
{
	return ::gfx::blur::type::Zoom;
}

void gfx::blur::box_zoom::set_center(double_t x, double_t y)
{
	_center.first  = x;
	_center.second = y;
}

void gfx::blur::box_zoom::get_center(double_t& x, double_t& y)
{
	x = _center.first;
	y = _center.second;
}

std::shared_ptr<::gs::texture> gfx::blur::box_zoom::render()
{
	auto gctx = gs::context();

#ifdef ENABLE_PROFILING
	auto gdmp = gs::debug_marker(gs::debug_color_azure_radiance, "Box Zoom Blur");
#endif

	float_t width  = float_t(_input_texture->get_width());
	float_t height = float_t(_input_texture->get_height());

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
	gs::effect effect = _data->get_effect();
	if (effect) {
		effect.get_parameter("pImage").set_texture(_input_texture);
		effect.get_parameter("pImageTexel").set_float2(float_t(1.f / width), float_t(1.f / height));
		effect.get_parameter("pStepScale").set_float2(float_t(_step_scale.first), float_t(_step_scale.second));
		effect.get_parameter("pSize").set_float(float_t(_size));
		effect.get_parameter("pSizeInverseMul").set_float(float_t(1.0f / (float_t(_size) * 2.0f + 1.0f)));
		effect.get_parameter("pCenter").set_float2(float_t(_center.first), float_t(_center.second));

		{
			auto op = _rendertarget->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect.get_object(), "Zoom")) {
				streamfx::gs_draw_fullscreen_tri();
			}
		}
	}

	gs_blend_state_pop();

	return _rendertarget->get_texture();
}
