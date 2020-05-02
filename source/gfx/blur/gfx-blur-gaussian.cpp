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

#include "gfx-blur-gaussian.hpp"
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

// FIXME: This breaks when MAX_KERNEL_SIZE is changed, due to the way the Gaussian
//  function first goes up at the point, and then once we pass the critical point
//  will go down again and it is not handled well. This is a pretty basic
//  approximation anyway at the moment.
#define MAX_KERNEL_SIZE 128
#define MAX_BLUR_SIZE (MAX_KERNEL_SIZE - 1)
#define SEARCH_DENSITY double_t(1. / 500.)
#define SEARCH_THRESHOLD double_t(1. / (MAX_KERNEL_SIZE * 5))
#define SEARCH_EXTENSION 1
#define SEARCH_RANGE MAX_KERNEL_SIZE * 2

gfx::blur::gaussian_data::gaussian_data()
{
	auto gctx = gs::context();
	{
		char* file = obs_module_file("effects/blur/gaussian.effect");
		_effect    = gs::effect::create(file);
		bfree(file);
	}

	// Precalculate Kernels
	for (std::size_t kernel_size = 1; kernel_size <= MAX_BLUR_SIZE; kernel_size++) {
		std::vector<double_t> kernel_math(MAX_KERNEL_SIZE);
		std::vector<float_t>  kernel_data(MAX_KERNEL_SIZE);
		double_t              actual_width = 1.;

		// Find actual kernel width.
		for (double_t h = SEARCH_DENSITY; h < SEARCH_RANGE; h += SEARCH_DENSITY) {
			if (util::math::gaussian<double_t>(double_t(kernel_size + SEARCH_EXTENSION), h) > SEARCH_THRESHOLD) {
				actual_width = h;
				break;
			}
		}

		// Calculate and normalize
		double_t sum = 0;
		for (std::size_t p = 0; p <= kernel_size; p++) {
			kernel_math[p] = util::math::gaussian<double_t>(double_t(p), actual_width);
			sum += kernel_math[p] * (p > 0 ? 2 : 1);
		}

		// Normalize to fill the entire 0..1 range over the width.
		double_t inverse_sum = 1.0 / sum;
		for (std::size_t p = 0; p <= kernel_size; p++) {
			kernel_data.at(p) = float_t(kernel_math[p] * inverse_sum);
		}

		_kernels.push_back(std::move(kernel_data));
	}
}

gfx::blur::gaussian_data::~gaussian_data()
{
	auto gctx = gs::context();
	_effect.reset();
}

gs::effect gfx::blur::gaussian_data::get_effect()
{
	return _effect;
}

std::vector<float_t> const& gfx::blur::gaussian_data::get_kernel(std::size_t width)
{
	if (width < 1)
		width = 1;
	if (width > MAX_BLUR_SIZE)
		width = MAX_BLUR_SIZE;
	width -= 1;
	return _kernels[width];
}

gfx::blur::gaussian_factory::gaussian_factory() {}

gfx::blur::gaussian_factory::~gaussian_factory() {}

bool gfx::blur::gaussian_factory::is_type_supported(::gfx::blur::type v)
{
	switch (v) {
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

std::shared_ptr<::gfx::blur::base> gfx::blur::gaussian_factory::create(::gfx::blur::type v)
{
	switch (v) {
	case ::gfx::blur::type::Area:
		return std::make_shared<::gfx::blur::gaussian>();
	case ::gfx::blur::type::Directional:
		return std::static_pointer_cast<::gfx::blur::gaussian>(std::make_shared<::gfx::blur::gaussian_directional>());
	case ::gfx::blur::type::Rotational:
		return std::make_shared<::gfx::blur::gaussian_rotational>();
	case ::gfx::blur::type::Zoom:
		return std::make_shared<::gfx::blur::gaussian_zoom>();
	default:
		throw std::runtime_error("Invalid type.");
	}
}

double_t gfx::blur::gaussian_factory::get_min_size(::gfx::blur::type)
{
	return double_t(1.0);
}

double_t gfx::blur::gaussian_factory::get_step_size(::gfx::blur::type)
{
	return double_t(1.0);
}

double_t gfx::blur::gaussian_factory::get_max_size(::gfx::blur::type)
{
	return double_t(MAX_BLUR_SIZE);
}

double_t gfx::blur::gaussian_factory::get_min_angle(::gfx::blur::type v)
{
	switch (v) {
	case ::gfx::blur::type::Directional:
	case ::gfx::blur::type::Rotational:
		return -180.0;
	default:
		return 0;
	}
}

double_t gfx::blur::gaussian_factory::get_step_angle(::gfx::blur::type)
{
	return double_t(0.01);
}

double_t gfx::blur::gaussian_factory::get_max_angle(::gfx::blur::type v)
{
	switch (v) {
	case ::gfx::blur::type::Directional:
	case ::gfx::blur::type::Rotational:
		return 180.0;
	default:
		return 0;
	}
}

bool gfx::blur::gaussian_factory::is_step_scale_supported(::gfx::blur::type v)
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

double_t gfx::blur::gaussian_factory::get_min_step_scale_x(::gfx::blur::type)
{
	return double_t(0.01);
}

double_t gfx::blur::gaussian_factory::get_step_step_scale_x(::gfx::blur::type)
{
	return double_t(0.01);
}

double_t gfx::blur::gaussian_factory::get_max_step_scale_x(::gfx::blur::type)
{
	return double_t(1000.0);
}

double_t gfx::blur::gaussian_factory::get_min_step_scale_y(::gfx::blur::type)
{
	return double_t(0.01);
}

double_t gfx::blur::gaussian_factory::get_step_step_scale_y(::gfx::blur::type)
{
	return double_t(0.01);
}

double_t gfx::blur::gaussian_factory::get_max_step_scale_y(::gfx::blur::type)
{
	return double_t(1000.0);
}

std::shared_ptr<::gfx::blur::gaussian_data> gfx::blur::gaussian_factory::data()
{
	std::unique_lock<std::mutex>                ulock(_data_lock);
	std::shared_ptr<::gfx::blur::gaussian_data> data = _data.lock();
	if (!data) {
		data  = std::make_shared<::gfx::blur::gaussian_data>();
		_data = data;
	}
	return data;
}

::gfx::blur::gaussian_factory& gfx::blur::gaussian_factory::get()
{
	static ::gfx::blur::gaussian_factory instance;
	return instance;
}

gfx::blur::gaussian::gaussian() : _data(::gfx::blur::gaussian_factory::get().data()), _size(1.), _step_scale({1., 1.})
{
	auto gctx      = gs::context();
	_rendertarget  = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	_rendertarget2 = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
}

gfx::blur::gaussian::~gaussian() {}

void gfx::blur::gaussian::set_input(std::shared_ptr<::gs::texture> texture)
{
	_input_texture = texture;
}

::gfx::blur::type gfx::blur::gaussian::get_type()
{
	return ::gfx::blur::type::Area;
}

double_t gfx::blur::gaussian::get_size()
{
	return _size;
}

void gfx::blur::gaussian::set_size(double_t width)
{
	if (width < 1.)
		width = 1.;
	if (width > MAX_BLUR_SIZE)
		width = MAX_BLUR_SIZE;
	_size = width;
}

void gfx::blur::gaussian::set_step_scale(double_t x, double_t y)
{
	_step_scale.first  = x;
	_step_scale.second = y;
}

void gfx::blur::gaussian::get_step_scale(double_t& x, double_t& y)
{
	x = _step_scale.first;
	y = _step_scale.second;
}

double_t gfx::blur::gaussian::get_step_scale_x()
{
	return _step_scale.first;
}

double_t gfx::blur::gaussian::get_step_scale_y()
{
	return _step_scale.second;
}

std::shared_ptr<::gs::texture> gfx::blur::gaussian::render()
{
	auto gctx = gs::context();

#ifdef ENABLE_PROFILING
	auto gdmp = gs::debug_marker(gs::debug_color_azure_radiance, "Gaussian Blur");
#endif

	gs::effect effect = _data->get_effect();
	auto       kernel = _data->get_kernel(size_t(_size));

	if (!effect || ((_step_scale.first + _step_scale.second) < std::numeric_limits<double_t>::epsilon())) {
		return _input_texture;
	}

	float_t width  = float_t(_input_texture->get_width());
	float_t height = float_t(_input_texture->get_height());

	// Setup
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

	effect.get_parameter("pImage").set_texture(_input_texture);
	effect.get_parameter("pStepScale").set_float2(float_t(_step_scale.first), float_t(_step_scale.second));
	effect.get_parameter("pSize").set_float(float_t(_size));
	effect.get_parameter("pKernel").set_value(kernel.data(), MAX_KERNEL_SIZE);

	// First Pass
	if (_step_scale.first > std::numeric_limits<double_t>::epsilon()) {
		effect.get_parameter("pImageTexel").set_float2(float_t(1.f / width), 0.f);

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

		std::swap(_rendertarget, _rendertarget2);
		effect.get_parameter("pImage").set_texture(_rendertarget->get_texture());
	}

	// Second Pass
	if (_step_scale.second > std::numeric_limits<double_t>::epsilon()) {
		effect.get_parameter("pImageTexel").set_float2(0.f, float_t(1.f / height));

		{
#ifdef ENABLE_PROFILING
			auto gdm = gs::debug_marker(gs::debug_color_azure_radiance, "Vertical");
#endif

			auto op = _rendertarget2->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect.get_object(), "Draw")) {
				streamfx::gs_draw_fullscreen_tri();
			}
		}

		std::swap(_rendertarget, _rendertarget2);
	}

	gs_blend_state_pop();

	return this->get();
}

std::shared_ptr<::gs::texture> gfx::blur::gaussian::get()
{
	return _rendertarget->get_texture();
}

gfx::blur::gaussian_directional::gaussian_directional() : m_angle(0.) {}

gfx::blur::gaussian_directional::~gaussian_directional() {}

::gfx::blur::type gfx::blur::gaussian_directional::get_type()
{
	return ::gfx::blur::type::Directional;
}

double_t gfx::blur::gaussian_directional::get_angle()
{
	return D_RAD_TO_DEG(m_angle);
}

void gfx::blur::gaussian_directional::set_angle(double_t angle)
{
	m_angle = D_DEG_TO_RAD(angle);
}

std::shared_ptr<::gs::texture> gfx::blur::gaussian_directional::render()
{
	auto gctx = gs::context();

#ifdef ENABLE_PROFILING
	auto gdmp = gs::debug_marker(gs::debug_color_azure_radiance, "Gaussian Directional Blur");
#endif

	gs::effect effect = _data->get_effect();
	auto       kernel = _data->get_kernel(size_t(_size));

	if (!effect || ((_step_scale.first + _step_scale.second) < std::numeric_limits<double_t>::epsilon())) {
		return _input_texture;
	}

	float_t width  = float_t(_input_texture->get_width());
	float_t height = float_t(_input_texture->get_height());

	// Setup
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

	effect.get_parameter("pImage").set_texture(_input_texture);
	effect.get_parameter("pImageTexel")
		.set_float2(float_t(1.f / width * cos(m_angle)), float_t(1.f / height * sin(m_angle)));
	effect.get_parameter("pStepScale").set_float2(float_t(_step_scale.first), float_t(_step_scale.second));
	effect.get_parameter("pSize").set_float(float_t(_size));
	effect.get_parameter("pKernel").set_value(kernel.data(), MAX_KERNEL_SIZE);

	// First Pass
	{
		auto op = _rendertarget->render(uint32_t(width), uint32_t(height));
		gs_ortho(0, 1., 0, 1., 0, 1.);
		while (gs_effect_loop(effect.get_object(), "Draw")) {
			streamfx::gs_draw_fullscreen_tri();
		}
	}

	gs_blend_state_pop();

	return this->get();
}

::gfx::blur::type gfx::blur::gaussian_rotational::get_type()
{
	return ::gfx::blur::type::Rotational;
}

std::shared_ptr<::gs::texture> gfx::blur::gaussian_rotational::render()
{
	auto gctx = gs::context();

#ifdef ENABLE_PROFILING
	auto gdmp = gs::debug_marker(gs::debug_color_azure_radiance, "Gaussian Rotational Blur");
#endif

	gs::effect effect = _data->get_effect();
	auto       kernel = _data->get_kernel(size_t(_size));

	if (!effect || ((_step_scale.first + _step_scale.second) < std::numeric_limits<double_t>::epsilon())) {
		return _input_texture;
	}

	float_t width  = float_t(_input_texture->get_width());
	float_t height = float_t(_input_texture->get_height());

	// Setup
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

	effect.get_parameter("pImage").set_texture(_input_texture);
	effect.get_parameter("pImageTexel").set_float2(float_t(1.f / width), float_t(1.f / height));
	effect.get_parameter("pStepScale").set_float2(float_t(_step_scale.first), float_t(_step_scale.second));
	effect.get_parameter("pSize").set_float(float_t(_size));
	effect.get_parameter("pAngle").set_float(float_t(m_angle / _size));
	effect.get_parameter("pCenter").set_float2(float_t(m_center.first), float_t(m_center.second));
	effect.get_parameter("pKernel").set_value(kernel.data(), MAX_KERNEL_SIZE);

	// First Pass
	{
		auto op = _rendertarget->render(uint32_t(width), uint32_t(height));
		gs_ortho(0, 1., 0, 1., 0, 1.);
		while (gs_effect_loop(effect.get_object(), "Rotate")) {
			streamfx::gs_draw_fullscreen_tri();
		}
	}

	gs_blend_state_pop();

	return this->get();
}

void gfx::blur::gaussian_rotational::set_center(double_t x, double_t y)
{
	m_center.first  = x;
	m_center.second = y;
}

void gfx::blur::gaussian_rotational::get_center(double_t& x, double_t& y)
{
	x = m_center.first;
	y = m_center.second;
}

double_t gfx::blur::gaussian_rotational::get_angle()
{
	return double_t(D_RAD_TO_DEG(m_angle));
}

void gfx::blur::gaussian_rotational::set_angle(double_t angle)
{
	m_angle = D_DEG_TO_RAD(angle);
}

::gfx::blur::type gfx::blur::gaussian_zoom::get_type()
{
	return ::gfx::blur::type::Zoom;
}

std::shared_ptr<::gs::texture> gfx::blur::gaussian_zoom::render()
{
	auto gctx = gs::context();

#ifdef ENABLE_PROFILING
	auto gdmp = gs::debug_marker(gs::debug_color_azure_radiance, "Gaussian Zoom Blur");
#endif

	gs::effect effect = _data->get_effect();
	auto       kernel = _data->get_kernel(size_t(_size));

	if (!effect || ((_step_scale.first + _step_scale.second) < std::numeric_limits<double_t>::epsilon())) {
		return _input_texture;
	}

	float_t width  = float_t(_input_texture->get_width());
	float_t height = float_t(_input_texture->get_height());

	// Setup
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

	effect.get_parameter("pImage").set_texture(_input_texture);
	effect.get_parameter("pImageTexel").set_float2(float_t(1.f / width), float_t(1.f / height));
	effect.get_parameter("pStepScale").set_float2(float_t(_step_scale.first), float_t(_step_scale.second));
	effect.get_parameter("pSize").set_float(float_t(_size));
	effect.get_parameter("pCenter").set_float2(float_t(m_center.first), float_t(m_center.second));
	effect.get_parameter("pKernel").set_value(kernel.data(), MAX_KERNEL_SIZE);

	// First Pass
	{
		auto op = _rendertarget->render(uint32_t(width), uint32_t(height));
		gs_ortho(0, 1., 0, 1., 0, 1.);
		while (gs_effect_loop(effect.get_object(), "Zoom")) {
			streamfx::gs_draw_fullscreen_tri();
		}
	}

	gs_blend_state_pop();

	return this->get();
}

void gfx::blur::gaussian_zoom::set_center(double_t x, double_t y)
{
	m_center.first  = x;
	m_center.second = y;
}

void gfx::blur::gaussian_zoom::get_center(double_t& x, double_t& y)
{
	x = m_center.first;
	y = m_center.second;
}
